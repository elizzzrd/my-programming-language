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

int main(void)
{
    Tree_t tree = {};
    ErrorCode error = SUCCESS;
    error = init_tree(&tree);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        return 1;
    }

    char * buffer = NULL;
    error = load_to_buffer(EXPRESSION_INPUT, &buffer);
    char * buffer_ptr = buffer;
    DEBUG_PRINT("buffer: %s", buffer);


    Node_t * tree_root = GetProgram(&buffer_ptr, &tree);
    free(buffer);
    tree.root->right = tree_root;
    error = build_parent_links(&tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        free(tree_root);
        free(buffer);
        return error;
    }
    GRAPH_DUMP(&tree);
    
    TokenList token_list = {};
    DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS START");
    error = lexicalAnalysis(&token_list);
    if (error != SUCCESS)
    {
        DEBUG_PRINT("error during lexical analysis");
    }
    else
        lexer_dump(&token_list);
    destroy_tokens(&token_list);
        
    DEBUG_PRINT("expression has been loaded successfully\n");
    /*
        print_latex(&tree, "logger/dump.tex");
        
        double result = 0.0;
        error = evaluate_const_node(tree.root->right, &result); // just to check if evaluation works
        if (error == SUCCESS)
        {
            DEBUG_PRINT("The expression is constant and its value is: %lf\n", result);
            }
            else
            {
                DEBUG_PRINT("The expression is not constant or evaluation error occurred\n");
                }
                
                int var_index = 0; // differentiate by first variable
                
                Tree_t diff_tree = {}; 
                error = differentiate_tree(&tree, &diff_tree, var_index);
                if (error != SUCCESS)
                {        
                    destroy_tree(&tree);
                    return 1;
                    }
                    //print_latex(&diff_tree, "logger/dump2.tex");
                    
                    optimize_tree(&diff_tree);
                    DEBUG_PRINT("differentiation and optimization completed successfully\n");
                    GRAPH_DUMP_DIFF(&tree, &diff_tree);
                    //print_latex(&diff_tree, "logger/dump3.tex");
                    */
                   
                   
    DEBUG_PRINT("Everything is good before deletion");
    destroy_tree(&tree);
    //destroy_tree(&diff_tree);
    //make_html();
    symbol_table_destroy(&symbols_table);
    DEBUG_PRINT("tree deleted succefully");
    printf("Programm is finished\n");
    return 0;
}