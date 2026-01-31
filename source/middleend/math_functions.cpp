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

