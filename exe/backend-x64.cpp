#include <stdio.h>
#include "tree_structure.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"
#include "translate_to_nasm.h"


int main(void)
{
    fprintf(stderr, "[INFO] BACKEND-x64 START\n");
    DEBUG_PRINT("[INFO] BACKEND-x64 START");
    ErrorCode error = SUCCESS;

    Tree_t tree_backend = {};
    error = init_tree(&tree_backend);
    if (error != SUCCESS)
    {
        destroy_tree(&tree_backend, "destroy_backend_error");
        fprintf(stderr, "[INFO] ERROR DURING BACKEND\n");
        return -1;
    }  

    error = build_middleend_tree(&tree_backend, AST_OUTPUT_MIDDLEEND);
    if (error != SUCCESS)
    {
        destroy_tree(&tree_backend, "destroy_backend_error");
        fprintf(stderr, "[INFO] ERROR DURING BACKEND\n");
        return -1;
    }
    GRAPH_DUMP(&tree_backend, "backend");
    DEBUG_PRINT("[DEBUG] backend tree was build successfully");
    
    error = SUCCESS;
    error = translate_to_nasm(&tree_backend, ASM_OUTPUT);
    if (error == SUCCESS)
    {
        
    }
    
    destroy_tree(&tree_backend, "destroy_backend");
    DEBUG_PRINT("[INFO] BACKEND END\n");
    fprintf(stderr, "[INFO] BACKEND COMPLETED\n");
    return 0;
}