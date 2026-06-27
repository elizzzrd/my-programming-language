#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "errors.h"
#include "tree_structure.h"
#include "translate_to_nasm.h"

//--------------------------------------------------------------
int label_counter = 0;
double constants[MAX_CONSTANTS];
int const_count = 0;

int add_constant(double val)
{    
    for (int i = 0; i < const_count; i++)
        if (fabs(constants[i] - val) < 1e-12)
            return i;
        
    if (const_count >= MAX_CONSTANTS)
        return -1;

    constants[const_count++] = val;
    return const_count - 1;
}

int new_label(void)
{
    return label_counter++;
}

//--------------------------------------------------------------
extern variables_t vars;


backend_err translate_to_nasm(Tree_t * tree, const char * filename)
{
    assert(tree && filename);
    backend_err error = BACKEND_SUCCESS; 

    init_variables();
    const_count = 0;
    label_counter = 0;

    DEBUG_PRINT("[DEBUG] variables initialized");

    vars.current_func_id = 0;                       // global area
    error = collect_variables(tree->root);
    if (error != BACKEND_SUCCESS)
        return error;

    DEBUG_PRINT("[DEBUG] valiables collected");
    assign_offset_for_function(0);
    DEBUG_PRINT("[DEBUG] offsets done");
    
    FILE * file_ptr = fopen(filename, "w");
    if (!file_ptr)
    {
        ERROR_MESSAGE_BACKEND(BACKEND_OPENING_FILE_ERROR);
        return BACKEND_OPENING_FILE_ERROR;
    }
    DEBUG_PRINT("[INFO] TRANSLATION START\n");

    fprintf(file_ptr, "default rel\n");
    fprintf(file_ptr, "%%include \"/home/gardina_elizaveta/projects/1sem/language/source/backend_x86_64/mystdlib.asm\"\n"); 

    fprintf(file_ptr, "section .text\n\n");
    vars.current_func_id = 1;
    error = emit_functions(tree->root->right, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    vars.current_func_id = 0;
    int frame_size = get_frame_size_for_func(0) + 32;
    frame_size = (frame_size + 15) & ~15;

    fprintf(file_ptr, "global main\n\n");
    fprintf(file_ptr, "main:\n");
    fprintf(file_ptr, "    push rbp\n");
    fprintf(file_ptr, "    mov rbp, rsp\n");
    
    if (frame_size > 0)
        fprintf(file_ptr, "    sub rsp, %d\n\n", frame_size);
    
    error = emit_main(tree->root->right, file_ptr);
    if (error != BACKEND_SUCCESS)
    {
        fclose(file_ptr);
        DEBUG_PRINT("[DEBUG] ERROR DURING TRANSLATION");
        return TRANSLATING_TO_ASM_ERROR;
    }
    
    fprintf(file_ptr, "\n    xor eax, eax\n");
    fprintf(file_ptr, "    mov rsp, rbp\n");
    fprintf(file_ptr, "    pop rbp\n");
    fprintf(file_ptr, "    ret\n");


    fprintf(file_ptr, "\nsection .data\n");
    fprintf(file_ptr, "fmt_print: db \"%%.3g\", 10, 0\n");
    fprintf(file_ptr, "fmt_scan: db \"%%lf\", 0\n");
    fprintf(file_ptr, "\n");

    if (const_count > 0)
    {
        for (int i = 0; i < const_count; i++)
            fprintf(file_ptr, "const_%d dq %f\n", i, constants[i]);
        fprintf(file_ptr, "\n");
    }


    destroy_variables();
    DEBUG_PRINT("[DEBUG] TRANSLATION COMPLETED");
    return BACKEND_SUCCESS;
}


backend_err emit_functions(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)      
        return BACKEND_SUCCESS;
    backend_err error = BACKEND_SUCCESS;

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
    {
        error = emit_statement(node, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        return error;
    }

    error = emit_functions(node->left, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    error = emit_functions(node->right, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    return BACKEND_SUCCESS;
}


backend_err emit_main(Node_t * node, FILE * file_ptr)
{
    if (!node)      
        return BACKEND_SUCCESS;
    backend_err error = BACKEND_SUCCESS;

    // if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
    //     return SUCCESS;

    // if (node->type == ROOT)
    // {
    //     if (node->left)
    //     {
    //         error = emit_main(node->left, file_ptr);
    //         IF_THERE_IS_TRANSLATE_ERROR(error);
    //     }
    //     if (node->right)
    //     {
    //         error = emit_main(node->right, file_ptr);
    //         IF_THERE_IS_TRANSLATE_ERROR(error);
    //     }
    //     return SUCCESS;
    // }


    if (node->type == ROOT)
    {
        error = emit_main(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        error = emit_main(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        return BACKEND_SUCCESS;
    }

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
        return BACKEND_SUCCESS;
    
    if (node->type == STATEMENT && (node->value.stmt == OP_END ||
                                    node->value.stmt == OP_STATEMENT))
    {
        error = emit_main(node->left, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        error = emit_main(node->right, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);

        return BACKEND_SUCCESS;
    }


    error = emit_statement(node, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    return BACKEND_SUCCESS;
}
    

backend_err emit_statement(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    backend_err error = BACKEND_SUCCESS;

    switch (node->value.stmt)
    {
        case OP_PROGRAM:
        case OP_STATEMENT:
        case OP_END:
        {
            error = emit_expression(node->left,  file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            error = emit_expression(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        case OP_PRINT:
        {
            Node_t * arg = (node->left) ? (node->left) : (node->right);

            error = emit_expression(arg, file_ptr);              
            IF_THERE_IS_TRANSLATE_ERROR(error);

            
            // if (arg->type == STRING)
            //     fprintf(file_ptr, "puts\n");
            // else
            //     fprintf(file_ptr, "out\n");
            // break;


            fprintf(file_ptr, "\n    mov rdi, fmt_print\n");
            fprintf(file_ptr, "    mov eax, 1\n");
            fprintf(file_ptr, "    call printf\n");

            break;
        }
        case OP_ASSIGNMENT:
        case OP_VAR_DEF:
        {
            error = emit_expression(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            var_info_t * var = get_variable_by_index(node->left->id.id_index);
            if (var)
            {
                var->initialized = true;
                fprintf(file_ptr, "    movsd [rbp%+d], xmm0\n", var->offset);
            }
            break;
        }
        case OP_IF:
        {
            int label_else = new_label();
            int label_endif = new_label();

            Node_t * condition = node->left;
            Node_t * stmt = node->right;
            assert(condition && stmt);

            Node_t * if_body = stmt->left;
            Node_t * else_body = stmt->right; 

            error = emit_expression(condition, file_ptr); // condition
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "    xorpd xmm1, xmm1\n");
            fprintf(file_ptr, "    comisd xmm0, xmm1\n");
            fprintf(file_ptr, "    je .L_else_%d\n", label_else);

            error = emit_statement(if_body, file_ptr); // if_body
            IF_THERE_IS_TRANSLATE_ERROR(error);

            if (else_body)
            {
                fprintf(file_ptr, "    jmp .L_end_%d\n", label_endif);
                fprintf(file_ptr, "    .L_else_%d:\n", label_else);

                error = emit_statement(else_body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            else
                fprintf(file_ptr, ".L_else_%d:\n", label_else);

            fprintf(file_ptr, ".L_end_%d:\n", label_endif);
            break;
        }
        case OP_WHILE:
        {
            int label_start = new_label();
            int label_end = new_label();

            fprintf(file_ptr, "    .L_while_start_%d:\n", label_start);

            error = emit_expression(node->left, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "\n    xorpd xmm1, xmm1\n");
            fprintf(file_ptr, "    comisd xmm0, xmm1\n");
            fprintf(file_ptr, "    je .L_while_end_%d\n", label_end);

            error = emit_statement(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "\n    jmp .L_while_start_%d\n", label_start);
            fprintf(file_ptr, ".L_while_end_%d:\n", label_end);
            break;
        }
        case OP_BLOCK:
        {
            if (node->left)
            {
                error = emit_statement(node->left, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            if (node->right)
            {
                error = emit_statement(node->right, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            break;
        }
        case OP_FUNC_DEF:
        {
            fprintf(file_ptr, "\n%s:\n", node->id.name);
            fprintf(file_ptr, "    push rbp\n");
            fprintf(file_ptr, "    mov rbp, rsp\n");

            int func_frame = get_frame_size_for_func(vars.current_func_id) + 32;
            func_frame = (func_frame + 15) & ~15;
            fprintf(file_ptr, "    sub rsp, %d\n", func_frame);

            Node_t * params = node -> left;
            int param_count = 0;

            if (params && params->type == STATEMENT && params->value.stmt == OP_PARAMS)
            {
                Node_t * param = params->left;
                while (param && param_count < 16)
                {
                    if (param->type == IDENTIFIER)      
                        if (param_count < 4)
                        {
                            var_info_t * var = get_variable_by_index(param->id.id_index);
                            if (var)
                                fprintf(file_ptr, "    movsd [rbp%+d], xmm%d\n", var->offset, param_count);
                        }
                        else
                        {
                             // TODO: processing parameters on the stack
                        }
                    param = param -> left;
                    param_count++;
                }
            }

            Node_t * body = node->right;
            if (body)
            {
                error = emit_statement(body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }


            fprintf(file_ptr, "    mov rsp, rbp\n");
            fprintf(file_ptr, "    pop rbp\n");
            fprintf(file_ptr, "    ret\n");

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

            int stack_args_count = (arg_count > 4) ? (arg_count - 4) : 0;
            if (stack_args_count > 0)
            {
                int stack_size = ((stack_args_count * 8 + 15) & ~ 15);
                fprintf(file_ptr, "    sub rsp, %d\n", stack_size);
            }

            for (int i = arg_count - 1; i >= 4; i--)
            {
                error = emit_expression(arg_list[i], file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);

                int offset = (i - 4) * 8;
                fprintf(file_ptr, "    movsd [rsp+%d], xmm0\n", offset);
            }

            for (int i = 3; i >= 0 && i < arg_count; i--)
            {
                error = emit_expression(arg_list[i], file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);

                fprintf(file_ptr, "    movapd xmm%d, xmm0\n", i);
            }

            fprintf(file_ptr, "    call %s\n", node->id.name);

            if (stack_args_count > 0)
            {
                int stack_size = ((stack_args_count * 8 + 15) & ~ 15);
                fprintf(file_ptr, "    add rsp, %d\n", stack_size);
            }
            break;
        }
        case OP_RETURN:
        {
            Node_t * expr = node -> left;
            if (expr)
            {
                error = emit_expression(expr, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            else
            {
                // no return value
                fprintf(file_ptr, "    xorpd xmm0, xmm0\n");
            }
            
            fprintf(file_ptr, "\n    mov rsp, rbp\n");
            fprintf(file_ptr, "    pop rbp\n");
            fprintf(file_ptr, "    ret\n");
            break;
        }
        default: break;
    }
    return BACKEND_SUCCESS;
}


backend_err emit_expression(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)
        return BACKEND_SUCCESS;
    backend_err error = BACKEND_SUCCESS;


    DEBUG_PRINT("[DEBUG] emit_expression: type = %s", get_string_type(node->type));
    switch (node->type)
    {
        case ROOT:
        {
            if (node->left)
            {
                error = emit_expression(node->left, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            if (node->right)
            {
                error = emit_expression(node->right, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }
            break;
        }
        case OPERATOR:      
        {
            error = emit_operator(node, file_ptr); 
            IF_THERE_IS_TRANSLATE_ERROR(error);  
            break;
        }
        case IDENTIFIER:
        {
            DEBUG_PRINT("%s -- %d\n", node->id.name, node->id.id_index);
            var_info_t * var = get_variable_by_index(node->id.id_index);
            if (!var)    
            {
                var = get_variable_by_name(node->id.name);
                if (!var)
                {
                    error = BACKEND_SEMANTIC_ERROR;    
                    IF_THERE_IS_TRANSLATE_ERROR(error);
                }
            }      
            fprintf(file_ptr, "    movsd xmm0, [rbp%+d]\n", var->offset);
            break;
        }
        case NUMBER:
        {
            int idx = add_constant(node->value.number);
            if (idx < 0)
                return BACKEND_ALLOCATION_ERROR;
            fprintf(file_ptr, "    movsd xmm0, [rel const_%d]\n", idx);
            break;
        }
        case STATEMENT:     
        {
            error = emit_statement(node, file_ptr); 
            IF_THERE_IS_TRANSLATE_ERROR(error);
            break;
        }
        // case STRING: 
        // {
        //     error = translate_string(node, file_ptr); 
        //     IF_THERE_IS_TRANSLATE_ERROR(error);
        //     break;
        // }
        default: 
        {
            ERROR_MESSAGE_BACKEND(BACKEND_INVALID_NODE);
            return error;
        }
    }

    return BACKEND_SUCCESS;
}


backend_err emit_operator(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    backend_err error = BACKEND_SUCCESS;

    operator_t op = node->value.op;
    
    if (is_binary_operator(op) && op != OP_READ)
    {
        if (node->left)
            error = emit_expression(node->left, file_ptr);     // xmm1
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "    sub rsp, 8\n");
        fprintf(file_ptr, "    movsd [rsp], xmm0\n\n");

        if (node->right)
            error = emit_expression(node->right, file_ptr);    // xmm0
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "    movsd xmm1, [rsp]\n");
        fprintf(file_ptr, "    add rsp, 8\n\n");

        switch (op)
        {
            case OP_ADD:            fprintf(file_ptr, "    addsd xmm1, xmm0\n"); break;
            case OP_SUB:            fprintf(file_ptr, "    subsd xmm1, xmm0\n"); break;
            case OP_MUL:            fprintf(file_ptr, "    mulsd xmm1, xmm0\n"); break;
            case OP_DIV:            fprintf(file_ptr, "    divsd xmm1, xmm0\n"); break;
            case OP_POW:
                                    fprintf(file_ptr, "    movapd xmm2, xmm0\n");
                                    fprintf(file_ptr, "    movapd xmm0, xmm1\n");
                                    fprintf(file_ptr, "    movapd xmm1, xmm2\n");
                                    fprintf(file_ptr, "    call pow\n");
                                    fprintf(file_ptr, "    movapd xmm1, xmm0\n");
                                    break;
                                
            case OP_EQUAL:          
            case OP_NON_EQUAL:      
            case OP_BELOW:          
            case OP_BELOW_EQUAL:    
            case OP_ABOVE:          
            case OP_ABOVE_EQUAL:    emit_cmp_x64(node, file_ptr, op); 
                                    fprintf(file_ptr, "    movapd xmm1, xmm0\n");
                                    break;
            default:
            {
                ERROR_MESSAGE_BACKEND(TRANSLATING_TO_ASM_ERROR);
                return TRANSLATING_TO_ASM_ERROR;
            }
        }
        fprintf(file_ptr, "    movapd xmm0, xmm1\n");
        return BACKEND_SUCCESS;
    }

    if (is_unary_operator(op))
    {
        Node_t * operand = (node->left) ? (node->left) : (node->right);
        error = emit_expression(operand, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        
        switch (op)
        {
        case OP_READ:
        {
            fprintf(file_ptr, "    call _my_read\n");
            if (node->left && node->left->type == IDENTIFIER)
            {
                var_info_t * var = get_variable_by_index(node->left->id.id_index);
                if (var)
                    fprintf(file_ptr, "    movsd [rbp%+d], xmm0\n", var->offset);
            }
            break;
        }
            case OP_UNARY_MINUS:
            {
                fprintf(file_ptr, "    xorpd xmm1, xmm1\n");
                fprintf(file_ptr, "    subsd xmm1, xmm0\n");
                fprintf(file_ptr, "    movapd xmm0, xmm1\n");
                break;
            }
            case OP_SQRT:   fprintf(file_ptr, "    call sqrt\n"); break;
            case OP_SIN:    fprintf(file_ptr, "    call sin\n");  break;
            case OP_COS:    fprintf(file_ptr, "    call cos\n");  break;
            case OP_TAN:    fprintf(file_ptr, "    call tan\n");  break;
            case OP_EXP:    fprintf(file_ptr, "    call exp\n");  break;
            case OP_LN:     fprintf(file_ptr, "    call log\n");   break;
            default:
                ERROR_MESSAGE_BACKEND(BACKEND_INVALID_OPERATOR);
                return error;
        }
        return BACKEND_SUCCESS;
    }

    ERROR_MESSAGE_BACKEND(BACKEND_INVALID_OPERATOR);
    return error;
}


backend_err emit_cmp_x64(Node_t * node, FILE * file_ptr, operator_t op)
{
    assert(node && file_ptr);
    backend_err error = emit_expression(node->left, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    fprintf(file_ptr, "    sub rsp, 8\n");
    fprintf(file_ptr, "    movsd [rsp], xmm0\n");

    error = emit_expression(node->right, file_ptr);
    IF_THERE_IS_TRANSLATE_ERROR(error);

    fprintf(file_ptr, "    movsd xmm1, [rsp]\n");
    fprintf(file_ptr, "    add rsp, 8\n");

    fprintf(file_ptr, "    comisd xmm1, xmm0\n");
    fprintf(file_ptr, "    mov eax, 0\n");

    const char * jcc = NULL;
    switch (op) 
    {
        case OP_EQUAL:       jcc = "sete"; break;
        case OP_NON_EQUAL:   jcc = "setne"; break;
        case OP_BELOW:       jcc = "setb"; break;
        case OP_BELOW_EQUAL: jcc = "setbe"; break;
        case OP_ABOVE:       jcc = "seta"; break;
        case OP_ABOVE_EQUAL: jcc = "setae"; break;
        default: break;
    }
    if (jcc) 
    {
        fprintf(file_ptr, "    %s al\n", jcc);
        fprintf(file_ptr, "    cvtsi2sd xmm0, rax\n");
    }

    return BACKEND_SUCCESS;
}
