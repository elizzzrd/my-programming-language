#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "load_expression.h"
#include "errors.h"
#include "utils.h"


SymbolTable symbols_table = {NULL, 0, 0};


int symbol_table_find(const char * name)
{
    for (size_t i = 0; i < symbols_table.count; i++)
    {
        if (strcmp(symbols_table.names[i], name) == 0)
            return (int)i;
    }
    return -1;
}

const char * get_id_name(int id_index)
{
    if (id_index >= 0 && id_index < symbols_table.count)
        return symbols_table.names[id_index];
    else    
        return NULL;
}


int symbol_table_add(const char * name)
{
    if (symbols_table.count == symbols_table.capasity)
    {
        if (!symbol_table_resize(&symbols_table))
            return -1;
    }

    symbols_table.names[symbols_table.count] = strdup(name);
    if (!symbols_table.names[symbols_table.count])
        return -1;

    return (int)symbols_table.count++;
}

int symbol_table_resize(SymbolTable *sb)
{
    size_t new_capacity = (sb->capasity == 0) ? 8 : sb -> capasity * 2;
    char ** new_names = (char **)realloc(sb->names, new_capacity * sizeof(char *));
    if (!new_names)     return 0;

    sb -> names = new_names;
    sb -> capasity = new_capacity;
    return 1;
}

int symbol_table_get_or_add(const char * name)
{
    int index = symbol_table_find(name);
    if (index >= 0)
        return index;
    else
        return symbol_table_add(name);
}

void symbol_table_destroy(SymbolTable * sb)
{
    assert(sb);

    for (int i = 0; i < sb->count; i++)
        free(sb->names[i]);
    free(sb->names);
}


int token_list_resize(TokenList * token_list)
{
    size_t new_capacity = (token_list->capasity == 0) ? 8 : token_list -> capasity * 2;
    Token * new_data = (Token *)realloc(token_list -> data, new_capacity * sizeof(Token));
    if (!new_data)     return 0;

    token_list->data = new_data;
    token_list -> capasity = new_capacity;
    return 1;
}

int token_list_push(TokenList * token_list, Token cur_token)
{
    if (token_list->count == token_list->capasity)
    {
        if (!token_list_resize(token_list))
            return -1;
    }

    token_list -> data[token_list -> count++] = cur_token;
    return token_list -> count;
}





token_t keyword(const char * word)
{
    if (strcmp(word, "while") == 0)    return TOK_WHILE;
    if (strcmp(word, "if") == 0)       return TOK_IF;
    if (strcmp(word, "print") == 0)    return TOK_PRINT;

    return TOK_IDENTIFIER;
}

static void skip_spaces(const char ** s)
{
    while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
        (*s)++;
}


ErrorCode lexicalAnalysis(TokenList * token_list)
{
    assert(token_list);

    char * buffer = NULL;
    ErrorCode error = load_to_buffer(EXPRESSION_INPUT, &buffer);
    if (error != SUCCESS)
        return error;
    char * buffer_ptr = buffer;
    DEBUG_PRINT("buffer: %s", buffer);

    token_list->capasity = 0;
    token_list->count = 0;
    token_list->data = NULL;
    
    const char * p = buffer;

    while (*p != '\0') 
    {
        skip_spaces(&p);

        // identifiers or keywords
        if (isalpha((unsigned char)*p) || *p == '_') 
        {
            const char *start = p;
            while (isalnum((unsigned char)*p) || *p == '_')
                p++;
            size_t len = p - start;

            char * word = strndup(start, len);
            token_t type = keyword(word);
            free(word);
            Token tk = make_token(type, start, len);
            token_list_push(token_list, tk);
            continue;
        }

        // Number
        if (isdigit((unsigned char)*p)) 
        {
            const char *start = p;
            while (isdigit((unsigned char)*p)) 
                p++;
          
            size_t len = p - start;
            Token tk = make_token_number(start, len);
            token_list_push(token_list, tk);
            continue;
        }

        // String
        if (*p == '"') 
        {
            p++; // skip "
            const char * start = p;
            while (*p && *p != '"') 
                p++;
            size_t len = p - start;
            Token tk = make_token(TOK_STRING, start, len);
            token_list_push(token_list, tk);
            if (*p == '"') 
                p++; // skip "
            continue;
        }

        // operators + punctuation
        switch (*p) 
        {
        case ';': token_list_push(token_list, make_token(TOK_SEMICOLON, p++, 1)); continue;
        case '{': token_list_push(token_list, make_token(TOK_LBRACE, p++, 1)); continue;
        case '}': token_list_push(token_list, make_token(TOK_RBRACE, p++, 1)); continue;
        case '(': token_list_push(token_list, make_token(TOK_LPAREN, p++, 1)); continue;
        case ')': token_list_push(token_list, make_token(TOK_RPAREN, p++, 1)); continue;
        case '+': token_list_push(token_list, make_token(TOK_PLUS, p++, 1)); continue;
        case '-': token_list_push(token_list, make_token(TOK_MINUS, p++, 1)); continue;
        case '*': token_list_push(token_list, make_token(TOK_MULTIPLY, p++, 1)); continue;
        case '^': token_list_push(token_list, make_token(TOK_POW, p++, 1)); continue;
        case '/': token_list_push(token_list, make_token(TOK_DIVIDE, p++, 1)); continue;
        case '=': token_list_push(token_list, make_token(TOK_ASSIGN, p++, 1)); continue;
        case '$': token_list_push(token_list, make_token(TOK_END, p++, 1)); continue;
        }
        printf("Unknown char: %c\n", *p);
        p++;
    }

    // EOF token
    token_list_push(token_list, make_token(TOK_EOF, "", 0));
    free(buffer);
    return SUCCESS;
}




Token make_token(token_t type, const char * start, size_t len) 
{
    Token t = {};
    t.type = type;
    t.string_value = (char *)malloc(len + 1);
    memcpy(t.string_value, start, len);
    t.string_value[len] = 0;
    return t;
}

Token make_token_number(const char * start, size_t len) 
{
    Token token_number = make_token(TOK_NUMBER, start, len);
    token_number.int_value = atoi(token_number.string_value);
    return token_number;
}

void destroy_tokens(TokenList * token_list)
{
    for (size_t i = 0; i < token_list -> count; i++) 
    {
        free(token_list->data[i].string_value);
        token_list->data[i].int_value = 0;
    }
    
    free(token_list->data);
    token_list->data = NULL;
}

const char * token_type_strings[] =
{
    "TOK_EOF",
    "TOK_END",
    "TOK_NUMBER",
    "TOK_IDENTIFIER",
    "TOK_STRING",

    "TOK_SEMICOLON",
    "TOK_LPAREN",
    "TOK_RPAREN",
    "TOK_LBRACE",
    "TOK_RBRACE",

    "TOK_MINUS",
    "TOK_PLUS",
    "TOK_MULTIPLY",
    "TOK_ASSIGN",
    "TOK_DIVIDE",
    "TOK_POW",

    "TOK_PRINT",
    "TOK_WHILE",
    "TOK_IF",
};

const char * get_string_token_type(token_t type)
{
    if (type >= TOK_EOF && type <= TOK_IF)
        return token_type_strings[type];
    else
        return "Unknown type";
}


void lexer_dump(const TokenList * token_list) 
{
    assert (token_list);
    
    DEBUG_PRINT("[LEXER DUMP]");
    DEBUG_PRINT("token_list.count = %lu, token_list->capacity = %lu", token_list->count, token_list->capasity);
    for (int i = 0; i < token_list->count; i ++)
    {
        DEBUG_PRINT("token_%d, %s, %d, %s", i, get_string_token_type(token_list->data[i].type) ,token_list->data[i].int_value, token_list->data[i].string_value);
    }

}