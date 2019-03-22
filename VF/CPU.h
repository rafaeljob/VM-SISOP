#define MEMORY_SIZE 1024 //1024
#define NUM_REGISTER 8

//declarcao das funcoes
void write_empty_memory();
void read_memory();
int fetch();
int* decode_instruction(int itr);
void exec();
void write_out_memory();
int cpu();
