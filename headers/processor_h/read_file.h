#pragma once 
#include "stack.h"

#define NMAX 128
#define NCOMMANDS 24

typedef enum 
{
    HLT, PUSH, DUMP, ADD, SUB, MUL,
    DIV, SQRT, OUT, POP, PUSHR, POPR,
    JB, JBE, JA, JAE, JE, JNE, JMP, IN, CALL, RET,
    PUSHM, POPM
} Stack_commands;



void to_upper_str(const char * input, char * output);
size_t get_file_size(const char * filename);
int check_option_with_stack_commands(char * option);
int check_register(const char * reg_buffer);
bool is_number(const char *str);
int is_label(const char * option);


// extern const char *stack_commands[];
// typedef Stack_Err (*stack_function_t)(stack_t *stack, StackElem value);

// Stack_Err cmd_push(stack_t *stack, StackElem value);
// Stack_Err cmd_dump(stack_t *stack, StackElem value);
// Stack_Err cmd_add(stack_t *stack, StackElem value);
// Stack_Err cmd_sub(stack_t *stack, StackElem value);
// Stack_Err cmd_mul(stack_t *stack, StackElem value);
// Stack_Err cmd_div(stack_t *stack, StackElem value);
// Stack_Err cmd_sqrt(stack_t *stack, StackElem value);
// Stack_Err cmd_out(stack_t *stack, StackElem value);
// Stack_Err cmd_hlt(stack_t *stack, StackElem value);

// extern stack_function_t operators[NCOMMANDS];