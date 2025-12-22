#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "stack.h"
#include "errors_spu.h"
#include "read_file.h"
#include "assembler.h"
#include "spu.h"



#define IF_THERE_IS_STACK_ERROR(stack_errors, fmt) \
    do { \
        if ((stack_errors) != STACK_OK) { \
            log_message(fmt, __FILE__, __LINE__); \
            spu_errors = SPU_STACK_ERROR; \
            return spu_errors; \
        } \
    } while(0)

Spu_Err spu_init(spu_t * spu)
{
    Spu_Err errors = SPU_OK;

    if (spu == NULL) 
    {
        errors |= SPU_NULL_PTR;
        return errors;
    }

    spu -> code_size = 0;
    spu -> instructor_ptr = 0;

    spu -> code = load_bytecode("text/byte_code.txt", &(spu -> code_size));
    
    spu -> stack = {};
    if((stack_init(&(spu -> stack), 8)) != STACK_OK) errors |= SPU_STACK_ERROR;
    if((stack_init(&(spu -> return_address), MAX_LABELS)) != STACK_OK) errors |= SPU_STACK_ERROR;
    for (size_t i = 0; i < REGS_COUNT; i++) spu->regs[i] = 0;
    for (size_t i = 0; i < RAM_COUNT; i++) spu->RAM[i] = 0;

    return errors;
}


Spu_Err run_spu(spu_t * spu)
{
    assert(spu != NULL);
    
    Spu_Err spu_errors = SPU_OK;
    Stack_Err stack_errors = STACK_OK;

    printf("stack.data = %p, capacity = %zu, size = %zu\n", 
       spu->stack.data, spu->stack.capacity, spu->stack.size);

    while ((spu -> instructor_ptr) < (spu -> code_size))
    {
        int cmd = (spu -> code)[(spu -> instructor_ptr)++];
        switch(cmd) 
        {
            case HLT: (spu -> instructor_ptr) = (spu -> code_size); break;
            case PUSH: 
            {
                if ((spu -> instructor_ptr) >= (spu -> code_size))
                {
                    (spu -> instructor_ptr) = spu -> code_size;
                    spu_errors |= SPU_INVALID_COMMAND;
                    log_message("Error: missing argument for PUSH\n", __FILE__, __LINE__); 
                    break;
                }
                int value = (spu -> code)[spu -> instructor_ptr];


                //printf("INSTR_PTR=%zu CMD=%d NEXT=%d\n", spu->instructor_ptr - 1, cmd, value);
                stack_errors = stack_push(&(spu->stack), value);                            IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                (spu->instructor_ptr)++;
                //stack_dump(&(spu->stack), STACK_OK, __FILE__, __LINE__);
                spu_dump(spu, SPU_OK, __FILE__, __LINE__);
                break;
            }
            case POPR:
            {
                int reg_num = (spu -> code)[spu -> instructor_ptr++];
                StackElem a = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POPR");

                if (reg_num >= 0 && reg_num < REGS_COUNT) 
                    spu -> regs[reg_num] = a;
                else {
                    log_message("Invalid register index in POPR", __FILE__, __LINE__);
                    spu_errors |= SPU_INVALID_COMMAND;
                    return spu_errors;
                }
                break;
            }
            case PUSHR:
            {
                int reg_num = (spu -> code)[spu -> instructor_ptr++]; 
                if (reg_num >= 0 && reg_num < REGS_COUNT)
                {
                    stack_errors = stack_push(&(spu -> stack), spu -> regs[reg_num]);       IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSHR");
                }
                else {
                    log_message("Invalid register index in PUSHR", __FILE__, __LINE__);
                    spu_errors |= SPU_INVALID_COMMAND;
                    return spu_errors;
                }
                break;
            }
            case POPM:
            {
                int ram_num = (spu -> code)[spu -> instructor_ptr++];
                StackElem a = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POPR");

                if (ram_num >= 0 && ram_num < RAM_COUNT) 
                    spu -> RAM[ram_num] = a;
                else {
                    log_message("Invalid ram index in POPR", __FILE__, __LINE__);
                    spu_errors |= SPU_INVALID_COMMAND;
                    return spu_errors;
                }
                break;
            }
            case PUSHM:
            {
                int ram_num = (spu -> code)[spu -> instructor_ptr++]; 
                if (ram_num >= 0 && ram_num < REGS_COUNT)
                {
                    stack_errors = stack_push(&(spu -> stack), spu -> RAM[ram_num]);       IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSHR");
                }
                else {
                    log_message("Invalid ram index in PUSHR", __FILE__, __LINE__);
                    spu_errors |= SPU_INVALID_COMMAND;
                    return spu_errors;
                }
                break;
            }
            case DUMP: 
            {
                log_message("\n\nstack dump from run_spu:", __FILE__, __LINE__);
                spu_dump(spu, spu_errors, __FILE__, __LINE__); 
                break;
            }
            case ADD: 
            {
                StackElem a = 0, b = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                stack_errors = stack_pop(&(spu -> stack), &b);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                stack_errors = stack_push(&(spu -> stack), a + b);                          IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                break;
            }
            case SUB:
            {
                StackElem a = 0, b = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu -> stack), &b);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_push(&(spu -> stack), b - a);                          IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                break;                    
            }
            case MUL:
            {
                StackElem a = 0, b = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: MUL");
                stack_errors = stack_pop(&(spu -> stack), &b);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: MUL");
                stack_errors = stack_push(&(spu -> stack), a * b);                          IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: MUL");
                break;    
            }     
            case DIV:
            {
                StackElem a = 0, b = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: DIV");
                stack_errors = stack_pop(&(spu -> stack), &b);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: DIV");
                if (a == 0)
                {
                    log_message("Error: division by zero\n", __FILE__, __LINE__); 
                    spu_errors |= SPU_DIVISION_BY_ZERO;
                    stack_push(&(spu -> stack), a);                                         IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                    stack_push(&(spu -> stack), b);                                         IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                }
                stack_errors = stack_push(&(spu -> stack), b / a);                          IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                break;
            }       
            case SQRT:
            {
                StackElem a = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);
                if (a < 0) 
                {
                    spu_errors |= SPU_INVALID_COMMAND;
                    log_message("Error: sqrt of negative number\n", __FILE__, __LINE__); 
                    stack_push(&(spu -> stack), a);
                    return spu_errors;
                }
                else
                {
                    stack_errors = stack_push(&(spu -> stack), (StackElem) sqrt(a));       IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                }
                break;
            }    
            case OUT:
            {
                StackElem a = 0;
                stack_errors = stack_pop(&(spu -> stack), &a);                              IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                printf("value for out %d\n", a);
                if (stack_errors != STACK_OK)
                {
                    printf("Error with OUT\n");
                }
                printf("OUT: %d\n", a);
                break;
            }
            case JB:                                                    //jump if below
            {
                StackElem a = 0, b = 0;
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a < b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JBE:                                                   // jump if below or equal
            {
                StackElem a = 0, b = 0;
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a <= b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JA:
            {
                StackElem a = 0, b = 0;                                 // jump if above
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a > b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JAE:                                                  // jump if above or equal                     
            {
                StackElem a = 0, b = 0;
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a >= b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JE:                                                   // jump if equal
            {
                StackElem a = 0, b = 0;
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a == b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JNE:                                                   // jump if not equal
            {
                StackElem a = 0, b = 0;
                StackElem target = spu->code[spu->instructor_ptr++];
                stack_errors = stack_pop(&(spu->stack), &b);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                stack_errors = stack_pop(&(spu->stack), &a);                                               IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: POP");
                if (a != b) spu->instructor_ptr = (size_t)target;
                break;
            }
            case JMP:                                                   //just jump
            {
                StackElem target = spu->code[spu->instructor_ptr++];
                spu->instructor_ptr = (size_t)target;
                break;
            }
            case IN:
            {
                printf("Enter number: ");
                int n = 0;
                while (scanf("%d", &n) != 1)
                {
                    printf("Invalid number. Please try again\n");
                    continue;
                }
                stack_errors |= stack_push(&(spu -> stack), n);                             IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: PUSH");
                break;
            }
            case CALL:
            {
                StackElem target = spu->code[spu->instructor_ptr++];
                StackElem next_ptr = spu->instructor_ptr;
                stack_errors = stack_push(&(spu->return_address), next_ptr);                         IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: CALL PUSH");
                spu->instructor_ptr = (size_t) target;
                break;
            }
            case RET:
            {
                StackElem return_ptr = 0;
                stack_errors = stack_pop(&(spu->return_address), &return_ptr);                       IF_THERE_IS_STACK_ERROR(stack_errors, "Error during spu running: RET POP");
                spu->instructor_ptr = (size_t) return_ptr;
                break;
            }
            default: 
            {
                log_message("Unknown command\n", __FILE__, __LINE__);
                spu_errors |= SPU_INVALID_COMMAND;
                return spu_errors;
                break;
            } 
        }

        if (stack_errors != STACK_OK) 
        {
            spu_errors |= SPU_STACK_ERROR;
            return spu_errors;
        }
    }
    
    printf("spu running is finished!\n");
    return spu_errors;
}


bool is_command(int code)
{
    return (code >= HLT && code <= POPM);
}


Spu_Err spu_destroy(spu_t * spu)
{
    assert(spu != NULL);
    Spu_Err errors = SPU_OK;
    
    if (spu -> code != NULL)
    {
        free(spu -> code);
        spu -> code = NULL;
    }

    spu -> code_size = 0;
    spu -> instructor_ptr = 0;
    if ((stack_destroy(&(spu -> stack))) != STACK_OK) errors |= SPU_STACK_ERROR;
    if ((stack_destroy(&(spu -> return_address))) != STACK_OK) errors |= SPU_STACK_ERROR;
    memset(spu->regs, 0, sizeof(spu->regs)); 

    return SPU_OK; 
}