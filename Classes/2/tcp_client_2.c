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

#define BUF_SIZE 1024

void erro(char *msg);

int main(int argc, char *argv[])
{
  char endServer[100];
  int fd;
  struct sockaddr_in addr;
  struct hostent *hostPtr;

  if (argc != 3)
  {
    printf("cliente <host> <port>\n");
    exit(-1);
  }

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
  // write(fd, argv[3], 1 + strlen(argv[3]));

  // ========== Ex2 ==========
  char msg[BUF_SIZE];
  read(fd, msg, BUF_SIZE - 1);
  printf("%s\n", msg);

  char choice[1];

  scanf("%c", choice);

  if ((choice[0] == '1') || (choice[0] == '2') || (choice[0] == '3'))
  {
    write(fd, choice, 2);
    // printf("%c", choice);
  }
  else
  {
    printf("Opcao invalida\n");
    fflush(stdout);
    close(fd);
    exit(0);
  }

  char resposta[BUF_SIZE];
  read(fd, resposta, BUF_SIZE - 1);
  printf("%s\n", resposta);

  fflush(stdout);

  //  =========================

  close(fd);
  exit(0);
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
