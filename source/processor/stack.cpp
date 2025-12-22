#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stack.h"

#define CANARY_LEFT_VALUE  0x7ABCDEFA
#define CANARY_RIGHT_VALUE 0x12345678

Stack_Err stack_init(stack_t *stack, size_t capacity)
{
    if (!stack) return STACK_NULL_PTR;
    Stack_Err errors = STACK_OK;

    if (capacity < 1) capacity = 4;

    stack->left_canary  = CANARY_LEFT_VALUE;
    stack->right_canary = CANARY_RIGHT_VALUE;
    stack->size = 0;
    stack->capacity = capacity;

    stack->data = (StackElem *)calloc(capacity, sizeof(StackElem));
    if (!stack->data) return STACK_MEMORY_ALLOCATION_ERROR;

    return errors;
}

Stack_Err stack_destroy(stack_t *stack)
{
    if (!stack) return STACK_NULL_PTR;

    free(stack->data);
    stack->data = NULL;
    stack->size = 0;
    stack->capacity = 0;
    stack->left_canary  = 0;
    stack->right_canary = 0;

    return STACK_OK;
}

Stack_Err stack_push(stack_t *stack, StackElem value)
{
    if (!stack) return STACK_NULL_PTR;

    Stack_Err errors = STACK_OK;

    if (stack->size >= stack->capacity)
    {
        errors |= stack_resize(stack, stack->capacity * 2);
        if (errors & STACK_MEMORY_ALLOCATION_ERROR) return errors;
    }

    stack->data[stack->size++] = value;
    //printf("[STACK_PUSH] pushed %d (size=%lu, cap=%lu)\n", value, stack->size, stack->capacity);

    return errors;
}

Stack_Err stack_pop(stack_t *stack, StackElem *value)
{
    if (!stack || !value) return STACK_NULL_PTR;

    if (stack->size == 0)
    {
        *value = 0;
        return STACK_UNDERFLOW;
    }

    *value = stack->data[--stack->size];
    //printf("[STACK_POP] popped %d (size=%lu, cap=%lu)\n", *value, stack->size, stack->capacity);

    
    if (stack->size > 0 && stack->size < stack->capacity / 4)
    {
        stack_resize(stack, stack->capacity / 2);
    }

    return STACK_OK;
}

Stack_Err stack_resize(stack_t *stack, size_t new_capacity)
{
    if (!stack) return STACK_NULL_PTR;
    if (new_capacity < 4) new_capacity = 4;

    StackElem *new_data = (StackElem *)realloc(stack->data, new_capacity * sizeof(StackElem));
    if (!new_data) return STACK_MEMORY_ALLOCATION_ERROR;

    stack->data = new_data;
    if (stack->size > new_capacity) stack->size = new_capacity;
    stack->capacity = new_capacity;

    return STACK_OK;
}

bool stack_is_empty(const stack_t *stack)
{
    if (!stack) return true;
    return stack->size == 0;
}

bool stack_is_full(const stack_t *stack)
{
    if (!stack) return false;
    return stack->size >= stack->capacity;
}
