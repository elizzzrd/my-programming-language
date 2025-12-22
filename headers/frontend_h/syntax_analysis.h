#pragma once
#include "utils.h"
#include "tree_structure.h"
#include "tree_operations.h"
#include "build_tree.h"
#include "lexer.h"


Node_t * GetProgram_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetStatement_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetPrintStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetAssignment_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetIfStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetWhileStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetBlock_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetAddSub_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetMulDiv_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetUnary_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
Node_t * GetPrimaryExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree);
