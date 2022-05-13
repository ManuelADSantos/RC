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
#include <signal.h>

#define BUF_SIZE 1024

pid_t pid = 0;

// ==================== Prototypes ====================
void sig_handler(int sig);
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

    // ========== From here ==========
    if ((pid = fork()) == 0)
    // Child
    {
        while (1)
        {
            char buffer_read[BUF_SIZE] = "";
            if (recv(fd, NULL, 0, MSG_PEEK) >= 0)
            {
                fflush(stdout);
                // ssize_t recebeu =
                recv(fd, buffer_read, sizeof(buffer_read), 0);
                // printf("Foram recebidos %ld bytes de informação\n", recebeu);
                fflush(stdout);
                printf("%s", buffer_read);
                fflush(stdout);
            }
        }
    }
    else
    // Parent
    {
        // Update signal handler
        signal(SIGINT, sig_handler);
        while (1)
        {

            char buffer_write[BUF_SIZE] = "";
            if (send(fd, NULL, 0, 0) >= 0)
            {
                fflush(stdout);
                fgets(buffer_write, sizeof(buffer_write), stdin);
                buffer_write[strcspn(buffer_write, "\n")] = 0;
                // ssize_t enviou =
                send(fd, buffer_write, strlen(buffer_write) + 2, 0);
                // printf("Foram enviados %ld bytes de informação\n", enviou);
                fflush(stdout);
                fflush(stdin);

                sleep(1);
            }
        }
    }
    // ========== To Here ==========
}

void erro(char *msg)
{
    printf("Erro: %s\n", msg);
    exit(-1);
}

// ==================== Signal Handler Function ====================
void sig_handler(int sig)
{
    // If SIGINT is cacthed
    if (sig == SIGINT)
    {
        kill(pid, SIGINT);
        wait(NULL);
        printf("\nProcess terminating by ctrl-C ...\n");
        fflush(stdout);
        exit(0);
    }
}
