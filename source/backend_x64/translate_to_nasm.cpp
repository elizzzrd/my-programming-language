#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "errors.h"
#include "tree_structure.h"
#include "utils.h"
#include "translate_to_nasm.h"

//--------------------------------------------------------------
#define MAX_CONSTANTS 1024
static double constants[MAX_CONSTANTS];
static int const_count = 0;

#define MAX_VARS 256
typedef struct 
{
    char * name;
    int offset;
    bool initialized;
} var_info_t;

static var_info_t variables[MAX_VARS];
static int var_count = 0;

static int label_counter = 0;
static int total_frame_size = 0; 

//--------------------------------------------------------------
static int add_constant(double val)
{
    for (int i = 0; i < const_count; i++)
        if (fabs(constants[i] - val) < 1e-12)
            return i;
        
    if (const_count >= MAX_CONSTANTS)
        return -1;

    constants[const_count++] = val;
    return const_count - 1;
}

static int new_label(void)
{
    return label_counter++;
}

//--------------------------------------------------------------
static int find_variable(const char * name)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(variables[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int add_variable(const char * name)
{
    int idx = find_variable(name);
    if (idx != -1)
        return variables[idx].offset;

    if (var_count >= MAX_VARS)
        return -1;
    
    int offset = -(var_count + 1)*8;
    variables[var_count].name = strdup(name);
    variables[var_count].offset = offset;
    variables[var_count].initialized = false;

    var_count++;
    total_frame_size += 8;

    return offset;
}

static var_info_t * get_variable_by_name(const char * name)
{
    int idx = find_variable(name);
    return (idx == -1) ? NULL: &variables[idx];
}

static void clear_variables(void)
{
    for (int i = 0; i < var_count; i++)
        free(variables[i].name);
    
    var_count = 0;
    total_frame_size = 0;
}
//--------------------------------------------------------------


ErrorCode translate_to_nasm(Tree_t * tree, const char * filename)
{
    assert(tree && filename);
    ErrorCode error = SUCCESS; 

    const_count = 0;
    clear_variables();
    label_counter = 0;
    
    FILE * file_ptr = fopen(filename, "w");
    if (!file_ptr)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }
    DEBUG_PRINT("[INFO] TRANSLATION START\n");

    fprintf(file_ptr, "default rel\n");
    fprintf(file_ptr, "global main\n\n");
    fprintf(file_ptr, "%%include \"mystdlib.asm\"\n"); 

    fprintf(file_ptr, "section .text\n\n");
    fprintf(file_ptr, "main:\n");
    fprintf(file_ptr, "    push rbp\n");
    fprintf(file_ptr,"     mov rbp, rsp\n");
    
    int frame_size = total_frame_size + 32;
    if (frame_size > 0)
        fprintf(file_ptr, "    sub rsp, %d\n", frame_size);

    if (tree->root && tree->root->right)
    {
        error = emit_program(tree->root->right, file_ptr);
        if (error != SUCCESS)
        {
            fclose(file_ptr);
        DEBUG_PRINT("[DEBUG] ERROR DURING TRANSLATION");
        return TRANSLATING_TO_ASM_ERROR;
        }
    }

    fprintf(file_ptr, "    xor eax, eax\n");
    fprintf(file_ptr, "    mov rsp, rbp\n");
    fprintf(file_ptr, "    pop rbp\n");
    fprintf(file_ptr, "    ret\n\n");


    fprintf(file_ptr, "section .data\n");
    fprintf(file_ptr, "fmt_print: db \"%%.10g\\n\", 0\n");
    fprintf(file_ptr, "fmt_scan: db \"%%lf\", 0\n");
    fprintf(file_ptr, "\n");

    if (const_count > 0)
    {
        for (int i = 0; i < const_count; i++)
            fprintf(file_ptr, "const_%d dq %g\n", i, constants[i]);
        fprintf(file_ptr, "\n");
    }


    DEBUG_PRINT("[DEBUG] TRANSLATION COMPLETED");
    return SUCCESS;
}


ErrorCode emit_program(Node_t * node, FILE * file_ptr)
{
    if (!node)
        return SUCCESS;
    ErrorCode error = SUCCESS;

    if (node->type == ROOT)
    {
        if (node->left)
        {
            error = emit_program(node->left, file_ptr);
            if (error != SUCCESS)
                return error;
        }
        if (node->right)
        {
            error = emit_program(node->right, file_ptr);
            if (error != SUCCESS)
                return error;
        }
    }
    return emit_statement(node, file_ptr);
}



ErrorCode emit_expression(Node_t * node, FILE * file_ptr)
{
    assert(file_ptr);
    if (!node)
        return TREE_NULL_POINTER;
    ErrorCode error = SUCCESS;


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
            var_info_t * var = get_variable_by_name(node->id.name);
            if (!var)      
                error = SEMANTIC_ERROR;    
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "    movsd xmm0, [rbp%+d]\n", var->offset);
            break;
        }
        case NUMBER:
        {
            int idx = add_constant(node->value.number);
            if (idx < 0)
                return TREE_MEMORY_ALLOCATION_ERROR;
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
            ERROR_MESSAGE(TREE_INVALID_NODE_TYPE, error);
            return error;
        }
    }

    return SUCCESS;
}


ErrorCode emit_operator(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

    operator_t op = node->value.op;
    
    if (is_binary_operator(op) && op != OP_READ)
    {
        if (node->left)
            error = emit_expression(node->left, file_ptr);     // xmm1
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "    sub rsp, 8\n");
        fprintf(file_ptr, "    movsd [rsp], xmm0\n");

        if (node->right)
            error = emit_expression(node->right, file_ptr);    // xmm0
        IF_THERE_IS_TRANSLATE_ERROR(error);

        fprintf(file_ptr, "    movsd xmm1, [rsp]\n");
        fprintf(file_ptr, "    add rsp, 8\n");

        switch (op)
        {
            case OP_ADD:            fprintf(file_ptr, "    addsd xmm1, xmm0\n"); break;
            case OP_SUB:            fprintf(file_ptr, "    subsd xmm1, xmm0\n"); break;
            case OP_MUL:            fprintf(file_ptr, "    mulsd xmm1, xmm0\n"); break;
            case OP_DIV:            fprintf(file_ptr, "    divsd xmm1, xmm0\n"); break;
            case OP_POW:            fprintf(file_ptr, "    movapd xmm0, xmm1\n");
                                    fprintf(file_ptr, "    call pow\n"); break;

            case OP_EQUAL:          
            case OP_NON_EQUAL:      
            case OP_BELOW:          
            case OP_BELOW_EQUAL:    
            case OP_ABOVE:          
            case OP_ABOVE_EQUAL:    emit_cmp_x64(node, file_ptr, op); break;

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
        Node_t * operand = (node->left) ? (node->left) : (node->right);
        error = emit_expression(operand, file_ptr);
        IF_THERE_IS_TRANSLATE_ERROR(error);
        
        switch (op)
        {
        case OP_READ:
        {
            fprintf(file_ptr, "    call read_double\n");
            if (node->left && node->left->type == IDENTIFIER)
            {
                var_info_t * var = get_variable_by_name(node->left->id.name);
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
                ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
                return error;
        }
        return SUCCESS;
    }

    ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);
    return error;
}


ErrorCode emit_cmp_x64(Node_t * node, FILE * file_ptr, operator_t op)
{
    assert(node && file_ptr);
    ErrorCode error = emit_expression(node->left, file_ptr);
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

    return SUCCESS;
}


ErrorCode emit_statement(Node_t * node, FILE * file_ptr)
{
    assert(node && file_ptr);
    ErrorCode error = SUCCESS;

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
            Node_t * arg = NULL;
            if (node->left)
                arg = node->left;
            else if (node->right)
                arg = node->right;

            error = emit_expression(arg, file_ptr);              
            IF_THERE_IS_TRANSLATE_ERROR(error);
            
            // if (arg->type == STRING)
            //     fprintf(file_ptr, "puts\n");
            // else
            //     fprintf(file_ptr, "out\n");
            // break;

            fprintf(file_ptr, "    mov rdi, fmt_print\n");
            fprintf(file_ptr, "    mov al, 1\n");
            fprintf(file_ptr, "    call printf\n");

            break;
        }
        case OP_ASSIGNMENT:
        case OP_VAR_DEF:
        {
            error = emit_expression(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            int offset = add_variable(node->left->id.name);
            if (offset == -1)
                return SEMANTIC_ERROR;

            var_info_t * var = get_variable_by_name(node->left->id.name);
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

            Node_t * if_body = stmt->left;
            Node_t * else_body = stmt->right; 

            error = emit_expression(condition, file_ptr); // condition
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "    xorpd xmm1, xmm1\n");
            fprintf(file_ptr, "    comisd xmm0, xmm1\n");
            fprintf(file_ptr, "    je .L_else_%d\n", label_else);

            error = emit_expression(if_body, file_ptr); // if_body
            IF_THERE_IS_TRANSLATE_ERROR(error);

            if (else_body)
            {
                fprintf(file_ptr, "    jmp .L_end_%d\n", label_endif);
                fprintf(file_ptr, "    .L_else_%d:\n", label_else);

                error = emit_expression(else_body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);
            }

            fprintf(file_ptr, "    .L_end_%d:\n", label_endif);
            break;
        }
        case OP_WHILE:
        {
            int label_start = new_label();
            int label_end = new_label();

            fprintf(file_ptr, "    .L_while_start_%d:\n", label_start);

            error = emit_expression(node->left, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "    xorpd xmm1, xmm1\n");
            fprintf(file_ptr, "    comisd xmm0, xmm1\n");
            fprintf(file_ptr, "    je .L_while_end_%d\n", label_end);

            error = emit_expression(node->right, file_ptr);
            IF_THERE_IS_TRANSLATE_ERROR(error);

            fprintf(file_ptr, "    jmp .L_while_start_%d\n", label_start);
            fprintf(file_ptr, ".L_while_end_%d:\n", label_end);
            break;
        }
        case OP_BLOCK:
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
        case OP_FUNC_DEF:
        {
            fprintf(file_ptr, "%s:\n", node->id.name);
            fprintf(file_ptr, "    push rbp\n");
            fprintf(file_ptr, "    mov rbp, rsp\n");
            fprintf(file_ptr, "    sub rsp, 32\n");

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

            // the parameters are passed through xmm0, xmm1, xmm2, xmm3
            for (int i = 0; i < param_count && i < 4; i++)
            {
                int offset = -(i + 1) * 8;
                param_list[i]->id.id_index = offset;
                fprintf(file_ptr, "    movsd [rbp%+d], xmm%d\n", offset, i);
            }

            if (param_count > 4)
            {
                // TODO: processing parameters on the stack
            }

            Node_t * body = node->right;
            if (body)
            {
                int saved_var_count = var_count;
                var_count = param_count;
    
                error = emit_statement(body, file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);

                var_count = saved_var_count;
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

            // evalute args for pushing in xmm% and stack
            double temp_values[16];
            for (int i = 0; i < arg_count; i++)
            {
                error = emit_expression(arg_list[i], file_ptr);
                IF_THERE_IS_TRANSLATE_ERROR(error);

                if (i < 4)
                {
                    fprintf(file_ptr, "    movapd xmm%d, xmm0\n", i);
                }
                else
                {
                    fprintf(file_ptr, "    sub rsp, 8\n");
                    fprintf(file_ptr, "    movsd [rsp], xmm0\n");
                }
            }
            
            fprintf(file_ptr, "    call %s\n", node->id.name);

            if (arg_count > 4)
                fprintf(file_ptr, "    add rsp, %d\n", (arg_count - 4) * 8);

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
            
            fprintf(file_ptr, "    mov rsp, rbp\n");
            fprintf(file_ptr, "    pop rbp\n");
            fprintf(file_ptr, "    ret\n");
            break;
        }
        default: break;
    }
    return SUCCESS;
}



