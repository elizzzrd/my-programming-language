#pragma once

#include <stdbool.h>
#include "node_values.h"
#include "errors.h"


typedef struct treenode
{
    type_t type;        // NUMBER, VARIABLE, OPERATOR, STATEMENT, BLOCK
    
    value_t value;

    struct treenode * left; 
    struct treenode * right;
    struct treenode * prev;

    bool is_unary;
    identifier_t id;

    size_t param_count;
} Node_t;


typedef struct 
{
    Node_t * root;
    int tree_size;
} Tree_t;



typedef enum
{
    TREE_SUCCESS,
    TREE_NULL_POINTER,
    TREE_DELETION_ERROR,
    TREE_CREATING_NODE_ERROR,
    TREE_ALLOCATION_ERROR,
    EMPTY_TREE,
    TREE_INVALID_NODE
} tree_err;

typedef struct 
{
        Node_t * node;
        tree_err error;    
} Node_result_t; 
    
