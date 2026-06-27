#pragma once

#include "tree_structure.h"
#include "tree_operations.h"
#include "errors.h"


typedef struct  
{   
    Tree_t * ast_tree;      
    optimize_err state;
} optimizer_t; 


optimize_err optimizer_ctor(char * input_name, optimizer_t * optimizer);

optimize_err optimizer_dtor(optimizer_t * optimizer);

optimize_err optimize_AST(optimizer_t optimizer);


optimize_err evaluate_const_node(const Node_t * node, double * result);
Node_result_t replace_node_by_number(Tree_t * tree, Node_t * old_node, double value);
Node_t * simplify_node(Node_t * node, Tree_t * tree);

Node_t *     optimize_const_node_recursive(Node_t * node, Tree_t * tree);
Node_t *     optimize_simple_arithmetic_recursive(Node_t * node, Tree_t * tree);
optimize_err optimize_tree_recursive(Tree_t * tree);
