#include "tree_operations.h"
#include "tree_structure.h"
#include "load_expression.h"
#include "optimize_tree.h"
#include "tree_dump.h"


int main(void)
{
    fprintf(stderr, "[INFO] MIDDLEEND START\n");

    Tree_t tree_middleend = {};
    DEBUG_PRINT("[INFO] MIDDLEEND START\n");
    tree_err t_error = tree_ctor(&tree_middleend);
    if (t_error != TREE_SUCCESS)
    {
        tree_dtor(&tree_middleend, "destroy_middleend_error");
        fprintf(stderr, "[ERROR] ERROR DURING MIDDLEEND\n");
        return -1;
    }    

    optimizer_t opt = {.ast_tree = &tree_middleend, .state = OPTIMIZER_SUCCESS};
    optimize_err error = optimizer_ctor(AST_OUTPUT_FRONTEND, &opt);
    if (error != OPTIMIZER_SUCCESS)
    {
        tree_dtor(&tree_middleend, "destroy_middleend_error");
        fprintf(stderr, "[ERROR] ERROR DURING MIDDLEEND\n");
        return -1;
    }
    GRAPH_DUMP_OPTIMIZER(&tree_middleend);
    save_tree(&tree_middleend, AST_OUTPUT_MIDDLEEND);
    tree_dtor(&tree_middleend, "destroy_middleend");


    DEBUG_PRINT("[INFO] MIDDLEEND END\n");
    fprintf(stderr, "[INFO] MIDDLEEND COMPLETED\n\n");
    return 0;
}