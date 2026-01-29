#pragma once 
#include "stack.h"

#define NMAX 128
#define NCOMMANDS 30

typedef enum 
{
    HLT, PUSH, DUMP, ADD, SUB, MUL,
    DIV, SQRT, OUT, POP, PUSHR, POPR,
    JB, JBE, JA, JAE, JE, JNE, JMP, IN, CALL, RET,
    PUSHM, POPM, SIN, COS, LN, EXP, ARCTG, PUTS
} Stack_commands;



void to_upper_str(const char * input, char * output);
int check_option_with_stack_commands(char * option);
int check_register(const char * reg_buffer);
bool is_number(const char *str);


static const double EPSILON = 1e-12;


bool is_zero_double(double num);
bool is_positive_double(double num);
void clamp_to_zero_double(double * number);
bool double_comparison(double number1, double number2);

