#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "read_file.h"
#include "assembler.h"
#include "utils.h"
#include "errors_spu.h"

#define DISABLE_DEBUG_PRINT

const char * stack_commands[] = 
{
    "HLT", "PUSH", "DUMP", "ADD", "SUB", "MUL", 
    "DIV", "SQRT","OUT", "POP", "PUSHR", "POPR",
    "JB", "JBE", "JA", "JAE", "JE", "JNE", "JMP", "IN", "CALL", "RET",
    "PUSHM", "POPM", "SIN", "COS", "LN", "EXP", "ARCTG", "PUTS"
};

int check_option_with_stack_commands(char * option)
{
    char option_upper[NMAX] = {};
    to_upper_str(option, option_upper);
    for (int i = 0; i < NCOMMANDS; i++)
    {
        if (strcmp(option_upper, stack_commands[i]) == 0) return i;
    }
    return -1;
}

void to_upper_str(const char * input, char * output)
{
    int i = 0;
    for (i = 0; input[i] != '\0'; i++)
    {
        output[i] = (char)toupper((unsigned char) input[i]);
    }
    output[i] = '\0';
    return;
}


bool is_number(const char * str) 
{
    if (!str || !*str) return false;
    
    char *endptr;
    strtod(str, &endptr);
    
    return (endptr != str && *endptr == '\0');
}

int check_register(const char * reg_buffer)
{
    int reg_num = (toupper(reg_buffer[1]) - 'A') + 1;
    if (strlen(reg_buffer) != 3 || reg_num < 1 || reg_num > 16) return 0;
    else return reg_num;
}



bool is_zero_double(double num)
{
    return (fabs(num) < EPSILON);
}


bool is_positive_double(double num)
{
    return (num > EPSILON);
}


void clamp_to_zero_double(double * number)
{
    assert(number != NULL && "NULL pointer");
    if (is_zero_double(*number))
    {
        *number = 0;
    }
}


bool double_comparison(double number1, double number2)
{
    return (fabs(number1 - number2) < EPSILON);
}
