#pragma once

#include <stdio.h>
#include "errors.h"


typedef enum
{
    TOK_EOF,
    TOK_END,
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_STRING,

    TOK_SEMICOLON,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMA,

    TOK_MINUS,
    TOK_PLUS,
    TOK_MULTIPLY,
    TOK_ASSIGN,
    TOK_DIVIDE,
    TOK_POW,

    TOK_BELOW,
    TOK_BELOW_EQUAL,
    TOK_ABOVE,
    TOK_ABOVE_EQUAL,
    TOK_EQUAL_EQUAL,
    TOK_NON_EQUAL,

    TOK_PRINT,
    TOK_WHILE,
    TOK_IF,
    TOK_ELSE,
    TOK_READ,
    TOK_DEF,
    TOK_ID_DEF,
    TOK_OUT,

    TOK_SIN,
    TOK_COS,
    TOK_TG,
    TOK_LN,
    TOK_SQRT,
    TOK_EXP,
    TOK_UNARY_MINUS
} token_t;


typedef struct 
{
    token_t type;
    int int_value;
    char * string_value;    
} Token;

typedef struct 
{
    Token * data;
    size_t count;
    size_t capasity;
} TokenList;


#define LEXICAL_ANALYSIS_ERROR(error) \
    do { \
        if (error != SUCCESS) {\
            DEBUG_PRINT("[ERROR] LEXICAL_ANALYSIS END WITH ERROR"); \
            fprintf(stderr, "[INFO] ERROR DURING FRONTEND\n"); \
            destroy_tokens(&token_list); \
            return -1; }\
        else { \
            DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS END");\
            lexer_dump(&token_list);\
        }\
    } while (0)


ErrorCode lexicalAnalysis(TokenList * token_list);
void destroy_tokens(TokenList * token_list);
void token_list_init(TokenList * token_list);
int token_list_resize(TokenList * token_list);
int token_list_push(TokenList * token_list, Token cur_token);
token_t keyword(const char * word);
Token make_token(token_t type, const char * start, size_t len);
Token make_token_number(const char * start, size_t len);
void lexer_dump(const TokenList * token_list);
const char * get_string_token_type(token_t type);
void skip_spaces(const char ** s);
