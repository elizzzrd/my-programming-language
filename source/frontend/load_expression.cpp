#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tree_structure.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "optimize_tree.h"
#include "buffer.h"
#include "lexer.h"
#include "errors.h"



//optimize_err optimizer_dtor(optimizer_t * optimizer);

//optimize_err optimize_AST(optimizer_t optimizer);


optimize_err optimizer_ctor(char * expression_input, optimizer_t * optimizer)
{
    assert(optimizer && expression_input);

    buffer_err b_error = BUFFER_SUCCESS;
    char * buffer = buffer_ctor(&b_error, expression_input);
    if (b_error != BUFFER_SUCCESS)
    {
        ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_BUFFER_ERROR);
        return OPTIMIZER_BUFFER_ERROR;
    }
    DEBUG_PRINT("[DEBUG]: expression has been loaded to buffer\n");
    DEBUG_PRINT("[DEBUG] buffer:\n");
    DEBUG_PRINT("%s\n", buffer);

    size_t pos = 0;
    Node_t * first_node = read_node(buffer, &pos, optimizer->ast_tree);
    if (!first_node)
    {
        ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_LOADING_EXPRESSION_ERROR);
        free(buffer);
        return OPTIMIZER_LOADING_EXPRESSION_ERROR;
    }
    DEBUG_PRINT("[DEBUG] all nodes was read");

    optimizer->ast_tree->root->right = first_node;
    tree_err t_error = build_parent_links(optimizer->ast_tree);
    if (t_error != TREE_SUCCESS)
    {
        ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_LOADING_EXPRESSION_ERROR);
        free(first_node);
        free(buffer);
        return OPTIMIZER_LOADING_EXPRESSION_ERROR;
    }
    
    free(buffer);
    return OPTIMIZER_SUCCESS;
}


Node_t * read_node(char * buffer, size_t * pos, Tree_t * tree) 
{
    assert(buffer);
    assert(tree);
    assert(pos);
    
    while (isspace(buffer[*pos])) 
        (*pos)++;
        
    if (buffer[*pos] == '(') 
    {
        (*pos)++;     
                                                                    
        while (isspace(buffer[*pos])) (*pos)++;                     
                                        
        token_res token = define_token_type(buffer, pos);
        if (token.type == ROOT)
        {
            ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_LOADING_EXPRESSION_ERROR);
            return NULL;
        }
        
        type_t type = token.type;

        Node_result_t current = {.node = NULL, .error = TREE_SUCCESS};
        switch (type)
        {
            case IDENTIFIER:  
            {
                current = ID_NODE(token.id.name);
                current.node->id.id_index = token.id.id_index;
                if (current.error != TREE_SUCCESS)
                {
                    ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
                    return NULL;
                }

                free((void *)token.id.name);
                break;
            }
            case NUMBER:
            {
                current = NUMBER_NODE(token.value.number);
                if (current.error != TREE_SUCCESS)
                {
                    ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
                    return NULL;
                }
                break;
            }
            case OPERATOR:
            {
                current = OPERATOR_NODE(token.value.op);
                if (current.error != TREE_SUCCESS)
                {
                    ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
                    return NULL;
                }
                break;
            }
            case STATEMENT:
            {
                current = STATEMENT_NODE(token.value.stmt);
                if (current.error != TREE_SUCCESS)
                {
                    ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
                    return NULL;
                }
                if (token.value.stmt == OP_FUNC_DEF || token.value.stmt == OP_CALL)
                {
                    current.node->id.name = strdup(token.id.name);
                    free(token.id.name);
                }
                break;
            }
            case STRING:
            {
                current = STRING_NODE(token.value.string_value);
                if (current.error != TREE_SUCCESS)
                {
                    ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_CREATING_NODE_ERROR);
                    free((void*)token.value.string_value);
                    return NULL;
                }
                free((void*)token.value.string_value);  
                token.value.string_value = NULL;
                break;
            }
            default:
            {
                ERROR_MESSAGE_FRONTEND(INVALID_NODE);
                return NULL;
                break;
            }
        }
        if (current.error == TREE_SUCCESS)
        {    
            DEBUG_PRINT("[DEBUG] node has been succesfully created: %s", get_string_type(current.node->type));
            tree->tree_size++;
        }
        else 
        {
            ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_LOADING_EXPRESSION_ERROR);
            return NULL;
        }

        if (current.node->is_unary)
        {
            current.node->left = read_node(buffer, pos, tree);
            current.node->right = NULL;
        }
        else
        {
            current.node->left = read_node(buffer, pos, tree);
            current.node->right = read_node(buffer, pos, tree);
        }

        while (isspace(buffer[*pos]))                             
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


void savenode(Node_t *node, FILE *f)
{
    if (!node)
    {
        fprintf(f, "nil");
        return;
    }

    fprintf(f, "(");

    switch (node->type)
    {
        case NUMBER:
            fprintf(f, "%g", node->value.number);
            break;

        case IDENTIFIER:
            fprintf(f, "%s##%d", node->id.name, node->id.id_index);
            break;

        case OPERATOR:
            fprintf(f, "%s", get_string_operator(node->value.op));
            break;

        case STATEMENT:
            if (node->value.stmt == OP_FUNC_DEF || node->value.stmt == OP_CALL)
                fprintf(f, "%s##%s", get_statement_name(node->value.stmt), node->id.name);
            else
                fprintf(f, "%s", get_statement_name(node->value.stmt));
            break;

        case STRING:
            fprintf(f, "\"%s\"", node->value.string_value);
            break;

        default:
            fprintf(f, "UNKNOWN");
    }

    if (!node -> left && !node -> right)
        fprintf(f, " nil nil");

    if (node -> left)
        savenode(node->left, f);
    if (node -> right)
        savenode(node->right, f);

    fprintf(f, ")");
}



optimize_err save_tree(Tree_t * tree, const char * filename)
{
    assert(tree && filename);
    optimize_err error = OPTIMIZER_SUCCESS;

    FILE * file_ptr = fopen(filename, "w");
    if (!file_ptr)
    {
        ERROR_MESSAGE_OPTIMIZER(OPTIMIZER_LOADING_EXPRESSION_ERROR);
        return OPTIMIZER_LOADING_EXPRESSION_ERROR;
    }

    savenode(tree->root->right, file_ptr);

    fclose(file_ptr);
    DEBUG_PRINT("[INFO] TREE SAVED IN %s", filename);
    return error;
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

    token_res result = {.type = ROOT};

    while (isspace(buffer[*pos]))
        (*pos)++;

    char * token = get_token(buffer, pos);

    size_t len = strlen(token);

    if (!token || token[0] == '\0')
    {
        free(token);
        return result;
    }

    static check_func_t check_functions[] = 
    {
        check_for_statement,
        check_for_operator,
        check_for_number,
        check_for_string,
        check_for_identifier,
        NULL
    };

    for (int i = 0; check_functions[i] != NULL; i++)
    {
        result = check_functions[i](token);
        if (result.type != ROOT)
        {
            free(token);
            return result;
        }
    }

    *(pos) += len;

    free(token);
    return result;
}

