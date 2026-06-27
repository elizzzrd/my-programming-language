#include <stdio.h>
#include "tree_structure.h"
#include "tree_operations.h"
#include "optimize_tree.h"
#include "load_expression.h"
#include "translate_to_nasm.h"
#include "tree_dump.h"


int main(void)
{
    fprintf(stderr, "[INFO] BACKEND-x64 START\n");
    DEBUG_PRINT("[INFO] BACKEND-x64 START");
    backend_err error = BACKEND_SUCCESS;

    Tree_t tree_backend = {};
    tree_err t_error = tree_ctor(&tree_backend);
    if (error != TREE_SUCCESS)
    {
        tree_dtor(&tree_backend, "destroy_backend_error");
        fprintf(stderr, "[INFO] ERROR DURING BACKEND\n");
        return -1;
    }  

    optimizer_t opt = {.ast_tree = &tree_backend, .state = OPTIMIZER_SUCCESS};
    optimize_err o_error = optimizer_ctor(AST_OUTPUT_MIDDLEEND, &opt);
    if (o_error != OPTIMIZER_SUCCESS)
    {
        tree_dtor(&tree_backend, "destroy_backend_error");
        fprintf(stderr, "[INFO] ERROR DURING BACKEND\n");
        return -1;
    }
    GRAPH_DUMP(&tree_backend, "backend");
    DEBUG_PRINT("[DEBUG] backend tree was build successfully");
    
    error = BACKEND_SUCCESS;
    error = translate_to_nasm(&tree_backend, NASM_OUTPUT);
    if (error == BACKEND_SUCCESS)
    
    tree_dtor(&tree_backend, "destroy_backend");
    DEBUG_PRINT("[INFO] BACKEND END\n");
    fprintf(stderr, "[INFO] BACKEND COMPLETED\n");
    return 0;
}