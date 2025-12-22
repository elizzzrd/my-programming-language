#pragma once

#include <stdio.h>
#include "stack.h"
#include "errors_spu.h"
#include "spu.h"

typedef enum
{
    NUM,
    LABEL,
    REG,
    RAM,
    UNKNOWN_TYPE
} type_arg;

Stack_Err assembler(const char * txt_filename, const char * byte_filename);
Stack_Err parse_line(const char * line, FILE * file_byte);
int * load_bytecode(const char * file_byte, size_t * size);
void init_labels(void);
Spu_Err parse_argument(const char *arg, int * value, type_arg * type);
Spu_Err first_pass(const char * txt_filename);
Spu_Err second_pass(const char *txt_filename);
void label_dump(const char * func, int line, const char * filename);


#define MAX_LABELS 16
#define MAX_CODE_SIZE 1024
