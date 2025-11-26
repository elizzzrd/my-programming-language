#pragma once

#include "tree_operations.h"


#ifdef DIFFERENTIATION_TREE_OPERATIONS

#define CREATE_BINARY(operator, L, R) \
({                \
    Node_result_t res = create_operator_node(tree, operator); \
    if (res.error != SUCCESS)           \
        NULL;                    \   
    res.node->left = (L);              \
    res.node->right = (R);             \
    res.node;              })

#define CREATE_UNARY(operator, L, R) \
({                \
    Node_result_t res = create_operator_node(tree, operator); \
    if (res.error != SUCCESS)           \
        NULL;                    \   
    res.node->left = (L);              \
    res.node->right = NULL;            \
    res.node;                    })
                


#define c(x) copy_subtree((x), tree)
#define d(x) differentiate_node((x), tree, var_index)

#define ADD_(L,R)   CREATE_BINARY(OP_ADD, L, R)
#define SUB_(L,R)   CREATE_BINARY(OP_SUB, L, R)
#define MUL_(L,R)   CREATE_BINARY(OP_MUL, L, R)
#define DIV_(L,R)   CREATE_BINARY(OP_DIV, L, R)
#define POW_(L,R)   CREATE_BINARY(OP_POW, L, R)

#define SIN_(X)     CREATE_UNARY(OP_SIN, X, NULL)
#define COS_(X)     CREATE_UNARY(OP_COS, X, NULL)
#define TAN_(X)     CREATE_UNARY(OP_TAN, X, NULL)
#define CTG_(X)     CREATE_UNARY(OP_CTG, X, NULL)
#define ARCSIN_(X)  CREATE_UNARY(OP_ARCSIN, X, NULL)
#define ARCCOS_(X)  CREATE_UNARY(OP_ARCCOS, X, NULL)
#define ARCTAN_(X)  CREATE_UNARY(OP_ARCTAN, X, NULL)
#define ARCCTG_(X)  CREATE_UNARY(OP_ARCCTG, X, NULL)
#define SINH_(X)    CREATE_UNARY(OP_SINH, X, NULL)
#define COSH_(X)    CREATE_UNARY(OP_COSH, X, NULL)
#define TANH_(X)    CREATE_UNARY(OP_TANH, X, NULL)
#define CTGH_(X)    CREATE_UNARY(OP_CTGH, X, NULL)
#define LOG_(X)     CREATE_UNARY(OP_LOG, X, NULL)
#define EXP_(X)     CREATE_UNARY(OP_EXP, X, NULL)
#define LN_(X)      CREATE_UNARY(OP_LN, X, NULL)
#define SQRT_(X)    CREATE_UNARY(OP_SQRT, X, NULL)
#define ABS_(X)     CREATE_UNARY(OP_ABS, X, NULL)
#define UNARY_MINUS_(X) CREATE_UNARY(OP_UNARY_MINUS, X, NULL)
#endif
