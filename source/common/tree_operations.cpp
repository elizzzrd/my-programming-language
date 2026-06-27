#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


#include "tree_structure.h"
#include "tree_operations.h"
#include "errors.h"
#include "tree_dump.h"


int graph_dump_count = 0;
int graph_dump_count_node = 0;
int graph_dump_diff = 0;


tree_err tree_ctor(Tree_t * tree)
{
    assert(tree);

    Node_result_t root = create_node(tree, ROOT);
    if (root.error != TREE_SUCCESS)
        return root.error;  

    tree->root = root.node;
    tree -> tree_size = 1;

    return TREE_SUCCESS;
}

void node_dtor(Node_t * node) 
{
    assert(node);

    if (node->left)     node_dtor(node->left);
    if (node->right)    node_dtor(node->right);
    
    node->left = NULL;
    node->right = NULL;
    node->prev = NULL;  

    if (node->type == ROOT && node->value.root)
        free(node->value.root);
    if (node->type == STRING && node->value.string_value)
        free((void*)node->value.string_value);
    if (node->type == IDENTIFIER && node->value.string_value)   
        free((void *)node->value.string_value);
    if (node->type == IDENTIFIER && node->id.name)
        free((void *)node->id.name);
    if (node->type == STATEMENT && (node->value.stmt == OP_FUNC_DEF || node->value.stmt == OP_CALL))
        free(node->id.name);
    

    //DEBUG_PRINT("Destroying node %p, type=%s", (void*)node, get_string_type(node->type));
    free(node);
    return;
}

void tree_dtor(Tree_t * tree, const char * label)
{
    assert(tree);
    GRAPH_DUMP(tree, label);

    if (tree->root)
        node_dtor(tree->root);
    tree->tree_size = 0;

    DEBUG_PRINT("[INFO] Tree_%s was successfully destroyed", label);
    return;
}

// -------------------------------------------------------------------------------

Node_result_t create_node(Tree_t * tree, type_t type)
{
    assert(tree && (type >= ROOT && type <= STRING));
    Node_result_t res = {.node = NULL, .error = TREE_SUCCESS};

    Node_t * new_node = (Node_t *) calloc(1, sizeof(Node_t));
    if (!new_node) 
    {
        res.error = TREE_ALLOCATION_ERROR;
        ERROR_MESSAGE_TREE(res.error);
        return res;
    }
    
    new_node->type = type;
    new_node->value = {};
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->prev = NULL;

    res.error = TREE_SUCCESS; res.node = new_node;
    return res;
}

Node_result_t create_operator_node(Tree_t * tree, operator_t op)
{
    assert(tree);
    
    Node_result_t res = create_node(tree, OPERATOR);
    if (res.error != TREE_SUCCESS)
        return res;
    
    res.node->value.op = op;
    if ((res.node->is_unary = is_unary_operator(op)))
        res.node->right = NULL;  
    return res;
}

Node_result_t create_statement_node(Tree_t * tree, statement_t stmt_op)
{
    assert(tree);
    
    Node_result_t res = create_node(tree, STATEMENT);
    if (res.error != TREE_SUCCESS)
        return res;
    
    res.node->value.stmt = stmt_op;

    return res;
}

Node_result_t create_number_node(Tree_t * tree, double number)
{
    assert(tree);

    Node_result_t res = create_node(tree, NUMBER);
    if (res.error != TREE_SUCCESS)
        return res;
    
    res.node->value.number = number;

    return res;
}

Node_result_t create_identifier_node(Tree_t * tree, const char * name)
{
    assert(tree);

    Node_result_t res = create_node(tree, IDENTIFIER);
    if (res.error != TREE_SUCCESS)
        return res;
    
    res.node->id.id_index = -1;
    res.node->id.name = strdup(name);
     
    return res;
}

Node_result_t create_string_node(Tree_t * tree, const char * str)
{
    assert(tree && str);

    Node_result_t res = create_node(tree, STRING);
    if (res.error != TREE_SUCCESS)
        return res;
    
    DEBUG_PRINT("in create_string_node, str = %s", str);
    res.node->value.string_value = strdup(str);
    if (!res.node->value.string_value)
    {
        res.error = TREE_ALLOCATION_ERROR;
        node_dtor(res.node);
        res.node = NULL;
        return res;
    }
    
    return res;
}

// -------------------------------------------------------------------------------

tree_err build_parent_links(Tree_t * tree)
{
    assert(tree);
    tree_err error = TREE_SUCCESS;
    
    DEBUG_PRINT("[INFO] start build parent links\n");
    if (!tree->root)    
        return EMPTY_TREE;

    tree->root->prev = NULL;
    error = build_parent_links_recursive(tree->root, tree);
    if (error != TREE_SUCCESS)
        return error;
    
    DEBUG_PRINT("[INFO] end build parent links\n");
    return TREE_SUCCESS;
}

tree_err build_parent_links_recursive(Node_t * node, Tree_t * tree)
{
    if (!node)
        return TREE_SUCCESS;
    
    if (node->left)
    {
        node->left->prev = node;
        tree_err error = build_parent_links_recursive(node->left, tree);
        if (error != TREE_SUCCESS)
            return error;
    }
    
    if (node->right)
    {
        node->right->prev = node;
        tree_err error = build_parent_links_recursive(node->right, tree);
        if (error != TREE_SUCCESS)
            return error;
    }
    
    return TREE_SUCCESS;
}


Node_t * copy_subtree(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if (!node) return NULL;

    type_t type = node -> type;
    Node_result_t new_node = {};

    switch (type)
    {
        case NUMBER: 
        {   
            new_node = NUMBER_NODE(node -> value.number);
            break;
        }
        case IDENTIFIER:
        {
            new_node = ID_NODE(node -> id.name);
            break;
        }
        case OPERATOR:
        {
            new_node = OPERATOR_NODE(node -> value.op);
            new_node.node -> left  = copy_subtree(node -> left, tree);
            new_node.node -> right = copy_subtree(node -> right, tree);
            break;
        }
        case STATEMENT:
        {
            statement_t stmt = node -> value.stmt;
            new_node = STATEMENT_NODE(stmt);
            if (stmt == OP_PROGRAM || stmt == OP_PRINT || stmt == OP_BLOCK ||
                stmt == OP_PARAMS || stmt == OP_ARGS || stmt == OP_CALL || stmt == OP_RETURN)
                new_node.node -> right = copy_subtree(node -> right, tree);
            else
            {
                new_node.node -> left  = copy_subtree(node -> left, tree);
                new_node.node -> right = copy_subtree(node -> right, tree);
            }
            break;
        }
        case STRING:
        {
            new_node = STRING_NODE(node -> value.string_value);
            break;
        }
        default:
        {
            ERROR_MESSAGE_TREE(TREE_INVALID_NODE);
            return NULL;
        }
    }

    if (new_node.error != TREE_SUCCESS)
        return NULL;

    return new_node.node;
}
    