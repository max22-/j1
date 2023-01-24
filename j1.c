#include <stdio.h>
#include <stdlib.h>

#define DEBUG(...) \
    do { \
        fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)

typedef struct {
    unsigned char *mem;
    unsigned short *dst, *rst;
    unsigned char dptr, rptr;
    unsigned short T, N, R, PC; /* PC is 13 bit*/
} j1_vm;

enum j1_err {
    J1_OK,
    J1_STACK_OVERFLOW,
    J1_STACK_UNDERFLOW,
    J1_RETURN_STACK_OVERFLOW,
    J1_RETURN_STACK_UNDERFLOW
};

j1_vm j1_new(unsigned char *mem, unsigned short *dst, unsigned short *rst)
{
    j1_vm j1;
    j1.mem = mem;
    j1.dst = dst;
    j1.rst = rst;
    j1.dptr = 0;
    j1.rptr = 0;
    j1.PC = 0;
    return j1;
}

#define PUSH() \
    do { \
        if(j1->dptr == 32) return J1_STACK_OVERFLOW; \
        j1->dst[j1->dptr++] = x; \
    } while(0)

#define POP() \
    do { \
        if(j1->dptr == 0) return J1_STACK_UNDERFLOW; \
        x = j1->dst[--j1->dptr]; \
    } while(0) 

#define RPUSH() \
    do { \
        if(j1->rptr == 31) return J1_RETURN_STACK_OVERFLOW; \
        j1->rst[j1->rptr++] = x; \
    } while(0)

#define RPOP() \
    do { \
        if(j1->rptr == 0) return J1_RETURN_STACK_UNDERFLOW; \
        x = j1->rst[--j1->rptr]; \
    } while(0) 


int j1_run(j1_vm *j1, unsigned int cycles)
{
    unsigned short instr;
    unsigned short x; /* scratch "register" */
    while(cycles--) {
        instr = j1->mem[j1->PC] | j1->mem[j1->PC+1] << 8;
        j1->PC += 2;
        if(instr & 0x8000) {
           x = instr & 0x7fff;
           DEBUG("literal 0x%x\n", x);
           PUSH();
           continue;
        }
        switch(instr >> 13) {
            case 0x0: /* jump*/
                j1->PC = instr & 0x1fff;
                DEBUG("jumping to 0x%x\n", j1->PC);
                break;
            case 0x1: /* conditional jump */
                POP();
                DEBUG("conditional jump\n");
                if(!x)
                    j1->PC = instr & 0x1fff;
                break;
            case 0x2: /* call */
                x = j1->PC;
                RPUSH();
                j1->PC = instr & 0x1fff;
                DEBUG("calling 0x%x\n", j1->PC);
                break;
            case 0x3: /* ALU */
                DEBUG("ALU\n");
                break;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    FILE *f;
    unsigned char mem[16384];
    unsigned short dst[33], rst[32];
    j1_vm j1 = j1_new(mem, dst, rst);

    if(argc < 2) {
        fprintf(stderr, "usage: j1 file.bin");
        return 1;
    }
    f = fopen(argv[1], "r");
    if(!f) {
        perror(argv[1]);
        return 1;
    }
    fread(mem, sizeof(mem), 1, f);
    if(ferror(f)) {
        perror(argv[1]);
        fclose(f);
        return 1;
    }
    fclose(f);
    j1_run(&j1, 10);

}