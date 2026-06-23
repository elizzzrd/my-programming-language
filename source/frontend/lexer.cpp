#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "load_expression.h"
#include "errors.h"
#include "utils.h"

#define DISABLE_DEBUG_PRINT

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
    "TOK_COMMA",

    "TOK_MINUS",
    "TOK_PLUS",
    "TOK_MULTIPLY",
    "TOK_ASSIGN",
    "TOK_DIVIDE",
    "TOK_POW",

    "TOK_BELOW",
    "TOK_BELOW_EQUAL",
    "TOK_ABOVE",
    "TOK_ABOVE_EQUAL",
    "TOK_EQUAL_EQUAL",
    "TOK_NON_EQUAL",

    "TOK_PRINT",
    "TOK_WHILE",
    "TOK_IF",
    "TOK_ELSE",
    "TOK_READ",
    "TOK_DEF",
    "TOK_ID_DEF",
    "TOK_OUT",

    "TOK_SIN",
    "TOK_COS",
    "TOK_TG",
    "TOK_LN",
    "TOK_SQRT",
    "TOK_EXP",
    "TOK_UNARY_MINUS"
};

token_t keyword(const char * word)
{
    if (strcmp(word, "while")  == 0)     return TOK_WHILE;
    if (strcmp(word, "if")     == 0)     return TOK_IF;
    if (strcmp(word, "else")   == 0)     return TOK_ELSE;
    if (strcmp(word, "print")  == 0)     return TOK_PRINT;
    if (strcmp(word, "out")    == 0)     return TOK_OUT;
    if (strcmp(word, "read")   == 0)     return TOK_READ;
    if (strcmp(word, "def")    == 0)     return TOK_DEF;
    if (strcmp(word, "ID_def") == 0)     return TOK_ID_DEF;
    if (strcmp(word, "Sin")    == 0)     return TOK_SIN;
    if (strcmp(word, "Cos")    == 0)     return TOK_COS;
    if (strcmp(word, "Tg")     == 0)     return TOK_TG;
    if (strcmp(word, "Ln")     == 0)     return TOK_LN;
    if (strcmp(word, "Sqrt")   == 0)     return TOK_SQRT;
    if (strcmp(word, "Exp")    == 0)     return TOK_EXP;

    return TOK_IDENTIFIER;
}


ErrorCode lexicalAnalysis(TokenList * token_list)
{
    assert(token_list);
    DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS START");

    char * buffer = NULL;
    ErrorCode error = load_to_buffer(EXPRESSION_INPUT, &buffer);
    if (error != SUCCESS)
        return error;
    DEBUG_PRINT("buffer:\n%s", buffer);

    token_list->capasity = 0;
    token_list->count = 0;
    token_list->data = NULL;
    
    const char * p = buffer;

    while (*p != '\0') 
    {
        skip_spaces(&p);

        if (*p == 'u' && *(p + 1) == '-')
        {
            token_list_push(token_list, make_token(TOK_UNARY_MINUS, p, 2));
            p += 2;
            continue;
        }

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
            else
            {
                ERROR_MESSAGE(SYNTAX_ERROR, error);
                return error;
            }
            continue;
        }

        // operators + punctuation
        switch (*p) 
        {
            case ';':  token_list_push(token_list, make_token(TOK_SEMICOLON, p++, 1)); continue;
            case '{':  token_list_push(token_list, make_token(TOK_LBRACE, p++, 1)); continue;
            case '}':  token_list_push(token_list, make_token(TOK_RBRACE, p++, 1)); continue;
            case '(':  token_list_push(token_list, make_token(TOK_LPAREN, p++, 1)); continue;
            case ')':  token_list_push(token_list, make_token(TOK_RPAREN, p++, 1)); continue;
            case '+':  token_list_push(token_list, make_token(TOK_PLUS, p++, 1)); continue;
            case '-':  token_list_push(token_list, make_token(TOK_MINUS, p++, 1)); continue;
            case '*':  token_list_push(token_list, make_token(TOK_MULTIPLY, p++, 1)); continue;
            case '^':  token_list_push(token_list, make_token(TOK_POW, p++, 1)); continue;
            case '/':  token_list_push(token_list, make_token(TOK_DIVIDE, p++, 1)); continue;
            case '$':  token_list_push(token_list, make_token(TOK_END, p++, 1)); continue;
            case ',': token_list_push(token_list, make_token(TOK_COMMA, p++, 1)); continue;
            case '<':  
            {
                if (*(p + 1) != '\0' && *(p+1) == '=')
                {
                    token_list_push(token_list, make_token(TOK_BELOW_EQUAL, p, 2)); 
                    p += 2;
                    continue;
                }
                else
                {
                    token_list_push(token_list, make_token(TOK_BELOW, p++, 1)); 
                    continue;
                }
            }
            case '>':  
            {
                if (*(p + 1) != '\0' && *(p+1) == '=')
                {
                    token_list_push(token_list, make_token(TOK_ABOVE_EQUAL, p, 2)); 
                    p += 2;
                    continue;
                }
                else
                {
                    token_list_push(token_list, make_token(TOK_ABOVE, p++, 1)); 
                    continue;
                }
            }
            case '=':  
            {
                if (*(p + 1) != '\0' && *(p+1) == '=')
                {
                    token_list_push(token_list, make_token(TOK_EQUAL_EQUAL, p, 2)); 
                    p += 2;
                    continue;
                }
                else
                {
                    token_list_push(token_list, make_token(TOK_ASSIGN, p++, 1)); 
                    continue;
                }
            }
            case '!': 
            {
                if (*(p + 1) != '\0' && *(p+1) == '=')
                {
                    token_list_push(token_list, make_token(TOK_NON_EQUAL, p, 2));
                    p += 2; 
                    continue;
                }
                else    
                    DEBUG_PRINT("Unknown char: %d\n", *p); 
                break;
            }
        }
        if (*p == '\0')
            ;
        else
        {
            printf("Unknown char: %d\n", *p); 
            break;
        }
        p++;
    }

    // EOF token
    token_list_push(token_list, make_token(TOK_EOF, "", 0));
    free(buffer);
    return SUCCESS;
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

void token_list_init(TokenList * token_list)
{
    assert(token_list);
    token_list->data = NULL;
    token_list->count = 0;
    token_list->capasity = 0;
}

void destroy_tokens(TokenList * token_list)
{
    assert(token_list);
    for (size_t i = 0; i < token_list -> count; i++) 
    {
        free(token_list->data[i].string_value);
        token_list->data[i].int_value = 0;
    }
    
    free(token_list->data);
    token_list->data = NULL;
}


const char * get_string_token_type(token_t type)
{
    if (type >= TOK_EOF && type <= TOK_UNARY_MINUS)
        return token_type_strings[type];
    else
        return "Unknown type";
}

void skip_spaces(const char ** s)
{
    while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
        (*s)++;
}


void lexer_dump(const TokenList * token_list) 
{
    assert (token_list);
    
    DEBUG_PRINT("[LEXER DUMP]");
    DEBUG_PRINT("token_list.count = %lu, token_list->capacity = %lu\n", token_list->count, token_list->capasity);
    for (int i = 0; i < token_list->count; i ++)
    {
        DEBUG_PRINT("token_%d, %s, %d, %s", i, get_string_token_type(token_list->data[i].type), token_list->data[i].int_value, token_list->data[i].string_value);
    }
}