#include <stdio.h>

#define MEMORY_SIZE 1024 //1024
#define NUM_REGISTER 8

//declaracao variaveis globais
int memory_array[MEMORY_SIZE];
int REGS[NUM_REGISTER];
int PC = 0;
int run = 1;

//declarcao das funcoes
void write_empty_memory();
void read_memory();
int fetch();
int* decode_instruction(int itr);
void exec(int* wire);


void write_empty_memory() {

	FILE *file;
   	int i;
   	file = fopen ("memory.txt","w");
 	
	printf("WRITING EMPTY MEMORY FILE!\n");
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"0x00000000\n");
	}
  
   	fclose(file);
}

void read_memory() {
	FILE *file;
	int i = 0;
	file = fopen ("memory.txt","r");
	
	printf("READING MEMORY FILE!\n");
	for(i=0;i<MEMORY_SIZE;i++) {
		fscanf(file, "%x", &memory_array[i]);
		//printf("%d -- number: %d -- %x\n",i, memory_array[i], memory_array[i]);
	}

	fclose(file);
}

int fetch() {
	printf("LIDO:%d---%x\n", PC, memory_array[PC]);
	return memory_array[PC++];
}

int* decoder(int itr) {
	int decode[5];
	decode[0] = (itr & 0xF8000000) >> 27;	//OPCODE
	decode[1] = (itr & 0x07000000) >> 24;	//PARAMETRO 0
	decode[2] = (itr & 0x00E00000) >> 21;	//PARAMETRO 1
	decode[3] = (itr & 0x001F0000) >> 16;	//PARAMETRO 3 
	decode[4] = (itr & 0x0000FFFF);			//IMEDIATO
	printf("OP-%x-%d\n", decode[0], decode[0]);
	printf("PR_1-%x-%d\n", decode[1], decode[1]);
	printf("PR_2-%x-%d\n", decode[2], decode[2]);
	printf("PR_3-%x-%d\n", decode[3], decode[3]);
	printf("IMM-%x-%d\n", decode[4], decode[4]);
	return decode;
}

void exec(int* wire) {
	printf("itr=%x\n", wire[0]);
	switch(wire[0]) {
		//STOP
		case 0x1F:
			run = 0;
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
			memory_array[wire[1]] = REGS[wire[2]];	//[rd] = rs
			break;
		//LDX
		case 0x14:
			REGS[wire[1]] = memory_array[wire[2]];	//rd = [rs]
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
			REGS[wire[1]] = REGS[wire[1]] - REGS[wire[2]];	//rd = rd + rs
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
			printf("***ERROR***\n");
			break;
	}
}

int main() {
	int itr = 0;
	int* dc;
	int i;

	//write_empty_memory();
	read_memory();
		
	while(run) {
		printf("PC:%d\n", PC);
		itr = fetch();
		printf("fetched instruction: %x\n", itr);
		dc = decoder(itr);
		for(i=0;i<5;i++) {
			printf("%d-%d-%d-%d-%d\n", dc[0], dc[1], dc[2], dc[3], dc[4]);
		}
		printf("decoded instruction: %d\n", dc);
		exec(dc);
		printf("REGS:\n0:%x\n1:%x\n2:%x\n3:%x\n4:%x\n5:%x\n6:%x\n7:%x\n", REGS[0], REGS[1], REGS[2], REGS[3], REGS[4], REGS[5], REGS[6], REGS[7]);
	}
	return 0;
}
