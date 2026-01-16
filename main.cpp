#include <stdio.h>
#include <stdlib.h>

#include "tree_structure.h"
#include "errors.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"
#include "math_functions.h"
#include "latex_dump.h"
#include "build_tree.h"
#include "lexer.h"
#include "syntax_analysis.h"
#include "load_expression.h"
#include "translate_to_asm.h"
#include "spu.h"
#include "assembler.h"
#include "read_file.h"
#include "stack.h"
#include "errors_spu.h"



int main(void)
{
    ErrorCode error = SUCCESS;
    
    TokenList token_list = {};
    token_list_init(&token_list);
    DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS START");
    error = lexicalAnalysis(&token_list);
    if (error != SUCCESS)
    {
        DEBUG_PRINT("error during lexical analysis");
        destroy_tokens(&token_list);
        return -1;
    }
    else
    {
        DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS END");
        lexer_dump(&token_list);
    }
    

    Tree_t tree = {};
    error = init_tree(&tree);
    if (error != SUCCESS)
    {
        destroy_tokens(&token_list);
        destroy_tree(&tree);
        return -1;
    }
    
    DEBUG_PRINT("\n[INFO] SYNTAX_ANALYSIS START");
    size_t pos = 0;
    Node_t * tree_root = GetProgram_tokens(&token_list, &pos, &tree);
    if (!tree_root)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        destroy_tokens(&token_list);
        
        symbol_table_destroy(&symbols_table[SB_VAR]);
        symbol_table_destroy(&symbols_table[SB_FUNC]);
        return -1;
    }
    tree.root->right = tree_root;
    error = build_parent_links(&tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        destroy_tokens(&token_list);
        destroy_tree(&tree);
        symbol_table_destroy(&symbols_table[SB_VAR]);
        symbol_table_destroy(&symbols_table[SB_FUNC]);
        return -1;
    }
    GRAPH_DUMP(&tree);
    DEBUG_PRINT("[INFO] SYNTAX_ANALYSIS END");
    destroy_tokens(&token_list);
    
    DEBUG_PRINT("expression has been loaded successfully\n");
    save_tree(&tree, AST_OUTPUT);
    DEBUG_PRINT("tree was saved successfully. trying to build it");

    
    Tree_t tree_middleend = {};
    error = SUCCESS;
    error = init_tree(&tree_middleend);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        destroy_tree(&tree_middleend);
        return -1;
    }

    error = load_expression_prefix(&tree_middleend, AST_OUTPUT);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        destroy_tree(&tree_middleend);
        symbol_table_destroy(&symbols_table[SB_VAR]);
            symbol_table_destroy(&symbols_table[SB_FUNC]);
        return -1;
    }
    GRAPH_DUMP(&tree_middleend);
    DEBUG_PRINT("tree_middleend was created");


    DEBUG_PRINT("[INFO] BACKEND START\n");
    error = SUCCESS;
    error = translate_to_asm(&tree, "asm_output.txt");
    if (error == SUCCESS)
    {
        puts("ASSEMBLY START");
        Stack_Err stack_err = assembler("asm_output.txt", "byte_code.txt");
        if (stack_err != STACK_OK)
        {
            destroy_tree(&tree);
            destroy_tree(&tree_middleend);
            symbol_table_destroy(&symbols_table[SB_VAR]);
            symbol_table_destroy(&symbols_table[SB_FUNC]);
            DEBUG_PRINT("Assembly error\n");
            printf("Programm is finished\n");
            return -1;
        }
        puts("ASSEMBLY END\n\n");

        spu_t spu = {};
        Spu_Err spu_err = spu_init(&spu);
        if (spu_err != SPU_OK)
        {
            destroy_tree(&tree);
            destroy_tree(&tree_middleend);
            symbol_table_destroy(&symbols_table[SB_VAR]);
            symbol_table_destroy(&symbols_table[SB_FUNC]);
            DEBUG_PRINT("SPU initialization error\n");
            finish_program(&spu);
            return -1;
        }
        puts("SPU INITED\n");
        spu_dump(&spu, SPU_OK, __FILE__, __LINE__);

        puts("START RUNNING SPU\n");
        spu_err |= run_spu(&spu);
        if (spu_err != SPU_OK)
        {
            DEBUG_PRINT("Run spu error\n");
            destroy_tree(&tree);
            destroy_tree(&tree_middleend);
            symbol_table_destroy(&symbols_table[SB_VAR]);
            symbol_table_destroy(&symbols_table[SB_FUNC]);
            finish_program(&spu);
            printf("with error\n");
            return -1;
    }

    spu_dump(&spu, SPU_OK, __FILE__, __LINE__);
    finish_program(&spu);
    }
    DEBUG_PRINT("[INFO] BACKEND END\n");

    if (tree_middleend.root)
        destroy_tree(&tree_middleend);
    if (tree.root)
        destroy_tree(&tree);
    
    symbol_table_destroy(&symbols_table[SB_VAR]);
    symbol_table_destroy(&symbols_table[SB_FUNC]);
    DEBUG_PRINT("tree deleted succefully");
    printf("Programm is finished\n");
    return 0;
}

