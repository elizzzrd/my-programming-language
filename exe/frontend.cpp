#include <stdio.h>

#include "tree_structure.h"
#include "lexer.h"
#include "tree_operations.h"
#include "syntax_analysis.h"
#include "errors.h"
#include "load_expression.h"
#include "tree_dump.h"
#include "semantic_analysis.h"

extern const error_struct frontend_error_list[];


int main(void)
{
    fprintf(stderr, "[INFO] FRONTEND START\n");
    frontend_err error = FRONTEND_SUCCESS;

    TokenList token_list = {};
    error = lexical_analysis(&token_list);
    LEXICAL_ANALYSIS_ERROR(error);

    Tree_t tree = {};
    tree_err t_error = tree_ctor(&tree);
    if (t_error != TREE_SUCCESS)
    {
        token_list_dtor(&token_list);
        tree_dtor(&tree, "destroy_frontend_error");
        fprintf(stderr, "[ERROR] ERROR DURING FRONTEND\n");
        return -1;
    }
    
    size_t pos = 0;
    Node_t * tree_root = GetProgram_tokens(&token_list, &pos, &tree, &error);
    SYNTAX_ANALISYS_ERROR(error, tree_root);
    tree.root->right = tree_root;
    t_error = build_parent_links(&tree);
    BUILDING_FRONTEND_TREE_ERROR(t_error);

    error = semantic_analysis(&tree);
    if (error != FRONTEND_SUCCESS)
    {
        ERROR_MESSAGE_FRONTEND(SEMANTIC_ERROR);
        tree_dtor(&tree, "destroy_frontend_error");
        fprintf(stderr, "[ERROR] ERROR DURING FRONTEND\n");
        return -1;
    }
    DEBUG_PRINT("[INFO] SEMANTIC_ANALYSIS END");
    GRAPH_DUMP(&tree, "frontend"); 

    save_tree(&tree, AST_OUTPUT_FRONTEND);
    tree_dtor(&tree, "destroy_frontend");
    fprintf(stderr, "[INFO] FRONTEND COMPLETED\n\n");

    return 0;
}