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


#define OUTPUT_FILE "output.txt"    
#define OUTPUT(fmt, ...)                                               \
    do {                                                                    \
        FILE *fp = fopen(OUTPUT_FILE, "a");                         \
        if (fp) {                                                       \
            fprintf(fp, fmt, ##__VA_ARGS__);          \
            fclose(fp);                                                 \
        }                                                                   \
    } while (0)

typedef struct 
{
    const char * name;
    int label;
    int param_count;
} function_info_t;


ErrorCode translate_to_asm(Tree_t * tree, const char * filename);
ErrorCode translate_node(Node_t * node, FILE * file_ptr);
int get_id_address(const char * name);
ErrorCode translate_operator(Node_t * node, FILE * file_ptr);
ErrorCode translate_statement(Node_t * node, FILE * file_ptr);
ErrorCode translate_string(Node_t * node, FILE * file_ptr);

void emit_op_pow(FILE * file_ptr);
int emit_strings(FILE * file_ptr, const char * s);
void emit_cmp(FILE * file_ptr, const char * jmp);