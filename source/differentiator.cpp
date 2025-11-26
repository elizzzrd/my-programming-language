#include <assert.h>
#include <stdlib.h>

#define DIFFERENTIATION_TREE_OPERATIONS
#include "tree_structure.h"
#include "tree_operations.h"
#include "math_functions.h"


Node_t * differentiate_node(Node_t * node, Tree_t * tree, int var_index) // variable with var_index is the one we differentiate by
{
    assert(node && tree);
    
    switch (node->type)
    {
        case NUMBER:
        {
            Node_result_t res = create_number_node(tree, 0.0);
            if (res.error != SUCCESS)
                return NULL;
            return res.node;
        }
        case VARIABLE:
        {
            if (node->value.var_index == var_index)
            {
                Node_result_t res = create_number_node(tree, 1.0);
                if (res.error != SUCCESS)
                    return NULL;
                return res.node;
            }
            else
            {
                Node_result_t res = create_number_node(tree, 0.0);
                if (res.error != SUCCESS)
                    return NULL;
                return res.node;
            }
        }
        case OPERATOR:
        {
            operator_t op = node->value.op;
            Node_t * u = node->left;
            Node_t * v = node->right;

            
            switch (op)
            {
                // binary
                case OP_ADD:    ADD_(d(u), d(v));
                case OP_SUB:    SUB_(d(u), d(v));
                case OP_MUL:    ADD_(MUL_(d(u), c(v)), MUL_(c(u), d(v)));
                case OP_DIV:   
                {
                                Node_t * numerator = SUB_(MUL_(d(u), c(v)), MUL_(c(u), d(v)));
                                Node_t * denominator = MUL_(c(v), c(v));
                                DIV_(numerator, denominator);
                } 
                case OP_POW: // d(u^v) = u^v * (v' * ln(u) + v * u'/u)
                {
                                Node_t * left_subtree = POW_(c(u), c(v));
                                Node_t * left_1 = MUL_(d(v), LN_(c(u)));
                                Node_t * right_1 = MUL_(DIV_(c(v), c(u)), d(u));
                                Node_t * right_subtree = ADD_(left_1, right_1);
                                MUL_(left_subtree, right_subtree);
                }    

                // unary
                case OP_SIN:    MUL_(COS_(c(u)), d(u));
                case OP_COS:    MUL_(UNARY_MINUS_(SIN_(c(u))), d(u));
                case OP_TAN:     
                {
                                Node_t * external_func = DIV_(create_number_node(tree, 1.0).node, 
                                                              POW_(COS_(c(u)), create_number_node(tree, 2.0).node));
                                Node_t * internal_func = d(u);
                                MUL_(external_func, internal_func);
                }
                case OP_CTG:     
                {
                                Node_t * external_func = UNARY_MINUS_(DIV_(create_number_node(tree, 1.0).node, 
                                                              POW_(SIN_(c(u)), create_number_node(tree, 2.0).node)));
                                Node_t * internal_func = d(u);
                                MUL_(external_func, internal_func);
                }
                case OP_ARCSIN:  
                {
                                Node_t * denominator = SQRT_(SUB_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node)));
                                DIV_(d(u), denominator);
                }
                case OP_ARCCOS:  
                {
                                Node_t * denominator = SQRT_(SUB_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node)));
                                UNARY_MINUS_(DIV_(d(u), denominator));
                }
                case OP_ARCTAN:
                {
                                Node_t * denominator = ADD_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node));
                                DIV_(d(u), denominator);
                }
                case OP_ARCCTG:
                {
                                Node_t * denominator = ADD_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node));
                                UNARY_MINUS_(DIV_(d(u), denominator));
                }
                case OP_SINH:   MUL_(COSH_(c(u)), d(u));
                case OP_COSH:   MUL_(SINH_(c(u)), d(u));
                case OP_TANH:
                {
                                Node_t * external_func = DIV_(create_number_node(tree,1.0).node, 
                                                              POW_(COSH_(c(u)), create_number_node(tree,2.0).node));
                                Node_t * internal_func = d(u);
                                MUL_(external_func, internal_func);
                }
                case OP_CTGH:
                {
                                Node_t * external_func = UNARY_MINUS_(DIV_(create_number_node(tree,1.0).node, 
                                                              POW_(SINH_(c(u)), create_number_node(tree,2.0).node)));
                                Node_t * internal_func = d(u);
                                MUL_(external_func, internal_func);
                }
                case OP_LN:     MUL_(DIV_(create_number_node(tree,1.0).node, c(u)), d(u));
                case OP_EXP:    MUL_(EXP_(c(u)), d(u));
                case OP_SQRT:   MUL_(DIV_(create_number_node(tree, 0.5).node, SQRT_(c(u))), d(u));
                default:
                {               
                                ErrorCode error = SUCCESS;
                                ERROR_MESSAGE(TREE_INVALID_OPERATOR, error); 
                                return NULL;
                }
            }
        }
        default: 
        {
            ErrorCode error = SUCCESS;
            ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, error); 
            return NULL;
        }
    }
    return NULL; 
}


Tree_t * differentiate_tree(Tree_t * source_tree, int var_index)
{
    assert(source_tree && (var_index >= 0 && var_index <= MAX_VARIABLES));

    ErrorCode error = SUCCESS;
    Tree_t * diff_tree = {};
    error = init_tree(diff_tree);
    if (error != SUCCESS)
    {
        destroy_tree(diff_tree);
        return NULL;
    }
    Node_t * source_root = source_tree -> root -> right; 
    Node_t * diff_root = differentiate_node(source_root, diff_tree, var_index);
    if (!diff_root)
    {
        destroy_tree(diff_tree);
        return NULL;
    }

    diff_tree -> root -> right = diff_root;
    error = build_parent_links(diff_tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(DIFFERENTIATION_ERROR, error);
        destroy_tree(diff_tree);
        return NULL;
    }

    GRAPH_DUMP(diff_tree);
    GRAPH_DUMP_DIFF(source_tree, diff_tree);
}