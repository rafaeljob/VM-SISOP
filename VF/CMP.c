#include <stdio.h>
#include <string.h>
#include "CMP.h"

void write_memory() {

	FILE *file;
   	int i;
   	file = fopen ("memory.txt","w");
 	
   	for(i=0;i<MEMORY_SIZE;i++) {
		fprintf(file,"%x\n", memory_array[i]);
	}
  
   	fclose(file);
}

void read_program(char* pFile) {
	FILE *file;
	file = fopen (pFile,"r");
	int i;
	printf("%s", pFile);
	for(i=0;i<MEMORY_SIZE;i++) {
		fscanf(file, "%x", &memory_array[i]);
		//printf("memoria-%d: %x\n",i,memory_array[i]);
	}

	fclose(file);
}

int cmp(char* p[]) {
	//char p[20];
	//printf("qual prog a compilar?\n");
	//scanf("%s", &p);
	//printf("%s", p);
	read_program(p);
	write_memory();
	return 0;
}
