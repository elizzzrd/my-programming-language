#include <stdio.h>

#include "tree_structure.h"
#include "lexer.h"
#include "utils.h"
#include "tree_operations.h"
#include "syntax_analysis.h"
#include "load_expression.h"
#include "semantic_analysis.h"


int main(void)
{
    fprintf(stderr, "[INFO] FRONTEND START\n");
    ErrorCode error = SUCCESS;

    TokenList token_list = {};
    error = lexicalAnalysis(&token_list);
    LEXICAL_ANALYSIS_ERROR(error);

    Tree_t tree = {};
    error = init_tree(&tree);
    if (error != SUCCESS)
    {
        destroy_tokens(&token_list);
        destroy_tree(&tree, "destroy_frontend_error");
        fprintf(stderr, "[ERROR] ERROR DURING FRONTEND\n");
        return -1;
    }
    
    size_t pos = 0;
    Node_t * tree_root = GetProgram_tokens(&token_list, &pos, &tree, &error);
    SYNTAX_ANALISYS_ERROR(error);
    tree.root->right = tree_root;
    error = build_parent_links(&tree);
    BUILDING_FRONTEND_TREE_ERROR(error);

    error = semantic_analysis(&tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(SEMANTIC_ERROR, error);
        destroy_tree(&tree, "destroy_frontend_error");
        fprintf(stderr, "[ERROR] ERROR DURING FRONTEND\n");
        return -1;
    }
    DEBUG_PRINT("[INFO] SEMANTIC_ANALYSIS END");
    GRAPH_DUMP(&tree, "frontend"); 

    save_tree(&tree, AST_OUTPUT_FRONTEND);
    destroy_tree(&tree, "destroy_frontend");
    fprintf(stderr, "[INFO] FRONTEND COMPLETED\n\n");

    return 0;
}