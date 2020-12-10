#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

uint8_t GetLeftRegister(uint8_t byte){
    return byte >> 4;
}
uint8_t GetRightRegister(uint8_t byte){
    return byte & 15;
}

void fill(uint8_t *code,uint32_t nops,uint32_t *code_pointer){
  for(uint32_t i=0;i < nops; i++)
    code[(*code_pointer)++] = 0x90;
}

uint8_t *Generate_code(uint8_t *memory){
  uint8_t *code =(uint8_t*)malloc(sizeof(code)*512);
  uint32_t code_pointer = 0;
  for(uint32_t i=0;i<128;i+=4){
    switch(memory[i]){
      case 0x00:
        code[code_pointer++] = 0xb9;
        code[code_pointer++] = memory[i+2];
        code[code_pointer++] = memory[i+3];
        code[code_pointer++] = 0x00;
        code[code_pointer++] = 0x00;
        uint8_t reg = GetLeftRegister(memory[i+1]);
        if(reg == 0 ){
          code[code_pointer++] = 0x89;
          code[code_pointer++] = 0x0e;
          fill(code,9,&code_pointer);
        }
        else{
          code[code_pointer++] = 0x89;
          code[code_pointer++] = 0x4e;
          code[code_pointer++] = -reg;
          fill(code,8,&code_pointer);
        }
        break;
    }
  }
  code[code_pointer++] = 0xC3;
  return code;
}

int main(int argc, char** argv){
  FILE *input = fopen(argv[1],"r");
  FILE *output = fopen(argv[2],"w");
  uint8_t *memory = (uint8_t*)malloc(sizeof(uint32_t)*128);
  int readBytes = 0;
  while(fscanf(input,"%hhx",&memory[readBytes]) != EOF){
    readBytes++;
  }
  uint8_t *code_temp = Generate_code(memory);
  uint8_t code[512];
  for(uint32_t i=0;i<512;i++)
    code[i] = code_temp[i];
  uint32_t lenght = 512;
  void *executable_memory = mmap(0,lenght,PROT_NONE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
  mprotect(executable_memory, lenght, PROT_WRITE);
  memcpy(executable_memory, (void*)(code),sizeof(code));
  mprotect(executable_memory,lenght,PROT_EXEC);
  const uint64_t (*jit) ( uint8_t*,  uint32_t*,  uint32_t*) = ( uint64_t(*) ( uint8_t*, uint32_t*, uint32_t*))(executable_memory);
  
  uint32_t *registers = (uint32_t *) malloc(sizeof(uint32_t)*16);
  uint32_t *histogram = (uint32_t *) malloc(sizeof(uint32_t)*16);
  (*jit)(memory,registers,histogram);
  munmap(executable_memory,lenght);
  return 0;
}   