#pragma once
#include "stack.h"
#include <stdio.h>
#include <stdbool.h>


#define REGS_COUNT 16
#define RAM_COUNT 512

typedef struct 
{
    StackElem * code;
    size_t instructor_ptr; 
    size_t code_size;
    stack_t stack;
    StackElem regs[REGS_COUNT];
    stack_t return_address;
    StackElem RAM[RAM_COUNT];
} spu_t;


enum
{
    SPU_OK                  = 0u,

    SPU_STACK_ERROR         = 1u << 0,
    SPU_UNKNOWN_COMMAND     = 1u << 1,   
    SPU_DIVISION_BY_ZERO    = 1u << 2,
    SPU_FILE_ERROR          = 1u << 3,
    SPU_NULL_PTR            = 1u << 4,
    SPU_INVALID_COMMAND     = 1u << 5,
    SPU_INVALID_CODE        = 1u << 6,
    SPU_INVALID_IP          = 1u << 7,
}; 

typedef unsigned int Spu_Err;

Spu_Err spu_init(spu_t * spu);
Spu_Err run_spu(spu_t * spu);
Spu_Err spu_destroy(spu_t * spu);
bool is_command(int code);