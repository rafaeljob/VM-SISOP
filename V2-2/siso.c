/*	NOME: RAFAEL BASSO
//	gcc -lpthread siso.c -o s
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

//#include "CMP.h"
//#include "CPU.h"
//#include "shell.h"

#define MEMORY_SIZE 1024
#define NUM_REGISTER 8
#define PARTITION_SIZE 128

//--------------------------------------------------------------
//	VAR. GLOB
//--------------------------------------------------------------
int interrupt = 0;
int memory_array[MEMORY_SIZE];
int REGS[NUM_REGISTER];
int decode[5];
int PC = 0;
int runcpu = 1;
int baseID;




//--------------------------------------------------------------
//	TIMER
//--------------------------------------------------------------

void* timer() {
	while(1) {
		sleep(50);
		interrupt = 1;
	}
}

//--------------------------------------------------------------
//	GP
//--------------------------------------------------------------
struct PCB;

struct PCB {
	int id;
	int base;
	int limite;
	int regs[NUM_REGISTER];
	struct PCB* next;
};

struct PCB* running = NULL;
struct PCB* ready = NULL;

int ret_id() {
	return baseID++;
}
void append(struct PCB* pcb, struct PCB* fready) {
	if(fready == NULL) {
		fready = pcb;
	} else if(fready->next == NULL){
		fready->next = pcb;	
	} else {
		append(pcb, fready->next);
	}
}

int cria_process(/*programa*/) {
	//p = aloca_particao();
	printf("\n**ERROR** --> particao0\n"); 
	int p = 0;
	if(p == -1) { 
		printf("\n**ERROR** --> particao1\n"); 
		return p;
	} else {
		struct PCB int_pcb;
		int_pcb.id = ret_id();
		int_pcb.base = p * PARTITION_SIZE;
		int_pcb.base = ((p+1) * PARTITION_SIZE) - 1;
		for(int i = 0;i < NUM_REGISTER;i++) {
			int_pcb.regs[i] = 0;
		}
		int_pcb.next = NULL;

		//r = carga(programa, p) 
		//if(r < 0) {	return r;}
		append(&int_pcb, &ready);
	}
}

void finalizaProcesso(pcb){
	
}

//--------------------------------------------------------------
//	GM
//--------------------------------------------------------------

//--------------------------------------------------------------
//	CPU
//--------------------------------------------------------------

int test_interrupt() {
	if(interrupt == 1) {
		return 1;
	}
	return 0;
}

int tratamento_interrupt() {
	switch(interrupt) {
		case 1:

		case 2:
			printf("\n**ERROR** --> INTERRUPT 2\n");
			break;
		case 3:	
			printf("\n**ERROR** --> INTERRUPT 3\n");
			break;
		default:
			printf("\n**ERROR** --> DEFAULT\n");
			break;
	}
	interrupt = 0;
	//acessa fila do gp
}

void write_empty_memory() {

	FILE *file;
   	int i;
   	file = fopen ("memory.txt","w");
 	
	//printf("WRITING EMPTY MEMORY FILE!\n");
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"0x00000000\n");
	}
  
   	fclose(file);
}

void write_out_memory() {

	FILE *file;
   	int i;
   	file = fopen ("memory_out.txt","w");
 	
	//printf("WRITING MEMORY FILE!\n");
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"%x\n", memory_array[i]);
	}
  
   	fclose(file);
}

void read_memory() {
	FILE *file;
	int i = 0;
	file = fopen ("memory.txt","r");
	
	//printf("READING MEMORY FILE!\n");
	for(i=0;i<MEMORY_SIZE;i++) {
		fscanf(file, "%x", &memory_array[i]);
	}

	fclose(file);
}

int fetch() {
	return memory_array[PC++];
}

void decoder(int itr) {
	//int decode[5];
	decode[0] = (itr & 0xF8000000) >> 27;	//OPCODE
	decode[1] = (itr & 0x07000000) >> 24;	//PARAMETRO 0
	decode[2] = (itr & 0x00E00000) >> 21;	//PARAMETRO 1
	decode[3] = (itr & 0x001F0000) >> 16;	//PARAMETRO 3 
	decode[4] = (itr & 0x0000FFFF);			//IMEDIATO
}

void exec() {
	int* wire = &decode;
	switch(wire[0]) {
		//STOP
		case 0x1F:
			runcpu = 0;
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
				PC = REGS[wire[1]];						//pc = rs
			}											//por default pc++
			break;
		//JMPIL
		case 0x3:
			if(REGS[wire[2]] < 0) {
				PC = REGS[wire[1]];						//pc = rs
			}											//por default pc++
			break;
		//JMPIG
		case 0x2:
			if(REGS[wire[2]] > 0) {
				PC = REGS[wire[1]];						//pc = rs
			} 											//por default pc++
			break;
		//JMPI 
		case 0x1:
			PC = REGS[wire[1]];							//pc = rs
			break;
		//JMP
		case 0x0:
			PC = wire[4];								//pc = k
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

	//write_empty_memory();
	read_memory();
		
	while(runcpu) {
		if(test_interrupt() == 0) {
			itr = fetch();
			decoder(itr);
			exec();
		} else {
			tratamento_interrupt();
		}
	}
	write_out_memory();
}

//--------------------------------------------------------------
//	SHELL
//--------------------------------------------------------------

void print() {
	printf("\033[1;13m");
	printf("\nuser@SISOP");
	printf("\033[0m");
	printf(":");
	printf("\033[1;34m");
	printf("~");
	printf("\033[0m");
	printf("$ ");	
}

void* shell(int argc, char* argv[], char* envp[]) {

	char cmd[20] ;
	int j, pid;

	while(1) {
		print();
    		scanf("%s", cmd);
    		pid = fork();
    	if(pid != 0) {
       		sleep(1) ;
       		pid = wait(0) ;   
 		} else {
			cria_process();
			if(strstr(cmd, ".txt") != NULL) {
				//cmp(&cmd);
				//cpu();
			} else {
       			j = execve (cmd, argv, NULL);
			}
       	}
    }  
}


//--------------------------------------------------------------
//	MAIN
//--------------------------------------------------------------
void random_id() {
	baseID = rand();
}

int main() {
	pthread_t pid_cpu, pid_shell, pid_timer;
	int arg = 0;
	void *ret;
	
	random_id();
	printf("\n**ERROR** --> main0\n"); 
	pthread_create(&pid_shell, NULL, shell, (void*)arg);
	//pthread_create(&pid_cpu, NULL, cpu, (void*)arg);
	pthread_create(&pid_timer, NULL, timer, (void*)arg);
	printf("\n**ERROR** --> main1\n"); 
	pthread_join(pid_shell, &ret);
	//pthread_join(pid_cpu, &ret);
	//pthread_join(pid_timer, &ret);
}
