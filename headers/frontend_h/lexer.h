#pragma once

#include <stdio.h>
#include "errors.h"

typedef struct 
{
    char **names;
    size_t count;
    size_t capasity;
} SymbolTable;


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
    TOK_READ,
    TOK_DEF,
    TOK_ID_DEF,
    TOK_OUT,

    TOK_SIN,
    TOK_COS,
    TOK_TG,
    TOK_LN,
    TOK_SQRT,
    TOK_EXP
} token_t;

typedef enum
{
    SB_VAR  = 0,
    SB_FUNC = 1
} sb_mode_t;

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

extern SymbolTable symbols_table[2];

int symbol_table_find(const char * name, sb_mode_t mode);
int symbol_table_add(const char * name, sb_mode_t mode);
int symbol_table_resize(SymbolTable *sb, sb_mode_t mode);
int symbol_table_get_or_add(const char * name, sb_mode_t mode);
void symbol_table_destroy(SymbolTable * sb);
const char * get_id_name(size_t id_index, sb_mode_t mode);


