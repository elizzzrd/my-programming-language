#pragma once

#include "tree_structure.h"


tree_err tree_ctor(Tree_t * tree);
Node_t * copy_subtree(Node_t * node, Tree_t * tree);

Node_result_t create_node(Tree_t * tree, type_t type);
Node_result_t create_operator_node(Tree_t * tree, operator_t op);
Node_result_t create_number_node(Tree_t * tree, double number);
Node_result_t create_identifier_node(Tree_t * tree, const char * name);
Node_result_t create_statement_node(Tree_t * tree, statement_t stmt_op);
Node_result_t create_string_node(Tree_t * tree, const char * str);

void node_dtor(Node_t * node); 
void tree_dtor(Tree_t * tree, const char * label);

tree_err build_parent_links_recursive(Node_t * node, Tree_t * tree);
tree_err build_parent_links(Tree_t * tree);


#define OPERATOR_NODE(_op_)            create_operator_node(tree, (_op_))
#define NUMBER_NODE(_number_)          create_number_node(tree, (_number_))
#define ID_NODE(_name_)                create_identifier_node(tree, (_name_))
#define STATEMENT_NODE(_stmt_op_)      create_statement_node(tree, (_stmt_op_))
#define STRING_NODE(_str_)             create_string_node(tree, (_str_))