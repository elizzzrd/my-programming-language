#include <assert.h>
#include <math.h>

#include "tree_structure.h"
#include "errors.h"
#include "tree_operations.h"
#include "math_functions.h"
#include "utils.h"
#include "node_values.h"
#include "latex_dump.h"


ErrorCode evaluate_const_node(const Node_t * node, double * result)
{
    assert(result);
    if (!node)
        return SUCCESS;

    switch (node->type)
    {
        case NUMBER:
        {
            *result = node->value.number;
            return SUCCESS;
        }
        case OPERATOR:
        {
            double left_val = 0.0;
            double right_val = 0.0;
            ErrorCode error_l = SUCCESS, error_r = SUCCESS;

            operator_t op = node->value.op;
            if (op >= OP_READ && op <= OP_ABOVE_EQUAL)
                break;
            
            if (is_unary_operator(op))
            {
                error_l = evaluate_const_node(node->left, &left_val);
                if (error_l != SUCCESS)
                    return error_l;

                switch(node->value.op)
                {
                    case OP_SIN:        * result = sin(left_val); break;
                    case OP_COS:        * result = cos(left_val); break;
                    case OP_TAN:        * result = tan(left_val); break;
                    // case OP_CTG:        * result = 1.0 / tan(left_val); break;
                    // case OP_ARCSIN:     * result = asin(left_val); break;
                    // case OP_ARCCOS:     * result = acos(left_val); break;
                    // case OP_ARCTAN:     * result = atan(left_val); break;
                    // case OP_ARCCTG:     * result = atan(1.0 / left_val); break;
                    // case OP_SINH:       * result = sinh(left_val); break;
                    // case OP_COSH:       * result = cosh(left_val); break;
                    // case OP_TANH:       * result = tanh(left_val); break;
                    // case OP_CTGH:       * result = 1.0 / tanh(left_val); break;
                    case OP_EXP:        * result = exp(left_val); break;
                    case OP_LN:         * result = log(left_val); break;
                    case OP_SQRT:       * result = sqrt(left_val); break;
                    //case OP_ABS:        * result = fabs(left_val); break;
                    case OP_UNARY_MINUS:* result = -left_val; break;
                    default:
                    {
                        ERROR_MESSAGE(TREE_INVALID_OPERATOR, error_l); 
                        return error_l;
                    }
                }
            }
            else if (is_binary_operator(op))
            {
                error_l = evaluate_const_node(node->left, &left_val);
                if (error_l != SUCCESS)
                    return error_l;
                error_r = evaluate_const_node(node->right, &right_val);
                if (error_r != SUCCESS)
                    return error_r;
                
                switch(op)
                {
                    case OP_ADD:    * result = left_val + right_val; break;
                    case OP_SUB:    * result = left_val - right_val; break;
                    case OP_MUL:    * result = left_val * right_val; break;
                    case OP_DIV:    * result = left_val / right_val; break;
                    case OP_POW:    * result = pow(left_val, right_val); break;
                    default:
                    {
                        ERROR_MESSAGE(TREE_INVALID_OPERATOR, error_l);
                        return error_l;
                    }
                }
            }
            else
            {
                ErrorCode error = SUCCESS;
                ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
                return error;
            }
            return SUCCESS;

        }
        case IDENTIFIER:
            return DIFFERENTIATION_ERROR;
        case STATEMENT:
            return DIFFERENTIATION_ERROR;
        case STRING:
            return DIFFERENTIATION_ERROR;
        default: 
        {
            if (!(node->type == IDENTIFIER || node->type == ROOT))
            {
                ErrorCode error = SUCCESS;
                ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, error); 
                return error;
            }
        }
    }
    return TREE_INVALID_NODE_TYPE;
}
 

Node_t * optimize_const_node_recursive(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if (!node)   return NULL;

    if (node->type == STATEMENT && 
        (node->value.stmt == OP_ARGS || node->value.stmt == OP_PARAMS))
        return node;

    if (node->left)
        node->left = optimize_const_node_recursive(node->left, tree);
    if (node->right)
        node->right = optimize_const_node_recursive(node->right, tree);

    double result = 0.0;
    if (evaluate_const_node(node, &result) == SUCCESS)
    {
        // replace subtree with number node
        Node_result_t new_node =  replace_node_by_number(tree, node, result);
        if (new_node.error == SUCCESS)
            return new_node.node;
    }
    return node;
}


Node_result_t replace_node_by_number(Tree_t * tree, Node_t * old_node, double new_value)
{
    assert(tree && old_node);

    Node_result_t new_node = create_number_node(tree, new_value);
    if (new_node.error != SUCCESS)
    {
        ERROR_MESSAGE(TREE_CREATING_NODE_ERROR, new_node.error);
        return new_node;
    }
    
    new_node.node->prev = old_node->prev;
    if (old_node->prev)
    {
        if (old_node->prev->left == old_node)
            old_node->prev->left = new_node.node;
        else if (old_node->prev->right == old_node)
            old_node->prev->right = new_node.node;
    }
    else
    {
        tree->root = new_node.node;
    }

    destroy_node(old_node);
    return new_node;
}


Node_t * simplify_node(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if (!node) return NULL;

    if (node->type != OPERATOR)
        return node;

    Node_t * u = node -> left;
    Node_t * v = node -> right;
    operator_t op = node -> value.op;


    #define REPLACE_WITH(SUBTREE)                         \
    do {                                                  \
        Node_t * new_sub = copy_subtree((SUBTREE), tree);  \
        if (!new_sub) return node;                        \
        new_sub -> prev = node->prev;                       \
        destroy_node(node);                               \
        return new_sub;                                   \
    } while(0)

    // x * 0 or 0 * x → 0
    if (op == OP_MUL && (IS_ZERO(u) || IS_ZERO(v)))
        return replace_node_by_number(tree, node, 0.0).node;

    // 1 * x → x
    if (op == OP_MUL && IS_ONE(u))
        REPLACE_WITH(v);

    // x * 1 → x
    if (op == OP_MUL && IS_ONE(v))
        REPLACE_WITH(u);

    // x + 0 → x
    if (op == OP_ADD && IS_ZERO(v))
        REPLACE_WITH(u);

    // 0 + x → x
    if (op == OP_ADD && IS_ZERO(u))
        REPLACE_WITH(v);

    // x - 0 → x
    if (op == OP_SUB && IS_ZERO(v))
        REPLACE_WITH(u);

    // 0 - x → -x
    if (op == OP_SUB && IS_ZERO(u))
    {
        Node_result_t res = create_operator_node(tree, OP_UNARY_MINUS);
        if (res.error != SUCCESS) return node;
        res.node -> right  = copy_subtree(v, tree);
        res.node -> left = NULL;
        if (!res.node -> right)
            return node;

        res.node -> prev = node -> prev;
        destroy_node(node);
        return res.node;
    }

    // 0 / x → 0
    if (op == OP_DIV && IS_ZERO(u))
        return replace_node_by_number(tree, node, 0.0).node;

    // x / 1 → x
    if (op == OP_DIV && IS_ONE(v))
        REPLACE_WITH(u);

    // x^0 → 1
    if (op == OP_POW && IS_ZERO(v))
        return replace_node_by_number(tree, node, 1.0).node;

    // x^1 → x
    if (op == OP_POW && IS_ONE(v))
        REPLACE_WITH(u);

    return node;
}


Node_t * optimize_simple_arithmetic_recursive(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if(!node)   return NULL;

    if (node->type == STATEMENT &&
        (node->value.stmt == OP_ARGS || node->value.stmt == OP_PARAMS))
        return node;

    if (node->left)
        node->left = optimize_simple_arithmetic_recursive(node->left, tree);
    if (node->right)
        node->right = optimize_simple_arithmetic_recursive(node->right, tree);

    return simplify_node(node, tree);
}


ErrorCode optimize_tree_recursive(Tree_t * tree)
{
    assert(tree);

    if (!tree -> root)
        return TREE_EMPTY_TREE;

    tree -> root = optimize_const_node_recursive(tree -> root, tree);
    tree -> root = optimize_simple_arithmetic_recursive(tree -> root, tree);
    build_parent_links(tree);
    
    return SUCCESS;
}


#define DIFFERENTIATION_TREE_OPERATIONS

/*
Node_t * differentiate_node(Node_t * node, Tree_t * tree, int var_index) // variable with var_index is the one we differentiate by
{
    assert(node && tree);
    
    DEBUG_PRINT("Differentiating node of type %d\n", node->type);

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
                case OP_ADD:    return ADD_(d(u), d(v));
                case OP_SUB:    return SUB_(d(u), d(v));
                case OP_MUL:    return ADD_(MUL_(d(u), c(v)), MUL_(c(u), d(v)));
                case OP_DIV:   
                {
                                Node_t * numerator = SUB_(MUL_(d(u), c(v)), MUL_(c(u), d(v)));
                                Node_t * denominator = MUL_(c(v), c(v));
                                return DIV_(numerator, denominator);
                } 
                case OP_POW: // d(u^v) = u^v * (v' * ln(u) + v * u'/u)
                {
                                Node_t * left_subtree = POW_(c(u), c(v));
                                Node_t * left_1 = MUL_(d(v), LN_(c(u)));
                                Node_t * right_1 = MUL_(DIV_(c(v), c(u)), d(u));
                                Node_t * right_subtree = ADD_(left_1, right_1);
                                return MUL_(left_subtree, right_subtree);
                }    

                // unary
                case OP_SIN:    return MUL_(COS_(c(u)), d(u));
                case OP_COS:    return MUL_(UNARY_MINUS_(SIN_(c(u))), d(u));
                case OP_TAN:     
                {
                                Node_t * external_func = DIV_(create_number_node(tree, 1.0).node, 
                                                              POW_(COS_(c(u)), create_number_node(tree, 2.0).node));
                                Node_t * internal_func = d(u);
                                return MUL_(external_func, internal_func);
                }
                case OP_CTG:     
                {
                                Node_t * external_func = UNARY_MINUS_(DIV_(create_number_node(tree, 1.0).node, 
                                                              POW_(SIN_(c(u)), create_number_node(tree, 2.0).node)));
                                Node_t * internal_func = d(u);
                                return MUL_(external_func, internal_func);
                }
                case OP_ARCSIN:  
                {
                                Node_t * denominator = SQRT_(SUB_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node)));
                                return DIV_(d(u), denominator);
                }
                case OP_ARCCOS:  
                {
                                Node_t * denominator = SQRT_(SUB_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node)));
                                return UNARY_MINUS_(DIV_(d(u), denominator));
                }
                case OP_ARCTAN:
                {
                                Node_t * denominator = ADD_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node));
                                return DIV_(d(u), denominator);
                }
                case OP_ARCCTG:
                {
                                Node_t * denominator = ADD_(create_number_node(tree,1.0).node, POW_(c(u), create_number_node(tree,2.0).node));
                                return UNARY_MINUS_(DIV_(d(u), denominator));
                }
                case OP_SINH:   return MUL_(COSH_(c(u)), d(u));
                case OP_COSH:   return MUL_(SINH_(c(u)), d(u));
                case OP_TANH:
                {
                                Node_t * external_func = DIV_(create_number_node(tree,1.0).node, 
                                                              POW_(COSH_(c(u)), create_number_node(tree,2.0).node));
                                Node_t * internal_func = d(u);
                                return MUL_(external_func, internal_func);
                }
                case OP_CTGH:
                {
                                Node_t * external_func = UNARY_MINUS_(DIV_(create_number_node(tree,1.0).node, 
                                                              POW_(SINH_(c(u)), create_number_node(tree,2.0).node)));
                                Node_t * internal_func = d(u);
                                return MUL_(external_func, internal_func);
                }
                case OP_LN:     return MUL_(DIV_(create_number_node(tree,1.0).node, c(u)), d(u));
                case OP_EXP:    return MUL_(EXP_(c(u)), d(u));
                case OP_SQRT:   return MUL_(DIV_(create_number_node(tree, 0.5).node, SQRT_(c(u))), d(u));
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


ErrorCode differentiate_tree(Tree_t * source_tree, Tree_t * diff_tree, int var_index)
{
    assert(source_tree);
    assert(diff_tree);
    
    DEBUG_PRINT("Starting differentiation by variable index %d\n", var_index);

    ErrorCode error = SUCCESS;
    error = init_tree(diff_tree);
    if (error != SUCCESS)
    {
        destroy_tree(diff_tree);
        ERROR_MESSAGE(DIFFERENTIATION_ERROR, error);
        return error;
    }

    DEBUG_PRINT("differentiation tree initialized\n");
    DEBUG_PRINT("diff_tree->root->value.root: %s\n", diff_tree->root->value.root);

    Node_t * source_root = source_tree -> root -> right; 
    Node_t * diff_root = differentiate_node(source_root, diff_tree, var_index);
    if (!diff_root)
    {
        destroy_tree(diff_tree);
        ERROR_MESSAGE(DIFFERENTIATION_ERROR, error);
        return error;
    }

    DEBUG_PRINT("differentiation completed\n");

    diff_tree -> root -> right = diff_root;
    error = build_parent_links(diff_tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(DIFFERENTIATION_ERROR, error);
        destroy_tree(diff_tree);
        return error;
    }

    DEBUG_PRINT("parent links built successfully\n");

    GRAPH_DUMP_DIFF(source_tree, diff_tree);
    return SUCCESS;
}
*/




// bool simplify_node_step_by_step(Node_t * node, Tree_t * tree)
// {
//     assert(tree);
//     if (!node) return false;

//     if (node->type != OPERATOR)
//         return node;

//     Node_t * res = NULL;
//     Node_t * u = node->left;
//     Node_t * v = node->right;
//     operator_t op = node->value.op;

//     // x * 0 or 0 * x → 0
//     if (op == OP_MUL && (IS_ZERO(u) || IS_ZERO(v)))
//     {
//         res =  replace_node_by_number(tree, node, 0.0).node;
//         return true;
//     }

//     // 1 * x → x
//     if (op == OP_MUL && IS_ONE(u))
//     {
//         REPLACE_WITH(v);
//         return true;
//     }

//     // x * 1 → x
//     if (op == OP_MUL && IS_ONE(v))
//     {
//         REPLACE_WITH(u);
//         return true;
//     }

//     // x + 0 → x
//     if (op == OP_ADD && IS_ZERO(v))
//     {
//         REPLACE_WITH(u);
//         return true;
//     }

//     // 0 + x → x
//     if (op == OP_ADD && IS_ZERO(u))
//     {
//         REPLACE_WITH(v);
//         return true;
//     }

//     // x - 0 → x
//     if (op == OP_SUB && IS_ZERO(v))
//     {
//         REPLACE_WITH(u);
//         return true;
//     }

//     // 0 - x → -x
//     if (op == OP_SUB && IS_ZERO(u))
//     {
//         Node_result_t res = create_operator_node(tree, OP_UNARY_MINUS);
//         if (res.error != SUCCESS) 
//             return node;

//         res.node->left  = copy_subtree(v, tree);
//         res.node->right = NULL;

//         if (!res.node->left)
//             return node;
//         res.node->prev = node->prev;
//         destroy_node(node);
//         return true;
//     }

//     // 0 / x → 0
//     if (op == OP_DIV && IS_ZERO(u))
//     {
//         res = replace_node_by_number(tree, node, 0.0).node;
//         return true;
//     }

//     // x / 1 → x
//     if (op == OP_DIV && IS_ONE(v))
//     {
//         REPLACE_WITH(u);
//         return true;
//     }

//     // x^0 → 1
//     if (op == OP_POW && IS_ZERO(v))
//     {
//         res = replace_node_by_number(tree, node, 1.0).node;
//         return true;
//     }

//     // x^1 → x
//     if (op == OP_POW && IS_ONE(v))
//     {
//         REPLACE_WITH(u);
//         return true;
//     }

//     return false;
// }



// // void optimize_tree_step_by_step(Tree_t * tree)
// // {
// //     int step = 0;

// //     while (1)
// //     {
// //         bool changed = (optimize_const_once || optimize_simply_arith_once);

// //         if (!changed)
// //             break;
        
// //         latex_dump_step(LATEX_DUMP_FILE, tree);
// //     }
// // }

