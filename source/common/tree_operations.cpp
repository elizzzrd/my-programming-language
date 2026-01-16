#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


#include "tree_structure.h"
#include "tree_operations.h"
#include "errors.h"
#include "utils.h"

int graph_dump_count = 0;
int graph_dump_count_node = 0;
int graph_dump_diff = 0;


ErrorCode init_tree(Tree_t * tree)
{
    assert(tree);

    Node_result_t root = create_node(tree, ROOT);
    if (root.error != SUCCESS)
        return root.error;  

    tree->root = root.node;
    tree -> tree_size = 1;

    //GRAPH_DUMP(tree);
    return SUCCESS;
}

Node_result_t create_node(Tree_t * tree, type_t type)
{
    assert(tree && (type >= ROOT && type <= STRING));
    Node_result_t res = {.node = NULL, .error = SUCCESS};

    Node_t * new_node = (Node_t *) calloc(1, sizeof(Node_t));
    if (!new_node) 
    {
        ERROR_MESSAGE(TREE_MEMORY_ALLOCATION_ERROR, res.error); 
        return res;
    }
    
    new_node->type = type;
    new_node->value = {};
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->prev = NULL;

    //GRAPH_DUMP_NODE(new_node);
    res.error = SUCCESS; res.node = new_node;
    return res;
}


Node_result_t create_operator_node(Tree_t * tree, operator_t op)
{
    assert(tree);
    
    Node_result_t res = create_node(tree, OPERATOR);
    if (res.error != SUCCESS)
        return res;
    
    res.node->value.op = op;
    if ((res.node->is_unary = is_unary_operator(op)))
        res.node->right = NULL;
    GRAPH_DUMP_NODE(res.node);    
    return res;
}

Node_result_t create_statement_node(Tree_t * tree, statement_t stmt_op)
{
    assert(tree);
    
    Node_result_t res = create_node(tree, STATEMENT);
    if (res.error != SUCCESS)
        return res;
    
    res.node->value.stmt = stmt_op;
    
    GRAPH_DUMP_NODE(res.node);   
    return res;
}

Node_result_t create_number_node(Tree_t * tree, double number)
{
    assert(tree);

    Node_result_t res = create_node(tree, NUMBER);
    if (res.error != SUCCESS)
        return res;
    
    res.node->value.number = number;
    GRAPH_DUMP_NODE(res.node); 
    return res;
}

Node_result_t create_identifier_node(Tree_t * tree, int id_index)
{
    assert(tree);

    Node_result_t res = create_node(tree, IDENTIFIER);
    if (res.error != SUCCESS)
        return res;
    
    res.node->value.id_index = id_index;
    GRAPH_DUMP_NODE(res.node); 
    return res;
}


Node_result_t create_string_node(Tree_t * tree, const char * str)
{
    assert(tree && str);

    Node_result_t res = create_node(tree, STRING);
    if (res.error != SUCCESS)
        return res;
    
    DEBUG_PRINT("in create_string_node, str = %s", str);
    res.node->value.string_value = strdup(str);
    if (!res.node->value.string_value)
    {
        res.error = TREE_MEMORY_ALLOCATION_ERROR;
        destroy_node(res.node);
        res.node = NULL;
        return res;
    }
    GRAPH_DUMP_NODE(res.node); 
    return res;
}


void destroy_node(Node_t * node) 
{
    assert(node);

    if (node->left)     destroy_node(node->left);
    if (node->right)    destroy_node(node->right);
    
    node->left = NULL;
    node->right = NULL;
    node->prev = NULL;  

    if (node->type == ROOT && node->value.root)
        free(node->value.root);
    if (node->type == STRING && node->value.string_value)
        free((void*)node->value.string_value);

    DEBUG_PRINT("Destroying node %p, type=%s", (void*)node, get_string_type(node->type));
    free(node);
    return;
}

void destroy_tree(Tree_t * tree)
{
    assert(tree);
    //DEBUG_PRINT("[DEBUG] root->value.root before destroy: %s\n", tree->root->value.root);
    GRAPH_DUMP(tree);

    if (tree->root)
        destroy_node(tree->root);
    tree->tree_size = 0;
    return;
}

ErrorCode build_parent_links(Tree_t * tree)
{
    assert(tree);
    ErrorCode error = SUCCESS;
    
    DEBUG_PRINT("[INFO] start build parent links\n");
    if (!tree->root)
    {
        return TREE_EMPTY_TREE;
    }
    
    tree->root->prev = NULL;
    error = build_parent_links_recursive(tree->root, tree);
    if (error != SUCCESS)
        return error;
    
    char tree_size[40] = {};   
    sprintf(tree_size, "tree_size = %d", (tree->tree_size) - 1);
    tree->root->value.root = strdup(tree_size);

    DEBUG_PRINT("[INFO] end build parent links\n");
    return SUCCESS;
}

ErrorCode build_parent_links_recursive(Node_t * node, Tree_t * tree)
{
    if (!node)
        return SUCCESS;
    
    if (node->left)
    {
        node->left->prev = node;
        ErrorCode error = build_parent_links_recursive(node->left, tree);
        if (error != SUCCESS)
            return error;
    }
    
    if (node->right)
    {
        node->right->prev = node;
        ErrorCode error = build_parent_links_recursive(node->right, tree);
        if (error != SUCCESS)
            return error;
    }
    
    return SUCCESS;
}



Node_t * copy_subtree(Node_t * node, Tree_t * tree)
{
    assert(tree);
    if (!node) return NULL;

    type_t type = node ->type;
    Node_result_t new_node = {};

    switch (type)
    {
        case NUMBER: 
        {   
            new_node = create_number_node(tree, node -> value.number);
            break;
        }
        case IDENTIFIER:
        {
            new_node = create_identifier_node(tree, node ->value.id_index);
            break;
        }
        case OPERATOR:
        {
            new_node = create_operator_node(tree, node -> value.op);
            new_node.node -> left  = copy_subtree(node -> left, tree);
            new_node.node -> right = copy_subtree(node -> right, tree);
            break;
        }
        case STATEMENT:
        {
            new_node = create_statement_node(tree, node -> value.stmt);
            statement_t stmt = node -> value.stmt;
            if (stmt == OP_PROGRAM || stmt == OP_PRINT || stmt == OP_END || stmt == OP_BLOCK)
                new_node.node -> right = copy_subtree(node ->right, tree);
            else
            {
                new_node.node -> left  = copy_subtree(node -> left, tree);
                new_node.node -> right = copy_subtree(node -> right, tree);
            }
            break;
        }
        case STRING:
        {
            new_node = create_string_node(tree, node -> value.string_value);
            break;
        }
        default:
        {
            ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, new_node.error);
            return NULL;
        }
    }

    if (new_node.error != SUCCESS)
        return NULL;

    return new_node.node;
}
    