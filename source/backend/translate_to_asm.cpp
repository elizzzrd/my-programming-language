#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "errors.h"
#include "tree_structure.h"
#include "stack.h"
#include "spu.h"
#include "assembler.h"
#include "errors_spu.h"
#include "read_file.h"
#include "lexer.h"
#include "translate_to_asm.h"
#include "utils.h"

static int string_top = 256; // где в RAM начинаются строки
function_info_t functions[128];
int function_count = 0;


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

    DEBUG_PRINT("[INFO] TRANSLATION START\n");
    if (tree->root)
    {
        fprintf(file_ptr, "jmp MAIN\n\n");

        error = translate_functions(tree->root, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "\nMAIN:\n");
        error = translate_main(tree->root, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "hlt\n");

        fclose(file_ptr);
        DEBUG_PRINT("[DEBUG] TRANSLATION COMPLETED");
        return SUCCESS;
    }
    else
    {
        fclose(file_ptr);
        DEBUG_PRINT("[DEBUG] ERROR DURING TRANSLATION");
        return TRANSLATING_TO_ASM_ERROR;
    }
}

ErrorCode translate_functions(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)
        return SUCCESS;

    ErrorCode error = SUCCESS;

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
    {
        error = translate_statement(node, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        return SUCCESS;
    }

    error = translate_functions(node->left, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    error = translate_functions(node->right, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    return SUCCESS;
}

ErrorCode translate_main(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)
        return SUCCESS;

    ErrorCode error = SUCCESS;

    if (node->type == ROOT)
    {
        error = translate_main(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        error = translate_main(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        return SUCCESS;
    }

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
        return SUCCESS;
    
    if (node->type == STATEMENT && (node->value.stmt == OP_END ||
                                    node->value.stmt == OP_STATEMENT))
    {
        error = translate_main(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        error = translate_main(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        return SUCCESS;
    }

    error = translate_node(node, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    return SUCCESS;
}


ErrorCode translate_node(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)
        return TREE_NULL_POINTER;
    ErrorCode error = SUCCESS;


    DEBUG_PRINT("[DEBUG] translate_node: type = %s", get_string_type(node->type));
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
            DEBUG_PRINT("%s -- %d\n", node->id.name, node->id.id_index);
            int index = node->id.id_index;
            if (index < -1)      
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
        case STRING: 
        {
            error = translate_string(node, file_ptr); 
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        default: 
        {
            ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, error);
            return error;
        }
    }

    return SUCCESS;
}



ErrorCode translate_operator(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

    operator_t op = node->value.op;
    
    if (is_binary_operator(op))
    {
        if (node->left)
            error = translate_node(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        if (node->right)
            error = translate_node(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        switch (op)
        {
            case OP_ADD:            fprintf(file_ptr, "add\n"); break;
            case OP_SUB:            fprintf(file_ptr, "sub\n"); break;
            case OP_MUL:            fprintf(file_ptr, "mul\n"); break;
            case OP_DIV:            fprintf(file_ptr, "div\n"); break;
            case OP_POW:            emit_op_pow(file_ptr); break;

            case OP_EQUAL:          emit_cmp(file_ptr, "je"); break;
            case OP_NON_EQUAL:      emit_cmp(file_ptr, "jne"); break;
            case OP_BELOW:          emit_cmp(file_ptr, "jb"); break;
            case OP_BELOW_EQUAL:    emit_cmp(file_ptr, "jbe"); break;
            case OP_ABOVE:          emit_cmp(file_ptr, "ja"); break;
            case OP_ABOVE_EQUAL:    emit_cmp(file_ptr, "jae"); break;

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
        if (node->left)
            error = translate_node(node->left, file_ptr);
        else
            error = translate_node(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        
        switch (op)
        {
        case OP_READ:
        {
            fprintf(file_ptr, "in\n");
            if (node->left && node->left->type == IDENTIFIER)
            {
                int addr = node->left->id.id_index;
                fprintf(file_ptr, "popm [%d]\n", addr);
            }
            break;
        }
            case OP_UNARY_MINUS:
            {
                fprintf(file_ptr, "popr rax\n");
                fprintf(file_ptr, "push 0\n");
                fprintf(file_ptr, "push 1\n");
                fprintf(file_ptr, "sub\n"); 
                
                fprintf(file_ptr, "pushr rax\n");
                fprintf(file_ptr, "mul\n");
                break;
            }
            case OP_SQRT:   fprintf(file_ptr, "sqrt\n"); break;
            case OP_SIN:    fprintf(file_ptr, "sin\n");  break;
            case OP_COS:    fprintf(file_ptr, "cos\n");  break;
            case OP_TAN:    
            {
                fprintf(file_ptr, "popr rax\n");  
                fprintf(file_ptr, "pushr rax\n");
                fprintf(file_ptr, "sin\n");  
                fprintf(file_ptr, "pushr rax\n");
                fprintf(file_ptr, "cos\n");
                fprintf(file_ptr, "push 0\npopr rax\n");
                fprintf(file_ptr, "div\n");
                break;
            }
            // case OP_CTG:
            // {
            //     fprintf(file_ptr, "popr rax\n");  
            //     fprintf(file_ptr, "pushr rax\n");
            //     fprintf(file_ptr, "cos\n");  
            //     fprintf(file_ptr, "pushr rax\n");
            //     fprintf(file_ptr, "sin\n");
            //     fprintf(file_ptr, "push 0\npopr rax\n");
            //     fprintf(file_ptr, "div\n");
            //     break;
            // }
            // case OP_ARCTAN: fprintf(file_ptr, "arctg\n"); break;
            case OP_EXP:    fprintf(file_ptr, "exp\n");  break;
            case OP_LN:     fprintf(file_ptr, "ln\n");   break;
            default:
                ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
                return error;
        }
        return SUCCESS;
    }

    ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
    return error;
}


ErrorCode translate_string(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);

    int addr = emit_strings(file_ptr, node->value.string_value);

    fprintf(file_ptr, "push %d\n", addr);
    return SUCCESS;
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
            Node_t * arg = NULL;
            if (node->left)
                arg = node->left;
            else if (node->right)
                arg = node->right;

            error = translate_node(arg, file_ptr);              
            IF_THERE_IS_TRANSLATE_ERROR(error);
            
            if (arg->type == STRING)
                fprintf(file_ptr, "puts\n");
            else
                fprintf(file_ptr, "out\n");
            break;
        }
        case OP_ASSIGNMENT:
        case OP_VAR_DEF:
        {
            error = translate_node(node->right, file_ptr);
            int index = node->left->id.id_index;
            if (index < 0)      
                error = TRANSLATING_TO_ASM_ERROR;
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "popm [%d]\n", index);
            break;
        }
        case OP_IF:
        {
            static int label_id = 0;
            int cur = label_id++;

            Node_t * condition = node->left;
            Node_t * stmt = node->right;

            Node_t * if_body = stmt->left;
            Node_t * else_body = stmt->right; 

            error = translate_node(condition, file_ptr); // condition
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "push 0\n");

            if (else_body)
                fprintf(file_ptr, "je ELSE_%d\n", cur);     
            else
                fprintf(file_ptr, "je ENDIF_%d\n", cur);    

            error = translate_node(if_body, file_ptr); // if_body
            IF_THERE_IS_TRANSLATE_ERROR(error);

            if (else_body)
            {
                fprintf(file_ptr, "jmp ENDIF_%d\n", cur);
                fprintf(file_ptr, "ELSE_%d:\n", cur);

                error = translate_node(else_body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }

            fprintf(file_ptr, "ENDIF_%d:\n", cur);
            break;
        }
        case OP_WHILE:
        {
            static int label_id = 0;
            int cur = label_id++;

            fprintf(file_ptr, "WHILE_%d:\n", cur);

            error = translate_node(node->left, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "push 0\n");
            fprintf(file_ptr, "je ENDWHILE_%d\n", cur);

            error = translate_node(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "jmp WHILE_%d\n", cur);
            fprintf(file_ptr, "ENDWHILE_%d:\n", cur);
            break;
        }
        case OP_BLOCK:
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
        case OP_FUNC_DEF:
        {
            fprintf(file_ptr, "%s:\n", node->id.name);

            Node_t * params = node -> left;
            int param_count = 0;
            Node_t * param_list[16];

            if (params && params->type == STATEMENT && params->value.stmt == OP_PARAMS)
            {
                Node_t * param = params->left;
                while (param && param_count < 16)
                {
                    if (param->type == IDENTIFIER)      
                        param_list[param_count++] = param;
                    param = param -> left;
                }
            }

            // pop arguments from stack and assign them to params
            for (int i = param_count - 1; i >= 0; i--)
            {
                int param_index = param_list[i]->id.id_index;
                if (param_index < 0)
                {
                    ERROR_MESSAGE(TRANSLATING_TO_ASM_ERROR, error);
                    return error;
                }
                fprintf(file_ptr, "popm [%d]\n", param_index);
            }

            Node_t * body = node -> right;
            if (body)
            {
                error = translate_node(body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }

            fprintf(file_ptr, "push 0\n");
            fprintf(file_ptr, "ret\n");
            break;
        }
        case OP_CALL:
        {
            Node_t * args = node->left;
            int arg_count = 0;
            Node_t * arg_list[16];

            if (args && args->type == STATEMENT && args->value.stmt == OP_ARGS)
            {
                Node_t * arg = args -> left;
                while (arg && arg_count < 16)
                {
                    arg_list[arg_count++] = arg;
                    arg = arg->left;
                }
            }
            // evalute args for pushing in stack
            for (int i = 0; i < arg_count; i++)
            {
                error = translate_node(arg_list[i], file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            
            fprintf(file_ptr, "call %s\n", node->id.name);
            break;
        }
        case OP_RETURN:
        {
            Node_t * expr = node -> left;
            if (expr)
            {
                error = translate_node(expr, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            else
            {
                // no return value
                fprintf(file_ptr, "push 0\n");
            }
            fprintf(file_ptr, "ret\n");
            break;
        }
        default: break;
    }
    return SUCCESS;
}


  

int emit_strings(FILE * file_ptr, const char * s)
{
    assert(file_ptr && s);

    int addr = string_top;

    for (size_t i = 0; s[i]; i++)
    {
        fprintf(file_ptr, "push %d\n", (unsigned char)s[i]);
        fprintf(file_ptr, "popm [%d]\n", string_top++);
    }

    fprintf(file_ptr, "push 0\n");
    fprintf(file_ptr, "popm [%d]\n", string_top++);

    return addr;
}

void emit_cmp(FILE * file_ptr, const char * jmp)
{
    assert(file_ptr && jmp);

    static int cmp_labels = 0;
    int id = cmp_labels++;

    fprintf(file_ptr, "sub\n");
    fprintf(file_ptr, "push 0\n");
    fprintf(file_ptr, "%s TRUE_CONDITION_%d\n", jmp, id);
    fprintf(file_ptr, "push 0\n");
    fprintf(file_ptr, "jmp END_CONDITION_%d\n", id);
    fprintf(file_ptr, "TRUE_CONDITION_%d:\n", id);
    fprintf(file_ptr, "push 1\n");
    fprintf(file_ptr, "END_CONDITION_%d:\n", id);
}

void emit_op_pow(FILE * file_ptr)
{
    assert(file_ptr);

    static int pow_labels = 0;
    int id = pow_labels++;

    fprintf(file_ptr,     
    "popr rbx\npopr rax\n"
    "pushr rax\n"
    "push 0\n"
    "jbe NEGATIVE_BASE_%d\n\n"
    "pushr rbx\n"
    "push 0\n"
    "je POWER_ZERO_%d\n\n"
    "push 1\n"
    "popr rdx\n"
    "push 0\n"
    "popr rcx\n\n"
    "START_LOOP_%d:\n"
    "pushr rcx\n"
    "pushr rbx\n"
    "jae END_LOOP_%d\n\n"
    "pushr rdx\n"
    "pushr rax\n"
    "mul\n"
    "popr rdx\n\n"
    "pushr rcx\n"
    "push 1\n"
    "add\n"
    "popr rcx\n\n"
    "jmp START_LOOP_%d\n\n"
    "END_LOOP_%d:\n"
    "pushr rdx\n"
    "jmp END_%d\n\n"
    "NEGATIVE_BASE_%d:\n"
    "pushr rbx\n"
    "push 0\n"
    "je POWER_ZERO_%d\n\n"
    "push 0\n"
    "jmp END_%d\n\n"
    "POWER_ZERO_%d:\n"
    "push 1\n"
    "jmp END_%d\n\n"
    "END_%d:\n"
    //"out\n"
    "push 0\npopr rax\npush 0\npopr rbx\npush 0\npopr rcx\npush 0\npopr rdx\n",
    id, id, id, id, id, id, id, id, id, id, id, id, id);
}

