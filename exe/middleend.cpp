#include "tree_operations.h"
#include "tree_structure.h"
#include "utils.h"
#include "load_expression.h"
#include "math_functions.h"


int main(void)
{
    fprintf(stderr, "[INFO] MIDDLEEND START\n");

    Tree_t tree_middleend = {};
    ErrorCode error = SUCCESS;
    DEBUG_PRINT("[INFO] MIDDLEEND START\n");
    error = init_tree(&tree_middleend);
    if (error != SUCCESS)
    {
        destroy_tree(&tree_middleend, "destroy_middleend_error");
        fprintf(stderr, "[ERROR] ERROR DURING MIDDLEEND\n");
        return -1;
    }    

    error = build_middleend_tree(&tree_middleend, AST_OUTPUT_FRONTEND);
    if (error != SUCCESS)
    {
        destroy_tree(&tree_middleend, "destroy_middleend_error");
        fprintf(stderr, "[ERROR] ERROR DURING MIDDLEEND\n");
        return -1;
    }
    GRAPH_DUMP_MIDDLEEND(&tree_middleend);
    save_tree(&tree_middleend, AST_OUTPUT_MIDDLEEND);
    destroy_tree(&tree_middleend, "destroy_middleend");


    DEBUG_PRINT("[INFO] MIDDLEEND END\n");
    fprintf(stderr, "[INFO] MIDDLEEND COMPLETED\n");
    return 0;
}