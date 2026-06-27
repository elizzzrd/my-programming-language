#include <stdio.h>
#include "tree_structure.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "translate_to_asm.h"
#include "optimize_tree.h"
#include "assembler.h"
#include "errors_spu.h"
#include "tree_dump.h"

int main(void)
{
    fprintf(stderr, "[INFO] BACKEND START\n");
    DEBUG_PRINT("[INFO] BACKEND START");
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
    if (error != OPTIMIZER_SUCCESS)
    {
        tree_dtor(&tree_backend, "destroy_backend_error");
        fprintf(stderr, "[INFO] ERROR DURING BACKEND\n");
        return -1;
    }
    GRAPH_DUMP(&tree_backend, "backend");
    DEBUG_PRINT("[DEBUG] backend tree was build successfully");
    
    error = translate_to_asm(&tree_backend, ASM_OUTPUT);
    if (error == BACKEND_SUCCESS)
    {
        DEBUG_PRINT("ASSEMBLY START");
        Stack_Err stack_err = assembler("output/asm_output.txt", "output/byte_code.txt");
        if (stack_err != STACK_OK)
        {
            tree_dtor(&tree_backend, "destroy_backend_error");
            DEBUG_PRINT("[ERROR] Assembly error\n");
            fprintf(stderr, "[ERROR] ERROR DURING BACKEND\n");
            printf("Programm is finished\n");
            return -1;
        }
        DEBUG_PRINT("ASSEMBLY END\n\n");

        spu_t spu = {};
        Spu_Err spu_err = spu_init(&spu);
        if (spu_err != SPU_OK)
        {
            DEBUG_PRINT("[ERROR] SPU initialization error\n");
            return -1;
        }
        DEBUG_PRINT("SPU INITED\n");
        spu_dump(&spu, SPU_OK, __FILE__, __LINE__);

        DEBUG_PRINT("START RUNNING SPU\n");
        spu_err |= run_spu(&spu);
        if (spu_err != SPU_OK)
        {
            DEBUG_PRINT("Run spu error\n");
            return -1;
        }
        spu_dump(&spu, SPU_OK, __FILE__, __LINE__);
        spu_destroy(&spu);
    }
    
    tree_dtor(&tree_backend, "destroy_backend");
    DEBUG_PRINT("[INFO] BACKEND END\n");
    fprintf(stderr, "[INFO] BACKEND COMPLETED\n");
    return 0;
}