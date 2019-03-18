#include <unistd.h>
#include <stdio.h>


void print() {
	printf("\033[1;32m");
    printf("user@SISOP");
	printf("\033[0m");
	printf(":");
	printf("\033[1;34m");
	printf("~");
	printf("\033[0m");
	printf(" $");	
}

void main(int argc, char* argv[], char* envp[]) {

	char nome[20] ;
	int i, j, pid, k;
	int run = 1;

	while(run) {
		print();
    	scanf ("%s", nome);
    	pid = fork();
    	if(pid != 0) {
       		sleep(1) ;
       		pid = wait(0) ;   
 		} else {
       		j = execve (nome, argv, NULL);
       }
    }  
}


