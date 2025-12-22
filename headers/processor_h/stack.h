#pragma once
#include <stdbool.h>
#include <stdio.h>

typedef int StackElem;

typedef struct 
{
    StackElem left_canary;
    StackElem * data;
    size_t size;
    size_t capacity;
    StackElem right_canary;
} stack_t;


enum 
{
    STACK_OK                      = 0u,        // 00000000

    STACK_MEMORY_ALLOCATION_ERROR = 1u << 0,   // 00000001
    STACK_NULL_PTR                = 1u << 1,   // 00000010
    STACK_UNINITIALIZED           = 1u << 2,   // 00000100

    STACK_OVERFLOW                = 1u << 3,   // 00001000
    STACK_UNDERFLOW               = 1u << 4,   // 00010000

    STACK_INVALID_CAPACITY        = 1u << 5,   // 00100000
    STACK_INVALID_SIZE            = 1u << 6,   // 01000000

    STACK_CANARY_CORRUPTED        = 1u << 7,   // 10000000

};

typedef unsigned int Stack_Err;

Stack_Err stack_init(stack_t * stack, size_t capacity);
Stack_Err stack_destroy(stack_t * stack);
Stack_Err stack_push(stack_t * stack, StackElem value);
Stack_Err stack_pop(stack_t * stack, StackElem * value);
Stack_Err stack_resize(stack_t * stack, size_t new_capacity);
bool stack_is_empty(const stack_t * stack);
bool stack_is_full(const stack_t * stack);
