#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "tree_structure.h"
#include "tree_operations.h"
#include "build_tree.h"
#include "lexer.h"
#include "syntax_analysis.h"


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
MulDiv      ::= Unary ( ('*' | '/') Unary )*
Unary       ::= '-' Unary | Primary
Primary     ::= '(' Expression ')'
                | Number
                | Identifier
                | StringLiteral

StringLiteral ::= '"' [^"]* '"'
Identifier  ::= ['a'-'z', 'A'-'Z', '_']['a'-'z', 'A'-'Z', '0'-'9', '_']*
Number      ::= ['0'-'9']+
------------------------------------------------------------------------------*/



#define REQUIRE_TOKEN(expected_type, token) \
    do { \
        if (!token) return NULL; \
        if (token->type != (expected_type)) \
            return NULL; \
    } while(0)


static Token * current_token(TokenList * tokens, size_t pos)
{
    if (!tokens || !tokens->data)                return NULL;
    if (pos >= tokens->count)                    return NULL;

    return &tokens->data[pos];
}


//  Program     ::= Statement* '$'
Node_t * GetProgram_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && tree && pos);

    Node_t * first_stmt = NULL;
    Node_t * prev_stmt = NULL;
    Node_t * cur_stmt = NULL;

    Token * t = current_token(tokens, *pos);
    while (t && t->type != TOK_END && t->type != TOK_EOF)
    {
        cur_stmt = GetStatement_tokens(tokens, pos, tree);

        if (cur_stmt == NULL)
        {
            DEBUG_PRINT("[ERROR] Error during parsing statement");
            destroy_tree(tree);
            return NULL;
        }
        
        if (!first_stmt)                                                    // проверка на парсинг первого выражения
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
        
        t = current_token(tokens, *pos);
        if (t && t->type == TOK_SEMICOLON)
            (*pos)++;
        else if (t && (t->type == TOK_END || t->type == TOK_EOF || t->type == TOK_RBRACE)) // конец программы или блока
        {
            ;
        }
        else
        {
            DEBUG_PRINT("[SYNTAX ERROR] expected ';' oe end\n");
            DEBUG_PRINT("got token %s at %lu", get_string_token_type(t->type), *pos);
            return NULL;
        }

        t = current_token(tokens, *pos);
    }

    
    REQUIRE_TOKEN(TOK_END, t);
    DEBUG_PRINT("all expression was parsed in tree");
    return prev_stmt;
}



//    Statement   ::= ( PrintStmt | Assignment | ExprStmt | IfStmt | WhileStmt ) ';'
Node_t * GetStatement_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    
    Token * t = current_token(tokens, *pos);
    if (!t)     return NULL;

    if (t -> type == TOK_PRINT)
        return GetPrintStmt_tokens(tokens, pos, tree);
    else if (t -> type == TOK_IF)
        return GetIfStmt_tokens(tokens, pos, tree);
    else if (t -> type == TOK_WHILE)
        return GetWhileStmt_tokens(tokens, pos, tree);
    else if (t -> type == TOK_IDENTIFIER)
    {
        // ищем assignment
        Token * next = current_token(tokens, *(pos) + 1);
        if (next && next->type == TOK_ASSIGN)
            return GetAssignment_tokens(tokens, pos, tree);
    }
    return GetExpression_tokens(tokens, pos, tree);
}


// PrintStmt ::= "print" Expression
Node_t * GetPrintStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    REQUIRE_TOKEN(TOK_PRINT, current_token(tokens, *pos));
    (*pos)++;

    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expects expression\n");
        return NULL;
    }

    Node_result_t print_res = create_statement_node(tree, OP_PRINT);
    if (print_res.error != SUCCESS)     return NULL;

    print_res.node->right = expr;
    expr->prev = print_res.node;

    return print_res.node;
}


// Assignment ::= Identifier '=' Expression
Node_t * GetAssignment_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);

    Token * t = current_token(tokens, *pos);
    if (!t || t -> type != TOK_IDENTIFIER)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected identifier\n");
        return NULL;
    }

    int id_index = symbol_table_get_or_add(t->string_value);
    (*pos)++; 
    Node_result_t id_res = create_identifier_node(tree, id_index);
    if (id_res.error != SUCCESS)    return NULL;

    REQUIRE_TOKEN(TOK_ASSIGN, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expects expression after '='\n");
        destroy_node(id_res.node);
        return NULL;
    }

    Node_result_t assign_res = create_statement_node(tree, OP_ASSIGNMENT);
    if (assign_res.error != SUCCESS)    
    {
        destroy_node(id_res.node);
        destroy_node(expr);
        return NULL;
    }
    DEBUG_PRINT("assignment node created");

    assign_res.node-> left = id_res.node;
    assign_res.node -> right = expr;

    id_res.node->prev = assign_res.node;
    expr->prev = assign_res.node;

    return assign_res.node;
}


// IfStmt ::= "if" '(' Expression ')' Block
Node_t * GetIfStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    REQUIRE_TOKEN(TOK_IF, current_token(tokens, (*pos)));
    (*pos)++;

    REQUIRE_TOKEN(TOK_LPAREN, current_token(tokens, (*pos)));
    (*pos)++;
    Node_t * condition = GetExpression_tokens(tokens, pos, tree);
    if (!condition)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected condition\n");
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RPAREN, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * body = GetBlock_tokens(tokens, pos, tree);
    if (!body)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected block\n");
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
Node_t * GetWhileStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    
    REQUIRE_TOKEN(TOK_WHILE, current_token(tokens, (*pos)));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LPAREN, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * condition = GetExpression_tokens(tokens, pos, tree);
    if (!condition)
    {
        printf("[SYNTAX ERROR] parser expected condition\n");
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RPAREN, current_token(tokens, (*pos)));
    (*pos)++;

   Node_t * body = GetBlock_tokens(tokens, pos, tree);
    if (!body)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected block\n");
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


// Block ::= '{' Statement* '}'
Node_t * GetBlock_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * prev_stmt = NULL;
    Node_t * cur_stmt = NULL;
    Node_t * first_stmt = NULL;

    Token * t = current_token(tokens, *pos);
    while (t && t -> type != TOK_RBRACE && t -> type != TOK_EOF)
    {
        if (!t || t->type == TOK_RBRACE || t->type == TOK_EOF)
            break;

        cur_stmt = GetStatement_tokens(tokens, pos, tree);
        if (cur_stmt == NULL)
        {
            DEBUG_PRINT("[ERROR] Error during parsing statement");
            destroy_tree(tree);
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
        
        t = current_token(tokens, *pos);
        if (t && t->type == TOK_SEMICOLON)
            (*pos)++;
        else if (t && (t->type == TOK_END || t->type == TOK_EOF || t->type == TOK_RBRACE)) // конец программы или блока
        {
            ;
        }
        else
        {
            DEBUG_PRINT("[SYNTAX ERROR] expected ';' oe end\n");
            DEBUG_PRINT("got token %s at %lu", get_string_token_type(t->type), *pos);
            destroy_tree(tree);
            return NULL;
        }

        t = current_token(tokens, *pos);
    }

    REQUIRE_TOKEN(TOK_RBRACE, t);
    (*pos)++;

    Node_result_t block_res = create_statement_node(tree, OP_BLOCK);
    if (block_res.error != SUCCESS)     return NULL;
   
    block_res.node->right = prev_stmt;
    if (prev_stmt)
        prev_stmt->prev = block_res.node;

    return block_res.node;
}


// Expression ::= AddSub
Node_t * GetExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    return GetAddSub_tokens(tokens, pos, tree);
}


// AddSub ::= MulDiv ( ('+' | '-') MulDiv )*
Node_t * GetAddSub_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetMulDiv_tokens(tokens, pos, tree);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_PLUS || t -> type == TOK_MINUS)
    {
        token_t op = t -> type;
        (*pos)++;
        
        Node_t * val2 = GetMulDiv_tokens(tokens, pos, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }

        Node_result_t res_op = {};
        if (op == TOK_PLUS)
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
        t = current_token(tokens, *pos);
    }
    return val;
}

// MulDiv      ::= Unary ( ('*' | '/') Unary )*
Node_t * GetMulDiv_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetUnary_tokens(tokens, pos, tree);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_MULTIPLY || t -> type == TOK_DIVIDE)
    {
        token_t op = t -> type;
        (*pos)++;
        
        Node_t * val2 = GetUnary_tokens(tokens, pos, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }

        Node_result_t res_op = {};
        if (op == TOK_MULTIPLY)
            res_op = create_operator_node(tree, OP_MUL);
        else    
            res_op = create_operator_node(tree, OP_DIV);
        
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
        t = current_token(tokens, *pos);
    }
    return val;
}


// Unary       ::= '-' Unary | Primary
Node_t * GetUnary_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Token * t = current_token(tokens, *pos);
    if (!t)     return NULL;

    if (t->type == TOK_MINUS)
    {
        (*pos)++;

        Node_t * operand = GetUnary_tokens(tokens, pos, tree);
        if (!operand)
        {
            DEBUG_PRINT("[SYNTAX ERROR] expected expression after unary '-'\n");
            return NULL;
        }

        Node_result_t res = create_operator_node(tree, OP_UNARY_MINUS);
        if (res.error != SUCCESS)
            return NULL;
        
        res.node->right = operand;
        operand->prev = res.node;

        return res.node;
    }
    if (t->type == TOK_IDENTIFIER)
    {
        operator_t op = get_enum_operator_from_string(t->string_value);

        if (is_function_operator(op))
        {
            (*pos)++;

            Node_t * arg = GetUnary_tokens(tokens, pos, tree);
            if (!arg)   return NULL;

            Node_result_t res = create_operator_node(tree, op);
            if (res.error != SUCCESS)   return NULL;

            res.node->right = arg;
            arg->prev = res.node;

            return res.node;
        }
    }
    
    return GetPrimaryExpression_tokens(tokens, pos, tree);
}

// Primary ::= '(' Expression ')' | Number | Identifier
Node_t * GetPrimaryExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Token * t = current_token(tokens, *pos);
    if (!t)     return NULL;

    if (t -> type == TOK_LPAREN)
    {
        (*pos)++;
    
        Node_t * val = GetExpression_tokens(tokens, pos, tree);
        if (val == NULL) 
        {
            DEBUG_PRINT("[SYNTAX ERROR] parser expected expression\n");
            return NULL;
        }

        REQUIRE_TOKEN(TOK_RPAREN, current_token(tokens, (*pos)));
        (*pos)++;
        return val;
    }
    else if (t -> type == TOK_NUMBER)
    {
        int value = t->int_value;
        (*pos)++;
        Node_result_t num_res = create_number_node(tree, value);
        if (num_res.error != SUCCESS)   return NULL;
        return num_res.node;
    }
    else if (t -> type == TOK_IDENTIFIER)
    {
        int id_index = symbol_table_get_or_add(t->string_value);
        (*pos)++;
        Node_result_t id_res = create_identifier_node(tree, id_index);
        if (id_res.error != SUCCESS)    return NULL;
        return id_res.node;
    }
    else if (t -> type == TOK_STRING)
    {
        char * str = strdup(t->string_value);
        (*pos)++;
        Node_result_t str_res = create_string_node(tree, str);
        free(str);
        if (str_res.error != SUCCESS)   return NULL;
        return str_res.node;
    }
    else
    {
        printf("[SYNTAX ERROR] Unexpected token\n");
        return NULL;
    }
}

