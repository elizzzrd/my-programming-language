#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "tree_structure.h"
#include "errors.h"
#include "lexer.h"
#include "optimize_tree.h"


const error_struct frontend_error_list[] =
{
    GEN_ERROR(FRONTEND_SUCCESS, "frontend success")
    GEN_ERROR(NULL_POINTER, "tree null pointer")
    GEN_ERROR(DELETION_ERROR, "tree deletion error")
    GEN_ERROR(INVALID_NODE, "tree invalid node")
    GEN_ERROR(INVALID_OPERATOR, "tree invalid operator")
    GEN_ERROR(CREATING_NODE_ERROR, "creating node error")
    
    GEN_ERROR(OPENING_FILE_ERROR, "openening file error")
    GEN_ERROR(LOADING_EXPRESSION_ERROR, "loading expression error")
    GEN_ERROR(GRAPH_DUMP_ERROR, "graph dump error")
    GEN_ERROR(MEMORY_ALLOCATION_ERROR, "memory allocation error")

    GEN_ERROR(LEXER_ERROR, "lexer error")
    GEN_ERROR(PARSER_ERROR, "parser error")
    GEN_ERROR(SYNTAX_ERROR, "syntax error")
    GEN_ERROR(SEMANTIC_ERROR, "semantic error")

    GEN_ERROR(UNKNOWN_CHAR, "syntax error: unknown char")
    GEN_ERROR(FORGOTTEN_SEMICOLON, "syntax error: missing semicolon")
    GEN_ERROR(FORGOTTEN_QUATATION_MARK, "syntax error: missing quatation mark")
    GEN_ERROR(FORGOTTEN_L_BRACE, "syntax error: missing left brace")
    GEN_ERROR(FORGOTTEN_R_BRACE, "syntax error: missing right brace")
    GEN_ERROR(FORGOTTEN_L_PAREN, "syntax error: missing left parentheses")
    GEN_ERROR(FORGOTTEN_R_PAREN, "syntax error: missing right parentheses")
    GEN_ERROR(FORGOTTEN_COMMA, "syntax error: missing comma")
    GEN_ERROR(FORGOTTEN_EOF, "syntax error: missing $")
    GEN_ERROR(UNEXPECTED_SYNTAX, "syntax error: unexpected syntax")
    GEN_ERROR(UNDEFINED_ID, "syntax error: undefined identifier")
    GEN_ERROR(NO_CONDITION, "syntax error: no condition")
    GEN_ERROR(INCORRECT_ARGUMENT_AMOUNT, "syntax error: incorrect argument amount")
    GEN_ERROR(ASSIGNMENT_NO_VALUE, "syntax error: after assignment no value")
    GEN_ERROR(NO_BLOCK, "syntax error: no block")
    GEN_ERROR(NO_EXPECTED_EXPRESSION, "syntax error: expected expression")
};
const size_t error_amount = sizeof(frontend_error_list) / sizeof(frontend_error_list[0]);


// -----------------------------------------------------------------------------------------------------
const error_struct optimizer_error_list[] =
{
    GEN_ERROR(OPTIMIZER_SUCCESS, "optimizer success")
    GEN_ERROR(OPTIMIZER_ALLOCATION_ERROR, "optimizer allocation error")
    GEN_ERROR(OPTIMIZER_BUFFER_ERROR, "buffer error")
    GEN_ERROR(OPTIMIZER_DIVISION_BY_ZERO, "division by zero")
    GEN_ERROR(OPTIMIZER_INVALID_OPERATOR, "invalid operator")
    GEN_ERROR(OPTIMIZER_INVALID_NODE, "invalid node")
    GEN_ERROR(OPTIMIZER_CREATING_NODE_ERROR, "creating node error")
    GEN_ERROR(OPTIMIZER_LOADING_EXPRESSION_ERROR, "loading expression error")
};

// ------------------------------------------------------------------------------------------------------
const error_struct tree_error_list[] = 
{
    GEN_ERROR(TREE_SUCCESS, "tree success")
    GEN_ERROR(TREE_NULL_POINTER, "tree null pointer")
    GEN_ERROR(TREE_DELETION_ERROR, "tree deletion error")
    GEN_ERROR(TREE_CREATING_NODE_ERROR, "creating node error")
    GEN_ERROR(TREE_ALLOCATION_ERROR, "tree allocation error")
    GEN_ERROR(EMPTY_TREE, "empty tree")
    GEN_ERROR(TREE_INVALID_NODE, "invalid node")
};

//-------------------------------------------------------------------------------------------------------
const error_struct backend_error_list[] = 
{
    GEN_ERROR(BACKEND_SUCCESS, "backend success")
    GEN_ERROR(BACKEND_OPENING_FILE_ERROR, "backend opening file error")
    GEN_ERROR(TRANSLATING_TO_ASM_ERROR, "translating to asm error")
    GEN_ERROR(BACKEND_INVALID_OPERATOR, "backend invalid operator")
    GEN_ERROR(BACKEND_SEMANTIC_ERROR, "backend semantic error")
    GEN_ERROR(BACKEND_ALLOCATION_ERROR, "backend allocation error")
    GEN_ERROR(BACKEND_INVALID_NODE, "backend invalid node")
};
