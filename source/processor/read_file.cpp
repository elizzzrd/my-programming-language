#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "read_file.h"
#include "assembler.h"
#include "errors_spu.h"


const char * stack_commands[] = 
{
    "HLT", "PUSH", "DUMP", "ADD", "SUB", "MUL", 
    "DIV", "SQRT","OUT", "POP", "PUSHR", "POPR",
    "JB", "JBE", "JA", "JAE", "JE", "JNE", "JMP", "IN", "CALL", "RET",
    "PUSHM", "POPM"
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

size_t get_file_size(const char * filename) 
{
    assert(filename != NULL);
    
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) 
    {
        return (size_t)-1;
    }

    return (size_t)(file_stat.st_size);
}


int is_label(const char * option)
{
    int label_num = 0;
    if (option[0] == ':' && isdigit(option[1]))
    {
        int id = atoi(option + 1);
            if (id >= 0 && id < MAX_LABELS)
                return id;
            else
            {
                log_message("Label index out of range", __FILE__, __LINE__);
                return 0;
            }
    }
    return -1;
}


bool is_number(const char * str) 
{
    if (!str || !*str) return false;
    if (*str == '-' || *str == '+') str++;
    for (; *str; str++)     
    {
        if (!isdigit((unsigned char)*str)) return false;
    }
    return true;
}

int check_register(const char * reg_buffer)
{
    int reg_num = (toupper(reg_buffer[1]) - 'A') + 1;
    if (strlen(reg_buffer) != 3 || reg_num < 1 || reg_num > 16) return 0;
    else return reg_num;
}