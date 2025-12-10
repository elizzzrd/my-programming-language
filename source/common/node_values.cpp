#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "node_values.h"
#include "utils.h"
#include "lexer.h"


const char * type_strings[] =
{
    "ROOT",
    "OPERATOR",
    "IDENTIFIER",
    "NUMBER",
    "STATEMENT",
    "STRING"
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

    "sin",          // OP_SIN
    "cos",          // OP_COS
    "tan",          // OP_TAN
    "ctg",          // OP_CTG
    "arcsin",       // OP_ARCSIN
    "arccos",       // OP_ARCCOS
    "arctan",       // OP_ARCTAN
    "arcctg",       // OP_ARCCTG

    "sinh",         // OP_SINH
    "cosh",         // OP_COSH
    "tanh",         // OP_TANH
    "ctgh",         // OP_CTGH

    "exp",          // OP_EXP
    "ln",           // OP_LN

    "sqrt",         // OP_SQRT
    "abs",          // OP_ABS
    "-"             // OP_UNARY_MINUS
};
   
const char * statement_strings[] =
{
    "program",      // OP_PROGRAMM
    "op_statement", // OP_STATEMENT
    "op_end",       // OP_END
    "print",        // PRINT
    "assignment",   // ASSIGNMENT
    "if",           // IF
    "while",        // WHILE
    "block"         // BLOCK
};


const char * get_statement_name(statement_t op)
{
    if (op >= OP_PROGRAM && op <= OP_BLOCK)
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


// const char * var_names[] = 
// {
//     "x",
//     "y",
//     "z",
//     "v",
//     "w",
// };

// const char * get_var_name(int var_index)
// {
//     if (var_index >= 0 && var_index < MAX_VARIABLES)
//         return var_names[var_index];
//     else    
//         return NULL;
// }

token_res check_for_identifier(const char * token)
{
    assert(token);
    
    token_res result = {.type = ROOT};
    int id_index = symbol_table_find(token);
    if (id_index == -1)
        return result;
    else
    {
        result.type = IDENTIFIER;
        result.value.id_index = id_index;
    }
        
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
    return (op >= OP_ADD && op <= OP_POW);
}

bool is_unary_operator(operator_t op) 
{
    return (op >= OP_SIN && op <= OP_UNARY_MINUS);
}

bool is_function_operator(operator_t op) 
{
    return (op >= OP_SIN && op <= OP_ABS);
}



operator_t get_enum_operator_from_string(const char* str) 
{
    if (strcmp(str, "+") == 0)          return OP_ADD;
    if (strcmp(str, "-") == 0)          return OP_SUB;
    if (strcmp(str, "*") == 0)          return OP_MUL;
    if (strcmp(str, "/") == 0)          return OP_DIV;
    if (strcmp(str, "^") == 0)          return OP_POW;
    if (strcmp(str, "sin") == 0)        return OP_SIN;
    if (strcmp(str, "cos") == 0)        return OP_COS;
    if (strcmp(str, "tan") == 0)        return OP_TAN;
    if (strcmp(str, "ctg") == 0)        return OP_CTG;
    if (strcmp(str, "arcsin") == 0)     return OP_ARCSIN;
    if (strcmp(str, "arccos") == 0)     return OP_ARCCOS;
    if (strcmp(str, "arctan") == 0)     return OP_ARCTAN;
    if (strcmp(str, "exp") == 0)        return OP_EXP;
    if (strcmp(str, "ln") == 0)         return OP_LN;
    if (strcmp(str, "sqrt") == 0)       return OP_SQRT;
    if (strcmp(str, "abs") == 0)        return OP_ABS;
    
    return OP_ADD; // default
}


