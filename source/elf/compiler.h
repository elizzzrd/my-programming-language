#pragma once

#include <stdio.h>
#include <stdint.h>
#include "tree_structure.h"
#include "my_elf.h"
#include "emiters.h"

#include "buffer.h"

typedef struct  
{  
    buffer_t buffer;
    tree_t   compiler_tree;
    FILE*    file_output;
    size_t   label_count;
};
typedef compiler_s* compiler_t;

typedef enum 
{   
    COMPILER_SUCCESS,
    COMPILER_ALLOCATION_ERROR,
    COMPILER_BUFFER_ERROR,
    COMPILER_TREE_ERROR,
    COMPILER_AST_STANDARD_ERROR,
    COMPILER_FILE_OPEN_ERROR,
    COMPILER_FILE_CLOSE_ERROR,
    COMPILER_INCORRECT_AST
} Compiler_Error;

// =========================== MEMORY_CONTROLLING =============================

compiler_return_e
CompilerCtor(const char* input_name,
             const char* output_name,
             compiler_t* compiler);

compiler_return_e
CompilerDtor(compiler_t* compiler);

// =============================== COMPILE_AST ================================

compiler_return_e
CompileAST(compiler_t compiler);

