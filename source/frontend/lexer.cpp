#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "load_expression.h"
#include "errors.h"
#include "buffer.h"


const char * token_type_strings[] =
{
    "TOK_EOF", "TOK_END", "TOK_NUMBER", "TOK_IDENTIFIER", "TOK_STRING",

    "TOK_SEMICOLON", "TOK_LPAREN", "TOK_RPAREN", "TOK_LBRACE", "TOK_RBRACE", "TOK_COMMA",

    "TOK_MINUS", "TOK_PLUS", "TOK_MULTIPLY", "TOK_ASSIGN", "TOK_DIVIDE", "TOK_POW",

    "TOK_BELOW", "TOK_BELOW_EQUAL", "TOK_ABOVE", "TOK_ABOVE_EQUAL", "TOK_EQUAL_EQUAL", "TOK_NON_EQUAL",

    "TOK_PRINT", "TOK_WHILE", "TOK_IF", "TOK_ELSE", "TOK_READ", "TOK_DEF", "TOK_ID_DEF", "TOK_OUT",

    "TOK_SIN", "TOK_COS", "TOK_TG", "TOK_LN", "TOK_SQRT", "TOK_EXP", "TOK_UNARY_MINUS"
};

const char * get_string_token_type(token_t type)
{
    if (type >= TOK_EOF && type <= TOK_UNARY_MINUS)
        return token_type_strings[type];
    else
        return "UNKNOWN TYPE";
}


token_t is_token_keyword(const char * word)
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


// -------------------------------------------------------------------------------------------------------

frontend_err lexical_analysis(TokenList * token_list)
{
    assert(token_list);
    DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS START");
    frontend_err error = FRONTEND_SUCCESS;
    token_list_ctor(token_list);

    char * buffer = NULL;
    buffer_err b_err = buffer_ctor(buffer, EXPRESSION_INPUT);
    if (b_err != BUFFER_SUCCESS)   
        return LOADING_EXPRESSION_ERROR;
    DEBUG_PRINT("buffer:\n%s", buffer);


    const char * p = buffer;
    while (*p != '\0') 
    {
        skip_spaces_buffer(&p);

        // unary minus
        if (*p == 'u' && *(p + 1) == '-')
        {
            TOKEN_PUSH(make_token(TOK_UNARY_MINUS, p, 2));
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
            if (!word)
                return MEMORY_ALLOCATION_ERROR;

            token_t type = is_token_keyword(word);
            free(word);

            TOKEN_PUSH(make_token(type, start, len));
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
            TOKEN_PUSH(tk);

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
            TOKEN_PUSH(tk);

            if (*p == '"') 
                p++; // skip "
            else
            {
                buffer_dtor(buffer);
                return FORGOTTEN_QUATATION_MARK;
            }
            continue;
        }

        // operators + punctuation
        switch (*p) 
        {
            case ';':  TOKEN_PUSH(make_token(TOK_SEMICOLON, p++, 1));      continue;
            case '{':  TOKEN_PUSH(make_token(TOK_LBRACE,    p++, 1));      continue;
            case '}':  TOKEN_PUSH(make_token(TOK_RBRACE,    p++, 1));      continue;
            case '(':  TOKEN_PUSH(make_token(TOK_LPAREN,    p++, 1));      continue;
            case ')':  TOKEN_PUSH(make_token(TOK_RPAREN,    p++, 1));      continue;
            case '+':  TOKEN_PUSH(make_token(TOK_PLUS,      p++, 1));      continue;
            case '-':  TOKEN_PUSH(make_token(TOK_MINUS,     p++, 1));      continue;
            case '*':  TOKEN_PUSH(make_token(TOK_MULTIPLY,  p++, 1));      continue;
            case '^':  TOKEN_PUSH(make_token(TOK_POW,       p++, 1));      continue;
            case '/':  TOKEN_PUSH(make_token(TOK_DIVIDE,    p++, 1));      continue;
            case '$':  TOKEN_PUSH(make_token(TOK_END,       p++, 1));      continue;
            case ',':  TOKEN_PUSH(make_token(TOK_COMMA,     p++, 1));      continue;
            case '<':  
            {   
                if (*(p + 1) != '\0' && *(p + 1) == '=')
                {
                    TOKEN_PUSH(make_token(TOK_BELOW_EQUAL, p, 2)); 
                    p += 2;
                }
                else
                {
                    TOKEN_PUSH(make_token(TOK_BELOW, p++, 1)); 
                }
                continue;
            }
            case '>':  
            {
                if (*(p + 1) != '\0' && *(p + 1) == '=')
                {
                    TOKEN_PUSH(make_token(TOK_ABOVE_EQUAL, p, 2)); 
                    p += 2;
                }
                else
                {
                    TOKEN_PUSH(make_token(TOK_ABOVE, p++, 1)); 
                }
                continue;
            }
            case '=':  
            {
                if (*(p + 1) != '\0' && *(p + 1) == '=')
                {
                    TOKEN_PUSH(make_token(TOK_EQUAL_EQUAL, p, 2)); 
                    p += 2;
                }
                else
                {
                    TOKEN_PUSH(make_token(TOK_ASSIGN, p++, 1)); 
                }
                continue;
            }
            case '!': 
            {
                if (*(p + 1) != '\0' && *(p + 1) == '=')
                {
                    TOKEN_PUSH( make_token(TOK_NON_EQUAL, p, 2));
                    p += 2; 
                    continue;
                }
                else
                {    
                    buffer_dtor(buffer);
                    DEBUG_PRINT("Unknown char: %d\n", *p); 
                    return UNKNOWN_CHAR;
                }
                break;
            }
        }
        if (*p == '\0')
            ;
        else
        {
            buffer_dtor(buffer);
            printf("Unknown char: %d\n", *p); 
            return UNKNOWN_CHAR;
        }
        p++;
    }

    // EOF token
    if (*p == '$')
        TOKEN_PUSH(make_token(TOK_EOF, "", 0));
    else
    {
        buffer_dtor(buffer);
        return FORGOTTEN_EOF;
    }

    buffer_dtor(buffer);
    return FRONTEND_SUCCESS;
}

// ----------------------------------------------------------------------------------------------------

void token_list_ctor(TokenList * token_list)
{
    assert(token_list);

    token_list->data = NULL;
    token_list->count = 0;
    token_list->capasity = 0;
    return;
}

void token_list_dtor(TokenList * token_list)
{
    assert(token_list);

    for (size_t i = 0; i < token_list->count; i++) 
    {
        free(token_list->data[i].string_value);
        token_list->data[i].string_value = NULL;
        token_list->data[i].int_value = 0;
    }
    
    free(token_list->data);
    token_list->data = NULL;
    return;
}

frontend_err token_list_resize(TokenList * token_list)
{
    assert(token_list);

    size_t new_capacity = (token_list->capasity == 0) ? 8 : (token_list->capasity * 2);
    Token * new_data = (Token *)realloc(token_list -> data, new_capacity * sizeof(Token));
    if (!new_data)     
        return MEMORY_ALLOCATION_ERROR;

    token_list->data = new_data;
    token_list->capasity = new_capacity;
    return FRONTEND_SUCCESS;
}


int token_list_push(TokenList * token_list, Token cur_token)
{
    assert(token_list);

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


void lexer_dump(const TokenList * token_list) 
{
    assert (token_list);
    
    DEBUG_PRINT("[========================== LEXER DUMP ==========================]");
    DEBUG_PRINT("token_list.count = %lu, token_list->capacity = %lu\n", token_list->count, token_list->capasity);
    for (int i = 0; i < token_list->count; i ++)
    {
        DEBUG_PRINT("token_%d, %s, %d, %s", i, get_string_token_type(token_list->data[i].type), 
                                               token_list->data[i].int_value, 
                                               token_list->data[i].string_value);
    }
    DEBUG_PRINT("[========================== LEXER DUMP ==========================]");
    return;
}