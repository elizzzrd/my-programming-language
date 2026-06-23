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
#define NASM_OUTPUT "output/nasm_output.asm"


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



typedef struct 
{
    char * name;
    int offset;
    bool initialized;
    bool is_parametr;
    int func_id;
    int param_index;
} var_info_t;

typedef struct 
{
    var_info_t * var_list;
    int var_count;
    int var_capacity;

    int current_func_id;
    int func_counter;
} variables_t;


void init_variables(void);
int find_variable_in_current_func(const char * name);
int add_variable(const char * name, bool is_parameter);
var_info_t * get_variable_by_index(int index);
void clear_variables_for_func(int func_id);
int get_frame_size_for_func(int func_id);
void clear_variables(void);
void assign_offset_for_function(int func_id);
ErrorCode collect_variables(Node_t * node);
int new_label(void);
int add_constant(double val);


ErrorCode translate_to_nasm(Tree_t * tree, const char * filename);
ErrorCode emit_program(Node_t * node, FILE * file_ptr);
ErrorCode emit_expression(Node_t * node, FILE * file_ptr);
ErrorCode emit_operator(Node_t * node, FILE * file_ptr);
ErrorCode emit_cmp_x64(Node_t * node, FILE * file_ptr, operator_t op);
ErrorCode emit_statement(Node_t * node, FILE * file_ptr);