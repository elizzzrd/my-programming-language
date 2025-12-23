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


ErrorCode translate_to_asm(Tree_t * tree, const char * filename);
ErrorCode translate_node(Node_t * node, FILE * file_ptr);
int get_id_address(const char * name);
ErrorCode translate_operator(Node_t * node, FILE * file_ptr);
ErrorCode translate_statement(Node_t * node, FILE * file_ptr);
