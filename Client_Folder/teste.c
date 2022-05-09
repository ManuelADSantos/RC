/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * Uso: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE 1024

void erro(char *msg);

int main(int argc, char *argv[])
{
    char endServer[100];
    int fd;
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Nao consegui obter endereço");

    bzero((void *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short)atoi(argv[2]));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        erro("socket");
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        erro("Connect");

    // File descriptors sets
    /*fd_set current_socket, ready_socket;

    FD_ZERO(&current_socket);
    FD_SET(fd, &current_socket);*/

    pid_t pid = 0;
    if ((pid = fork()) == 0)
    // Child
    {
        while (1)
        {
            char buffer_read[BUF_SIZE] = "";
            if (recv(fd, NULL, 0, MSG_PEEK) >= 0)
            {
                fflush(stdout);
                recv(fd, buffer_read, sizeof(buffer_read), 0);
                fflush(stdout);
                printf("%s", buffer_read);
                // printf("<RECV>");
                fflush(stdout);
            }
        }
    }
    else
    // Parent
    {
        while (1)
        {
            char buffer_write[BUF_SIZE] = "";
            if (send(fd, NULL, 0, 0) >= 0)
            {
                fflush(stdout);
                scanf("%s", buffer_write);
                // buffer_write[strlen(buffer_write) - 2] = '\0';
                // printf("DEBUG->%s<-/%ld/", buffer_write, strlen(buffer_write));
                send(fd, buffer_write, sizeof(buffer_write), 0);
                // printf("<SEND>");
                fflush(stdout);

                sleep(1);
            }
        }
    }
    /*while (1)
    {
        // ready_socket = current_socket;
        char buffer[BUF_SIZE] = "";

        if (recv(fd, NULL, 0, MSG_PEEK) >= 0)
        {
            recv(fd, buffer, sizeof(buffer), 0);
            printf("%s", buffer);
            // printf("<RECV>");
            fflush(stdout);
        }

        select(FD_SETSIZE, NULL, &ready_socket, NULL, NULL);
        printf("<AQUI>");
        if (FD_ISSET(fd, &ready_socket))
            {
                printf("<SEND>");
                scanf("%s", buffer);
                buffer[strlen(buffer)] = '\0';
                printf("DEBUG->%s<-", buffer);
                send(fd, buffer, sizeof(buffer), 0);
                fflush(stdout);
            }
        if (send(fd, NULL, 0, 0) >= 0)
        {
            scanf("%s", buffer);
            buffer[strlen(buffer)] = '\0';
            // printf("DEBUG->%s<-", buffer);
            send(fd, buffer, sizeof(buffer), 0);
            // printf("<SEND>");
            fflush(stdout);

            sleep(1);
        }
    }*/

    close(fd);
    exit(0);
}

void erro(char *msg)
{
    printf("Erro: %s\n", msg);
    exit(-1);
}