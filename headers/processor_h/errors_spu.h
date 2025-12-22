#pragma once
#include "stack.h"
#include "spu.h"

#define LOG_FILENAME "log_file.txt"
extern FILE * log_file;

#define CANARY_LEFT_VALUE ((StackElem)0xCAFEBABE)
#define CANARY_RIGHT_VALUE ((StackElem)0xDEADBEEF)

#define STACK_CHECK(stack_ptr) \
    do { \
        Stack_Err err = stack_verify(stack_ptr); \
        if (err != STACK_OK) { \
            stack_dump(stack_ptr, err, __FILE__, __LINE__);  \
        } \
        return err; \
    } while (0)


#define SPU_CHECK(spu_ptr) \
    do { \
        Spu_Err err = spu_verify(spu_ptr); \
        if (err != SPU_OK) { \
            spu_dump(spu_ptr, err, __FILE__, __LINE__); \
        } \
        return err; \
    } while (0)

    
FILE * log_init(const char * filename);
void log_close(void);
void log_message(const char * format, const char * file, int line);

void stack_dump(const stack_t * stack, Stack_Err error, const char * file, int line);
void spu_dump(const spu_t * spu, Spu_Err err, const char * file, int line);
Stack_Err stack_verify(const stack_t * stack); 
Spu_Err spu_verify(const spu_t *spu);
const char * stack_error_string(Stack_Err error);
const char * spu_error_string(Spu_Err err);
void finish_program(spu_t * spu);