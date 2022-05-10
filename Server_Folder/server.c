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
#include <sys/stat.h>

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
// /----------/ send_visual /----------/
void send_visual(int fd, char msg[]);
// /----------/ recv_visual /----------/
void recv_visual(int fd, char buffer[]);

/*-----------------------------------------------------------------------
                                MAIN
-----------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    // Create users directory
    mkdir("./users", 0777);

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
    send_visual(client_fd, msg_inicial);

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
        char filename_user_path[BUF_SIZE] = "./users/";
        strcat(filename_user_path, filename_user);

        if ((file_user = fopen(filename_user_path, "r")) == NULL)
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
    // /----------/ Print Menu /----------/
    char menu_client[] = "\n==========================================\n                  MENU\n\n";
    send(client_fd, menu_client, strlen(menu_client), 0);
    fflush(stdout);

    char client_options[] = "        (1) Utilizadores Online\n\
        (2) Enviar Mensagem\n\
        (3) Sair\n\n ";

    send(client_fd, client_options, strlen(client_options), 0);
    fflush(stdout);

    // /----------/ Escolher o que fazer /----------/
    char option[BUF_SIZE] = "";
    ssize_t size_choice = recv(client_fd, option, BUF_SIZE, 0);
    fflush(stdout);
    option[size_choice - 2] = '\0';

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
        send_visual(admin_fd, menu_admin);

        char admin_options[] = "        (1) Consultar Palavras\n\
        (2) Adicionar Palavra\n\
        (3) Remover Palavra\n\
        (4) Adicionar Utilzador\n\
        (5) Remover Utilzador\n\
        (6) Sair\n\n ";
        send_visual(admin_fd, admin_options);

        // /----------/ Escolher o que fazer /----------/
        char option[BUF_SIZE] = "";
        ssize_t size_choice = recv(admin_fd, option, BUF_SIZE, 0);
        fflush(stdout);
        option[size_choice - 2] = '\0';

        // /----------/ Preparar manipulação de ficheiro /----------/
        FILE *file_words;
        char word[BUF_SIZE];

        // /====================/ (1) Consultar palavras no ficheiro /====================/
        if (option[0] == '1')
        {
            // /----------/ Printf escolha /----------/
            char menu_consulta[] = "\n==========================================\n      CONSULTAR PALAVRAS\n\n";
            send(admin_fd, menu_consulta, strlen(menu_consulta), 0);
            fflush(stdout);

            // /----------/ Print Palavras /----------/
            file_words = fopen("words.txt", "a+");
            // chmod("words.txt", 0777);
            while (fgets(word, sizeof(word), file_words))
            {
                send(admin_fd, word, strlen(word), 0);
                fflush(stdout);
            }

            // /----------/ Fechar ficheiro /----------/
            fclose(file_words);
        }
        // /====================/ (2) Adicionar palavra ao ficheiro /====================/
        else if (option[0] == '2')
        {
            // /----------/ Printf escolha /----------/
            char menu_adicionar[] = "\n==========================================\n       ADICIONAR PALAVRA\n\n";
            send(admin_fd, menu_adicionar, strlen(menu_adicionar), 0);
            fflush(stdout);

            // /----------/ Recolher Palavra a Adicionar /----------/
            char new_word[BUF_SIZE] = "";
            recv(admin_fd, new_word, BUF_SIZE + 1, 0);
            fflush(stdout);

            // =====================
            printf("Recebi >%s<\n", new_word);
            fflush(stdout);
            // =====================

            // /----------/ Escrever nova palavra no ficheiro /----------/
            if ((file_words = fopen("words.txt", "a")) != NULL)
            {
                // chmod("words.txt", 0777);
                fwrite(new_word, sizeof(char), strlen(new_word), file_words);

                // /----------/ Ajustar \n /----------/
                int count = 0;
                while (new_word[count] != '\n')
                    count++;

                if ((int)strlen(new_word) - count != 1)
                {
                    char aux[] = "\n";
                    fwrite(aux, sizeof(char), strlen(aux), file_words);
                    fflush(stdout);
                }

                // /----------/ Fechar ficheiro /----------/
                fclose(file_words);
                fflush(stdout);
            }
            else
            {
                printf("ERRO: Abrir ficheiro em adicionar palavra\n");
                fflush(stdout);
            }
        }
        // /====================/ (3) Remover palavra do ficheiro /====================/
        else if (option[0] == '3')
        {
            if ((file_words = fopen("words.txt", "r")) != NULL)
            {
                // chmod("words.txt", 0777);
                FILE *file_words_aux;
                if ((file_words_aux = fopen("words_aux.txt", "wt+")) != NULL)
                {
                    // chmod("words_aux.txt", 0777);

                    // /----------/ Printf remover /----------/
                    char menu_remover[] = "\n==========================================\n       REMOVER PALAVRA\n\n";
                    send(admin_fd, menu_remover, strlen(menu_remover), 0);
                    fflush(stdout);

                    // /----------/ Pedir palavra a remover (word\n) /----------/
                    char word_to_delete[BUF_SIZE] = "";
                    recv(admin_fd, word_to_delete, BUF_SIZE, 0);
                    fflush(stdout);

                    // /----------/ Ajustar \n /----------/
                    int count = 0;
                    while (word_to_delete[count] != '\n')
                        count++;

                    if ((int)strlen(word_to_delete) - count != 1)
                    {
                        strcat(word_to_delete, "\n");
                    }

                    // /----------/ Escrever no ficheiro auxiliar /----------/
                    while (fgets(word, sizeof(word), file_words))
                    {
                        fflush(stdout);
                        if (strcmp(word_to_delete, word) != 0)
                        {
                            fwrite(word, sizeof(char), strlen(word), file_words_aux);
                            fflush(stdout);
                        }
                    }
                    fclose(file_words);

                    file_words = fopen("words.txt", "wt");
                    // chmod("words.txt", 0777);
                    fseek(file_words_aux, 0, SEEK_SET);
                    while (fgets(word, sizeof(word), file_words_aux))
                    {
                        fflush(stdout);
                        fwrite(word, sizeof(char), strlen(word), file_words);
                        fflush(stdout);
                    }

                    fclose(file_words_aux);
                    // remove("words_aux.txt");
                }
                fclose(file_words);
                fflush(stdout);
            }
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
            ssize_t size_user = recv(admin_fd, new_username, BUF_SIZE, 0);
            fflush(stdout);
            new_username[size_user - 2] = '\0';

            FILE *file_new_user;
            strcat(new_username, ".txt");
            char new_username_path[BUF_SIZE] = "./users/";
            strcat(new_username_path, new_username);

            // Username ainda não registado
            if (fopen(new_username_path, "r") == NULL)
            {
                // /----------/ Enviar print de pedido da password do novo username /----------/
                char msg_get_new_user_password[] = "\n New username password: ";
                send(admin_fd, msg_get_new_user_password, strlen(msg_get_new_user_password), 0);
                fflush(stdout);

                // /----------/ Receber password do novo username inserido /----------/
                char new_username_password[BUF_SIZE] = "";
                ssize_t size_user_password = recv(admin_fd, new_username_password, BUF_SIZE, 0);
                new_username_password[size_user_password - 2] = '\0';

                // /----------/ Receber password do novo username inserido /----------/
                file_new_user = fopen(new_username_path, "wt");
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
        // /====================/ (5) Remover utilizador /====================/
        else if (option[0] == '5')
        {
            char menu_admin_remove_user[] = "\n==========================================\n        REMOVER UTILIZADOR\n\n";
            send(admin_fd, menu_admin_remove_user, strlen(menu_admin_remove_user), 0);
            fflush(stdout);

            // /----------/ Enviar print de pedido do novo username /----------/
            char msg_get_remove_user[] = "\n Username to remove: ";
            send(admin_fd, msg_get_remove_user, strlen(msg_get_remove_user), 0);
            fflush(stdout);

            // /----------/ Receber novo username inserido /----------/
            char remove_username[BUF_SIZE] = "";
            ssize_t size_user_remove = recv(admin_fd, remove_username, BUF_SIZE, 0);
            remove_username[size_user_remove - 2] = '\0';

            strcat(remove_username, ".txt");
            char remove_username_path[BUF_SIZE] = "./users/";
            strcat(remove_username_path, remove_username);
            strcpy(remove_username, remove_username_path);

            if (fopen(remove_username, "r") == NULL)
            {
                char msg_get_remove_user_dont_exist[] = "\n User does not exist";
                send(admin_fd, msg_get_remove_user_dont_exist, strlen(msg_get_remove_user_dont_exist), 0);
                fflush(stdout);
            }
            else
            {
                FILE *admin_auth;
                char admin_pass[BUF_SIZE] = "";
                admin_auth = fopen("./users/admin.txt", "r");
                fread(admin_pass, sizeof(admin_pass), 1, admin_auth);
                fclose(admin_auth);

                int tentativas = 0;
                while (1)
                {
                    // /----------/ Pedir password /----------/
                    char msg_get_password[] = "\n Admin password: ";
                    send(admin_fd, msg_get_password, strlen(msg_get_password), 0);
                    fflush(stdout);

                    // /----------/ Receber password inserido /----------/
                    char password[BUF_SIZE] = "";
                    ssize_t size_password = recv(admin_fd, password, BUF_SIZE, 0);
                    fflush(stdout);
                    password[size_password - 2] = '\0';

                    if (strcmp(password, admin_pass) == 0)
                    {
                        // /----------/ LOGIN AUTORIZADO /----------/
                        char msg_remove_user_sucess[] = "\n User removed";
                        send(admin_fd, msg_remove_user_sucess, strlen(msg_remove_user_sucess), 0);
                        fflush(stdout);

                        remove(remove_username);

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
                        send(admin_fd, msg_get_password_error, strlen(msg_get_password_error), 0);
                        fflush(stdout);
                        if (tentativas == 3)
                            break;
                    }
                }
            }
        }
        // /====================/ (6) Sair /====================/
        else if (option[0] == '6')
        {
            char menu_admin_logout[] = "                 LOGOUT\n==========================================\n\n";
            send(admin_fd, menu_admin_logout, strlen(menu_admin_logout), 0);
            fflush(stdout);
            break;
        }
    }
}

void send_visual(int fd, char msg[])
{
    send(fd, msg, strlen(msg), 0);
    fflush(stdout);
}

void recv_visual(int fd, char buffer[])
{
    send(fd, buffer, BUF_SIZE, 0);
    fflush(stdout);
}