#pragma once

#include "tree_structure.h"
#include "tree_operations.h"

#ifdef DIFFERENTIATION_TREE_OPERATIONS


static inline Node_t* CREATE_BINARY(Tree_t* tree, operator_t op, Node_t* L, Node_t* R) 
{
    Node_result_t res = create_operator_node(tree, op);
    if (res.error != SUCCESS) return NULL;
    res.node->left = L;
    res.node->right = R;
    return res.node;
}

static inline Node_t* CREATE_UNARY(Tree_t* tree, operator_t op, Node_t* X) {
    Node_result_t res = create_operator_node(tree, op);
    if (res.error != SUCCESS) return NULL;
    res.node->left = X;
    res.node->right = NULL;
    return res.node;
}

#define c(x) copy_subtree((x), tree)
#define d(x) differentiate_node((x), tree, var_index)

#define ADD_(L,R)   CREATE_BINARY(tree, OP_ADD, L, R)
#define SUB_(L,R)   CREATE_BINARY(tree, OP_SUB, L, R)
#define MUL_(L,R)   CREATE_BINARY(tree, OP_MUL, L, R)
#define DIV_(L,R)   CREATE_BINARY(tree, OP_DIV, L, R)
#define POW_(L,R)   CREATE_BINARY(tree, OP_POW, L, R)

#define SIN_(X)     CREATE_UNARY(tree, OP_SIN, X)
#define COS_(X)     CREATE_UNARY(tree, OP_COS, X)
#define TAN_(X)     CREATE_UNARY(tree, OP_TAN, X)
#define CTG_(X)     CREATE_UNARY(tree, OP_CTG, X)
#define ARCSIN_(X)  CREATE_UNARY(tree, OP_ARCSIN, X)
#define ARCCOS_(X)  CREATE_UNARY(tree, OP_ARCCOS, X)
#define ARCTAN_(X)  CREATE_UNARY(tree, OP_ARCTAN, X)
#define ARCCTG_(X)  CREATE_UNARY(tree, OP_ARCCTG, X)
#define SINH_(X)    CREATE_UNARY(tree, OP_SINH, X)
#define COSH_(X)    CREATE_UNARY(tree, OP_COSH, X)
#define TANH_(X)    CREATE_UNARY(tree, OP_TANH, X)
#define CTGH_(X)    CREATE_UNARY(tree, OP_CTGH, X)
#define LOG_(X)     CREATE_UNARY(tree, OP_LOG, X)
#define EXP_(X)     CREATE_UNARY(tree, OP_EXP, X)
#define LN_(X)      CREATE_UNARY(tree, OP_LN, X)
#define SQRT_(X)    CREATE_UNARY(tree, OP_SQRT, X)
#define ABS_(X)     CREATE_UNARY(tree, OP_ABS, X)
#define UNARY_MINUS_(X) CREATE_UNARY(tree, OP_UNARY_MINUS, X)

#endif


Node_t * differentiate_node(Node_t * node, Tree_t * tree, int var_index);
ErrorCode differentiate_tree(Tree_t * source_tree, Tree_t * diff_tree, int var_index);

ErrorCode evaluate_const_node(const Node_t * node, double * result);
Node_result_t replace_node_by_number(Tree_t * tree, Node_t * old_node, double value);
//Node_t * optimize_const_node(Node_t * node, Tree_t * tree);
//Node_t * optimize_simple_arithmetic(Node_t * node, Tree_t * tree);
Node_t * optimize_const_node_recursive(Node_t * node, Tree_t * tree);
Node_t * simplify_node(Node_t * node, Tree_t * tree);
//bool simplify_node_step_by_step(Node_t * node, Tree_t * tree);
Node_t * optimize_simple_arithmetic_recursive(Node_t * node, Tree_t * tree);
ErrorCode optimize_tree_recursive(Tree_t * tree);