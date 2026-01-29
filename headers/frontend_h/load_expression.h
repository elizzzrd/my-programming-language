#pragma once
#include <stdio.h>

#include "tree_structure.h"
#define EXPRESSION_INPUT "expression.txt"
#define AST_OUTPUT_FRONTEND "output/ast_output_frontend.txt"
#define AST_OUTPUT_MIDDLEEND "output/ast_output_middleend.txt"

typedef token_res (*check_func_t)(const char * token);
    

Node_t * find_node_by_name(Node_t * node, const char * target, int * count);
Node_result_t seek_item(const char * data, const Tree_t * tree,  int * count);
char ** collect_path(Node_t * node, size_t tree_size, size_t * path_size, Tree_t * tree);

ErrorCode save_tree(Tree_t * tree, const char * filename);
void savenode(Node_t *node, FILE *f);

ErrorCode build_middleend_tree(Tree_t * tree, const char * expression_input);
Node_t * read_node(char * buffer, size_t * pos, Tree_t * tree);

token_res define_token_type(char * buffer, size_t * pos);
char * get_token(const char * buffer, size_t * pos);

