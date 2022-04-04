#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_PORT 9876
#define BUF_SIZE 1024

void process_client(int fd);
void erro(char *msg);
// Login
void login(int client_fd, char username[]);
// Menu Client
void menu_client(int client_fd, char username[]);
// Menu Admin
void menu_admin(int admin_fd);

int main(int argc, char *argv[])
{
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;

    bzero((void *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("na funcao socket");
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        erro("na funcao bind");
    if (listen(fd, 5) < 0)
        erro("na funcao listen");
    client_addr_size = sizeof(client_addr);
    while (1)
    {
        // clean finished child processes, avoiding zombies
        // must use WNOHANG or would block whenever a child process was working
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
        // wait for new connection
        client = accept(fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
        if (client > 0)
        {
            if (fork() == 0)
            {
                close(fd);
                process_client(client);
                exit(0);
            }
            close(client);
        }
    }
    return 0;
}

// ==================== Process Client ====================
// telnet 127.0.0.1 9876
void process_client(int client_fd)
{
    // Login de Clientes
    char username[BUF_SIZE] = "";
    login(client_fd, username);

    if (strcmp(username, "admin") == 0)
    {
        // printf("Login Admin\n");
        menu_admin(client_fd);
    }
    else
    {
        // printf("Login Cliente\n");
        menu_client(client_fd, username);
    }

    close(client_fd);
}

void erro(char *msg)
{
    printf("Erro: %s\n", msg);
    exit(-1);
}

// ==================== Login ====================
void login(int client_fd, char username[])
{
    char msg_inicial[] = "\n==========================================\n                  LOGIN\n\n";
    write(client_fd, msg_inicial, strlen(msg_inicial));

    int option = -1;
    while (option == -1)
    {
        // Enviar print de pedido do username
        char msg_get_user[] = "\n Username: ";
        // ssize_t size_msg_user =
        send(client_fd, msg_get_user, strlen(msg_get_user), 0);
        fflush(stdout);
        // printf("Enviados %ld bytes\n", size_msg_user);
        // fflush(stdout);

        // Receber username inserido
        ssize_t size_user = recv(client_fd, username, BUF_SIZE, 0);
        username[size_user - 2] = '\0';
        // printf("Recebidos %ld bytes\n", size_user);

        // DEBUG
        // printf("\n%s<->%ld\n", username, strlen(username));
        // fflush(stdout);

        FILE *file_user;
        char filename_user[BUF_SIZE];
        strcpy(filename_user, username);
        strcat(filename_user, ".txt");

        // DEBUG
        // printf("%s\n", filename_user);

        if ((file_user = fopen(filename_user, "r")) == NULL)
        {
            // Caso não exista o username em causa
            char msg_get_user_error[] = "  Username não existe. Insira um username válido\n";
            send(client_fd, msg_get_user_error, strlen(msg_get_user_error), 0);
            fflush(stdout);
            option = -1;
        }
        else
        {
            // Existe username em causa
            char user_password_auth[BUF_SIZE];
            fread(user_password_auth, sizeof(user_password_auth), 1, file_user);
            fflush(stdout);

            int tentativas = 0;
            while (1)
            {
                // Pedir password
                char msg_get_password[] = "\n Password: ";
                send(client_fd, msg_get_password, strlen(msg_get_password), 0);
                fflush(stdout);

                // Receber password inserido
                char password[BUF_SIZE];
                ssize_t size_password = recv(client_fd, password, BUF_SIZE, 0);
                password[size_password - 2] = '\0';
                // printf("Recebidos %ld bytes\n", size_password);

                // printf("file: %s\ninserida: %s\n", user_password_auth, password);

                if (strcmp(password, user_password_auth) == 0)
                {
                    // printf("LOGIN AUTORIZADO\n");
                    char msg_login_sucess[] = "\n Bem vind@ ";
                    strcat(msg_login_sucess, username);
                    strcat(msg_login_sucess, "\n\n");
                    send(client_fd, msg_login_sucess, strlen(msg_login_sucess), 0);
                    fflush(stdout);
                    option = 0;
                    break;
                }
                else
                {
                    // printf("PASSWORD ERRADA\n");
                    tentativas++;
                    char msg_get_password_error[] = "  Password Incorrreta. ";
                    char ten = (3 - tentativas) + '0';
                    char tenta[2] = "";
                    tenta[0] = ten;
                    strcat(msg_get_password_error, tenta);
                    strcat(msg_get_password_error, " tentativas restantes!\n");
                    send(client_fd, msg_get_password_error, strlen(msg_get_password_error), 0);
                    fflush(stdout);
                    // printf("%dº tentativa\n", tentativas);
                    if (tentativas == 3)
                    {
                        option = -1;
                        break;
                    }
                }
            }
        }
    }
}

// ==================== Menu Client ====================
void menu_client(int client_fd, char username[])
{
    char menu_client[] = "\n==========================================\n                  MENU\n\n";
    send(client_fd, menu_client, strlen(menu_client), 0);
    fflush(stdout);

    char lixo[BUF_SIZE];
    recv(client_fd, lixo, BUF_SIZE, 0);

    char menu_client_logout[] = "                 LOGOUT\n==========================================\n\n";
    send(client_fd, menu_client_logout, strlen(menu_client_logout), 0);
    fflush(stdout);
}

// ==================== Menu Admin ====================
void menu_admin(int admin_fd)
{
    char menu_admin[] = "\n==========================================\n             MENU ADMIN\n\n";
    send(admin_fd, menu_admin, strlen(menu_admin), 0);
    fflush(stdout);

    char admin_options[] = "  (1) Consultar\n  (2) Adicionar\n  (3) Remover\n  (4) Sair\n\n";
    send(admin_fd, admin_options, strlen(admin_options), 0);
    fflush(stdout);

    char option[2];
    recv(admin_fd, option, 2, 0);
    fflush(stdout);

    // printf("Selecionou a opção %c", option[0]);
    // printf("Foi lido isto -> %s", option);

    FILE *file_words;
    char word[BUF_SIZE];
    if (option[0] == '1')
    {
        file_words = fopen("words.txt", "r");

        while (fgets(word, sizeof(word), file_words))
        {
            word[strlen(word) - 1] = '\0';
            printf("%s\n", word);
        }

        fflush(stdout);

        fclose(file_words);
    }
    else if (option[0] == '2')
    {
    }
    else if (option[0] == '3')
    {
    }
    else if (option[0] == '4')
    {
        char menu_admin_logout[] = "                 LOGOUT\n==========================================\n\n";
        send(admin_fd, menu_admin_logout, strlen(menu_admin_logout), 0);
        fflush(stdout);
    }
    else
    {
    }
}
