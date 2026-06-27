#pragma once

#include "tree_structure.h"
#include <stdio.h>



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

typedef struct 
{
    const char * name;
    int label;
    int param_count;
} function_info_t;


backend_err translate_to_asm(Tree_t * tree, const char * filename);
backend_err translate_node(Node_t * node, FILE * file_ptr);
int get_id_address(const char * name);
backend_err translate_operator(Node_t * node, FILE * file_ptr);
backend_err translate_statement(Node_t * node, FILE * file_ptr);
backend_err translate_string(Node_t * node, FILE * file_ptr);

void emit_op_pow(FILE * file_ptr);
int emit_strings(FILE * file_ptr, const char * s);
void emit_cmp(FILE * file_ptr, const char * jmp);

backend_err translate_functions(Node_t * node, FILE * file_ptr);
backend_err translate_main(Node_t * node, FILE * file_ptr);