#pragma once
#include "utils.h"
#include "tree_structure.h"
#include "tree_operations.h"
#include "lexer.h"


#define SYNTAX_ANALISYS_ERROR(error) \
    do { \
        if (error != SUCCESS) {\
            ERROR_MESSAGE(PARSER_ERROR, error); \
            DEBUG_PRINT("[ERROR] SYNTAX ANALYSIS END WITH ERROR"); \
            fprintf(stderr, "[ERROR] SYNTAX ANALYSIS END WITH ERROR\n"); \
            destroy_tokens(&token_list); \
            return -1;} \
    } while (0)


#define BUILDING_FRONTEND_TREE_ERROR(error) \
    do { \
        if (!tree_root) {\
            ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error); \
            DEBUG_PRINT("[ERROR] SYNTAX ANALYSIS END WITH ERROR"); \
            destroy_tokens(&token_list); \
            destroy_tree(&tree, "destroy_frontend_error"); \
            return -1;} \
        else { \
            DEBUG_PRINT("[INFO] SYNTAX_ANALYSIS END"); \
            DEBUG_PRINT("[INFO] EXPRESSION HAS BEEN LOADED SUCCESSFULLY\n"); \
            destroy_tokens(&token_list); }\
    } while (0)

Node_t * GetProgram_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetStatement_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetPrintStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetAssignment_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetIfStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetWhileStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetBlock_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetAddSub_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetMulDiv_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetUnary_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetPrimaryExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);

Node_t * GetVarDef_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetPow_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetEquality_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetComp_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetFuncCall_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetReturn_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetArgs_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetParams_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetFuncDef(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);
Node_t * GetInputStmt(TokenList * tokens, size_t * pos, Tree_t * tree, ErrorCode * error);