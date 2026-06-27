#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tree_structure.h"
#include "tree_operations.h"
#include "lexer.h"
#include "syntax_analysis.h"



/*-----------------------------------GRAMMAR------------------------------------
Program     ::= Statement* '$'

Statement   ::=   Assignment
                | VarDef
                | FuncDef
                | FuncCall
                | Return
                | PrintStmt
                | IfStmt
                | WhileStmt
                 ';'



IfStmt      ::= 'if' '{' Expression '}' Block ('else' Block)?
WhileStmt   ::= 'while' '{' Expression '}' Block
Block ::= '{''{' Statement* '}''}'

------------------------------------------------------------------------------
Присваивание и объявление переменных
------------------------------------------------------------------------------
Assignment  ::= Var '=' Expression
VarDef      ::= 'ID_def' Var '=' Expression

Var         ::= Identifier

------------------------------------------------------------------------------
Функции
------------------------------------------------------------------------------
FuncDef     ::= 'def' Identifier '{' ParamList?'}' Block
ParamList   ::= Var ( ',' Var)*

FuncCall    ::= Identifier '{' ArgList? '}'
ArgList     ::= Expression ( ',' Expression)*

Return      ::= 'out' Expression

------------------------------------------------------------------------------
Ввод - вывод
------------------------------------------------------------------------------
InputStmt   ::= 'read' '{' '}'
PrintStmt   ::= 'print' '{' Expression '}'

------------------------------------------------------------------------------
Приоритеты операций
------------------------------------------------------------------------------
Expression  ::= Equality
Equality    ::= Comp ( ('==' | '!=') Comp)
Comp        ::= AddSub ( ('<' | '>' | '<=' | '>=') AddSub)*
AddSub      ::= MulDiv ( ('+' | '-') MulDiv )*
MulDiv      ::= Pow ( ('*' | '/') Pow )*
Pow         ::= Unary ('^' Unary)*
Unary       ::=  ( '+' | '-' | FuncOper )? Primary
FuncOper    ::= ('Sin' | 'Cos' | 'Tg' | 'Ln' | 'Sqrt' ) '{' Expression'}'
Primary     ::= '(' Expression ')'
                | Number
                | Identifier
                | StringLiteral
                | FuncCall
                | InputStmt  

------------------------------------------------------------------------------
Лексика
------------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------------------------------------------------



//  Program     ::= Statement* '$'
Node_t * GetProgram_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && tree && pos);
    *error = FRONTEND_SUCCESS;
    DEBUG_PRINT("\n[INFO] SYNTAX_ANALYSIS START");

    Node_t * first_stmt = NULL;
    Node_t * prev_stmt = NULL;
    Node_t * cur_stmt = NULL;

    Token * t = current_token(tokens, *pos);
    while (t && t->type != TOK_END && t->type != TOK_EOF)
    {
        cur_stmt = GetStatement_tokens(tokens, pos, tree, error);

        if (cur_stmt == NULL)
        {
            *error = PARSER_ERROR;
            return NULL;
        }
        
        if (!first_stmt)                                                    // проверка на парсинг первого выражения
        {
            first_stmt = cur_stmt;
            prev_stmt = cur_stmt;
        }
        else                                                                // связываем через узел разделитель
        {
            Node_result_t sep_res = STATEMENT_NODE(OP_END);
            if (sep_res.error != TREE_SUCCESS)   
            {
                *error = CREATING_NODE_ERROR;
                return NULL;
            }
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
            *error = FORGOTTEN_SEMICOLON;
            return NULL;
        }

        t = current_token(tokens, *pos);
    }

    
    REQUIRE_TOKEN(TOK_END, t);
    DEBUG_PRINT("all expression was parsed in tree");
    return prev_stmt;
}



//    Statement   ::=  PrintStmt | Assignment | IfStmt | WhileStmt |
//                     VarDef | FuncCall | FuncDef | Return';'
Node_t * GetStatement_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    * error = FRONTEND_SUCCESS;
    
    Token * t = current_token(tokens, *pos);
    if (!t)     
    {
        *error = PARSER_ERROR;
        ERROR_MESSAGE_FRONTEND(PARSER_ERROR);
        return NULL;
    }

    if (t -> type == TOK_PRINT)
        return GetPrintStmt_tokens(tokens, pos, tree, error);
    else if (t -> type == TOK_IF)
        return GetIfStmt_tokens(tokens, pos, tree, error);
    else if (t -> type == TOK_WHILE)
        return GetWhileStmt_tokens(tokens, pos, tree, error);
    else if (t -> type == TOK_IDENTIFIER)
    {
        // ищем assignment
        Token * next = current_token(tokens, *(pos) + 1);
        if (next && next->type == TOK_ASSIGN)
            return GetAssignment_tokens(tokens, pos, tree, error);
        else if (next && next->type == TOK_LBRACE)
            return GetFuncCall_tokens(tokens, pos, tree, error);

        *error = UNEXPECTED_SYNTAX;
    }
    else if (t -> type == TOK_DEF)
        return GetFuncDef(tokens, pos, tree, error);
    else if (t -> type == TOK_ID_DEF)
        return GetVarDef_tokens(tokens, pos, tree, error);
    else if (t -> type == TOK_OUT)
        return GetReturn_tokens(tokens, pos, tree, error);
    else if (t -> type == TOK_READ)
        return GetInputStmt(tokens, pos, tree, error);
    return GetExpression_tokens(tokens, pos, tree, error);
}


// FuncDef     ::= 'def' Identifier '{' ParamList?'}' Block
Node_t * GetFuncDef(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    REQUIRE_TOKEN(TOK_DEF, current_token(tokens, (*pos)));
    (*pos)++;

    Token * name = current_token(tokens, (*pos));
    REQUIRE_TOKEN(TOK_IDENTIFIER, name);
    (*pos)++;

    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * params = NULL;
    if (current_token(tokens, (*pos))->type != TOK_RBRACE)
    {
        params = GetParams_tokens(tokens, pos, tree, error);
        if (!params)
        {
            *error = PARSER_ERROR;
            return NULL;
        }
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * body = GetBlock_tokens(tokens, pos, tree, error);
    if (!body)
    {
        *error = PARSER_ERROR;
        return NULL;
    }

    Node_result_t res = STATEMENT_NODE(OP_FUNC_DEF);
    if (res.error != TREE_SUCCESS)
    {
        *error = PARSER_ERROR;
        if (res.node)
            node_dtor(res.node);
        return NULL;
    }
    res.node->id.name = strdup(name->string_value);
    res.node->left = params;
    res.node->right = body;
    if (params)     params->prev = res.node;
    
    return res.node;
}


//ParamList   ::= Var ( ',' Var)*
Node_t * GetParams_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Token * t = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, t);

    Node_result_t first_var = ID_NODE(t->string_value);
    if (first_var.error != TREE_SUCCESS)
    {
        *error = SYNTAX_ERROR;
        if (first_var.node)
            node_dtor(first_var.node);
        return NULL;
    }
    (*pos)++;

    Node_t * list = STATEMENT_NODE(OP_PARAMS).node;
    list->param_count = 1;
    if (!list)
    {
        *error = SYNTAX_ERROR;
        if (list)
            node_dtor(list);
        return NULL;
    }
    list -> right = first_var.node;
    first_var.node->prev = list;

    Node_t * prev = first_var.node;
    while (current_token(tokens, (*pos))->type == TOK_COMMA)
    {
        (*pos)++;

        t = current_token(tokens, (*pos));
        REQUIRE_TOKEN(TOK_IDENTIFIER, t);

        Node_result_t var = ID_NODE(t->string_value);
        if (var.error != TREE_SUCCESS)
        {
            *error = CREATING_NODE_ERROR;
            if (var.node)
                node_dtor(var.node);
            return NULL;
        }
        (*pos)++;

        prev->right = var.node;
        var.node->prev = prev;
        list->param_count++;
        prev = var.node;
    }

    return list;
}


// ArgList   ::= Expression ( ',' Expression)*
Node_t * GetArgs_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Node_t * first_expr = GetExpression_tokens(tokens, pos, tree, error);
    if (!first_expr)
    {
        *error = SYNTAX_ERROR;
        return NULL;
    }

    Node_t * list = STATEMENT_NODE(OP_ARGS).node;
    if (!list)
    {
        *error = CREATING_NODE_ERROR;
        if (list)
            node_dtor(list);
        return NULL;
    }
    list->param_count = 1;
    list->right = first_expr;
    first_expr->prev = list;

    Node_t * last = first_expr;
    while (current_token(tokens, (*pos)) && current_token(tokens, (*pos))->type == TOK_COMMA)
    {
        (*pos)++;

        Node_t * next_expr = GetExpression_tokens(tokens, pos, tree, error);
        if (!next_expr)
        {
            *error = SYNTAX_ERROR;
            node_dtor(list);
            return NULL;
        }

        last->right = next_expr;
        next_expr->prev = last;
        last = next_expr;
        list->param_count++;
    }
    return list;
}


// Return ::= 'out' Expression
Node_t * GetReturn_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    REQUIRE_TOKEN(TOK_OUT, current_token(tokens, (*pos)));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree, error);
    if (!expr)
    {
        *error = PARSER_ERROR;
        return NULL;
    }

    Node_result_t res = STATEMENT_NODE(OP_RETURN);
    if (res.error != TREE_SUCCESS)
    {
        *error = PARSER_ERROR;
        if (res.node)
            node_dtor(res.node);
        return NULL;
    }

    res.node->right = expr;
    expr->prev = res.node;

    return res.node;
}


// FuncCall    ::= Identifier '{' ArgList? '}'
Node_t * GetFuncCall_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Token * name = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, name);
    (*pos)++;
    
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * args = NULL;
    if (current_token(tokens, (*pos))->type != TOK_RBRACE)
    {
        args = GetArgs_tokens(tokens, pos, tree, error);
        if (!args)
        {
            *error = PARSER_ERROR;
            return NULL;
        }
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_result_t call = STATEMENT_NODE(OP_CALL);
    if (call.error != TREE_SUCCESS)
    {
        *error = PARSER_ERROR;
        if (call.node)  
            node_dtor(call.node);
        return NULL;
    }
    call.node -> right = args;
    call.node -> id.name = strdup(name->string_value);

    if (args)     args->prev = call.node;
    
    return call.node;
}


//PrintStmt   ::= 'print' '{' Expression '}'
Node_t * GetPrintStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    REQUIRE_TOKEN(TOK_PRINT, current_token(tokens, *pos));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;

    Node_t * expr = GetExpression_tokens(tokens, pos, tree, error);
    if (!expr)
    {
        *error = SYNTAX_ERROR;
        return NULL;
    }
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
    (*pos)++;

    Node_result_t print_res = STATEMENT_NODE(OP_PRINT);
    if (print_res.error != TREE_SUCCESS) 
    {
        *error = PARSER_ERROR;
        if (print_res.node)
            node_dtor(print_res.node);
        return NULL;
    }    

    print_res.node->right = expr;
    expr->prev = print_res.node;

    return print_res.node;
}



//InputStmt   ::= 'read' '{' IDENTIFIER '}'
Node_t * GetInputStmt(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    REQUIRE_TOKEN(TOK_READ, current_token(tokens, *pos));
    (*pos)++;

    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;

    Token * token = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, token);

    Node_result_t var_node = ID_NODE(token->string_value);
    if (var_node.error != TREE_SUCCESS)
    {
        *error = SYNTAX_ERROR;
        if (var_node.node)
            node_dtor(var_node.node);
        return NULL;
    }
    (*pos)++;

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
    (*pos)++;
    
    Node_result_t res = OPERATOR_NODE(OP_READ);
    if (res.error != TREE_SUCCESS)
    {
        *error = CREATING_NODE_ERROR;
        if (res.node)
            node_dtor(res.node);
        return NULL;
    }

    res.node->left = var_node.node;
    var_node.node->prev = res.node;

    return res.node;
}


// Assignment ::= Identifier '=' Expression
Node_t * GetAssignment_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Token * t = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, t);
    (*pos)++;

    Node_result_t id_res = ID_NODE(t->string_value);
    if (id_res.error != TREE_SUCCESS)    
    {
        *error = CREATING_NODE_ERROR;
        if (id_res.node)
            node_dtor(id_res.node);
        return NULL;
    }

    REQUIRE_TOKEN(TOK_ASSIGN, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree, error);
    if (!expr)
    {
        *error = ASSIGNMENT_NO_VALUE;
        node_dtor(id_res.node);
        return NULL;
    }

    Node_result_t assign_res = STATEMENT_NODE(OP_ASSIGNMENT);
    if (assign_res.error != TREE_SUCCESS)    
    {
        node_dtor(id_res.node);
        node_dtor(expr);
        *error = CREATING_NODE_ERROR;
        if (assign_res.node)
            node_dtor(assign_res.node);
        return NULL;
    }

    assign_res.node -> left = id_res.node;
    assign_res.node -> right = expr;

    id_res.node->prev = assign_res.node;
    expr->prev = assign_res.node;

    return assign_res.node;
}

//VarDef      ::= 'ID_def' Var '=' Expression
Node_t * GetVarDef_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert (tokens && pos && tree);
    *error = FRONTEND_SUCCESS;
    REQUIRE_TOKEN(TOK_ID_DEF, current_token(tokens, *pos));
    (*pos)++;

    Token * t = current_token(tokens, *pos);
    if (!t || t -> type != TOK_IDENTIFIER)
    {
        *error = UNDEFINED_ID;
        return NULL;
    }

    Node_result_t id_res = ID_NODE(t->string_value);
    if (id_res.error != TREE_SUCCESS)    
    {
        *error = CREATING_NODE_ERROR;
        if (id_res.node)
            node_dtor(id_res.node);
        return NULL;
    }
    (*pos)++;

    REQUIRE_TOKEN(TOK_ASSIGN, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree, error);
    if (!expr)
    {
        *error = ASSIGNMENT_NO_VALUE;
        node_dtor(id_res.node);
        return NULL;
    }

    Node_result_t def_res = STATEMENT_NODE(OP_VAR_DEF);
    if (def_res.error != TREE_SUCCESS)    
    {
        node_dtor(id_res.node);
        node_dtor(expr);
        *error = CREATING_NODE_ERROR;
        if (def_res.node)
            node_dtor(def_res.node);
        return NULL;
    }

    def_res.node -> left = id_res.node;
    def_res.node -> right = expr;

    id_res.node->prev = def_res.node;
    expr->prev = def_res.node;

    return def_res.node;
}



// IfStmt ::= "if" '{' Expression '}' Block ('else' Block)?
Node_t * GetIfStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;
    REQUIRE_TOKEN(TOK_IF, current_token(tokens, (*pos)));
    (*pos)++;

    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;
    Node_t * condition = GetExpression_tokens(tokens, pos, tree, error);
    if (!condition)
    {
        *error = NO_CONDITION;
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * if_body = GetBlock_tokens(tokens, pos, tree, error);
    if (!if_body)
    {
        *error = NO_BLOCK;
        return NULL;
    }

    Node_t * else_body = NULL;
    if (current_token(tokens, (*pos))->type == TOK_ELSE)
    {
        (*pos)++;
        else_body = GetBlock_tokens(tokens, pos, tree, error);
        if (!else_body)
        {
            *error = NO_BLOCK;
            return NULL;
        }   
    }

    Node_result_t if_res = STATEMENT_NODE(OP_IF);
    if (if_res.error != TREE_SUCCESS)    
    {
        *error = CREATING_NODE_ERROR;
        if (if_res.node)    
            node_dtor(if_res.node);
        return NULL;
    }

    Node_result_t stmt_res = STATEMENT_NODE(OP_STATEMENT);
    if (stmt_res.error != TREE_SUCCESS)  
    {
        *error = CREATING_NODE_ERROR;
        if (stmt_res.node)
            node_dtor(stmt_res.node);
        if (if_res.node)    
            node_dtor(if_res.node);
        return NULL;
    }

    if_res.node->left = condition;
    if_res.node->right = stmt_res.node;

    stmt_res.node->left = if_body;
    stmt_res.node->right = else_body;

    if_body->prev = stmt_res.node;
    if (else_body)  else_body->prev = stmt_res.node; 

    condition->prev = if_res.node;
    stmt_res.node->prev = if_res.node;

    return if_res.node;
}


// WhileStmt ::= "while" '{' Expression '}' Block
Node_t * GetWhileStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;
    
    REQUIRE_TOKEN(TOK_WHILE, current_token(tokens, (*pos)));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * condition = GetExpression_tokens(tokens, pos, tree, error);
    if (!condition)
    {
        *error = NO_CONDITION;
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

   Node_t * body = GetBlock_tokens(tokens, pos, tree, error);
    if (!body)
    {
        *error = NO_BLOCK;
        return NULL;
    }

    Node_result_t while_res = STATEMENT_NODE(OP_WHILE);
    if (while_res.error != TREE_SUCCESS)    
    {
        *error = CREATING_NODE_ERROR;
        if (while_res.node)
            node_dtor(while_res.node);
        return NULL;
    }

    // left - condition
    // right - body

    while_res.node->left = condition;
    while_res.node->right = body;

    condition->prev = while_res.node;
    body->prev = while_res.node;

    return while_res.node;
}

//Block ::= '{''{' Statement* '}''}'
Node_t * GetBlock_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err  * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;
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

        cur_stmt = GetStatement_tokens(tokens, pos, tree, error);
        if (cur_stmt == NULL)
        {
            *error = PARSER_ERROR;
            return NULL;
        }

        if (!first_stmt)                                                    // проверка на парсинг первого выражения
        {
            first_stmt = cur_stmt;
            prev_stmt = cur_stmt;
        }
        else                                                                // связываем через узел разделитель
        {
            Node_result_t sep_res = STATEMENT_NODE(OP_END);
            if (sep_res.error != TREE_SUCCESS)   
            {
                *error = CREATING_NODE_ERROR;
                if (sep_res.node)
                    node_dtor(sep_res.node);
                return NULL;
            }

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
            *error = FORGOTTEN_SEMICOLON;
            if (prev_stmt)
                node_dtor(prev_stmt);
            return NULL;
        }

        t = current_token(tokens, *pos);
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_result_t block_res = STATEMENT_NODE(OP_BLOCK);
    if (block_res.error != TREE_SUCCESS)     
    {
        *error = CREATING_NODE_ERROR;
        if (block_res.node)
            node_dtor(block_res.node);
        node_dtor(prev_stmt);
        return NULL;
    }
   
    block_res.node->right = prev_stmt;
    if (prev_stmt)
        prev_stmt->prev = block_res.node;

    return block_res.node;
}


// Expression ::= AddSub
Node_t * GetExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;
    return GetEquality_tokens(tokens, pos, tree, error);
}



// Equality    ::= Comp ( ('==' | '!=') Comp)
Node_t * GetEquality_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS; 

    Node_t * val = GetComp_tokens(tokens, pos, tree, error);
    if (!val)   
    {
        *error = PARSER_ERROR;
        return NULL;
    }

    Token * t = current_token(tokens, *pos);
    if (t -> type == TOK_EQUAL_EQUAL || t -> type == TOK_NON_EQUAL)
    {
        (*pos)++;
        operator_t op = (t -> type == TOK_EQUAL_EQUAL) ? OP_EQUAL : OP_NON_EQUAL;
        
        Node_t * val2 = GetComp_tokens(tokens, pos, tree, error);
        if (!val2)
        {
            node_dtor(val);
            *error = PARSER_ERROR;
            return NULL;
        }

        Node_result_t res_op = {};
        res_op = OPERATOR_NODE(op);
        if (res_op.error != TREE_SUCCESS)    
        {
            node_dtor(val);
            node_dtor(val2);
            *error = CREATING_NODE_ERROR;
            if (res_op.node)
                node_dtor(res_op.node);
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



// Comp        ::= AddSub ( ('<' | '>' | '<=' | '>=') AddSub)*
Node_t * GetComp_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Node_t * val = GetAddSub_tokens(tokens, pos, tree, error);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_BELOW || t -> type == TOK_BELOW_EQUAL ||
           t -> type == TOK_ABOVE || t -> type == TOK_ABOVE_EQUAL)
    {
        (*pos)++;
        operator_t op;
        switch (t->type)
        {
            case TOK_BELOW:         op = OP_BELOW;  break;
            case TOK_BELOW_EQUAL:   op = OP_BELOW_EQUAL;  break;
            case TOK_ABOVE:         op = OP_ABOVE;  break;
            case TOK_ABOVE_EQUAL:   op = OP_ABOVE_EQUAL;   break;
            default: return NULL;
        }
        
        Node_t * val2 = GetAddSub_tokens(tokens, pos, tree, error);
        if (!val2)
        {
            node_dtor(val);
            *error = PARSER_ERROR;
            return NULL;
        }

        Node_result_t res_op = OPERATOR_NODE(op);
        if (res_op.error != TREE_SUCCESS)    
        {
            node_dtor(val);
            node_dtor(val2);
            *error = CREATING_NODE_ERROR;
            if (res_op.node)
                node_dtor(res_op.node);
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


// AddSub ::= MulDiv ( ('+' | '-') MulDiv )*
Node_t * GetAddSub_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Node_t * val = GetMulDiv_tokens(tokens, pos, tree, error);
    if (!val)   
    {
        *error = PARSER_ERROR;
        return NULL;
    }

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_PLUS || t -> type == TOK_MINUS)
    {
        token_t op = t -> type;
        (*pos)++;
        
        Node_t * val2 = GetMulDiv_tokens(tokens, pos, tree, error);
        if (!val2)
        {
            node_dtor(val);
            *error = PARSER_ERROR;
            return NULL;
        }

        Node_result_t res_op = {};
        if (op == TOK_PLUS)
            res_op = OPERATOR_NODE(OP_ADD);
        else    
            res_op = OPERATOR_NODE(OP_SUB);
        
        if (res_op.error != TREE_SUCCESS)    
        {
            node_dtor(val);
            node_dtor(val2);
            *error = CREATING_NODE_ERROR;
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

// MulDiv      ::= Pow ( ('*' | '/') Pow )*
Node_t * GetMulDiv_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Node_t * val = GetPow_tokens(tokens, pos, tree, error);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_MULTIPLY || t -> type == TOK_DIVIDE)
    {
        token_t op = t -> type;
        (*pos)++;
        
        Node_t * val2 = GetPow_tokens(tokens, pos, tree, error);
        if (!val2)
        {
            node_dtor(val);
            *error = PARSER_ERROR;
            return NULL;
        }

        Node_result_t res_op = {};
        if (op == TOK_MULTIPLY)
            res_op = OPERATOR_NODE(OP_MUL);
        else    
            res_op = OPERATOR_NODE(OP_DIV);
        
        if (res_op.error != TREE_SUCCESS)    
        {
            node_dtor(val);
            node_dtor(val2);
            *error = CREATING_NODE_ERROR;
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


//Pow         ::= Unary ('^' Unary)*
Node_t * GetPow_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    * error = FRONTEND_SUCCESS;

    Node_t * val = GetUnary_tokens(tokens, pos, tree, error);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_POW)
    {
        (*pos)++;
        
        Node_t * val2 = GetUnary_tokens(tokens, pos, tree, error);
        if (!val2)
        {
            node_dtor(val);
            *error = PARSER_ERROR;
            return NULL;
        }

        Node_result_t res_op = {};
        res_op = OPERATOR_NODE(OP_POW);
        if (res_op.error != TREE_SUCCESS)    
        {
            node_dtor(val);
            node_dtor(val2);
            *error = CREATING_NODE_ERROR;
            if (res_op.node)
                node_dtor(res_op.node);
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


// Unary       ::=  ( '+' | '-' | FuncOper )? Primary
Node_t * GetUnary_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Token * t = current_token(tokens, *pos);
    if (!t)     
    {
        *error = PARSER_ERROR;
        return NULL;
    }

    if (t->type == TOK_UNARY_MINUS || t->type == TOK_PLUS)
    {
        token_t sign = t->type;
        (*pos)++;

        Node_t * operand = GetPrimaryExpression_tokens(tokens, pos, tree, error);
        if (!operand)
        {
            *error = NO_EXPECTED_EXPRESSION;
            return NULL;
        }

        if (sign == TOK_PLUS)
            return operand;

        Node_result_t res = OPERATOR_NODE(OP_UNARY_MINUS);
        if (res.error != TREE_SUCCESS)
        {
            *error = CREATING_NODE_ERROR;
            if (res.node)
                node_dtor(res.node);
            return NULL;
        }
        
        res.node->right = operand;
        operand->prev = res.node;

        return res.node;
    }
    else if (t->type == TOK_SIN || t->type == TOK_COS || t->type == TOK_TG ||
             t->type == TOK_LN  || t->type == TOK_SQRT || t->type == TOK_EXP)
    {
        operator_t op;
        switch (t->type)
        {
            case TOK_SIN:  op = OP_SIN;  break;
            case TOK_COS:  op = OP_COS;  break;
            case TOK_TG:   op = OP_TAN;  break;
            case TOK_LN:   op = OP_LN;   break;
            case TOK_SQRT: op = OP_SQRT; break;
            case TOK_EXP:  op = OP_EXP;  break;
            default: return NULL;
        }
        (*pos)++;

        REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
        (*pos)++;
        Node_t * prim = GetExpression_tokens(tokens, pos, tree, error);
        if (!prim)
        {
            *error = NO_EXPECTED_EXPRESSION;
            return NULL;
        }
        REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
        (*pos)++;

        Node_result_t res = OPERATOR_NODE(op);
        if (res.error != TREE_SUCCESS) return NULL;

        res.node->right = prim;
        prim->prev = res.node;
        return res.node;
    }
    
    return GetPrimaryExpression_tokens(tokens, pos, tree, error);
}


// Primary ::= '(' Expression ')' | Number | Identifier | StringLiteral | FuncCall
Node_t * GetPrimaryExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree, frontend_err * error)
{
    assert(tokens && pos && tree);
    *error = FRONTEND_SUCCESS;

    Token * t = current_token(tokens, *pos);
    if (!t)     return NULL;

    if (t -> type == TOK_LPAREN)
    {
        (*pos)++;
    
        Node_t * val = GetExpression_tokens(tokens, pos, tree, error);
        if (val == NULL) 
        {
            *error = NO_EXPECTED_EXPRESSION;
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
        Node_result_t num_res = NUMBER_NODE(value);
        if (num_res.error != TREE_SUCCESS)   return NULL;
        return num_res.node;
    }
    else if (t -> type == TOK_IDENTIFIER)
    {
        Token * next = current_token(tokens, (*pos) + 1);
        if (next && next->type == TOK_LBRACE)
            return GetFuncCall_tokens(tokens, pos, tree, error);

        (*pos)++;
        Node_result_t id_res = ID_NODE(t->string_value);
        if (id_res.error != TREE_SUCCESS)    
        {
            *error = CREATING_NODE_ERROR;
            return NULL;
        }

        return id_res.node;
    }
    else if (t -> type == TOK_STRING)
    {
        char * str = strdup(t->string_value);
        (*pos)++;
        Node_result_t str_res = STRING_NODE(str);
        free(str);
        if (str_res.error != TREE_SUCCESS)   
        {
            *error = CREATING_NODE_ERROR;
            return NULL;
        }
        return str_res.node;
    }
    else if (t -> type == TOK_READ)
    {
        return GetInputStmt(tokens, pos, tree, error);
    }
    else
    {
        *error = UNEXPECTED_SYNTAX;
        return NULL;
    }
}