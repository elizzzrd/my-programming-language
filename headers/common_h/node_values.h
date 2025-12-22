#pragma once

#define MAX_VARIABLES 5
//#include "tree_structure.h"

typedef enum
{
    ROOT,
    OPERATOR,
    IDENTIFIER,
    NUMBER,
    STATEMENT,
    STRING
} type_t;


typedef enum
{
    OP_ADD,
    OP_DIV,
    OP_MUL,
    OP_SUB,
    OP_POW,

    OP_SIN,
    OP_COS,
    OP_TAN,
    OP_CTG,
    OP_ARCSIN,
    OP_ARCCOS,
    OP_ARCTAN,
    OP_ARCCTG,

    OP_SINH,
    OP_COSH,
    OP_TANH,
    OP_CTGH,

    OP_EXP,
    OP_LN,

    OP_SQRT,
    OP_ABS,
    OP_UNARY_MINUS,

} operator_t;

typedef enum
{
    OP_PROGRAM,
    OP_STATEMENT,
    OP_END,
    OP_PRINT,
    OP_ASSIGNMENT,
    OP_IF,
    OP_WHILE,
    OP_BLOCK
} statement_t;

typedef union 
{
    char * root;
    operator_t op;
    statement_t stmt;
    double number;
    int id_index;
    char * string_value;
} value_t;

typedef struct 
{
    type_t type;
    value_t value;
} token_res;



const char * get_string_type(type_t type);
//const char * get_id_name(int id_index);
const char* get_string_operator(operator_t op);
const char * get_statement_name(statement_t op);

operator_t get_enum_operator_from_string(const char* str);

bool is_binary_operator(operator_t op);
bool is_unary_operator(operator_t op); 
bool is_function_operator(operator_t op);

token_res check_for_identifier(const char * token);
token_res check_for_number(const char * token) ;
token_res check_for_operator(const char * token);
token_res check_for_string(const char * token);
token_res check_for_statement(const char * token);