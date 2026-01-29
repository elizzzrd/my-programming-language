#include <stdio.h>
#include "tree_structure.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"
#include "translate_to_asm.h"
#include "assembler.h"
#include "errors_spu.h"

int main(void)
{
    fprintf(stderr, "[INFO] BACKEND START\n");
    DEBUG_PRINT("[INFO] BACKEND START");
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
    error = translate_to_asm(&tree_backend, ASM_OUTPUT);
    if (error == SUCCESS)
    {
        DEBUG_PRINT("ASSEMBLY START");
        Stack_Err stack_err = assembler("output/asm_output.txt", "output/byte_code.txt");
        if (stack_err != STACK_OK)
        {
            destroy_tree(&tree_backend, "destroy_backend_error");
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
    
    destroy_tree(&tree_backend, "destroy_backend");
    DEBUG_PRINT("[INFO] BACKEND END\n");
    fprintf(stderr, "[INFO] BACKEND COMPLETED\n");
    return 0;
}