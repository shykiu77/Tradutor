#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
int instructions = 0;
uint8_t GetLeftRegister(uint8_t byte){
    return byte >> 4;
}
uint8_t GetRightRegister(uint8_t byte){
    return byte & 15;
}
int16_t Sum2(uint16_t byte1, uint16_t byte2){
    int16_t result=0;
    result = result | byte2;
    result = result << 8;
    result = result | byte1;
    return result;
}
int32_t Sum4(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4){
    int32_t result = 0;
    result = result | byte4;
    result = result << 8;
    result = result | byte3;
    result = result << 8;
    result = result | byte2;
    result = result << 8;
    result = result | byte1;
    return result;
}

int8_t *Decompose4(int32_t value){
    uint8_t *result = (uint8_t*)malloc(sizeof(uint8_t)*4);
    result[0] = (uint8_t) (value & 255);
    result[1] = (uint8_t) ((value & 65280) >> 8);
    result[2] = (uint8_t) ((value & 16711680) >> 16);
    result[3] = (uint8_t) ((value & 4278190080) >> 24);
    return result;
}

uint8_t *process(uint8_t *memory,int16_t *pc,uint32_t *registers,uint8_t *a,uint8_t *b, uint8_t *e){
    instructions++;
    switch(memory[*pc]){
        case 0x00:
            registers[GetLeftRegister(memory[*pc+1])] = Sum2(memory[*pc+2],memory[*pc+3]);
            *pc += 4;
            break;
        case 0x01:
            registers[GetLeftRegister(memory[*pc+1])] = registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x02:
            registers[GetLeftRegister(memory[*pc+1])] = Sum4(memory[registers[GetRightRegister(memory[*pc+1])]],memory[registers[GetRightRegister(memory[*pc+1])]+1],memory[registers[GetRightRegister(memory[*pc+1])]+2],memory[registers[GetRightRegister(memory[*pc+1])]+3]);
            *pc += 4;
            break;
        case 0x03:
            memory[registers[GetLeftRegister(memory[*pc+1])]] = Decompose4(registers[GetRightRegister(memory[*pc+1])])[0];
            memory[registers[GetLeftRegister(memory[*pc+1])]+1] = Decompose4(registers[GetRightRegister(memory[*pc+1])])[1];
            memory[registers[GetLeftRegister(memory[*pc+1])]+2] = Decompose4(registers[GetRightRegister(memory[*pc+1])])[2];
            memory[registers[GetLeftRegister(memory[*pc+1])]+3] = Decompose4(registers[GetRightRegister(memory[*pc+1])])[3];
            *pc += 4;
            break;
        case 0x04:
            *a = registers[GetLeftRegister(memory[*pc+1])] > registers[GetRightRegister(memory[*pc+1])];
            *b = registers[GetLeftRegister(memory[*pc+1])] < registers[GetRightRegister(memory[*pc+1])];
            *e = registers[GetLeftRegister(memory[*pc+1])] == registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x05:
            *pc += (uint16_t) (4) + Sum2(memory[*pc+2],memory[*pc+3]);
            break;
        case 0x06:
            if(*a)
                *pc = 4 + Sum2(memory[*pc+2],memory[*pc+3]);
            break;
        case 0x07:
            if(*b)  
                *pc = 4 + Sum2(memory[*pc+2],memory[*pc+3]);
            break;
        case 0x08:
            if(*e)
                *pc = 4 + Sum2(memory[*pc+2],memory[*pc+3]);
            break;
        case 0x09:
            registers[GetLeftRegister(memory[*pc+1])] += registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x0A:
            registers[GetLeftRegister(memory[*pc+1])] -= registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x0B:
            registers[GetLeftRegister(memory[*pc+1])] &= registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x0C:
            registers[GetLeftRegister(memory[*pc+1])] |= registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x0D:  
            registers[GetLeftRegister(memory[*pc+1])] ^= registers[GetRightRegister(memory[*pc+1])];
            *pc += 4;
            break;
        case 0x0E:
            registers[GetLeftRegister(memory[*pc+1])] <<= memory[*pc+3];
            *pc += 4;
            break;
        case 0x0F:
            registers[GetLeftRegister(memory[*pc+1])] >>= memory[*pc+3];
            *pc += 4;
            break;
    }

    return memory;
}

void Execute(uint8_t *memory,uint32_t *registers){
    uint16_t pc = 0;
    uint8_t a=0,b=0,e=0;
    while(pc < 128){
        memory = process(memory,&pc,registers,&a,&b,&e);
    }
}

void Printsummary(uint8_t *memory,uint32_t *registers,FILE *stream){
    for(uint32_t i=0; i<128; i+= 4){
        fprintf(stream,"%02hhx %02hhx %02hhx %02hhx\n",memory[i],memory[i+1],memory[i+2],memory[i+3]);
    }
    for(uint32_t i=0; i<16;i++)
        fprintf(stream,"R[%d]=0x%08x\n",i,registers[i]);
    fprintf(stream,"#INSTRUCTIONS=%d\n",instructions);
}

uint32_t main(uint32_t argc, char** argv){
    FILE *input = fopen(argv[1],"r");
    FILE *output = fopen(argv[2],"w");
    uint8_t *memory = (uint8_t*)malloc(sizeof(uint8_t)*128);
    uint32_t *registers = (uint32_t*)malloc(sizeof(uint32_t)*16);
    int usedMem = 0;
    while(fscanf(input,"%hhx",&memory[usedMem]) != EOF){
        usedMem++;
    }
    Execute(memory,registers);
    Printsummary(memory,registers,output);
    fclose(input);
    fclose(output);
}