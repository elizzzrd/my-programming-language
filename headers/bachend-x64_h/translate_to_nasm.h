#pragma once
#include "tree_structure.h"
#include <stdio.h>

#define IF_THERE_IS_TRANSLATE_ERROR(errors) \
    do { \
        if (error != SUCCESS) \
            { \
                ERROR_MESSAGE(TRANSLATING_TO_ASM_ERROR, error); \
                return error; \
            } \
    } while(0)

#define ASM_OUTPUT "output/asm_output.txt"

#define OUTPUT_FILE "output/output.txt"    
#define OUTPUT(fmt, ...)                                               \
    do {                                                                    \
        FILE *fp = fopen(OUTPUT_FILE, "a");                         \
        if (fp) {                                                       \
            fprintf(fp, fmt, ##__VA_ARGS__);          \
            fclose(fp);                                                 \
        }                                                                   \
    } while (0)


#define MAX_CONSTANTS 1024
static double constants[MAX_CONSTANTS];
static int const_count = 0;


typedef struct 
{
    char * name;
    int offset;
    bool initialized;
    bool is_parametr;
    int func_id;
} var_info_t;

typedef struct 
{
    var_info_t * var_list;
    int var_count;
    int var_capacity;

    int current_func_id;
    int func_counter;
} variables_t;




ErrorCode translate_to_nasm(Tree_t * tree, const char * filename);
ErrorCode emit_program(Node_t * node, FILE * file_ptr);
ErrorCode emit_expression(Node_t * node, FILE * file_ptr);
ErrorCode emit_operator(Node_t * node, FILE * file_ptr);
ErrorCode emit_cmp_x64(Node_t * node, FILE * file_ptr, operator_t op);
ErrorCode emit_statement(Node_t * node, FILE * file_ptr);