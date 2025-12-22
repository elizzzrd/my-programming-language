#pragma once

#include "tree_structure.h"

void SyntaxError(int line, const char * func, char ch);


Node_t * GetProgram(char ** s, Tree_t * tree);
Node_t * GetStatement(char ** s, Tree_t * tree);
Node_t * GetPrintStmt(char ** s, Tree_t * tree);
Node_t * GetAssignment(char **s, Tree_t * tree);
Node_t * GetIfStmt(char **s, Tree_t * tree);
Node_t * GetWhileStmt(char ** s, Tree_t * tree);
Node_t * GetBlock(char ** s, Tree_t * tree);
Node_t * GetExpression(char ** s, Tree_t * tree);
Node_t * GetAddSub(char ** s, Tree_t * tree);
Node_t * GetMulDiv(char ** s, Tree_t * tree);
Node_t * GetPrimaryExpression(char ** s, Tree_t * tree);
Node_t * GetStringLiteral(char ** s, Tree_t * tree);
Node_t * GetIdentifier(char **s, Tree_t * tree);
Node_t * GetNumber(char ** s, Tree_t * tree);