#include <stdio.h>
#include <unistd.h>

int main()
{
    int i = fork(); 
    if(i!=0)
        printf("Eu sou o processo pai e o meu PID é %d\n",getpid());
    else
        printf("Eu sou o processo filho e o meu PID é %d e o do meu pai é %d\n",getpid(),getppid());
    return 0;
}