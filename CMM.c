
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define int int64_t

int MAX_SIZE;

int * code,         // code segment
    * code_dump,    // for dump
    * stack;        // stack segment
char* data;         // data segment

int * pc,           // pc register
    * sp,           // rsp register
    * bp;           // rbp register
int ax,             // common register
    cycle;

// 指令集声明
enum {IMM, LEA, JMP, JZ, JNZ, CALL, NVAR, DARG, RET, LI, LC, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT};

// main pointer
int * main_ptr;

int run_vm(int argc, char** argv) {
    int op;
    int* tmp;
    // exit code for main
    bp = sp = (int*)((int)stack + MAX_SIZE);
    *--sp = EXIT;
    *--sp = PUSH; tmp = sp;
    *--sp = argc; *--sp = (int)argv;
    *--sp = (int)tmp;
    cycle = 0;
    while (1) {
        cycle++; op = *pc++; // read instruction
        // 存取指令
        if (op == IMM)          ax = *pc++;                     // load immediate(or global addr)
        else if (op == LEA)     ax = (int)(bp + *pc++);         // load local addr
        else if (op == LC)      ax = *(char*)ax;                // load char
        else if (op == LI)      ax = *(int*)ax;                 // load int
        else if (op == SC)      *(char*)*sp++ = ax;             // save char to stack
        else if (op == SI)      *(int*)*sp++ = ax;              // save int to stack
        else if (op == PUSH)    *--sp = ax;                     // push ax to stack
        // 运算符，这里就不细讲了
        else if (op == OR)      ax = *sp++ |  ax;
        else if (op == XOR)     ax = *sp++ ^  ax;
        else if (op == AND)     ax = *sp++ &  ax;
        else if (op == EQ)      ax = *sp++ == ax;
        else if (op == NE)      ax = *sp++ != ax;
        else if (op == LT)      ax = *sp++ <  ax;
        else if (op == LE)      ax = *sp++ <= ax;
        else if (op == GT)      ax = *sp++ >  ax;
        else if (op == GE)      ax = *sp++ >= ax;
        else if (op == SHL)     ax = *sp++ << ax;
        else if (op == SHR)     ax = *sp++ >> ax;
        else if (op == ADD)     ax = *sp++ +  ax;
        else if (op == SUB)     ax = *sp++ -  ax;
        else if (op == MUL)     ax = *sp++ *  ax;
        else if (op == DIV)     ax = *sp++ /  ax;
        else if (op == MOD)     ax = *sp++ %  ax;
        // 跳转指令
        else if (op == JMP)     pc = (int*)*pc;                 // jump
        else if (op == JZ)      pc = ax ? pc + 1 : (int*)*pc;   // jump if ax == 0
        else if (op == JNZ)     pc = ax ? (int*)*pc : pc + 1;   // jump if ax != 0
        // 一些在函数调用中常用到的复杂操作的说明
        // call function: 将 pc + 1 压入栈顶 & pc 跳转至函数所在地址
        else if (op == CALL)    {*--sp = (int)(pc+1); pc = (int*)*pc;}
        // new stack frame for vars: 存储 bp 副本（用于找到原先位置）, bp 指向调用函数的地址, 栈中给函数的变量添加新的内存空间
        else if (op == NVAR)    {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}
        // delete stack frame for args: 与 x86 的设计一样，DARG N -> 销毁栈中前 N 个元素
        else if (op == DARG)    sp = sp + *pc++;
        // return caller: 将栈和对应指针恢复到调用前的样子, pc 指针指向调用函数的位置
        else if (op == RET)     {sp = bp; bp = (int*)*sp++; pc = (int*)*sp++;}        
        // end for call function.
        // native call，直接搬运 C4 项目中的配置
        else if (op == OPEN)    {ax = open((char*)sp[1], sp[0]);}
        else if (op == CLOS)    {ax = close(*sp);}
        else if (op == READ)    {ax = read(sp[2], (char*)sp[1], *sp);}
        else if (op == PRTF)    {tmp = sp + pc[1] - 1; ax = printf((char*)tmp[0], tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5]);}
        else if (op == MALC)    {ax = (int)malloc(*sp);}
        else if (op == FREE)    {free((void*)*sp);}
        else if (op == MSET)    {ax = (int)memset((char*)sp[2], sp[1], *sp);}
        else if (op == MCMP)    {ax = memcmp((char*)sp[2], (char*)sp[1], *sp);}
        else if (op == EXIT)    {printf("exit(%lld)\n", *sp); return *sp;}
        else {printf("ERROR: Unkown Instruction: %lld, cycle: %lld\n", op, cycle); return -1;}
    }
    return 0;
}

// 词法分析过程
// src code & dump
char* src,
    * src_dump;
