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
    ErrorCode error = SUCCESS;
    error = init_tree(&tree);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        return 1;
    }

    error = load_expression(&tree, EXPRESSION_INPUT);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        return 1;
    }

    destroy_tree(&tree);
    make_html();
    printf("Programm is finished\n");
    return 0;
}