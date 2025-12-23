#pragma once

#include <stdio.h>
#include "stack.h"
#include "error.h"
#include "spu.h"

typedef enum
{
    NUM,
    LABEL,
    REG,
    RAM,
    UNKNOWN_TYPE
} type_arg;

typedef struct 
{
    char * name;
    int instructor_ptr; 
} label_t;


Stack_Err assembler(const char * txt_filename, const char * byte_filename);
Stack_Err parse_line(const char * line, FILE * file_byte);
int * load_bytecode(const char * file_byte, size_t * size);
Spu_Err parse_argument(const char *arg, int * value, type_arg * type);
Spu_Err first_pass(const char * txt_filename);
Spu_Err second_pass(const char *txt_filename);

void init_labels(void);
void label_dump(const char * func, int line, const char * filename);
void destroy_labels(label_t * labels);
char * extract_label_name(const char * option, char * name);
int find_label(const char * name);
int is_label(const char * option, size_t ip);


#define MAX_LABELS 16
#define MAX_CODE_SIZE 1024
#define MAX_LABEL_LEN 64