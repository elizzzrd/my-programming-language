#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "tree_structure.h"
#include "errors.h"
#include "tree_operations.h"
#include "optimize_tree.h"
#include "node_values.h"

extern const error_struct optimizer_error_list[];

// ----------------------------------------------------------------------------------------
static bool IS_ONE(Node_t * node) 
{
    if (!node) return false;

    return (node->type == NUMBER && fabs(node->value.number - 1) < 1e-9);
}

static bool IS_ZERO(Node_t * node) 
{
    if (!node) return false;

    return (node->type == NUMBER && fabs(node->value.number) < 1e-9);
}
// ----------------------------------------------------------------------------------------

optimize_err evaluate_const_node(const Node_t * node, double * result)
{
    assert(result);
    if (!node)
        return OPTIMIZER_SUCCESS;

    switch (node->type)
    {
        case NUMBER:
        {
            *result = node->value.number;
            return OPTIMIZER_SUCCESS;
        }
        case OPERATOR:
        {
            double left_val = 0.0;
            double right_val = 0.0;
            optimize_err error_l = OPTIMIZER_SUCCESS, error_r = OPTIMIZER_SUCCESS;

            operator_t op = node->value.op;
            if (op >= OP_READ && op <= OP_ABOVE_EQUAL)
                break;
            
            if (is_unary_operator(op))
            {
                error_l = evaluate_const_node(node->left, &left_val);
                if (error_l != FRONTEND_SUCCESS)
                    return error_l;

                switch(node->value.op)
                {
                    case OP_SIN:        * result = sin(left_val); break;
                    case OP_COS:        * result = cos(left_val); break;
                    case OP_TAN:        * result = tan(left_val); break;
                    case OP_EXP:        * result = exp(left_val); break;
                    case OP_LN:         * result = log(left_val); break;
                    case OP_SQRT:       * result = sqrt(left_val); break;
                    //case OP_ABS:        * result = fabs(left_val); break;
                    case OP_UNARY_MINUS:* result = -left_val; break;
                    default:
                        return OPTIMIZER_INVALID_OPERATOR;
                }
            }
            else if (is_binary_operator(op))
            {
                error_l = evaluate_const_node(node->left, &left_val);
                if (error_l != OPTIMIZER_SUCCESS)
                    return error_l;
                error_r = evaluate_const_node(node->right, &right_val);
                if (error_r != OPTIMIZER_SUCCESS)
                    return error_r;
                
                switch(op)
                {
                    case OP_ADD:    * result = left_val + right_val; break;
                    case OP_SUB:    * result = left_val - right_val; break;
                    case OP_MUL:    * result = left_val * right_val; break;
                    case OP_DIV:    
                    {
                                    if (right_val != 0)
                                    {
                                        * result = left_val / right_val; 
                                        break;
                                    }
                                    else
                                    {
                                        return OPTIMIZER_DIVISION_BY_ZERO;
                                    }
                    }
                    
                    case OP_POW:    * result = pow(left_val, right_val); break;
                    default:
                        return OPTIMIZER_INVALID_OPERATOR;
                }
            }
            else
                return OPTIMIZER_INVALID_OPERATOR;
        
        }
        case IDENTIFIER:
            return OPTIMIZER_INVALID_NODE;
        case STATEMENT:
            return OPTIMIZER_INVALID_NODE;
        case STRING:
            return OPTIMIZER_INVALID_NODE;
        default: 
        {
            if (!(node->type == IDENTIFIER || node->type == ROOT)) 
                return OPTIMIZER_INVALID_NODE;   
        }
    }
    return OPTIMIZER_INVALID_NODE;
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
    optimize_err error = evaluate_const_node(node, &result);
    
    if (error == OPTIMIZER_SUCCESS)
    {
        // replace subtree with number node
        Node_result_t new_node =  replace_node_by_number(tree, node, result);
        if (new_node.error == TREE_SUCCESS)
            return new_node.node;
    }
    else
    {
        //ERROR_MESSAGE_OPTIMIZER(error);
    }
    return node;
}


Node_result_t replace_node_by_number(Tree_t * tree, Node_t * old_node, double new_value)
{
    assert(tree && old_node);

    Node_result_t new_node = NUMBER_NODE(new_value);
    if (new_node.error != TREE_SUCCESS)
    {
        ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
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

    node_dtor(old_node);
    return new_node;
}


#define REPLACE_WITH(SUBTREE)                                \
    do {                                                     \
        Node_t * new_sub = copy_subtree((SUBTREE), tree);    \
        if (!new_sub) return node;                           \
        new_sub -> prev = node->prev;                        \
        node_dtor(node);                                  \
        return new_sub;                                      \
    } while(0)

Node_t * simplify_node(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if (!node) return NULL;

    if (node->type != OPERATOR)
        return node;

    Node_t * u = node -> left;
    Node_t * v = node -> right;
    operator_t op = node -> value.op;

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
        Node_result_t res = OPERATOR_NODE(OP_UNARY_MINUS);
        if (res.error != TREE_SUCCESS) 
            return node;
        res.node -> right  = copy_subtree(v, tree);
        res.node -> left = NULL;
        if (!res.node -> right)
            return node;

        res.node -> prev = node -> prev;
        node_dtor(node);
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


optimize_err optimize_tree_recursive(Tree_t * tree)
{
    assert(tree);

    if (!tree -> root)
        return OPTIMIZER_INVALID_NODE;

    tree -> root = optimize_const_node_recursive(tree -> root, tree);
    tree -> root = optimize_simple_arithmetic_recursive(tree -> root, tree);
    build_parent_links(tree);
    
    return OPTIMIZER_SUCCESS;
}