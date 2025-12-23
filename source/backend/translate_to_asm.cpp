#include <assert.h>
#include <stdio.h>

#include "errors.h"
#include "tree_structure.h"
#include "stack.h"
#include "spu.h"
#include "assembler.h"
#include "errors_spu.h"
#include "read_file.h"
#include "lexer.h"
#include "translate_to_asm.h"

            

ErrorCode translate_to_asm(Tree_t * tree, const char * filename)
{
    assert(tree && filename);
    ErrorCode error = SUCCESS; 

    FILE * file_ptr = fopen(filename, "w");
    if (!file_ptr)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }

    error = translate_node(tree->root, file_ptr);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(TRANSLATING_TO_ASM_ERROR, error);
        fclose(file_ptr);
        return error;
    }
    fprintf(file_ptr, "hlt\n");
    fclose(file_ptr);
    return SUCCESS;
}

ErrorCode translate_node(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

    switch (node->type)
    {
        case ROOT:
        {
            if (node->left)
            {
                error = translate_node(node->left, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            if (node->right)
            {
                error = translate_node(node->right, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            break;
        }
        case OPERATOR:      
        {
            error = translate_operator(node, file_ptr); 
            IF_THERE_IS_TRANSLATE_ERROR(error);  
            break;
        }
        case IDENTIFIER:
        {
            int index = get_id_address(get_id_name(node->value.id_index));
            if (index < 0)      
                error = TRANSLATING_TO_ASM_ERROR;    
            IF_THERE_IS_TRANSLATE_ERROR(error);
            fprintf(file_ptr, "pushm [%d]\n", index);
            break;
        }
        case NUMBER:
        {
            fprintf(file_ptr, "push %lg\n", node->value.number);
            break;
        }
        case STATEMENT:     
        {
            error = translate_statement(node, file_ptr); 
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        case STRING: break;
        default: 
        {
            ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, error);
            return error;
        }
    }

    return SUCCESS;
}


int get_id_address(const char * name)
{
    assert(name);

    int index = symbol_table_find(name);
    if (index < 0)
    {
        ErrorCode error = SUCCESS;
        ERROR_MESSAGE(TRANSLATING_TO_ASM_ERROR, error);
        return -1;
    }

    return index;
}

ErrorCode translate_operator(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

    operator_t op = node->value.op;
    
    if (is_binary_operator(op))
    {
        error = translate_node(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        error = translate_node(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        switch (op)
        {
            case OP_ADD:    fprintf(file_ptr, "add\n"); break;
            case OP_SUB:    fprintf(file_ptr, "sub\n"); break;
            case OP_MUL:    fprintf(file_ptr, "mul\n"); break;
            case OP_DIV:    fprintf(file_ptr, "div\n"); break;
            case OP_POW:    
            {
                // реализация возведения в степень
                break;
            }
            default:
            {
                ERROR_MESSAGE(TRANSLATING_TO_ASM_ERROR, error);
                return error;
            }
        }
        return SUCCESS;
    }

    if (is_unary_operator(op))
    {
        error = translate_node(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        
        switch (op)
        {
            case OP_UNARY_MINUS:
            {
                fprintf(file_ptr, "push 0\n");
                fprintf(file_ptr, "sub\n");
                break;
            }
            case OP_SQRT: fprintf(file_ptr, "sqrt\n"); break;
            case OP_ABS:  /* abs */ break;
            default:
                ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
                return error;
        }
        return SUCCESS;
    }

    ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
    return error;
}




ErrorCode translate_statement(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

    switch (node->value.stmt)
    {
        case OP_PROGRAM:
        case OP_STATEMENT:
        case OP_END:
        {
            error = translate_node(node->left,  file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            error = translate_node(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        case OP_PRINT:
        {
            error = translate_node(node->right, file_ptr);              // строки??
            IF_THERE_IS_TRANSLATE_ERROR(error);
            fprintf(file_ptr, "out\n");
            break;
        }
        case OP_ASSIGNMENT:
        {
            error = translate_node(node->right, file_ptr);
            int index = get_id_address(get_id_name(node->left->value.id_index));
            if (index < 0)      error = TRANSLATING_TO_ASM_ERROR;
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "popm [%d]\n", index);
            break;
        }
        case OP_IF:
        {
            static int label_id = 0;
            int cur = label_id++;

            error = translate_node(node->left, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "push 0\n");
            fprintf(file_ptr, "je endif_%d\n", cur);

            error = translate_node(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "endif_%d:\n", cur);
            break;
        }
        case OP_WHILE:
        {
            static int label_id = 0;
            int cur = label_id++;

            fprintf(file_ptr, "while_%d\n", cur);

            error = translate_node(node->left, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "push 0\n");
            fprintf(file_ptr, "je endwhile_%d\n", cur);

            error = translate_node(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "jmp while_%d\n", cur);
            fprintf(file_ptr, "endwhile_%d:\n", cur);
            break;
        }
        case OP_BLOCK:
        {
            error = translate_node(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        default: break;
    }
    return SUCCESS;
}

