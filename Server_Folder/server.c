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

/*-----------------------------------------------------------------------
                                MACROS
-----------------------------------------------------------------------*/
#define SERVER_PORT 9876
#define BUF_SIZE 1024

/*-----------------------------------------------------------------------
                              PROTÓTIPOS
-----------------------------------------------------------------------*/
void process_client(int fd);
void erro(char *msg);
// /----------/ Login /----------/
void login(int client_fd, char username[]);
// /----------/ Menu Client /----------/
void menu_client(int client_fd, char username[]);
// /----------/ Menu Admin /----------/
void menu_admin(int admin_fd);

/*-----------------------------------------------------------------------
                                MAIN
-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------
                            PROCESS CLIENT
-----------------------------------------------------------------------*/
void process_client(int client_fd)
{
    while (1)
    {

        // /----------/ Login /----------/
        char username[BUF_SIZE] = "";
        login(client_fd, username);

        if (strcmp(username, "admin") == 0)
        {
            // /----------/ Menu Admin /----------/
            menu_admin(client_fd);
        }
        else
        {
            // /----------/ Menu Client /----------/
            menu_client(client_fd, username);
        }
    }
}

/*-----------------------------------------------------------------------
                                ERRO
-----------------------------------------------------------------------*/
void erro(char *msg)
{
    printf("Erro: %s\n", msg);
    exit(-1);
}

/*-----------------------------------------------------------------------
                                LOGIN
-----------------------------------------------------------------------*/
void login(int client_fd, char username[])
{
    char msg_inicial[] = "\n==========================================\n                  LOGIN\n\n";
    send(client_fd, msg_inicial, strlen(msg_inicial), 0);
    fflush(stdout);

    int option = -1;
    while (option == -1)
    {
        // /----------/ Enviar print de pedido do username /----------/
        char msg_get_user[] = "\n Username: ";
        send(client_fd, msg_get_user, strlen(msg_get_user), 0);
        fflush(stdout);

        // /----------/ Receber username inserido /----------/
        ssize_t size_user = recv(client_fd, username, BUF_SIZE, 0);
        username[size_user - 2] = '\0';

        FILE *file_user;
        char filename_user[BUF_SIZE];
        strcpy(filename_user, username);
        strcat(filename_user, ".txt");

        if ((file_user = fopen(filename_user, "r")) == NULL)
        {
            // /----------/ Caso não exista o username em causa /----------/
            char msg_get_user_error[] = "  Username não existe. Insira um username válido\n";
            send(client_fd, msg_get_user_error, strlen(msg_get_user_error), 0);
            fflush(stdout);
            option = -1;
        }
        else
        {
            // /----------/ Existe username em causa /----------/
            char user_password_auth[BUF_SIZE] = "";
            fread(user_password_auth, sizeof(user_password_auth), 1, file_user);
            fflush(stdout);

            int tentativas = 0;
            while (1)
            {
                // /----------/ Pedir password /----------/
                char msg_get_password[] = "\n Password: ";
                send(client_fd, msg_get_password, strlen(msg_get_password), 0);
                fflush(stdout);

                // /----------/ Receber password inserido /----------/
                char password[BUF_SIZE] = "";
                ssize_t size_password = recv(client_fd, password, BUF_SIZE, 0);
                fflush(stdout);
                password[size_password - 2] = '\0';

                // DEBUG
                /*printf("Comparação dá %d\npassword->%s\nreal_password->%s\n", strcmp(password, user_password_auth), password, user_password_auth);
                fflush(stdout);*/

                if (strcmp(password, user_password_auth) == 0)
                {
                    // /----------/ LOGIN AUTORIZADO /----------/
                    char msg_login_sucess[] = "\n             Bem vind@ ";
                    strcat(msg_login_sucess, username);
                    strcat(msg_login_sucess, "\n\n");
                    send(client_fd, msg_login_sucess, strlen(msg_login_sucess), 0);
                    fflush(stdout);
                    option = 0;
                    break;
                }
                else
                {
                    // /----------/ PASSWORD ERRADA /----------/
                    printf("%s\n", password);
                    fflush(stdout);

                    tentativas++;
                    char msg_get_password_error[] = "  Password Incorrreta. ";
                    char ten = (3 - tentativas) + '0';
                    char tenta[2] = "";
                    tenta[0] = ten;
                    strcat(msg_get_password_error, tenta);
                    strcat(msg_get_password_error, " tentativas restantes!\n");
                    send(client_fd, msg_get_password_error, strlen(msg_get_password_error), 0);
                    fflush(stdout);
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

/*-----------------------------------------------------------------------
                              MENU CLIENT
-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------
                             MENU ADMIN
-----------------------------------------------------------------------*/
void menu_admin(int admin_fd)
{
    while (1)
    {
        // /----------/ Print Menu /----------/
        char menu_admin[] = "\n==========================================\n               MENU ADMIN\n\n";
        send(admin_fd, menu_admin, strlen(menu_admin), 0);
        fflush(stdout);

        char admin_options[] = "        (1) Consultar Palavras\n\
        (2) Adicionar Palavra\n\
        (3) Remover Palavra\n\
        (4) Adicionar Utilzador\n\
        (5) Sair\n\n ";
        send(admin_fd, admin_options, strlen(admin_options), 0);
        fflush(stdout);

        // /----------/ Escolher o que fazer /----------/
        char option[BUF_SIZE] = "";
        ssize_t size_choice = recv(admin_fd, option, BUF_SIZE - 1, 0);
        option[size_choice - 2] = '\0';
        fflush(stdout);

        // /----------/ Preparar manipulação de ficheiro /----------/
        FILE *file_words;
        char word[BUF_SIZE];

        // /====================/ (1) Consultar palavras no ficheiro /====================/
        if (option[0] == '1')
        {
            file_words = fopen("words.txt", "r");

            // /----------/ Printf escolha /----------/
            char menu_consulta[] = "\n==========================================\n      CONSULTAR PALAVRAS\n\n";
            send(admin_fd, menu_consulta, strlen(menu_consulta), 0);
            fflush(stdout);

            while (fgets(word, sizeof(word), file_words))
            {
                send(admin_fd, word, strlen(word), 0);
                fflush(stdout);
            }

            fclose(file_words);
        }
        // /====================/ (2) Adicionar palavra ao ficheiro /====================/
        else if (option[0] == '2')
        {
            file_words = fopen("words.txt", "a");

            // /----------/ Printf escolha /----------/
            char menu_adicionar[] = "\n==========================================\n       ADICIONAR PALAVRA\n\n";
            send(admin_fd, menu_adicionar, strlen(menu_adicionar), 0);
            fflush(stdout);

            char new_word[BUF_SIZE] = "";
            recv(admin_fd, new_word, BUF_SIZE - 1, 0);
            fwrite(new_word, sizeof(char), strlen(new_word), file_words);
            fflush(stdout);

            fclose(file_words);
        }
        // /====================/ (3) Remover palavra do ficheiro /====================/
        else if (option[0] == '3')
        {
            file_words = fopen("words.txt", "r");
            FILE *file_words_aux;
            file_words_aux = fopen("words_aux.txt", "wt+");

            // /----------/ Printf remover /----------/
            char menu_remover[] = "\n==========================================\n       REMOVER PALAVRA\n\n";
            send(admin_fd, menu_remover, strlen(menu_remover), 0);
            fflush(stdout);

            char word_to_delete[BUF_SIZE] = "";
            recv(admin_fd, word_to_delete, BUF_SIZE - 1, 0);
            fflush(stdout);

            // /----------/ Escrever no ficheiro auxiliar /----------/
            while (fgets(word, sizeof(word), file_words))
            {
                if (strcmp(word_to_delete, word) != 0)
                {
                    fwrite(word, sizeof(char), strlen(word), file_words_aux);
                    fflush(stdout);
                }
            }
            fclose(file_words);

            file_words = fopen("words.txt", "wt");
            fseek(file_words_aux, 0, SEEK_SET);
            while (fgets(word, sizeof(word), file_words_aux))
            {
                fwrite(word, sizeof(char), strlen(word), file_words);
                fflush(stdout);
            }

            fclose(file_words_aux);
            remove("words_aux.txt");
            fclose(file_words);
        }
        // /====================/ (4) Adicinar utilizador /====================/
        else if (option[0] == '4')
        {
            char menu_admin_add_user[] = "\n==========================================\n        ADICIONAR UTILIZADOR\n\n";
            send(admin_fd, menu_admin_add_user, strlen(menu_admin_add_user), 0);
            fflush(stdout);

            // /----------/ Enviar print de pedido do novo username /----------/
            char msg_get_new_user[] = "\n New username: ";
            send(admin_fd, msg_get_new_user, strlen(msg_get_new_user), 0);
            fflush(stdout);

            // /----------/ Receber novo username inserido /----------/
            char new_username[BUF_SIZE] = "";
            ssize_t size_user = recv(admin_fd, new_username, BUF_SIZE - 1, 0);
            new_username[size_user - 2] = '\0';

            // DEBUG
            printf("new user->%s<-\n", new_username);

            FILE *file_new_user;
            strcat(new_username, ".txt");

            // Username ainda não registado
            if (fopen(new_username, "r") == NULL)
            {
                // /----------/ Enviar print de pedido da password do novo username /----------/
                char msg_get_new_user_password[] = "\n New username password: ";
                send(admin_fd, msg_get_new_user_password, strlen(msg_get_new_user_password), 0);
                fflush(stdout);

                // /----------/ Receber password do novo username inserido /----------/
                char new_username_password[BUF_SIZE] = "";
                ssize_t size_user_password = recv(admin_fd, new_username_password, BUF_SIZE - 1, 0);
                new_username_password[size_user_password - 2] = '\0';

                // DEBUG
                printf("new user password->%s<-\n", new_username_password);

                // /----------/ Receber password do novo username inserido /----------/
                file_new_user = fopen(new_username, "wt");
                fwrite(new_username_password, sizeof(char), strlen(new_username_password), file_new_user);
                fflush(stdout);
                fclose(file_new_user);
            }
            // Username já existe
            else
            {
                printf("User já existe\n");
                char msg_get_new_user_already_exists[] = "\n Username already exists";
                send(admin_fd, msg_get_new_user_already_exists, strlen(msg_get_new_user_already_exists), 0);
                fflush(stdout);
            }
        }
        // /====================/ (5) Sair ficheiro /====================/
        else if (option[0] == '5')
        {
            char menu_admin_logout[] = "                 LOGOUT\n==========================================\n\n";
            send(admin_fd, menu_admin_logout, strlen(menu_admin_logout), 0);
            fflush(stdout);
            break;
        }
    }
}
