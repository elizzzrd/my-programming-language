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

    TOK_MINUS,
    TOK_PLUS,
    TOK_MULTIPLY,
    TOK_ASSIGN,
    TOK_DIVIDE,
    TOK_POW,

    TOK_PRINT,
    TOK_WHILE,
    TOK_IF,
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

ErrorCode lexicalAnalysis(TokenList * token_list);
void tokenize(const char * source_buffer, TokenList * token_list);
void destroy_tokens(TokenList * token_list);
int token_list_resize(TokenList * token_list);
int token_list_push(TokenList * token_list, Token cur_token);
token_t keyword(const char * word);
Token make_token(token_t type, const char * start, size_t len);
Token make_token_number(const char * start, size_t len);
void lexer_dump(const TokenList * token_list);
const char * get_string_token_type(token_t type);

extern SymbolTable symbols_table;

int symbol_table_find(const char * name);
int symbol_table_add(const char * name);
int symbol_table_resize(SymbolTable *sb);
int symbol_table_get_or_add(const char * name);
void symbol_table_destroy(SymbolTable * sb);
const char * get_id_name(size_t id_index);