#pragma once

#include "errors.h"
#include "tree_operations.h"

#define REPLACE_WITH(SUBTREE)                         \
do {                                                  \
    Node_t *new_sub = copy_subtree((SUBTREE), tree);  \
    if (!new_sub) return node;                        \
    new_sub->prev = node->prev;                       \
    destroy_node(node);                               \
    return new_sub;                                   \
} while(0)


ErrorCode init_tree(Tree_t * tree);
Node_t * copy_subtree(Node_t * node, Tree_t * tree);

Node_result_t create_node(Tree_t * tree, type_t type);
Node_result_t create_operator_node(Tree_t * tree, operator_t op);
Node_result_t create_number_node(Tree_t * tree, double number);
Node_result_t create_identifier_node(Tree_t * tree, const char * name);
Node_result_t create_statement_node(Tree_t * tree, statement_t stmt_op);
Node_result_t create_string_node(Tree_t * tree, const char * str);

void destroy_node(Node_t * node); 
void destroy_tree(Tree_t * tree, const char * label);

ErrorCode build_parent_links_recursive(Node_t * node, Tree_t * tree);
ErrorCode build_parent_links(Tree_t * tree);

