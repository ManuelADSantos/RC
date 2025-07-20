#include <stdio.h>
#include <unistd.h>
#include <string.h> 
 
int main() 
{ 
  char msg[20]; 
  char buffer[20]; 
  int fileids[2]; 
 
  if (pipe(fileids) == 0)   /* Criou o pipe, sem erros */ 
  {
    if (fork() == 0)        /* Processo Filho */ 
    {
      close(fileids[0]);    /* Fecha o descritor de leitura */ 
      strcpy(msg, "Olá Pai!"); 
      write (fileids[1], msg, sizeof(msg)); 
      close(fileids[1]);    /* Fecha o descritor de escrita */ 
    } 
    else                    /* Processo Pai */ 
    {
      close(fileids[1]);    /* Fecha o descritor de escrita */ 
      read (fileids[0], buffer, sizeof(buffer)); 
      printf("Recebi do pipe a mensagem: %s\n",buffer); 
      close(fileids[0]);    /* Fecha o descritor de leitura */ 
    } 
  }
  else 
    printf("ERRO na criação do PIPE!\n"); 
 
  return(0); 
}