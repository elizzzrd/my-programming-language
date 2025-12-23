#pragma once

#include <stdbool.h>
#include "node_values.h"

typedef enum 
{
    SUCCESS = 0,

    TREE_NULL_POINTER,
    TREE_MEMORY_ALLOCATION_ERROR,
    TREE_EMPTY_TREE,
    TREE_DELETION_ERROR,
    TREE_INVALID_NODE_TYPE,
    TREE_INVALID_OPERATOR,
    TREE_CREATING_NODE_ERROR,
    
    OPENING_FILE_ERROR,
    LOADING_EXPRESSION_ERROR,
    SAVING_LATEX_ERROR,
    GRAPH_DUMP_ERROR,
    DIFFERENTIATION_ERROR,

    LEXER_ERROR,
    PARSER_ERROR,
    TRANSLATING_TO_ASM_ERROR
} ErrorCode;



typedef struct treenode
{
    type_t type;        // NUMBER, VARIABLE, OPERATOR, STATEMENT, BLOCK
    
    value_t value;

    bool is_unary;

    struct treenode * left; 
    struct treenode * right;
    struct treenode * prev;
} Node_t;


typedef struct 
{
    Node_t * root;
    int tree_size;
} Tree_t;


typedef struct 
{
    Node_t * node;
    ErrorCode error;
} Node_result_t; 


