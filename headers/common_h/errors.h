#pragma once

#include <stdio.h>


extern int graph_dump_count;
extern int graph_dump_count_node;
extern int graph_dump_diff;

typedef struct  
{
    int error_type;
    const char * error_message;
} error_struct;

#define GEN_ERROR(___TYPE___, ___STRING___) \
    {(___TYPE___), (___STRING___)},

typedef enum 
{
    FRONTEND_SUCCESS,

    NULL_POINTER,
    DELETION_ERROR,
    INVALID_NODE,
    INVALID_OPERATOR,
    CREATING_NODE_ERROR,
    
    OPENING_FILE_ERROR,
    LOADING_EXPRESSION_ERROR,
    GRAPH_DUMP_ERROR,
    MEMORY_ALLOCATION_ERROR,

    LEXER_ERROR,
    PARSER_ERROR,
    SYNTAX_ERROR,
    SEMANTIC_ERROR,

    UNKNOWN_CHAR,
    FORGOTTEN_SEMICOLON,
    FORGOTTEN_QUATATION_MARK,
    FORGOTTEN_L_BRACE,
    FORGOTTEN_R_BRACE,
    FORGOTTEN_L_PAREN,
    FORGOTTEN_R_PAREN,
    FORGOTTEN_COMMA,
    FORGOTTEN_EOF,
    UNEXPECTED_SYNTAX,
    UNDEFINED_ID,
    NO_CONDITION,
    INCORRECT_ARGUMENT_AMOUNT,
    ASSIGNMENT_NO_VALUE, 
    NO_BLOCK,
    NO_EXPECTED_EXPRESSION
} frontend_err;


typedef enum 
{
    OPTIMIZER_SUCCESS,
    OPTIMIZER_ALLOCATION_ERROR,
    OPTIMIZER_BUFFER_ERROR,
    OPTIMIZER_DIVISION_BY_ZERO,
    OPTIMIZER_INVALID_OPERATOR,
    OPTIMIZER_INVALID_NODE,
    OPTIMIZER_CREATING_NODE_ERROR,
    OPTIMIZER_LOADING_EXPRESSION_ERROR
} optimize_err;


typedef enum
{
    BACKEND_SUCCESS,
    BACKEND_OPENING_FILE_ERROR,
    TRANSLATING_TO_ASM_ERROR,
    BACKEND_INVALID_OPERATOR,
    BACKEND_SEMANTIC_ERROR,
    BACKEND_ALLOCATION_ERROR,
    BACKEND_INVALID_NODE
} backend_err;


extern const error_struct frontend_error_list[];
extern const error_struct optimizer_error_list[];
extern const error_struct tree_error_list[];
extern const error_struct backend_error_list[];


#define ERROR_MESSAGE_FRONTEND(error_enum) \
    printf("\n\n%s:%d --- %s\n\n", __FILE__, __LINE__, frontend_error_list[error_enum].error_message);     \

#define ERROR_MESSAGE_OPTIMIZER(error_enum) \
    printf("\n\n%s:%d --- %s\n\n", __FILE__, __LINE__, optimizer_error_list[error_enum].error_message);     \

#define ERROR_MESSAGE_BACKEND(error_enum) \
    printf("\n\n%s:%d --- %s\n\n", __FILE__, __LINE__, backend_error_list[error_enum].error_message);     \

#define ERROR_MESSAGE_TREE(error_enum)   \
    printf("\n\n%s:%d --- %s\n\n", __FILE__, __LINE__, tree_error_list[error_enum].error_message);     \

   

#define DEBUG_LOG_FILE "logger/debug.log"
#define DEBUG_PRINT(fmt, ...)                                               \
    do {                                                                    \
        FILE *dbg_fp = fopen(DEBUG_LOG_FILE, "a");                         \
        if (dbg_fp) {                                                       \
            fprintf(dbg_fp, "%s:%d: %s: " fmt "\n",                       \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);          \
            fclose(dbg_fp);                                                 \
        }                                                                   \
    } while (0)


#define IF_THERE_IS_TRANSLATE_ERROR(error) \
    do { \
        if (error != BACKEND_SUCCESS) \
            { \
                ERROR_MESSAGE_BACKEND(error); \
                return error; \
            } \
    } while(0)


 
// ErrorCode node_verify(Node_t * node)
// ErrorCode tree_verify(Tree_t * tree);

