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
#include <arpa/inet.h>
#include <time.h>

/*-----------------------------------------------------------------------
                                MACROS
-----------------------------------------------------------------------*/
#define SERVER_PORT 9876
#define BUF_SIZE 1024
#define SA struct sockaddr

/*-----------------------------------------------------------------------
                          VARIÁVEIS GLOBAIS
-----------------------------------------------------------------------*/
int client_port = 49152;

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
            client_port++;
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
        send_visual(client_fd, msg_get_user);

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
                    // Construir mensagem de sucesso
                    char msg_login_sucess[] = "\n             Bem vind@ ";
                    strcat(msg_login_sucess, username);
                    strcat(msg_login_sucess, "\n\n");

                    send_visual(client_fd, msg_login_sucess);

                    // =======================
                    char online_status[BUF_SIZE] = "";
                    strcat(online_status, username);
                    strcat(online_status, ",");
                    char porto[10] = "";
                    sprintf(porto, "%d\n", client_port);
                    fflush(stdout);
                    // client_port ++;
                    strcat(online_status, porto);
                    fflush(stdout);

                    FILE *status;
                    status = fopen("status.txt", "a");
                    fwrite(online_status, sizeof(char), strlen(online_status), status);
                    fclose(status);
                    // =======================

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
    pid_t pid;

    // /----------/ Processo de Scanf /----------/
    if ((pid = fork()) > 0)
    {
        while (1)
        {

            // /----------/ Print Menu /----------/
            char menu_client[] = "\n==========================================\n                  MENU\n\n";
            send_visual(client_fd, menu_client);
            char client_options[] = "        $ help - em caso de dúvida \n\n ";
            send_visual(client_fd, client_options);

            // /----------/ Escolher o que fazer /----------/
            char cmd[BUF_SIZE] = "", option[BUF_SIZE] = "";
            // ssize_t size_choice =
            recv(client_fd, cmd, BUF_SIZE, 0);
            fflush(stdout);

            /*printf("CMD>%s<\n", cmd);
            fflush(stdout);*/

            strncpy(option, cmd, 4);

            /*printf("OPTION>%s<\n", option);
            fflush(stdout);*/

            if (strcmp(option, "send") == 0)
            {
                // /----------/ Variáveis /----------/
                char user_destino[BUF_SIZE] = "", msg[BUF_SIZE] = "";
                int point = 5, aux = 0;

                // /----------/ Obter Username de Destino /----------/
                do
                {
                    /*printf("char>%c<\n", cmd[point]);
                    fflush(stdout);*/
                    user_destino[aux] = cmd[point];
                    point++;
                    /*printf("point>%d<\n", point);
                    fflush(stdout);*/
                    aux++;
                    /*printf("point>%d<\n", point);
                    fflush(stdout);*/
                } while (cmd[point] != ' ');

                aux = 0;
                point++;

                // /----------/ Obter Mensagem a Enviar /----------/
                while (cmd[point] != 0)
                {
                    msg[aux] = cmd[point];
                    point++;
                    aux++;
                }

                // /----------/ Tratar Mensagem a Enviar /----------/
                char msg_aux[BUF_SIZE] = "";
                struct tm *ptr;
                time_t t;
                t = time(NULL);
                ptr = localtime(&t);

                strcat(msg_aux, "\n  (");
                strcat(msg_aux, asctime(ptr));
                strcat(msg_aux, ") De ");
                strcat(msg_aux, username);
                strcat(msg_aux, "\n  ");
                strcat(msg_aux, msg);
                strcat(msg_aux, "\n\n");

                strcpy(msg, msg_aux);

                /*printf("USER>%s<\n", user);
                fflush(stdout);
                printf("MESSAGE>%s<\n", msg);
                fflush(stdout);*/

                // /----------/ Procurar Porto do Destinatário /----------/
                int port_destino = 0;
                FILE *destiny_port;
                char user_port[BUF_SIZE] = "", reader[BUF_SIZE] = "", reader_aux[BUF_SIZE] = "";

                // /----------/ Fechar ficheiro /----------/
                destiny_port = fopen("status.txt", "r");

                // /----------/ Procurar porto para enviar informação /----------/
                while (fgets(reader, sizeof(reader), destiny_port))
                {
                    int ind = 0, count = 0;
                    while (reader[ind] != ',')
                    {
                        reader_aux[ind] = reader[ind];
                        ind++;
                    }

                    /*printf("DESTINY_USER_NAME_READ<%s>\n", reader_aux);
                    fflush(stdout);*/

                    if (strcmp(user_destino, reader_aux) == 0)
                    {
                        /*printf("BATEU_CERTO<%s>\n", reader_aux);
                        fflush(stdout);*/

                        ind++;
                        while (reader[ind] != '\n')
                        {
                            user_port[count] = reader[ind];
                            ind++;
                            count++;
                        }
                        // === Verificação ===
                        /*user_port[count] = '\0';
                        printf("USER_PORT<%s>\n", user_port);
                        fflush(stdout);*/

                        port_destino = atoi(user_port);

                        /*printf("USER_PORT_INT<%d>\n", port_destino);
                        fflush(stdout);*/
                    }

                    strcpy(reader, "");
                }
                // /----------/ Fechar ficheiro /----------/
                fclose(destiny_port);

                // /----------/ Socket para enviar mensagem /----------/
                int send_fd;
                //, connection_fd;
                struct sockaddr_in sender_adrr, receiver_addr;

                if ((send_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0)
                {
                    bzero(&receiver_addr, sizeof(receiver_addr));

                    // socket() com sucesso

                    sender_adrr.sin_family = AF_INET;
                    sender_adrr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    sender_adrr.sin_port = htons(port_destino);

                    if (connect(send_fd, (SA *)&sender_adrr, sizeof(sender_adrr)) == 0)
                    {
                        // connect() com sucesso

                        send(send_fd, msg, strlen(msg), 0);

                        close(send_fd);
                    }
                }
            }
            else if (strcmp(option, "list") == 0)
            {

                FILE *online_status;
                char user[BUF_SIZE] = "";
                char user_aux[BUF_SIZE] = "";

                // /----------/ Printf escolha /----------/
                char menu_online[] = "\n==========================================\n      Utilizadores Online\n\n";
                send_visual(client_fd, menu_online);

                // /----------/ Print Palavras /----------/
                online_status = fopen("status.txt", "a+");
                while (fgets(user_aux, sizeof(user_aux), online_status))
                {
                    strcpy(user, "");
                    int ind = 0;
                    while (user_aux[ind] != ',')
                    {
                        user[ind] = user_aux[ind];
                        ind++;
                    }
                    user[ind] = '\0';
                    /*printf("USER<%s>\n", user);
                    fflush(stdout);*/

                    if (strcmp(user, username) != 0 && strcmp(user, "admin") != 0)
                    {
                        send(client_fd, user, strlen(user), 0);
                        send(client_fd, "\n", strlen("\n"), 0);
                        fflush(stdout);
                    }
                }

                // /----------/ Fechar ficheiro /----------/
                fclose(online_status);
            }
            else if (strcmp(option, "help") == 0)
            {
                char help_menu[] = "\n==========================================\n                  HELP\n\n";
                send_visual(client_fd, help_menu);
                char help[] = "        Commands\n\
$ send [user] [message] - Enviar mensagem[message] para o utilizador[user]\n\
$ help - Mostrar menu de ajuda\n\
$ list - Listar utilizadores online\n\
$ exit - Sair / Logout \n\n ";
                send_visual(client_fd, help);
            }
            else if (strcmp(option, "exit") == 0)
            {
                char menu_client_logout[] = "                 LOGOUT\n==========================================\n\n";
                send_visual(client_fd, menu_client_logout);

                // Registar estado online
                FILE *status_aux, *status;
                status_aux = fopen("status_aux.txt", "wt+");
                status = fopen("status.txt", "r");

                // /----------/ Escrever no ficheiro auxiliar /----------/
                char read_status[BUF_SIZE] = "";
                char to_off_status[BUF_SIZE] = "";
                char aux[10] = "";
                strcat(to_off_status, username);
                strcat(to_off_status, ",");
                sprintf(aux, "%d\n", client_port);
                strcat(to_off_status, aux);

                while (fgets(read_status, sizeof(read_status), status))
                {
                    // printf("Word>%s<|Word_to_delete>%s<|Comparation>%d<\n", read_status, to_off_status, strcmp(to_off_status, read_status));
                    fflush(stdout);

                    if (strcmp(to_off_status, read_status) != 0)
                    {
                        fwrite(read_status, sizeof(char), strlen(read_status), status_aux);
                        fflush(stdout);
                    }
                }
                fclose(status);

                status = fopen("status.txt", "wt");
                fseek(status_aux, 0, SEEK_SET);
                while (fgets(read_status, sizeof(read_status), status_aux))
                {
                    fflush(stdout);
                    fwrite(read_status, sizeof(char), strlen(read_status), status);
                    fflush(stdout);
                }
                fclose(status);
                fclose(status_aux);
                // remove("status_aux.txt");

                kill(pid, SIGINT);
                wait(NULL);

                break;
            }
        }
    }

    else
    {
        while (1)
        {
            int port_lido = 0;
            FILE *available_ports;
            char user_port[BUF_SIZE] = "", reader[BUF_SIZE] = "";

            // /----------/ Fechar ficheiro /----------/
            available_ports = fopen("status.txt", "r");

            // /----------/ Ler um Porto para tentar receber informação /----------/
            while (fgets(reader, sizeof(reader), available_ports))
            {
                strcpy(reader, "");
                int ind = 0, count = 0;
                while (reader[ind] != ',')
                    ind++;

                ind++;

                while (reader[ind] != '\n')
                {
                    user_port[count] = reader[ind];
                    ind++;
                    count++;
                }

                user_port[count] = '\0';
                /*printf("USER_PORT<%s>\n", user_port);
                fflush(stdout);*/

                port_lido = atoi(user_port);

                /*printf("USER_PORT_INT<%d>\n", port_lido);
                fflush(stdout);*/
            }
            // /----------/ Fechar ficheiro /----------/
            fclose(available_ports);

            // /----------/ Socket para receber mensagem /----------/
            int receive_fd, connection_fd, client_addr_size;
            struct sockaddr_in receiver_adrr, sender_addr;

            if ((receive_fd = socket(AF_INET, SOCK_STREAM, 0)) > 0)
            {
                // socket() com sucesso

                receiver_adrr.sin_family = AF_INET;
                receiver_adrr.sin_addr.s_addr = htonl(INADDR_ANY);
                receiver_adrr.sin_port = htons(port_lido);

                if ((bind(receive_fd, (SA *)&receiver_adrr, sizeof(receiver_adrr))) == 0)
                {
                    // bind() com sucesso
                    if ((listen(receive_fd, 5)) == 0)
                    {
                        // listen() com sucesso
                        connection_fd = accept(receive_fd, (SA *)&sender_addr, (socklen_t *)&client_addr_size);
                        if (connection_fd >= 0)
                        {
                            // receiver aceitou o sender

                            char msg_received[BUF_SIZE] = "";
                            // ssize_t size_msg_received =
                            recv(connection_fd, msg_received, BUF_SIZE, 0);

                            send_visual(client_fd, msg_received);

                            close(connection_fd);
                        }
                    }
                }
            }
        }
    }
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

            // /----------/ Escrever nova palavra no ficheiro /----------/
            if ((file_words = fopen("words.txt", "a")) != NULL)
            {
                // chmod("words.txt", 0777);
                fwrite(new_word, sizeof(char), strlen(new_word), file_words);

                // /----------/ Ajustar \n /----------/
                int count = 0;
                while (new_word[count] != '\n' && count < BUF_SIZE)
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
                    send_visual(admin_fd, menu_remover);

                    // /----------/ Pedir palavra a remover (word\n) /----------/
                    char word_to_delete[BUF_SIZE] = "";
                    recv(admin_fd, word_to_delete, BUF_SIZE, 0);
                    fflush(stdout);

                    // /----------/ Ajustar \n /----------/
                    int count = 0;
                    while (word_to_delete[count] != '\n' && count < BUF_SIZE)
                        count++;

                    int tamanho = 0;
                    if ((int)strlen(word_to_delete) - count != 1)
                    // Não é telnet
                    {
                        tamanho = strlen(word_to_delete);
                        strcat(word_to_delete, "\n");
                    }
                    else
                    {
                        tamanho = strlen(word_to_delete) - 2;
                    }

                    // /----------/ Escrever no ficheiro auxiliar /----------/
                    while (fgets(word, sizeof(word), file_words))
                    {
                        // printf("Word>%s<|Word_to_delete>%s<|Comparation>%d<\n", word, word_to_delete, strncmp(word_to_delete, word, tamanho));
                        fflush(stdout);
                        if (strncmp(word_to_delete, word, tamanho) != 0)
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
            send_visual(admin_fd, menu_admin_logout);

            FILE *status_aux, *status;
            status_aux = fopen("status_aux.txt", "wt+");
            status = fopen("status.txt", "r");

            // /----------/ Escrever no ficheiro auxiliar /----------/
            char read_status[BUF_SIZE] = "";
            char to_off_status[BUF_SIZE] = "";
            char aux[10] = "";
            strcat(to_off_status, "admin,");
            sprintf(aux, "%d\n", client_port);
            strcat(to_off_status, aux);

            while (fgets(read_status, sizeof(read_status), status))
            {
                // printf("Word>%s<|Word_to_delete>%s<|Comparation>%d<\n", read_status, to_off_status, strcmp(to_off_status, read_status));
                fflush(stdout);

                if (strcmp(to_off_status, read_status) != 0)
                {
                    fwrite(read_status, sizeof(char), strlen(read_status), status_aux);
                    fflush(stdout);
                }
            }
            fclose(status);

            status = fopen("status.txt", "wt");
            fseek(status_aux, 0, SEEK_SET);
            while (fgets(read_status, sizeof(read_status), status_aux))
            {
                fflush(stdout);
                fwrite(read_status, sizeof(char), strlen(read_status), status);
                fflush(stdout);
            }
            fclose(status);
            fclose(status_aux);
            // remove("status_aux.txt");

            break;
        }
    }
}

void send_visual(int fd, char msg[])
{
    send(fd, msg, strlen(msg), 0);
    fflush(stdout);
}
