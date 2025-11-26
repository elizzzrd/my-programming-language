#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tree_structure.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"
#include "errors.h"

/*
void savenode(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);

    fprintf(file_ptr, "(");
    if (node -> value)
        fprintf(file_ptr, "\"%c\"", node->data);

    if (!node -> left && !node -> right)
        fprintf(file_ptr, " nil nil");

    if (node -> left)    
        savenode(node -> left, file_ptr);
    if (node -> right)
        savenode(node -> right, file_ptr);

    fprintf(file_ptr, ")");
}

ErrorCode save_database(Tree_t * tree)
{
    assert(tree);

    ErrorCode error = SUCCESS;

    FILE * file_ptr = fopen(DATABASE_OUTPUT, "w");
    if (!file_ptr)
    {
        ERROR_MESSAGE(TREE_OPENING_FILE_ERROR, error);
        return error;
    }

    savenode(tree->root->right, file_ptr);

    //GRAPH_DUMP(tree);
    fclose(file_ptr);
    return error;
}
*/

ErrorCode load_expression(Tree_t * tree, const char * filename)
{
    assert(tree && filename);

    ErrorCode error = SUCCESS;

    char * buffer = NULL;
    error = load_to_buffer(EXPRESSION_INPUT, &buffer);
    if (error != SUCCESS)
        return error;
    DEBUG_PRINT("[DEBUG]: expression has been loaded to buffer\n");
    DEBUG_PRINT("[DEBUG] buffer:\n");
    DEBUG_PRINT("%s\n", buffer);

    size_t pos = 0;
    Node_t * first_node = read_node(buffer, &pos, tree);
    if (!first_node)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        free(buffer);
        return error;
    }

    tree -> root -> right = first_node;
    error = build_parent_links(tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        free(first_node);
        free(buffer);
        return error;
    }

    DEBUG_PRINT("[DEBUG] root->value.root after loading: %s\n", tree->root->value.root);
    GRAPH_DUMP(tree);
    
    free(buffer);
    return error;
}


Node_t * read_node(char * buffer, size_t * pos, Tree_t * tree) 
{
    assert(buffer && pos && tree);
    ErrorCode error = SUCCESS;
    
    DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);

    while (isspace(buffer[*pos])) 
        (*pos)++;

    DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);    
    
    if (buffer[*pos] == '(') 
    {
        (*pos)++;     
                                                      // пропускаем '('
        DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);

        while (isspace(buffer[*pos])) (*pos)++;                     // пропускаем пробелы после '('
                                        
        DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);

        token_res token = define_token_type(buffer, pos);
        if (token.type == ROOT)
        {
            ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
            return NULL;
        }
        
        type_t type = token.type;

        DEBUG_PRINT("[DEBUG] in read_node: define_token_type return: %s\n", get_string_type(type));

        Node_result_t current = {.node = NULL, .error = SUCCESS};
        switch (type)
        {
            case VARIABLE:  
            {
                current = create_variable_node(tree, token.value.var_index);
                if (current.error != SUCCESS)
                {
                    ERROR_MESSAGE(TREE_CREATING_NODE_ERROR, error);
                    return NULL;
                }
                break;
            }
            case NUMBER:
            {
                current = create_number_node(tree, token.value.number);
                if (current.error != SUCCESS)
                {
                    ERROR_MESSAGE(TREE_CREATING_NODE_ERROR, error);
                    return NULL;
                }
                break;
            }
            case OPERATOR:
            {
                current = create_operator_node(tree, token.value.op);
                if (current.error != SUCCESS)
                {
                    ERROR_MESSAGE(TREE_CREATING_NODE_ERROR, error);
                    return NULL;
                }
                break;
            }
            default:
            {
                ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, current.error);
                return NULL;
                break;
            }
        }
        if (current.error == SUCCESS)
        {    
            DEBUG_PRINT("[DEBUG] node has been succesfully created: %s\n", get_string_type(current.node->type));
            tree->tree_size++;
        }
        else 
        {
            ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
            return NULL;
        }

        if (current.node->is_unary)
        {
            current.node->left = read_node(buffer, pos, tree);
            current.node->right = NULL;
        }
        else
        {
            DEBUG_PRINT("[DEBUG] started processing left subtree\n");
            current.node->left = read_node(buffer, pos, tree);
            DEBUG_PRINT("[DEBUG] started processing right subtree\n");
            current.node->right = read_node(buffer, pos, tree);
        }
        
        while (isspace(buffer[*pos]))                             // пробелы перед закрывающей скобкой
            (*pos)++;
        
        if (buffer[*pos] == ')') 
            (*pos)++;
        
        return current.node;
    }
    else if (strncmp(buffer + *pos, "nil", 3) == 0) 
    {
        *pos += 3;
        return NULL;
    }
    return NULL;
}


char * get_token(const char * buffer, size_t * pos)
{
    size_t start = * pos;

    while (buffer[*pos] && !isspace(buffer[*pos]) && buffer[*pos] != ')' && buffer[*pos] != '(')
        (*pos)++;

    size_t len = *pos - start;
    char * token = (char*)calloc(len + 1, sizeof(char));
    memcpy(token, buffer + start, len);
    token[len] = '\0';

    return token;
} 



token_res define_token_type(char * buffer, size_t * pos)
{
    assert(buffer && pos);

    DEBUG_PRINT("[DEBUG] in define_token_type\n");
    DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);

    token_res result = {.type = ROOT};

    while (isspace(buffer[*pos]))
        (*pos)++;

    char * token = get_token(buffer, pos);

    DEBUG_PRINT("[DEBUG] get_token return: %s\n", token);
    size_t len = strlen(token);

    if (!token || token[0] == '\0')
    {
        free(token);
        return result;
    }

    static check_func_t check_functions[] = 
    {
        check_for_variable,
        check_for_operator,
        check_for_number,
        NULL
    };

    for (int i = 0; check_functions[i] != NULL; i++)
    {
        result = check_functions[i](token);
        if (result.type != ROOT)
        {
            DEBUG_PRINT("[DEBUG] token type was found: %s\n", get_string_type(result.type));
            free(token);
            return result;
        }
    }


    DEBUG_PRINT("[DEBUG] cycle check_function return: %s\n", get_string_type(result.type));
    *(pos) += len;
    DEBUG_PRINT("[DEBUG] pos = %lu\n", *pos);

    free(token);
    return result;
}


