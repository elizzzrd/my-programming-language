#pragma once

#include "errors.h"
#include "tree_operations.h"


ErrorCode init_tree(Tree_t * tree);
Node_t * copy_subtree(Node_t * node, Tree_t * tree);

Node_result_t create_node(Tree_t * tree, type_t type);
Node_result_t create_operator_node(Tree_t * tree, operator_t op);
Node_result_t create_number_node(Tree_t * tree, double number);
Node_result_t create_variable_node(Tree_t * tree, int var_index);

void destroy_node(Node_t * node); 
void destroy_tree(Tree_t * tree);

ErrorCode build_parent_links_recursive(Node_t * node, Tree_t * tree);
ErrorCode build_parent_links(Tree_t * tree);

