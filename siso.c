/*	NOME: RAFAEL BASSO
//	gcc siso.c -o s -lpthread
//  gcc console.c -o console -lpthread
//  **O executavel do console deve se chamar console**
//
//	O programa encontra-se 100% funcional. Para rodar os programas ./s
//		quando aparecer o bash user@SISOP:~$ p1.txt roda o programa p1
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSZ     27

#define MEMORY_SIZE 1024
#define NUM_REGISTER 8
#define PARTITION_SIZE 128
#define NUM_PAGE 64				//*************************************** 64 frames e paginas 16 linhas por frame // numero de paginas do programa posto na primeira linha do programa
#define PROGRAM_LIMIT_SIZE 256

//--------------------------------------------------------------
//	VAR. GLOB
//--------------------------------------------------------------

int interrupt = 0;
	//memoria do sistema
int memory_array[MEMORY_SIZE];
	//registradores da cpu
int REGS[NUM_REGISTER];
	//
int decode[5];
	//program counter da cpu
int PC = 0;
	//identificador base (numero aleatorio gerado para identificar processos)
int id_base;
	//semaforos do sistema
sem_t mutex_ready, mutex_blocked, sem_trap, sem_cpu, sem_io, sem_timer, sem_print, mutex_carga;
	//threads do sistema
pthread_t pid_cpu, pid_shell, pid_gp, pid_timer, pid_console;
	//flag para tratar i/o
int io = 0;
	//array para leitura do programa
int program[PROGRAM_LIMIT_SIZE];

int* aloca_paginas(int n);

//--------------------------------------------------------------
//	PRINT
//--------------------------------------------------------------

void print_scheduler() {
	sem_wait(&sem_print);
	printf("\033[0;33m");
	printf("SCHED:");
	printf("\033[0m");
	sem_post(&sem_print);
}

void print_exit() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;31m");
	printf("QUIT");
	printf("\033[0m");
	printf("]\n");
	sem_post(&sem_print);
}

void print_gp() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;32m");
	printf("OK");
	printf("\033[0m");
	printf("]");
	printf("\033[0m");
	printf("\tGP\t\tPID(%d)\n", pid_gp);
	printf("\033[0m");
	sem_post(&sem_print);
}

void print_timer() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;32m");
	printf("OK");
	printf("\033[0m");
	printf("]");
	printf("\033[0m");
	printf("\tTIMER\t\tPID(%d)\n", pid_timer);
	printf("\033[0m");	
	sem_post(&sem_print);
}

void print_cpu() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;32m");
	printf("OK");
	printf("\033[0m");
	printf("]");
	printf("\033[0m");
	printf("\tCPU\t\tPID(%d)\n", pid_cpu);
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_shell() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;32m");
	printf("OK");
	printf("\033[0m");
	printf("]");
	printf("\033[0m");
	printf("\tSHELL\t\tPID(%d)\n", pid_shell);
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_console() {
	sem_wait(&sem_print);
	printf("[");
	printf("\033[0;32m");
	printf("OK");
	printf("\033[0m");
	printf("]");
	printf("\033[0m");
	printf("\tCONSOLE\t\tPID(%d)\n", pid_console);
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_int_timer() {
	sem_wait(&sem_print);
	printf("\033[0;33m");
	printf("\tTIMER\n");
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_int_trap_read() {
	sem_wait(&sem_print);
	printf("\033[0;33m");
	printf("\tTRAP READ\n");
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_int_trap_write() {
	sem_wait(&sem_print);
	printf("\033[0;33m");
	printf("\tTRAP WRITE\n");
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_io() {
	sem_wait(&sem_print);
	printf("\033[0;35m");
	printf("\nI/O PROTOCOL\n");
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_io_read(int cont) {
	printf("wtf");
	sem_wait(&sem_print);
	printf("\033[0;32m");
	printf("* ");
	printf("\033[0;35m");
	printf("READ");
	printf("\033[0m");
	printf(": %d", cont);
	sem_post(&sem_print);	
}

void print_io_write(int cont) {
	sem_wait(&sem_print);
	printf("\033[0;35m");
	printf("\nI/O PROTOCOL\n\n");
	printf("\033[0m");
	sem_post(&sem_print);	
}

void print_break() {
	sem_wait(&sem_print);
	printf("\033[0;31m");
	printf("\tBREAK");
	printf("\033[0m");
	printf("\n");
	sem_post(&sem_print);
}

void print_carga() {
	sem_wait(&sem_print);
	printf("\033[0;34m");
	printf("\nEscrevendo DUMP-MEM!\n\n");
	printf("\033[0m");
	sem_post(&sem_print);
}

//--------------------------------------------------------------
//	GP
//--------------------------------------------------------------

struct ProcessControlBlock {
	int id;									//id do processo
	int base;								//endereco base do prog
	int limite;								//limite do prog
	int regs[NUM_REGISTER];					//contexto do prog
	int partition;							//particao alocada pelo prog
	int st;									//status do pc
	int io_add;								//endereco de I/O
	int io_cont;							//conteudo de I/O
	int io_st;								//status do I/O
	int	pg_n;								//numero de paginas
	int pg[16];								//vetor de paginas do prog - limite de 16 paginas por programa
	
	struct ProcessControlBlock* next;		//ponteiro pra proximo PCB
};

typedef struct ProcessControlBlock PCB;

//ptr PCB rodando
PCB* running = NULL;

//fila ready
PCB* head_ready = NULL;
PCB* tail_ready = NULL;

//fila blocked
PCB* head_blocked = NULL;
PCB* tail_blocked = NULL;

int ret_id() {
	return id_base++;
}

void append_ready(struct PCB* pcb) {
	sem_wait(&mutex_ready);
	if(head_ready == NULL) {
		head_ready = pcb;
		tail_ready = pcb;	
	} else {
		tail_ready->next = pcb;
		tail_ready = pcb;
	}
	sem_post(&mutex_ready);
}

void append_blocked(struct PCB* pcb) {
	sem_wait(&mutex_blocked);
	if(head_blocked == NULL) {
		head_blocked = pcb;
		tail_blocked = pcb;	
	} else {
		tail_blocked->next = pcb;
		tail_blocked = pcb;
	}
	sem_post(&mutex_blocked);
}

struct PCB* pop_ready() {
	sem_wait(&mutex_ready);
	PCB* temp = head_ready;
	if(head_ready->next != NULL) {
		head_ready = head_ready->next;		
	} else {
		head_ready = NULL;
		tail_ready = NULL;
	}
	sem_post(&mutex_ready);
	temp->next = NULL;
	return temp;
}

struct PCB* pop_blocked() {
	sem_wait(&mutex_blocked);
	PCB* temp = head_blocked;
	if(head_blocked->next != NULL) {
		head_blocked = head_blocked->next;		
	} else {
		head_blocked = NULL;
		tail_blocked = NULL;
	}
	sem_post(&mutex_blocked);
	temp->next = NULL;
	return temp;
}

void scheduler() {
	running = pop_ready();
	PC = running->st;
}

int cria_process(int n) {
	int *p = aloca_paginas(n);
	if(p == NULL) {  
		printf("\nNao foi possivel alocar MEM\n");
		return -1;
	} else {
		PCB* pcb;
		pcb = malloc(sizeof(PCB));
		pcb->id = ret_id();
		pcb->partition = p;
		pcb->st = p[0]*16;
		pcb->io_add = 0;
		pcb->io_st = 0;
		pcb->io_cont = 0;
		pcb->pg_n = n;
		for(int i = 0;i < n;i++) {
			pcb->pg[i] = p[i];
		}
		free(p);
		for(int i = 0;i < NUM_REGISTER;i++) {
			pcb->regs[i] = 0;
		}
		pcb->next = NULL;

		int r = carga(&program, pcb->pg, n);
		if(r < 0) {return r;}

		append_ready(pcb);
	}
	return 0;
}

void finalizaProcesso(){
	sem_wait(&sem_cpu);
	desaloca_particao(running->pg, running->pg_n);
	free(running);
	running = NULL;
}

int test_interrupt() {
	if(interrupt == 1) {
		return 1;
	}
	return 0;
}

void tratamento_timer() {
	running->regs[0] = REGS[0];
	running->regs[1] = REGS[1];
	running->regs[2] = REGS[2];
	running->regs[3] = REGS[3];
	running->regs[4] = REGS[4];
	running->regs[5] = REGS[5];
	running->regs[6] = REGS[6];
	running->regs[7] = REGS[7];
	running->st = PC;
	append_ready(running);
	running = NULL;
}

void tratamento_io() {
	running->regs[0] = REGS[0];
	running->regs[1] = REGS[1];
	running->regs[2] = REGS[2];
	running->regs[3] = REGS[3];
	running->regs[4] = REGS[4];
	running->regs[5] = REGS[5];
	running->regs[6] = REGS[6];
	running->regs[7] = REGS[7];
	running->st = PC;
	append_blocked(running);
	running = NULL;
	io = 1;
}

int tratamento_interrupt() {	
	switch(interrupt) {
		//TIMER
		case 1:
			print_int_timer();
			tratamento_timer();
			break;
		//I/O READ 
		case 2:
			print_int_trap_read();
			tratamento_io();
			break;
		//I/O WRITE
		case 3:
			print_int_trap_write();	
			tratamento_io();
			break;
		default:
			printf("\n**ERROR** --> DEFAULT\n");
			break;
	}
}

void* gp() {
	print_gp();
	while(1) {
		sleep(1);
		if(interrupt != 0) {
			sem_wait(&sem_cpu);
			tratamento_interrupt();
			interrupt = 0;
		} else if(head_ready != NULL && running == NULL) {
			print_scheduler();
			scheduler();
			sem_post(&sem_cpu);
			sem_post(&sem_timer);
		} else if(head_blocked != NULL) {						
			//read			
			if(head_blocked->io_st == 1) {
				head_blocked->io_st = 0;
				memory_array[head_blocked->io_add] = head_blocked->io_cont;
				head_blocked->io_add = 0;
				head_blocked->io_cont = 0;
				append_ready(pop_blocked()); 
			//write
			} else if(head_blocked->io_st == 2) {
				head_blocked->io_st = 0;
				head_blocked->io_add = 0;
				head_blocked->io_cont = 0;
				append_ready(pop_blocked());
			}
		}
	}
}

//--------------------------------------------------------------
//	TIMER
//--------------------------------------------------------------

void* timer() {
	print_timer();
	while(1) {
		sem_wait(&sem_timer);
		sleep(8);
		if(interrupt == 0 && head_ready != NULL) {		
			interrupt = 1;
		}
	}
}

//--------------------------------------------------------------
//	GM
//--------------------------------------------------------------

int pages[NUM_PAGE];
int tam_prog = 0;
int fim_prog = 0;

void log_memoria_carga() {

	FILE *file;
   	int i;
   	file = fopen ("log_carga.txt","w");
 	print_carga();
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"%x\n", memory_array[i]);
	}
  
   	fclose(file);
}

int* aloca_paginas(int n) {
	sem_wait(&mutex_carga);
	int *pg = (int*)calloc(n, sizeof(int));
	int j = 0;
	for(int i = 0;i<NUM_PAGE;i++) {
		if(n != 0) {
			if(pages[i] == 0) {
				pages[i] = 1;
				n--;
				pg[j] = i;
				j++;
			}
		} else {break;}	
	}
	sem_post(&mutex_carga);
	/*
	 *	Caso nao tenha todas as N paginas disponiveis - desaloca as paginas alocadas e retorna erro
	 */
	if(n > 0) {
		for(int i = 0; i<j;i++) {
			pages[pg[i]] = 0;			
		}
		return NULL;
	}
	return pg;
}

void area_prog(int data) {
	if(data == 0xF8000000) {
		fim_prog = 1;
	}
}

void clear_prog(int* pr) {
	for(int i = 0; i<PROGRAM_LIMIT_SIZE; i++) {
		pr[i] = 0;
	}
}

int carga(int* pr, int* p, int n) {
	int i,j;
	for(i = 0; i<n;i++) {
		int base_f = p[i] * 16;
		int base_p = i * 16;
		for(j=0; j<16;j++) {
			memory_array[j + base_f] = pr[j + base_p];
		}

	}
	clear_prog(pr);
	log_memoria_carga();
	return 0;
}

int le_programa(char* pFile) {
	FILE *file;
	int i = 0;
	int aux;
	file = fopen (pFile,"r");

	if (file == NULL) {
        	printf("\nNao foi possivel abrir o arquivo: %s\n", pFile);
        	return -1;
    }
	while(1) {
		
		fscanf(file, "%x", &aux);
		
		if(aux == 0xF1F1F1F1) {
			break;
		}
		program[i] = aux;
		i++;
	}

	fclose(file);
	return (int)ceil(i/16.0); 
}

void desaloca_particao(int *p, int n) {
	sem_wait(&mutex_carga);
	for(int i = 0; i < n; i++) {
		pages[p[i]] = 0;
	}
	sem_post(&mutex_carga);
}

void print_mem() {
	printf("MEM\n");
	for(int i = 0;i<1024;i++) {
		printf("%d---%x\n", i, memory_array[i]);
	}
}
//--------------------------------------------------------------
//	CPU
//--------------------------------------------------------------

int fetch() {
	return memory_array[PC++];
}

void decoder(int itr) {
	//int decode[5];
	decode[0] = (itr & 0xF8000000) >> 27;	//OPCODE
	decode[1] = (itr & 0x07000000) >> 24;	//PARAMETRO 0
	decode[2] = (itr & 0x00E00000) >> 21;	//PARAMETRO 1
	decode[3] = (itr & 0x001F0000) >> 16;	//PARAMETRO 3 
	decode[4] = (itr & 0x0000FFFF);		//IMEDIATO
}

void exec() {
	int* wire = &decode;
	int offset, pagina;
	switch(wire[0]) {
		//TRAP
		case 0x5:
			pagina = (int)ceil(REGS[wire[2]]/16.0) -1;
			offset = REGS[wire[2]] % 16;
			interrupt = REGS[wire[1]];
			running->io_add = running->pg[pagina] * 16 + offset;
			running->io_st = 0;
			running->io_cont = memory_array[REGS[wire[2]]];
			break;	
		//STOP
		case 0x1F:
			print_break();
			finalizaProcesso();
			break;
		//SWAP
		case 0x1B: ;

			int aux_0 = REGS[wire[1]] & 0x0000000F << 4;	//b(0-3) com shift p/ esq
			int aux_1 = REGS[wire[1]] & 0x000000F0 >> 4;	//b(4-7) com shift p/ dir

			int aux_2 = REGS[wire[1]] & 0x00000F00 << 4;	//b(8-11) com shift p/ esq
			int aux_3 = REGS[wire[1]] & 0x0000F000 >> 4;	//b(12-15) com shift p/ dir

			int aux_4 = REGS[wire[1]] & 0x000F0000 << 4;	//b(16-19) com shift p/ esq
			int aux_5 = REGS[wire[1]] & 0x00F00000 >> 4;	//b(20-23) com shift p/ dir

			int aux_6 = REGS[wire[1]] & 0x0F000000 << 4;	//b(24-27) com shift p/ esq
			int aux_7 = REGS[wire[1]] & 0xF0000000 >> 4;	//b(28-31) com shift p/ dir
			
			REGS[wire[1]] = aux_0 + aux_1 + aux_2 + aux_3 + aux_4 + aux_5 + aux_6 + aux_7;	//soma dos bytes com swap para gerar valor abs
			break;
		//SHR
		case 0x1A:
			REGS[wire[1]] = REGS[wire[1]] >> 1;	//rd(n) = rd(n+1)
			break;
		//SHL
		case 0x19:
			REGS[wire[1]] = REGS[wire[1]] << 1;	//rd(n+1) = rd(n)
			break;
		//NOT
		case 0x18:
			REGS[wire[1]] = REGS[wire[1]] ^ 0xFFFFFFFF;	//rd = NOT rd (feito com XOR)
			break;
		//STX
		case 0x15:
			memory_array[REGS[wire[1]]] = REGS[wire[2]];	//[rd] = rs
			break;
		//LDX
		case 0x14:
			REGS[wire[1]] = memory_array[REGS[wire[2]]];	//rd = [rs]
			break;
		//OR
		case 0x13:
			REGS[wire[1]] = REGS[wire[1]] | REGS[wire[2]];	//rd = rd OR rs
			break;
		//AND
		case 0x12:
			REGS[wire[1]] = REGS[wire[1]] & REGS[wire[2]];	//rd = rd AND rs
			break;
		//MULT
		case 0x1C:
			REGS[wire[1]] = REGS[wire[1]] * REGS[wire[2]];	//rd = rd * rs	
			break;
		//SUB
		case 0x11:
			REGS[wire[1]] = REGS[wire[1]] - REGS[wire[2]];	//rd = rd - rs
			break;
		//ADD
		case 0x10:
			REGS[wire[1]] = REGS[wire[1]] + REGS[wire[2]];	//rd = rd + rs
			break;
		//STD
		case 0xE:
			memory_array[wire[4]] = REGS[wire[1]];		//[A] = rs
			break;
		//LDD
		case 0xD:
			REGS[wire[1]] = memory_array[wire[4]];		//rd = [A]
			break;
		//LDI
		case 0xC:
			REGS[wire[1]] = wire[4];					//rd = k
			break;
		//ORI
		case 0xB:
			REGS[wire[1]] = REGS[wire[1]] | wire[4];	//rd = rd OR k
			break;
		//ANDI
		case 0xA:
			REGS[wire[1]] = REGS[wire[1]] & wire[4];	//rd = rd AND k
			break;
		//SUBI
		case 0x9:
			REGS[wire[1]] = REGS[wire[1]] - wire[4];	//rd = rd - k
			break;
		//ADDI
		case 0x8:
			REGS[wire[1]] = REGS[wire[1]] + wire[4];	//rd = rd + k		
			break;
		//JMPIE
		case 0x4:
			if(REGS[wire[2]] == 0) {
				pagina = (int)ceil(REGS[wire[1]]/16.0) -1;	
				offset = REGS[wire[1]] % 16;
				PC = running->pg[pagina] * 16 + offset;						//pc = rs
			}																//por default pc++
			break;
		//JMPIL
		case 0x3:
			if(REGS[wire[2]] < 0) {
				pagina = (int)ceil(REGS[wire[1]]/16.0) -1;	
				offset = REGS[wire[1]] % 16;
				PC = running->pg[pagina] * 16 + offset;						//pc = rs
			}																//por default pc++
			break;
		//JMPIG
		case 0x2:
			if(REGS[wire[2]] > 0) {
				pagina = (int)ceil(REGS[wire[1]]/16.0) -1;
				offset = REGS[wire[1]] % 16;
				PC = running->pg[pagina] * 16 + offset;						//pc = rs
			} 																//por default pc++
			break;
		//JMPI 
		case 0x1:						
			pagina = (int)ceil(REGS[wire[1]]/16.0) -1;						//pc = rs
			offset = REGS[wire[1]] % 16;
			PC = running->pg[pagina] * 16 + offset;	
			break;
		//JMP
		case 0x0:
			pagina = (int)ceil(wire[4]/16.0) -1;
			offset = wire[4] % 16;
			PC = running->pg[pagina] * 16 + offset;							//pc = k
			break;
		//valor default - printa erro na tela
		default:
			printf("\n**ERROR** --> DECODER\n");
			break;
	}
}

void* cpu() {
	int itr = 0;
	int* dc;
	print_cpu();
	while(1) {
		sleep(2);
		sem_wait(&sem_cpu);
		sem_post(&sem_cpu);
		if(interrupt == 0) {
			sem_wait(&sem_print);
			printf("\033[0;32m");
			printf("\t*");
			printf("\033[0m");
			printf("  RODANDO: %d --> pc: %4d -- int: %d\n", running->id, PC, interrupt);
			sem_post(&sem_print);
			itr = fetch();
			decoder(itr);
			exec();
		}
	}
}

//--------------------------------------------------------------
//	SHELL
//--------------------------------------------------------------

void send(char *s, char *msg) {
	while(*msg != '\0') {
        *s++ = *msg++;
	}
    *s = '\0';
}

void convert(char *msg, int x) {
	sprintf(msg, "%d", x);
}

void protocolo(char *shm) {
	int temp = io;
	io = 0;
	char msg[20];
	
	print_io();
	msg[0] = 'i';
	msg[1] = 'o';
	msg[2] = '\0';
	send(shm, msg);
	while(shm[0] != '*') {
		sleep(1);
	}
	convert(msg, head_blocked->id);
	send(shm, msg);
	while(shm[0] != '*') {
		sleep(1);
	}

	//read
	if(temp == 1) {
		msg[0] = 'r';
		send(shm, msg);
		while(shm[0] != '*') {
			sleep(1);
		}
		while(shm[0] == '*') {
			sleep(1);
		}
		head_blocked->io_cont = atoi(shm);
		head_blocked->io_st = 1;
		msg[0] = '*';
		send(shm, msg);
	//write
	} else if(temp == 2) {
		msg[0] = 'w';
		send(shm, msg);
		while(shm[0] != '*') {
			sleep(1);
		}
		convert(msg, head_blocked->io_cont);
		send(shm, msg);
		while(shm[0] != '*') {
			sleep(1);
		}

		head_blocked->io_st = 2;
	}
}

void* shell(int argc, char* argv[], char* envp[]) {

	int j, pid, erro_lp, erro_cp;
	char c;
    int shmid;
    key_t key;
    char *shm, *buffer_cmd;
	char temp[20];
	
    /*
     * Vamos chamar nosso segmento de memoria compartilhada de 
     * "5678".
     */
    key = 5678;

    /*
     * cria o segmento.
     */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * associa este segmento ao nosso espaço de dados - 'registra no PCB'.
     */
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    /*
     * Coloca valores na memoria para outro processo ler.
     */
	*shm = '*';
    buffer_cmd = shm;
	print_shell();
	while(1) {
		
		while(buffer_cmd[0] == '*') {
			sleep(1);
			if(io != 0) {
				protocolo(shm);	
			}
		}
	
		if(strstr(buffer_cmd, "exit") != NULL) {
			print_exit();
			kill(getpid(), 15);			
		} else if(strstr(buffer_cmd, ".txt") != NULL) {
			erro_lp = le_programa(buffer_cmd);
			if(erro_lp == -1) {
				printf("\n**ERRO - LEITURA DO PROGRAMA**\n");					
			} else {
				erro_cp = cria_process(erro_lp);
			}

			if(erro_cp == -1) {
				printf("\n**ERRO - CRIACAO DO PROCESSO**\n");	
			}

		} else {
       		j = execve (buffer_cmd, argv, NULL);
		}
		*shm = '*';
	}
  
}

//--------------------------------------------------------------
//	CONSOLE
//--------------------------------------------------------------

void* console() {
	int e;
	e = system("gnome-terminal sh -- './console'");
	if(e != -1) {
		print_console();
	}
}

//--------------------------------------------------------------
//	MAIN
//--------------------------------------------------------------

void random_id() {
	id_base = rand();
}

int main() {
	int arg = 0;
	void *ret;
    
    random_id();
	sem_init(&mutex_ready, 0, 1);
	sem_init(&mutex_blocked, 0, 1);
	sem_init(&sem_cpu, 0, 0);
	sem_init(&sem_trap, 0, 1);
	sem_init(&sem_timer, 0, 0);
	sem_init(&sem_print, 0, 1);
	sem_init(&mutex_carga, 0, 1);

	pthread_create(&pid_cpu, NULL, cpu, (void*)arg);
	pthread_create(&pid_gp, NULL, gp, (void*)arg);
	pthread_create(&pid_timer, NULL, timer, (void*)arg);
	pthread_create(&pid_shell, NULL, shell, (void*)arg);
	pthread_create(&pid_console, NULL, console, (void*)arg);

	pthread_join(pid_shell, &ret);
	pthread_join(pid_cpu, &ret);
	pthread_join(pid_timer, &ret);
	pthread_join(pid_console, &ret);
	
	return 0;		
}
