#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
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



IfStmt      ::= 'if' '{' Expression '}' Block
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

#define IF_THERE_IS_ERROR_WITH_FUNC_OPERATOR(res) \
    do { \
        if (res.error != error) {\
            ERROR_MESSAGE(TREE_INVALID_OPERATOR, error);\
            return NULL;}\
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
    DEBUG_PRINT("\n[INFO] SYNTAX_ANALYSIS START");

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
            destroy_tree(tree, "destroy_frontend_error");
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
            DEBUG_PRINT("[SYNTAX ERROR] expected ';'\n");
            DEBUG_PRINT("got token %s at %lu", get_string_token_type(t->type), *pos);
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
        else if (next && next->type == TOK_LPAREN)
            return GetFuncCall_tokens(tokens, pos, tree);

        DEBUG_PRINT("Unexpexted identifier at statement\n");
    }
    else if (t -> type == TOK_DEF)
        return GetFuncDef(tokens, pos, tree);
    else if (t -> type == TOK_ID_DEF)
        return GetVarDef_tokens(tokens, pos, tree);
    else if (t -> type == TOK_OUT)
        return GetReturn_tokens(tokens, pos, tree);
    else if (t -> type == TOK_READ)
        return GetInputStmt(tokens, pos, tree);
    return GetExpression_tokens(tokens, pos, tree);
}


// FuncDef     ::= 'def' Identifier '{' ParamList?'}' Block
Node_t * GetFuncDef(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

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
        params = GetParams_tokens(tokens, pos, tree);
        if (!params)
        {
            ERROR_MESSAGE(PARSER_ERROR, error);
            return NULL;
        }
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * body = GetBlock_tokens(tokens, pos, tree);
    if (!body)
    {
        ERROR_MESSAGE(PARSER_ERROR, error);
        return NULL;
    }

    Node_result_t res = create_statement_node(tree, OP_FUNC_DEF);
    if (res.error != SUCCESS)
    {
        ERROR_MESSAGE(PARSER_ERROR, error);
        return NULL;
    }
    res.node->id.name = name->string_value;
    res.node->left = params;
    res.node->right = body;

    if (params)     params->prev = res.node;
    
    return res.node;
}


//ParamList   ::= Var ( ',' Var)*
Node_t * GetParams_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

    Token * t = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, t);

    Node_result_t first_var = create_identifier_node(tree, t->string_value);
    if (first_var.error != SUCCESS)
    {
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        return NULL;
    }
    (*pos)++;

    Node_t * list = create_statement_node(tree, OP_PARAMS).node;
    if (!list)
    {
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        return NULL;
    }
    list -> right = first_var.node;

    Node_t * prev = first_var.node;
    while (current_token(tokens, (*pos))->type == TOK_COMMA)
    {
        (*pos)++;

        t = current_token(tokens, (*pos));
        REQUIRE_TOKEN(TOK_IDENTIFIER, t);

        Node_result_t var = create_identifier_node(tree, t->string_value);
        if (var.error != SUCCESS)
        {
            ERROR_MESSAGE(SYNTAX_ERROR, error);
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
Node_t * GetArgs_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

    Node_t * first_expr = GetExpression_tokens(tokens, pos, tree);
    if (!first_expr)
    {
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        return NULL;
    }

    Node_t * list = create_statement_node(tree, OP_ARGS).node;
    if (!list)
    {
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        return NULL;
    }
    list->right = first_expr;

    Node_t * prev = first_expr;
    while (current_token(tokens, (*pos))->type == TOK_COMMA)
    {
        (*pos)++;

        Node_t * next_expr = GetExpression_tokens(tokens, pos, tree);

        prev->right = next_expr;
        next_expr->prev = prev;
        list->param_count++;
        prev = next_expr;
    }
    return list;
}


// Return ::= 'out' Expression
Node_t * GetReturn_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

    REQUIRE_TOKEN(TOK_OUT, current_token(tokens, (*pos)));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        ERROR_MESSAGE(PARSER_ERROR, error);
        return NULL;
    }

    Node_result_t res = create_statement_node(tree, OP_RETURN);
    if (res.error != SUCCESS)
    {
        ERROR_MESSAGE(PARSER_ERROR, error);
        return NULL;
    }

    res.node->right = expr;
    expr->prev = res.node;

    return res.node;
}


// FuncCall    ::= Identifier '{' ArgList? '}'
Node_t * GetFuncCall_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

    Token * name = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, name);
    (*pos)++;
    
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * args = NULL;
    if (current_token(tokens, (*pos))->type != TOK_RBRACE)
    {
        args = GetArgs_tokens(tokens, pos, tree);
        if (!args)
        {
            ERROR_MESSAGE(PARSER_ERROR, error);
            return NULL;
        }
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_result_t call = create_statement_node(tree, OP_CALL);
    if (call.error != SUCCESS)
    {
        ERROR_MESSAGE(PARSER_ERROR, error);
        return NULL;
    }
    call.node -> right = args;
    call.node -> id.name = name->string_value;

    if (args)     args->prev = call.node;
    
    return call.node;
}


//PrintStmt   ::= 'print' '{' Expression '}'
Node_t * GetPrintStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    REQUIRE_TOKEN(TOK_PRINT, current_token(tokens, *pos));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;

    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expects expression\n");
        return NULL;
    }
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
    (*pos)++;

    Node_result_t print_res = create_statement_node(tree, OP_PRINT);
    if (print_res.error != SUCCESS)     return NULL;

    print_res.node->right = expr;
    expr->prev = print_res.node;

    return print_res.node;
}



//InputStmt   ::= 'read' '{''}'
Node_t * GetInputStmt(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    REQUIRE_TOKEN(TOK_READ, current_token(tokens, *pos));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, *pos));
    (*pos)++;
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
    (*pos)++;
    
    Node_result_t res = create_operator_node(tree, OP_READ);
    if (res.error != SUCCESS)
        return NULL;

    return res.node;
}


// Assignment ::= Identifier '=' Expression
Node_t * GetAssignment_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;

    Token * t = current_token(tokens, *pos);
    REQUIRE_TOKEN(TOK_IDENTIFIER, t);
    (*pos)++;

    Node_result_t id_res = create_identifier_node(tree, t->string_value);
    if (id_res.error != SUCCESS)    
        return NULL;

    REQUIRE_TOKEN(TOK_ASSIGN, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expects expression after '='\n");
        ERROR_MESSAGE(SYNTAX_ERROR, error);
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

    assign_res.node -> left = id_res.node;
    assign_res.node -> right = expr;

    id_res.node->prev = assign_res.node;
    expr->prev = assign_res.node;

    return assign_res.node;
}

//VarDef      ::= 'ID_def' Var '=' Expression
Node_t * GetVarDef_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert (tokens && pos && tree);
    ErrorCode error = SUCCESS;
    REQUIRE_TOKEN(TOK_ID_DEF, current_token(tokens, *pos));
    (*pos)++;

    Token * t = current_token(tokens, *pos);
    if (!t || t -> type != TOK_IDENTIFIER)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected identifier\n");
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        return NULL;
    }

    Node_result_t id_res = create_identifier_node(tree, t->string_value);
    if (id_res.error != SUCCESS)    
        return NULL;
    (*pos)++;

    REQUIRE_TOKEN(TOK_ASSIGN, current_token(tokens, *pos));
    (*pos)++;
    
    Node_t * expr = GetExpression_tokens(tokens, pos, tree);
    if (!expr)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expects expression after '='\n");
        ERROR_MESSAGE(SYNTAX_ERROR, error);
        destroy_node(id_res.node);
        return NULL;
    }

    Node_result_t def_res = create_statement_node(tree, OP_VAR_DEF);
    if (def_res.error != SUCCESS)    
    {
        destroy_node(id_res.node);
        destroy_node(expr);
        return NULL;
    }
    DEBUG_PRINT("var_def node created");

    def_res.node -> left = id_res.node;
    def_res.node -> right = expr;

    id_res.node->prev = def_res.node;
    expr->prev = def_res.node;

    return def_res.node;
}



// IfStmt ::= "if" '{' Expression '}' Block
Node_t * GetIfStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    REQUIRE_TOKEN(TOK_IF, current_token(tokens, (*pos)));
    (*pos)++;

    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;
    Node_t * condition = GetExpression_tokens(tokens, pos, tree);
    if (!condition)
    {
        DEBUG_PRINT("[SYNTAX ERROR] parser expected condition\n");
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
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


// WhileStmt ::= "while" '{' Expression '}' Block
Node_t * GetWhileStmt_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    
    REQUIRE_TOKEN(TOK_WHILE, current_token(tokens, (*pos)));
    (*pos)++;
    REQUIRE_TOKEN(TOK_LBRACE, current_token(tokens, (*pos)));
    (*pos)++;

    Node_t * condition = GetExpression_tokens(tokens, pos, tree);
    if (!condition)
    {
        printf("[SYNTAX ERROR] parser expected condition\n");
        return NULL;
    }
    
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
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

//Block ::= '{''{' Statement* '}''}'
Node_t * GetBlock_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
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

        cur_stmt = GetStatement_tokens(tokens, pos, tree);
        if (cur_stmt == NULL)
        {
            DEBUG_PRINT("[ERROR] Error during parsing statement");
            destroy_tree(tree, "destroy_frontend_error");
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
            destroy_tree(tree, "destroy_frontend_error");
            return NULL;
        }

        t = current_token(tokens, *pos);
    }

    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
    (*pos)++;
    REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, (*pos)));
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
    return GetEquality_tokens(tokens, pos, tree);
}



// Equality    ::= Comp ( ('==' | '!=') Comp)
Node_t * GetEquality_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetComp_tokens(tokens, pos, tree);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    if (t -> type == TOK_EQUAL_EQUAL || t -> type == TOK_NON_EQUAL)
    {
        (*pos)++;
        operator_t op = (t -> type == TOK_EQUAL_EQUAL) ? OP_EQUAL : OP_NON_EQUAL;
        
        Node_t * val2 = GetComp_tokens(tokens, pos, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }

        Node_result_t res_op = {};
        res_op = create_operator_node(tree, op);
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



// Comp        ::= AddSub ( ('<' | '>' | '<=' | '>=') AddSub)*
Node_t * GetComp_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetAddSub_tokens(tokens, pos, tree);
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
        
        Node_t * val2 = GetAddSub_tokens(tokens, pos, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }

        Node_result_t res_op = create_operator_node(tree, op);
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

// MulDiv      ::= Pow ( ('*' | '/') Pow )*
Node_t * GetMulDiv_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetPow_tokens(tokens, pos, tree);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_MULTIPLY || t -> type == TOK_DIVIDE)
    {
        token_t op = t -> type;
        (*pos)++;
        
        Node_t * val2 = GetPow_tokens(tokens, pos, tree);
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


//Pow         ::= Unary ('^' Unary)*
Node_t * GetPow_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Node_t * val = GetUnary_tokens(tokens, pos, tree);
    if (!val)   return NULL;

    Token * t = current_token(tokens, *pos);
    while (t -> type == TOK_POW)
    {
        (*pos)++;
        
        Node_t * val2 = GetUnary_tokens(tokens, pos, tree);
        if (!val2)
        {
            destroy_node(val);
            return NULL;
        }

        Node_result_t res_op = {};
        res_op = create_operator_node(tree, OP_POW);
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


// Unary       ::=  ( '+' | '-' | FuncOper )? Primary
Node_t * GetUnary_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);

    Token * t = current_token(tokens, *pos);
    if (!t)     return NULL;

    if (t->type == TOK_UNARY_MINUS || t->type == TOK_PLUS)
    {
        token_t sign = t->type;
        (*pos)++;

        Node_t * operand = GetPrimaryExpression_tokens(tokens, pos, tree);
        if (!operand)
        {
            DEBUG_PRINT("[SYNTAX ERROR] expected expression after unary '-'/'+'\n");
            return NULL;
        }

        if (sign == TOK_PLUS)
            return operand;

        Node_result_t res = create_operator_node(tree, OP_UNARY_MINUS);
        if (res.error != SUCCESS)
            return NULL;
        
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
        Node_t * prim = GetExpression_tokens(tokens, pos, tree);
        if (!prim)
        {
            DEBUG_PRINT("[SYNTAX ERROR] expected expression after function");
            return NULL;
        }
        REQUIRE_TOKEN(TOK_RBRACE, current_token(tokens, *pos));
        (*pos)++;

        Node_result_t res = create_operator_node(tree, op);
        if (res.error != SUCCESS) return NULL;

        res.node->right = prim;
        prim->prev = res.node;
        return res.node;
    }
    
    return GetPrimaryExpression_tokens(tokens, pos, tree);
}




// Primary ::= '(' Expression ')' | Number | Identifier | StringLiteral | FuncCall
Node_t * GetPrimaryExpression_tokens(TokenList * tokens, size_t * pos, Tree_t * tree)
{
    assert(tokens && pos && tree);
    ErrorCode error = SUCCESS;

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
        Token * next = current_token(tokens, (*pos) + 1);
        if (next && next->type == TOK_LPAREN)
            return GetFuncCall_tokens(tokens, pos, tree);

        (*pos)++;
        Node_result_t id_res = create_identifier_node(tree, t->string_value);
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
    else if (t -> type == TOK_READ)
    {
        return GetInputStmt(tokens, pos, tree);
    }
    else
    {
        printf("[SYNTAX ERROR] Unexpected token\n");
        return NULL;
    }
}

