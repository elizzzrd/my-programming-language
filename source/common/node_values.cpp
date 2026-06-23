#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#include "node_values.h"
#include "utils.h"
#include "lexer.h"

#define DISABLE_DEBUG_PRINT

const char * type_strings[] =
{
    "ROOT",
    "OPERATOR",
    "IDENTIFIER",
    "NUMBER",
    "STATEMENT",
    "STRING",
    "INVALID"
};


const char * get_string_type(type_t type)
{
    if (type >= ROOT && type <= STRING)
        return type_strings[type];
    else
        return "Unknown type";
}


const char * operator_strings[] = 
{
    "+",            // OP_ADD
    "/",            // OP_DIV  
    "*",            // OP_MUL
    "-",            // OP_SUB
    "^",            // OP_POW

    "read",         // OP_READ

    "==",           // OP_EQUAL
    "!=",           // OP_NON_EQUAL
    "<",            // OP_BELOW
    "<=",           // OP_BELOW_EQUAL
    ">",            // OP_ABOVE
    ">=",           // OP_ABOVE_EQUAL

    "sin",          // OP_SIN
    "cos",          // OP_COS
    "tan",          // OP_TAN
    // "ctg",          // OP_CTG
    // "arcsin",       // OP_ARCSIN
    // "arccos",       // OP_ARCCOS
    // "arctan",       // OP_ARCTAN
    // "arcctg",       // OP_ARCCTG

    // "sinh",         // OP_SINH
    // "cosh",         // OP_COSH
    // "tanh",         // OP_TANH
    // "ctgh",         // OP_CTGH

    "exp",          // OP_EXP
    "ln",           // OP_LN

    "sqrt",         // OP_SQRT
    //"abs",          // OP_ABS
    "u-"             // OP_UNARY_MINUS
};
   
const char * statement_strings[] =
{
    "program",      // OP_PROGRAMM
    "op_statement", // OP_STATEMENT
    "op_end",       // OP_END
    "print",        // PRINT
    "assignment",   // ASSIGNMENT
    "if",           // IF
    "else",         // ELSE
    "while",        // WHILE
    "block",        // BLOCK

    "func_def",     // OP_FUNC_DEF
    "func_call",    // OP_CALL
    "return",       // OP_RETURN 
    "var_def",      // OP_VAR_DEF

    "params",       // OP_PARAMS
    "args",         // OP_ARGS
};


const char * get_statement_name(statement_t op)
{
    if (op >= OP_PROGRAM && op <= OP_ARGS)
    return statement_strings[op];
    else
    return "Unknown statement";
}


token_res check_for_number(const char * token) 
{
    assert(token);
    
    token_res result = {.type = ROOT};
    char * endptr = NULL;
    double number = strtod(token, &endptr);
    
    if (endptr != token && *endptr == '\0')
    {
        result.type = NUMBER;
        result.value.number = number;
    }
    return result;
}

token_res check_for_operator(const char * token)
{
    assert(token);
    
    token_res result = {.type = ROOT};
    for (int i = OP_ADD; i <= OP_UNARY_MINUS; i++)
    {
        if (strcmp(token, operator_strings[i]) == 0)
        {
            result.type = OPERATOR;
            result.value.op = get_enum_operator_from_string(operator_strings[i]);
        }
    }
    return result;
}

token_res check_for_statement(const char * token)
{
    assert(token);

    token_res res = {.type = ROOT};
    if      (strcmp(token, "program")           == 0)   { res.type = STATEMENT; res.value.stmt = OP_PROGRAM; }
    else if (strcmp(token, "print")             == 0)   { res.type = STATEMENT; res.value.stmt = OP_PRINT; }
    else if (strcmp(token, "assignment")        == 0)   { res.type = STATEMENT; res.value.stmt = OP_ASSIGNMENT; }
    else if (strcmp(token, "if")                == 0)   { res.type = STATEMENT; res.value.stmt = OP_IF; }
    else if (strcmp(token, "else")              == 0)   { res.type = STATEMENT; res.value.stmt = OP_ELSE; }
    else if (strcmp(token, "while")             == 0)   { res.type = STATEMENT; res.value.stmt = OP_WHILE; }
    else if (strcmp(token, "block")             == 0)   { res.type = STATEMENT; res.value.stmt = OP_BLOCK; }
    else if (strcmp(token, "op_end")            == 0)   { res.type = STATEMENT; res.value.stmt = OP_END; }
    else if (strcmp(token, "return")            == 0)   { res.type = STATEMENT; res.value.stmt = OP_RETURN; }
    else if (strcmp(token, "var_def")           == 0)   { res.type = STATEMENT; res.value.stmt = OP_VAR_DEF; }
    else if (strcmp(token, "params")            == 0)   { res.type = STATEMENT; res.value.stmt = OP_PARAMS; }
    else if (strcmp(token, "args")              == 0)   { res.type = STATEMENT; res.value.stmt = OP_ARGS; }
    else if (strcmp(token, "op_statement")      == 0)   { res.type = STATEMENT; res.value.stmt = OP_STATEMENT; }
    else if (strncmp(token, "func_call##", 11)  == 0)   
    { 
        token_res res = {.type = ROOT};

        const char * delimiter = "##";
        const char * pos = strstr(token, delimiter);
        if (pos == NULL)
            return res;
        
        int name_len = strlen(token) - 2 - strlen("func_call");
        char * name_buffer = (char *) calloc(name_len + 1, 1);
        if (!name_buffer)
            return res;
    
        strncpy(name_buffer, pos + 2, name_len);
        name_buffer[name_len] = '\0';

        res.id.name = name_buffer;
        res.type = STATEMENT; 
        res.value.stmt = OP_CALL; 

        return res;
    }
    else if (strncmp(token, "func_def##", 10)   == 0)   
    { 
        token_res res = {.type = ROOT};

        const char * delimiter = "##";
        const char * pos = strstr(token, delimiter);
        if (pos == NULL)
            return res;
        
        int name_len = strlen(token) - 2 - strlen("func_def");
        char * name_buffer = (char *) calloc(name_len + 1, 1);
        if (!name_buffer)
            return res;
    
        strncpy(name_buffer, pos + 2, name_len);
        name_buffer[name_len] = '\0';

        res.type = STATEMENT; 
        res.value.stmt = OP_FUNC_DEF; 
        res.id.name = name_buffer;

        return res;
    }


    return res;
}


    


token_res check_for_string(const char * token)
{
    assert(token);

    token_res res = {.type = ROOT};
    size_t len = strlen(token);
    if (len >= 2 && token[0] == '"' && token[len - 1] == '"')
    {
        res.type = STRING;
        res.value.string_value = strndup(token + 1, len - 2);
        if (!res.value.string_value) 
            res.type = ROOT;
    }
    return res;
}



token_res check_for_identifier(const char * token)
{
    assert(token);
    
    token_res result = {.type = ROOT};

    const char * delimiter = "##";
    
    const char * pos = strstr(token, delimiter);
    if (pos == NULL)
        return result;
    
    int name_len = pos - token;
    char * name_buffer = (char *) malloc(name_len + 1);
    if (!name_buffer)
        return result;
    
    strncpy(name_buffer, token, name_len);
    name_buffer[name_len] = '\0';
    
    const char * digits_start = pos + 2;  
    for (const char * p = digits_start; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p)) 
        {
            free(name_buffer);
            return result;
        }
    }
    
    int id = atoi(digits_start);  
    
    result.type = IDENTIFIER;
    result.id.id_index = id;
    result.id.name = name_buffer;

    return result;
}

const char* get_string_operator(operator_t op) 
{
    if (op >= OP_ADD && op <= OP_UNARY_MINUS) {
        return operator_strings[op];
    }
    return "Unknown operator";
}


bool is_binary_operator(operator_t op) 
{
    return ((op >= OP_ADD && op <= OP_POW) || (op >= OP_EQUAL && op <= OP_ABOVE_EQUAL));
}


bool is_unary_operator(operator_t op) 
{
    return ((op >= OP_SIN && op <= OP_UNARY_MINUS) || (op == OP_READ));
}

bool is_function_operator(operator_t op) 
{
    return (op >= OP_SIN && op <= OP_UNARY_MINUS);
}



operator_t get_enum_operator_from_string(const char* str) 
{
    if (strcmp(str, "+")    == 0)          return OP_ADD;
    if (strcmp(str, "-")    == 0)          return OP_SUB;
    if (strcmp(str, "*")    == 0)          return OP_MUL;
    if (strcmp(str, "/")    == 0)          return OP_DIV;
    if (strcmp(str, "^")    == 0)          return OP_POW;

    if (strcmp(str, "read") == 0)          return OP_READ;

    if (strcmp(str, "==")   == 0)          return OP_EQUAL;
    if (strcmp(str, "!=")   == 0)          return OP_NON_EQUAL;
    if (strcmp(str, "<")    == 0)          return OP_BELOW;
    if (strcmp(str, "<=")   == 0)          return OP_BELOW_EQUAL;
    if (strcmp(str, ">")    == 0)          return OP_ABOVE;
    if (strcmp(str, ">=")   == 0)          return OP_ABOVE_EQUAL;

    if (strcmp(str, "sin")  == 0)          return OP_SIN;
    if (strcmp(str, "cos")  == 0)          return OP_COS;
    if (strcmp(str, "tan")  == 0)          return OP_TAN;
    // if (strcmp(str, "ctg") == 0)        return OP_CTG;
    // if (strcmp(str, "arcsin") == 0)     return OP_ARCSIN;
    // if (strcmp(str, "arccos") == 0)     return OP_ARCCOS;
    // if (strcmp(str, "arctan") == 0)     return OP_ARCTAN;
    if (strcmp(str, "exp")  == 0)          return OP_EXP;
    if (strcmp(str, "ln")   == 0)          return OP_LN;
    if (strcmp(str, "sqrt") == 0)          return OP_SQRT;
    //if (strcmp(str, "abs") == 0)        return OP_ABS;
    if (strcmp(str, "u-")   == 0)          return OP_UNARY_MINUS;
    
    return OP_ADD; // default
}


