#include <stdio.h>
#include <stdlib.h>

#include "tree_structure.h"
#include "errors.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"


int main(void)
{
    Tree_t tree = {};
    init_tree(&tree);
    ErrorCode error = SUCCESS;

    error = load_expression(&tree, EXPRESSION_INPUT);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        return 1;
    }

    DEBUG_PRINT("[DEBUG] root->value.root in main: %s\n", tree.root->value.root);    
    destroy_tree(&tree);
    make_html();
    printf("Programm is finished\n");
    return 0;
}