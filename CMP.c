#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE 1024

int memory_array[MEMORY_SIZE];

void write_empty_memory();
void read_program(char* pFile);
int convert(char* line);
/*void write_empty_memory() {

	FILE *file;
   	int i;
   	file = fopen ("memory.txt","w");
 	
	printf("WRITING EMPTY MEMORY FILE!\n");
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"0x00000000\n");
	}
  
   	fclose(file);
}*/
int convert(char* line) {
	
}
void read_program(char* pFile) {
	FILE *file;
	file = fopen (pFile,"r");

	int run = 5;
	char line[6];
	printf("%s", pFile);
	printf("READING PROGRAM!\n");
	while(run) {
		fscanf(file, "%s", line);
		printf("LINHA: %s\n", line);

		if('$' == line[0]) {

		} else if(';' == line[strlen(line)-1]) {
			
		} else {
			
		}

		run--;
		
	}

	fclose(file);
}

int main() {
	char p[20];
	printf("qual prog a compilar?\n");
	scanf("%s", &p);
	printf("%s", p);
	read_program(p);
	return 0;
}
