/**********************************************************************
 * CLIENTE liga ao servidor (definido em argv[1]) no porto especificado
 * (em argv[2]), escrevendo a palavra predefinida (em argv[3]).
 * USO: >cliente <enderecoServidor>  <porto>  <Palavra>
 **********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE    1024

void erro(char *msg);

int main(int argc, char *argv[])
{
    char endServer[100];
    int fd;
    struct sockaddr_in addr;
    struct hostent *hostPtr;

    /*if (argc != 4)
    {
        printf("cliente <host> <port> <string>\n");
        exit(-1);
    }*/

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

    while(1) {
        //Mensagem Inicial

        char msg_inicial[BUF_SIZE];
        recv(fd, msg_inicial, BUF_SIZE, 0);
        printf("%s\n", msg_inicial);

        //Mensagem a pedir username
        char msg_get_user[BUF_SIZE];
        recv(fd, msg_get_user, BUF_SIZE, 0);
        printf("%s\n", msg_get_user);

        //Enviar username
        char username[BUF_SIZE];
        scanf("%s", &username);
        send(fd, username, strlen(username), 0);
        fflush(stdout);

        //Aqui qualquer coisa se username não existe e o programa acaba
        char msg_erro[BUF_SIZE];
        recv(fd, msg_erro, strlen(msg_erro), 0);

        if (strcmp(msg_erro, "  Username não existe. Insira um username válido\n") == 0) {
            printf("%s", msg_erro);
            break;

        } else {
            //Mensagem a pedir Password
            /*char msg_get_password[BUF_SIZE];
            recv(fd, msg_get_password, BUF_SIZE, 0);
            printf("%s\n", msg_get_password);*/
            printf("%s\n", msg_erro);

            //Enviar Password
            char password[BUF_SIZE];
            scanf("%s", &password);
            send(fd, username, strlen(password), 0);
            fflush(stdout);

            //ERRO NA PASSWORD
            char msg_erro[BUF_SIZE];
            recv(fd, msg_erro, strlen(msg_erro), 0);

            //3 tentativas restantes
            if (strcmp(msg_erro, "  Password Incorrreta. 3 tentativas restantes!\n") == 0) {
                printf("%s", msg_erro);

                char msg_get_password[BUF_SIZE];
                recv(fd, msg_get_password, BUF_SIZE, 0);
                printf("%s\n", msg_get_password);

                char password[BUF_SIZE];
                scanf("%s", &password);
                send(fd, username, strlen(password), 0);
                fflush(stdout);

                if (strcmp(msg_erro, "  Password Incorrreta. 2 tentativas restantes!\n") == 0) {

                    printf(msg_get_password);

                    char password[BUF_SIZE];
                    scanf("%s", &password);
                    send(fd, username, strlen(password), 0);
                    fflush(stdout);

                    if (strcmp(msg_erro, "  Password Incorrreta. 1 tentativas restantes!\n") == 0) {

                        printf(msg_get_password);

                        char password[BUF_SIZE];
                        scanf("%s", &password);
                        send(fd, username, strlen(password), 0);
                        fflush(stdout);

                    }

                }


            }

            if (strcmp(username, "admin") == 0) {
                char menu_admin[BUF_SIZE];
                recv(fd, menu_admin, strlen(menu_admin), 0);
                printf("%s", menu_admin);

            } else {
                char menu_cliente[BUF_SIZE];
                recv(fd, menu_cliente, strlen(menu_cliente), 0);
                printf("%s", menu_cliente);
            }

        }
    }


    close(fd);
    exit(0);
}

void erro(char *msg)
{
    printf("Erro: %s\n", msg);
    exit(-1);
}
