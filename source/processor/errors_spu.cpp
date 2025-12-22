#include <stdio.h>
#include "stack.h"
#include "spu.h"
#include "errors_spu.h"

FILE * log_file = NULL;

FILE * log_init(const char * filename)
{
    log_file = fopen(filename, "a+");
    if (log_file) return log_file;
    else return NULL;
}

void log_close(void)
{
    if (log_file)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_message(const char * format, const char * file, int line)
{
    fprintf(log_file, "[%s %d]: %s\n", file, line, format);
    fprintf(log_file, "\n");
}

Stack_Err stack_verify(const stack_t * stack) 
{
    Stack_Err errors = STACK_OK;

    if (stack == NULL)                                          errors |= STACK_NULL_PTR;
    if (stack -> data == NULL)                                  errors |= STACK_UNINITIALIZED;

    if (stack -> left_canary != CANARY_LEFT_VALUE ||
        stack -> right_canary != CANARY_RIGHT_VALUE)            errors |= STACK_CANARY_CORRUPTED;
    if (stack->data[-1] != CANARY_LEFT_VALUE || 
        stack->data[stack->capacity] != CANARY_RIGHT_VALUE)     errors |= STACK_CANARY_CORRUPTED;

    if (stack->capacity == 0)                                   errors |= STACK_INVALID_CAPACITY;

    return errors; 
}

Spu_Err spu_verify(const spu_t *spu)
{
    if (spu == NULL)
        return SPU_NULL_PTR;

    Spu_Err err = SPU_OK;

    if (spu->code == NULL || spu->code_size == 0)
        err |= SPU_INVALID_CODE;

    if (spu->instructor_ptr > spu->code_size)
        err |= SPU_INVALID_IP;

    if (stack_verify(&(spu->stack)) != STACK_OK)
        err |= SPU_STACK_ERROR;

    return err;
}

const char * stack_error_string(Stack_Err error) 
{
    static const char * stack_error_strings[] = 
    {
        "Success",
        "Memory allocation failed",
        "Null pointer provided", 
        "Stack is uninitialized",
        "Stack overflow",
        "Stack underflow",
        "Invalid capacity",
        "Invalid size",
        "Stack canary corrupted"
    };
    
    if (error < STACK_OK || error > STACK_CANARY_CORRUPTED) return "Unknown error";
    
    return stack_error_strings[error];
}


void stack_dump(const stack_t * stack, Stack_Err error, const char * file, int line)
{
    fprintf(log_file, "============STACK DUMP============\n");
    fprintf(log_file, "\nstack_dump called from %s : %d\n", file, line);
    fprintf(log_file, "Error: %s (%u)\n", stack_error_string(error), error);

    if (stack == NULL) 
    {
        fprintf(log_file, "Stack pointer is NULL\n");
        return;
    }
    
    fprintf(log_file, "Stack [%p]\n", stack);
    fprintf(log_file, "{\n\tSize = %zu\n", stack->size);
    fprintf(log_file, "\tCapacity = %zu\n}\n", stack->capacity);
    fprintf(log_file, "data [%p]\n", (void*)stack->data);
    fprintf(log_file, "{\n");

    if (stack->data == NULL) 
    {
        fprintf(log_file, "Data array is NULL\n");
        return;
    }
    
    if (stack->size == 0) 
    {
        fprintf(log_file, "\t[empty]\n}\n");
    } 
    else 
    {
        size_t i = 0;
        for (i = 0; i < stack -> size; i++)
        {
            fprintf(log_file, "\t*[%zu] = %d\n", i, stack -> data[i]);
        }
        for (; i < stack->capacity; i++)
        {
            fprintf(log_file, "\t [%zu] = RUBBISH\n", i);
        }
        fprintf(log_file, "}\n");
    }
    fprintf(log_file, "============END STACK DUMP============\n");
}




void spu_dump(const spu_t * spu, Spu_Err err, const char * file, int line)
{
    fprintf(log_file, "============SPU DUMP============\n");
    fprintf(log_file, "SPU DUMP called from %s:%d\n", file, line);
    fprintf(log_file, "SPU ERROR: %s (%u)\n", spu_error_string(err), err);

    if (spu == NULL) {
        fprintf(log_file, "SPU pointer is NULL\n");
        return;
    }

    fprintf(log_file, "\nSPU [%p]\n", spu);
    fprintf(log_file, "{\n");
    fprintf(log_file, "    code_size:        %zu\n", spu->code_size);
    fprintf(log_file, "    code_size:        %zu\n", spu->code_size);
    fprintf(log_file, "    instructor_ptr:  %zu\n", spu->instructor_ptr);

    fprintf(log_file, "code [%p]\n", spu -> code);
    fprintf(log_file, "{\n");

    if (spu -> code == NULL) 
    {
        fprintf(log_file, "Code array is NULL\n");
        return;
    }
    
    if (spu -> code_size == 0) 
    {
        fprintf(log_file, "\t[empty]\n}\n");
    } 
    else 
    {
        size_t i = 0;
        for (i = 0; i < (spu -> code_size); i++)
        {
            fprintf(log_file, "\t*[%zu] = %d\n", i, spu -> code[i]);
        }
        fprintf(log_file, "}\n");
    }

    fprintf(log_file, "\n    Registers:\n");
    for (size_t i = 0; i < REGS_COUNT; i++)
        fprintf(log_file, "        R[%zu] = %d\n", i, spu->regs[i]);

    fprintf(log_file, "\n    Stack state:\n");
    stack_dump(&(spu->stack), STACK_OK, file, line);

    fprintf(log_file, "}\n========================================\n\n");
}

const char * spu_error_string(Spu_Err err)
{
    static const char * spu_error_strings[] =
    {
        "OK",
        "Spu's stack error",
        "Unknown command",
        "Division by zero",
        "File error",
        "SPU is null pointer",
        "Invalid command",
        "Invalid code",
        "Invalid instructor pointer"
    };

    if (err < SPU_OK || err > SPU_INVALID_IP) return "Unknown error";
    
    return spu_error_strings[err];
}


void finish_program(spu_t * spu)
{
    log_close();
    spu_destroy(spu);
    printf("Programm is finished\n");
}