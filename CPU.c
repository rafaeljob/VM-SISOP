#include <stdio.h>

#define MEMORY_SIZE 1024
#define NUM_REGISTER 8

int memory_array[MEMORY_SIZE];
int REGS[NUM_REGISTER];
int PC = 0;

void write_empty_memory();
void read_memory();
int fetch_instruction();
int decode_instruction(int itr);

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

int fetch_instruction() {
	return memory_array[PC++];
}

int decode_instruction(int itr) {
	int op = (itr & 0xF8000000) >> 27;
	printf("op-%x-%d\n", op, op);
	return op;
}

int main() {
	int itr = 0;
	int dc = 0;
	//write_empty_memory();
	read_memory();
	itr = fetch_instruction();
	dc = decode_instruction(itr);
	printf("fetched instruction: %x\n", itr);
	printf("decoded instruction: %d\n", dc);
	return 0;
}
