#include <stdio.h>
#include "stack.h"
#include "spu.h"
#include "assembler.h"
#include "errors_spu.h"
#include "utils.h"


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
    DEBUG_PRINT("============STACK DUMP============\n");
    DEBUG_PRINT("\nstack_dump called from %s : %d\n", file, line);
    DEBUG_PRINT("Error: %s (%u)\n", stack_error_string(error), error);

    if (stack == NULL) 
    {
        DEBUG_PRINT("Stack pointer is NULL\n");
        return;
    }
    
    DEBUG_PRINT("Stack [%p]\n", stack);
    DEBUG_PRINT("{\n\tSize = %zu\n", stack->size);
    DEBUG_PRINT("\tCapacity = %zu\n}\n", stack->capacity);
    DEBUG_PRINT("data [%p]\n", (void*)stack->data);
    DEBUG_PRINT("{\n");

    if (stack->data == NULL) 
    {
        DEBUG_PRINT("Data array is NULL\n");
        return;
    }
    
    if (stack->size == 0) 
    {
        DEBUG_PRINT("\t[empty]\n}\n");
    } 
    else 
    {
        size_t i = 0;
        for (i = 0; i < stack -> size; i++)
        {
            DEBUG_PRINT("\t*[%zu] = %d\n", i, stack -> data[i]);
        }
        for (; i < stack->capacity; i++)
        {
            DEBUG_PRINT("\t [%zu] = RUBBISH\n", i);
        }
        DEBUG_PRINT("}\n");
    }
    DEBUG_PRINT("============END STACK DUMP============\n");
}




void spu_dump(const spu_t * spu, Spu_Err err, const char * file, int line)
{
    DEBUG_PRINT("============SPU DUMP============\n");
    DEBUG_PRINT("SPU DUMP called from %s:%d\n", file, line);
    DEBUG_PRINT("SPU ERROR: %s (%u)\n", spu_error_string(err), err);

    if (spu == NULL) {
        DEBUG_PRINT("SPU pointer is NULL\n");
        return;
    }

    DEBUG_PRINT("\nSPU [%p]\n", spu);
    DEBUG_PRINT("{\n");
    DEBUG_PRINT("    code_size:        %zu\n", spu->code_size);
    DEBUG_PRINT("    code_size:        %zu\n", spu->code_size);
    DEBUG_PRINT("    instructor_ptr:  %zu\n", spu->instructor_ptr);

    DEBUG_PRINT("code [%p]\n", spu -> code);
    DEBUG_PRINT("{\n");

    if (spu -> code == NULL) 
    {
        DEBUG_PRINT("Code array is NULL\n");
        return;
    }
    
    if (spu -> code_size == 0) 
    {
        DEBUG_PRINT("\t[empty]\n}\n");
    } 
    else 
    {
        size_t i = 0;
        for (i = 0; i < (spu -> code_size); i++)
        {
            DEBUG_PRINT("\t*[%zu] = %d\n", i, spu -> code[i]);
        }
        DEBUG_PRINT("}\n");
    }

    DEBUG_PRINT("\n    Registers:\n");
    for (size_t i = 0; i < REGS_COUNT; i++)
        DEBUG_PRINT("        R[%zu] = %d\n", i, spu->regs[i]);

    DEBUG_PRINT("\n    Stack state:\n");
    stack_dump(&(spu->stack), STACK_OK, file, line);

    DEBUG_PRINT("}\n========================================\n\n");
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


const char * get_string_type_arg(type_arg type)
{
    static const char * type_arg_strings[] =
    {
        "NUM",
        "LABEL",
        "REG",
        "RAM",
        "UNKNOWN_TYPE"
    };

    if (type < NUM || type > UNKNOWN_TYPE) return "Unknown type_arg";
    
    return type_arg_strings[type];
}




void finish_program(spu_t * spu)
{
    spu_destroy(spu);
    printf("spu running is finished\n");
}