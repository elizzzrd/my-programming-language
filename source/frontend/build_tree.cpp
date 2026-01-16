#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "tree_structure.h"
#include "tree_operations.h"
#include "build_tree.h"
#include "lexer.h"


void SyntaxError(int line, const char * func, char ch)
{
    printf("[SYNTAX ERROR] from %s:%d with parsing %c\n", func, line, ch);
    return;
}

/*-----------------------------------GRAMMAR------------------------------------
Program     ::= Statement* '$'

Statement   ::= ( PrintStmt
                | Assignment
                | Expression
                | IfStmt
                | WhileStmt
                ) ';'

PrintStmt   ::= 'print' Expression
Assignment  ::= Identifier '=' Expression
IfStmt      ::= 'if' '(' Expression ')' Block
WhileStmt   ::= 'while' '(' Expression ')' Block

Block ::= '{' Statement* '}'

Expression  ::= AddSub
AddSub      ::= MulDiv ( ('+' | '-') MulDiv )*
MulDiv      ::= Primary ( ('*' | '/') Primary )*
Primary     ::= '(' Expression ')'
                | Number
                | Identifier
                | StringLiteral

StringLiteral ::= '"' [^"]* '"'
Identifier  ::= ['a'-'z', 'A'-'Z', '_']['a'-'z', 'A'-'Z', '0'-'9', '_']*
Number      ::= ['0'-'9']+
------------------------------------------------------------------------------*/


static int start_with_identifier(const char ch)
{
    return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_')); 
}

static int is_identifier(const char ch)
{
    return (start_with_identifier(ch) || (ch >= '0' && ch <= '9'));
}

static void skip_spaces(char ** s)
{
    while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
        (*s)++;
}

#define REQUIRE_STR(str) \
    do { \
        if (strncmp(*s, (str), strlen(str)) != 0) { \
            SyntaxError(__LINE__, __func__, **s); \
            return NULL; \
        } \
        *s += strlen(str); \
    } while(0)

#define REQUIRE_CHAR(ch) \
    do { \
        if (**s != ch) { \
            SyntaxError(__LINE__, __func__, **s); \
            return NULL; \
        } \
        (*s)++;  \
    } while(0)



//  Program     ::= Statement* '$'
Node_t * GetProgram(char ** s, Tree_t * tree)
{
    assert(s && tree);

    Node_t * first_stmt = NULL;
    Node_t * prev_stmt = NULL;
    Node_t * cur_stmt = NULL;

    while (**s != '$' && **s != '\0')
    {
        skip_spaces(s);

        cur_stmt = GetStatement(s, tree);
        if (cur_stmt == NULL)
        {
            SyntaxError(__LINE__, __func__, **s);
            DEBUG_PRINT("[ERROR] Error during parsing statement");
            destroy_tree(tree);
        }
        
        if (!first_stmt)                                              // проверка на парсинг первого выражения
        {
            first_stmt = cur_stmt;
            prev_stmt = cur_stmt;
        }
        else                                                                // связываем через узел разделитель
        {
            Node_result_t sep_res = create_statement_node(tree, OP_END);
            if (sep_res.error != SUCCESS)   return NULL;
            Node_t * sep = sep_res.node;

            sep->left  = prev_stmt;
            sep->right = cur_stmt;

            prev_stmt->prev = sep;
            cur_stmt->prev  = sep;

            prev_stmt = sep;
        }
        
        skip_spaces(s);
        // пропускаем ';'
        if (**s == ';')
            (*s)++;
        else if (**s != '$' && **s != '}' && **s != '\n' && **s != '\0') 
        {
            SyntaxError(__LINE__, __func__, **s);
            return NULL;
        }
    }

    skip_spaces(s);
    REQUIRE_CHAR('$');
    DEBUG_PRINT("all expression was parsed in tree");
    return prev_stmt;
}



//    Statement   ::= ( PrintStmt | Assignment | ExprStmt | IfStmt | WhileStmt ) ';'
Node_t * GetStatement(char ** s, Tree_t * tree)
{
    assert(s && tree);
    skip_spaces(s);

    if (strncmp(*s, "print", 5) == 0)
        return GetPrintStmt(s, tree);
    else if (strncmp(*s, "if", 2) == 0)
        return GetIfStmt(s, tree);
    else if (strncmp(*s, "while", 5) == 0)
        return GetWhileStmt(s, tree);
    else if (start_with_identifier(**s))
    {
        // проверяем на то, что это assignment, ищем '='
        char * saved_substring = *s; 
        while (is_identifier(**s))
            (*s)++;
        
        skip_spaces(s);
        
        if (**s == '=') 
        {
            *s = (char *)saved_substring;
            return GetAssignment(s, tree);
        }
        else 
        {
            *s = (char *)saved_substring;
            return GetExpression(s, tree);
        }
    }
    
    return GetExpression(s, tree);
}


// PrintStmt ::= "print" Expression
Node_t * GetPrintStmt(char ** s, Tree_t * tree)
{
    assert (s && tree);
    REQUIRE_STR("print");
    skip_spaces(s);

    Node_t * expr = GetExpression(s, tree);
    if (!expr)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }

    Node_result_t print_res = create_statement_node(tree, OP_PRINT);
    if (print_res.error != SUCCESS)     return NULL;

    print_res.node->right = expr;
    expr->prev = print_res.node;

    return print_res.node;
}


// Assignment ::= Identifier '=' Expression
Node_t * GetAssignment(char **s, Tree_t * tree)
{
    assert (s && tree);

    Node_t * id = GetIdentifier(s, tree);
    if (!id)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }
    DEBUG_PRINT("identifier %s", symbols_table[SB_VAR].names[(id->value.id_index)]);
    skip_spaces(s);

    REQUIRE_CHAR('=');
    DEBUG_PRINT("'=' was parsed");

    Node_t * expr = GetExpression(s, tree);
    if (!expr)
    {
        SyntaxError(__LINE__, __func__, **s);
        destroy_node(id);
        return NULL;
    }
    DEBUG_PRINT("Expression was parsed");

    Node_result_t assign_res = create_statement_node(tree, OP_ASSIGNMENT);
    if (assign_res.error != SUCCESS)    
    {
        destroy_node(id);
        destroy_node(expr);
        return NULL;
    }
    DEBUG_PRINT("assignment node created");

    assign_res.node-> left = id;
    assign_res.node -> right = expr;

    id->prev = assign_res.node;
    expr->prev = assign_res.node;

    return assign_res.node;
}


// IfStmt ::= "if" '(' Expression ')' Block
Node_t * GetIfStmt(char **s, Tree_t * tree)
{
    assert(s && tree);
    REQUIRE_STR("if");
    skip_spaces(s);

    REQUIRE_CHAR('(');
    Node_t * condition = GetExpression(s, tree);
    if (!condition)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }
    skip_spaces(s);
    REQUIRE_CHAR(')');
    skip_spaces(s);

    Node_t * body = GetBlock(s, tree);
    if (!body)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }

    Node_result_t if_res = create_statement_node(tree, OP_IF);
    if (if_res.error != SUCCESS)    return NULL;

    // left - condition
    // right - body

    if_res.node->left = condition;
    if_res.node->right = body;

    condition->prev = if_res.node;
    body->prev = if_res.node;

    return if_res.node;
}


// WhileStmt ::= "while" '(' Expression ')' Block
Node_t * GetWhileStmt(char ** s, Tree_t * tree)
{
    assert(s && tree);
    skip_spaces(s);
    REQUIRE_STR("while");
    skip_spaces(s);

    REQUIRE_CHAR('(');
    Node_t * condition = GetExpression(s, tree);
    if (!condition)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }
    skip_spaces(s);
    REQUIRE_CHAR(')');

    Node_t * body = GetBlock(s, tree);
    if (!body)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }

    Node_result_t while_res = create_statement_node(tree, OP_WHILE);
    if (while_res.error != SUCCESS)    return NULL;

    // left - condition
    // right - body

    while_res.node->left = condition;
    while_res.node->right = body;

    condition->prev = while_res.node;
    body->prev = while_res.node;

    return while_res.node;
}


// Block ::= '{' Statement '}'
Node_t * GetBlock(char ** s, Tree_t * tree)
{
    assert(s && tree);
    skip_spaces(s);
    REQUIRE_CHAR('{');
    skip_spaces(s);


    Node_t * prev_stmt = NULL;
    Node_t * cur_stmt = NULL;
    Node_t * first_stmt = NULL;

    while (**s != '}' && **s != '\0')
    {
        skip_spaces(s);

        cur_stmt = GetStatement(s, tree);
        if (cur_stmt == NULL)
        {
            SyntaxError(__LINE__, __func__, **s);
            DEBUG_PRINT("[ERROR] Error during parsing statement");
            return NULL;
        }

        if (!first_stmt)                                              // проверка на парсинг первого выражения
        {
            first_stmt = cur_stmt;
            prev_stmt = cur_stmt;
        }
        else                                                                // связываем через узел разделитель
        {
            Node_result_t sep_res = create_statement_node(tree, OP_END);
            if (sep_res.error != SUCCESS)   return NULL;
            Node_t * sep = sep_res.node;

            sep->left  = prev_stmt;
            sep->right = cur_stmt;

            prev_stmt->prev = sep;
            cur_stmt->prev  = sep;

            prev_stmt = sep;
        }
        
        skip_spaces(s);

        // пропускаем ';'
        if (**s == ';')
            (*s)++;
        else if (**s != '}' && **s != '\n' && **s != '\0') 
        {
            SyntaxError(__LINE__, __func__, **s);
            return NULL;
        }
    }

    REQUIRE_CHAR('}');

    Node_result_t block_res = create_statement_node(tree, OP_BLOCK);
    if (block_res.error != SUCCESS)     return NULL;
   
    block_res.node->right = first_stmt;
    if (first_stmt)     first_stmt->prev = block_res.node;

    return block_res.node;
}


// Expression ::= AddSub
Node_t * GetExpression(char ** s, Tree_t * tree)
{
    assert(s && tree);
    return GetAddSub(s, tree);
}


// AddSub ::= MulDiv ( ('+' | '-') MulDiv )*
Node_t * GetAddSub(char ** s, Tree_t * tree)
{
    assert(s && tree);
    skip_spaces(s);

    Node_t * val = GetMulDiv(s, tree);
    skip_spaces(s);
    while ((**s) == '+' || (**s) == '-')
    {
        char op = **s;
        (*s)++;
        skip_spaces(s);
        
        Node_t * val2 = GetMulDiv(s, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }
        Node_result_t res_op = {};
        if (op == '+')
            res_op = create_operator_node(tree, OP_ADD);
        else    
            res_op = create_operator_node(tree, OP_SUB);
        
        if (res_op.error != SUCCESS)    
        {
            destroy_node(val);
            destroy_node(val2);
            return NULL;
        }
        
        res_op.node->left = val;
        res_op.node->right = val2;
        res_op.node->left->prev = res_op.node;
        res_op.node->right->prev = res_op.node;
        
        val = res_op.node;
    }
    return val;
}


// MulDiv ::= Primary( ('*' | '/') Primary )*
Node_t * GetMulDiv(char ** s, Tree_t * tree)
{
    assert(s && tree);
    
    skip_spaces(s);
    Node_t * val = GetPrimaryExpression(s, tree);
    if (!val)
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }
    skip_spaces(s);

    while ((**s) == '*' || (**s) == '/')
    {
        char op = **s;
        (*s)++;
        skip_spaces(s);
        Node_t * val2 = GetPrimaryExpression(s, tree);
        if (val2 == NULL) 
        {
            SyntaxError(__LINE__, __func__, **s);
            return NULL;
        }

        Node_result_t res_op = {};
        if (op == '*')
            res_op = create_operator_node(tree, OP_MUL);
        else if (op == '/')
            res_op = create_operator_node(tree, OP_DIV);
        
        if (res_op.error != SUCCESS)    return NULL;
        
        res_op.node->left = val;
        res_op.node->right = val2;
        res_op.node->left->prev = res_op.node;
        res_op.node->right->prev = res_op.node;
        
        val = res_op.node;
    }
    
    return val;
}


// Primary ::= '(' Expression ')' | Number | Identifier
Node_t * GetPrimaryExpression(char ** s, Tree_t * tree)
{
    assert(s && tree);
    skip_spaces(s);

    if (**s == '(')
    {
        (*s)++;
        skip_spaces(s);
    
        Node_t * val = GetExpression(s, tree);
        if (val == NULL) 
        {
            SyntaxError(__LINE__, __func__, **s);
            return NULL;
        }

        skip_spaces(s);
        if (**s == ')')
            (*s)++;
        else   
            SyntaxError(__LINE__, __func__, **s);
        return val;
    }
    else if ('0' <= **s && **s <= '9')
        return GetNumber(s, tree);
    else if (start_with_identifier(**s))
        return GetIdentifier(s, tree);
    else if (**s == '"')
        return GetStringLiteral(s, tree);
    else
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }
}


// StringLiteral ::= '"' [^"]* '"'
Node_t * GetStringLiteral(char ** s, Tree_t * tree)
{
    assert(s && tree);

    if (**s != '"')
        return NULL;
    (*s)++;

    const char * start = *s;
    while (**s != '\0' && **s != '"')
        (*s)++;
    
    if (**s != '"')
    {
        SyntaxError(__LINE__, __func__, **s);
        return NULL;
    }

    size_t len = (size_t)(*s - start);
    char * str = (char *)calloc(len + 1, sizeof(char));
    if (!str)   return NULL;

    strncpy(str, start, len);
    str[len] = '\0';

    (*s)++;                                 // пропускаем закрывающую кавычку

    Node_result_t str_res = create_string_node(tree, str);
    free(str);
    if (str_res.error != SUCCESS)
        return NULL;

    return str_res.node;
}

//  Identifier ::= ['a'-'z', 'A'-'Z', '_']['a'-'z', 'A'-'Z', '0'-'9', '_']*
Node_t * GetIdentifier(char **s, Tree_t * tree)
{
    assert(s && tree);

    if (!start_with_identifier(**s))
        return NULL;

    const char * start = *s;    
    while (is_identifier(**s))
        (*s)++;
    
    size_t len = *s - start;
    char * name = (char *) calloc(len + 1, sizeof(char));
    if (!name)  return NULL;

    strncpy(name, start, len);
    name[len] = '\0';
    DEBUG_PRINT("var_name was parsed, name = %s", name);

    int id_index = symbol_table_get_or_add(name, SB_VAR);
    free(name);
    Node_result_t var_res = create_identifier_node(tree, id_index);  
    if (var_res.error != SUCCESS)   
    {
        free(name);
        return NULL;
    }
    
    DEBUG_PRINT("node with variable was created");
    return var_res.node;
}


// Number ::= ['0' - '9']+
Node_t * GetNumber(char ** s, Tree_t * tree)
{
    assert(s && tree);

    int val = 0;
    const char * ptr = *s; 
    while ('0' <= (**s) && (**s) <= '9')
    {
        val = val * 10 + (**s - '0');
        (*s)++;
    }
    
    if (*s == ptr)
    {
        SyntaxError(__LINE__, __func__, **s);
    }
    DEBUG_PRINT("Number was read: %d", val);
    Node_result_t res = create_number_node(tree, val);
    if (res.error != SUCCESS)
        return NULL;
    
    DEBUG_PRINT("Node with number was created");    
    return res.node;
}





