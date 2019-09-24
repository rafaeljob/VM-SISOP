/*	NOME: RAFAEL BASSO
//	gcc siso.c -o s -lpthread
//  gcc console.c -o console -lpthread
//  **O executavel do console deve se chamar console**
//
//	O programa encontra-se 100% funcional. Para rodar os programas ./s
//		quando aparecer o bash user@SISOP:~$ p1.txt roda o programa p1
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#define SHMSZ     27

sem_t print;
pthread_t pid_standard, pid_io;

char buffer_cmd[20];
char *sm;
int block = 0;

void printt() {
	sem_wait(&print);
	printf("\033[0;31m");
	printf("user@SISOP-CONSOLE");
	printf("\033[0m");
	printf(":");
	printf("\033[1;34m");
	printf("~");
	printf("\033[0m");
	printf("$ ");
	sem_post(&print);	
}

void print_io(char *s) {
	sem_wait(&print);
	printf("\r");
	printf("\033[0;31m");
	printf("user@SISOP-CONSOLE (");
	printf("\033[0;32m");
	printf("%s", s);
	printf("\033[0;31m");
	printf(")");
	printf("\033[0m");
	printf(":");
	printf("\033[1;34m");
	printf("~");
	printf("\033[0m");
	printf("$ ");
	sem_post(&print);	
}

void send(char *s) {
	for(int i = 0; i < 20; i++) {
        *s++ = buffer_cmd[i];
		if(buffer_cmd[i] == '\0') {
			break;
		}
	}
    *s = NULL;
}

void* standard() {
	while(1) {
		printt();
    	scanf("%s", &buffer_cmd);
		send(sm);
		if(strstr(buffer_cmd, "exit") != NULL) {
			sleep(2);
			exit(1);
		}

		while (*sm != '*') {
        	sleep(1);
		}
	}
}
void* io() {
	while (1) {
		sleep(2);
		if(sm[0] == 'i' && sm[1] == 'o' && sm[2] == '\0') {
			pthread_cancel(pid_standard);
			*sm = '*';
			while (*sm == '*') {
        		sleep(1);
			}
			print_io(sm);
			*sm = '*';
			while (*sm == '*') {
        		sleep(1);
			}
		
			if(*sm == 'r') {
				*sm= '*';
				scanf("%s", sm);
			} else if(*sm == 'w') {
				*sm = '*';
				while(*sm == '*') {
					sleep(1);
				}
				printf("%s\n", sm);
				*sm = '*';
			}
			block = 1;
		}
	}
}

void console() {
	int shmid, pid;
    key_t key;
    char *shm, *s;
    /*
     * Sabemos o nome do segmento criado pelo servidor.
     * "5678".
     */
    key = 5678;

    /*
     * Localiza o segmento. 
     */
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Associa o segmento ao nosso espaco de dados.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

	s = shm;
	sm = s;

	pthread_create(&pid_io, NULL, io, NULL);
	pthread_create(&pid_standard, NULL, standard, NULL);

	while(1) {
		sleep(1);
		if(block == 1) {
			pthread_create(&pid_standard, NULL, standard, NULL);
			block = 0;
		}
	}
	pthread_join(pid_standard, NULL);
	pthread_join(pid_io, NULL);
	
}

int main() {
	sem_init(&print, 0, 1);
	console();
	return 0;		
}
