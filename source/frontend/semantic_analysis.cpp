#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "semantic_analysis.h"
#include "tree_structure.h"



int symbol_table_find(const char * name, SymbolTable * st, st_mode_t mode)
{
    for (size_t i = 0; i < st[mode].count; i++)
    {
        if (strcmp(st[mode].names[i], name) == 0)
            return (int)i;
    }
    return -1;
}


const char * get_id_name(size_t id_index, SymbolTable * st, st_mode_t mode)
{
    if (id_index >= 0 && id_index < st[mode].count)
        return st[mode].names[id_index];
    else    
        return NULL;
}


int symbol_table_add(const char * name, SymbolTable * st, st_mode_t mode)
{
    if (st[mode].count == st[mode].capacity)
    {
        if (!symbol_table_resize(&st[mode]))
            return -1;
    }

    st[mode].names[st[mode].count] = strdup(name);
    if (!st[mode].names[st[mode].count])
        return -1;

    return (int)st[mode].count++;
}


int symbol_table_resize(SymbolTable *st)
{
    size_t new_capacity = (st->capacity == 0) ? 8 : st->capacity * 2;

    char **new_names = (char **)realloc(st->names,
                                        new_capacity * sizeof(char *));
    if (!new_names)
        return 0;

    st->names = new_names;
    st->capacity = new_capacity;
    return 1;
}

void symbol_table_init(SymbolTable *st)
{
    st->names = NULL;
    st->count = 0;
    st->capacity = 0;
}


SymbolTable * st_init(void)
{
    SymbolTable * st = (SymbolTable *) calloc(2, sizeof(SymbolTable));
    if (!st)
        return NULL;

    symbol_table_init(&st[ST_FUNC]);
    symbol_table_init(&st[ST_VAR]);

    return st;
}


void symbol_table_destroy(SymbolTable * st)
{
    assert(st);

    for (size_t i = 0; i < st->count; i++)
    {
        if (st->names[i])
            free(st->names[i]);
    }
    if (st->names)
        free(st->names);
}


ErrorCode semantic_analysis(Tree_t * tree)
{
    assert(tree);
    assert(tree->root);
    DEBUG_PRINT("[INFO] SEMANTIC_ANALYSIS START");

    ErrorCode error = SUCCESS;

    SymbolTable * st = st_init();
    if (!st)
    {
        ERROR_MESSAGE(SEMANTIC_ERROR, error);
        return error;
    }


    // first pass
    error = collect_definitions(tree->root->right, st);
    if (error != SUCCESS)
        return error;

    // second pass
    error = check_semantics(tree->root->right, st);

    symbol_table_destroy(&st[ST_FUNC]);
    symbol_table_destroy(&st[ST_VAR]);
    free(st);

    return error;
}


ErrorCode collect_definitions(Node_t * node, SymbolTable * st)
{
    assert(st);

    if (!node)
        return SUCCESS;
    
    ErrorCode error = SUCCESS;

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
    {
        const char * name = node->id.name;

        if (symbol_table_find(name, st, ST_FUNC) >= 0)
        {
            DEBUG_PRINT("[ERROR] Multuple definition of function %s", name);
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }
        else
        {
            int index = symbol_table_add(name, st, ST_FUNC) + 50;
            node->id.id_index = index;
        }
    }
    else if (node->type == STATEMENT && node->value.stmt == OP_VAR_DEF)
    {
        const char * name = node->left->id.name;

        if (symbol_table_find(name, st, ST_VAR) >= 0)
        {
            DEBUG_PRINT("[ERROR] Multuple definition of identifier %s", name);
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }
        else
        {
            int index = symbol_table_add(name, st, ST_VAR);
            node->left->id.id_index = index;
        }
    }
    else if (node->value.stmt == OP_READ)
    {
        ;
    }

    error = collect_definitions(node->left, st);
    if (error != SUCCESS)
        return error;
    error = collect_definitions(node->right, st);
    
    return error;
}


ErrorCode check_semantics(Node_t * node, SymbolTable * st)
{
    if (!node)
        return SUCCESS;

    ErrorCode error = SUCCESS;

    if (node->type == STATEMENT && node->value.stmt == OP_FUNC_DEF)
    {
        size_t old_var_count = st[ST_VAR].count;

        // local variables
        Node_t * params = node->left;
        if (params)
        {
            Node_t * p = params->right;
            while (p)
            {
                const char * name = p->id.name;

                if (symbol_table_find(name, st, ST_VAR) >= 0)
                {
                    DEBUG_PRINT("[ERROR] parameter shadows variable %s", name);
                    ERROR_MESSAGE(SEMANTIC_ERROR, error);
                    return error;
                }

                int index = symbol_table_add(name, st, ST_VAR);
                p->id.id_index = index;

                p = p->right;
            }
        }
        // function body
        error = check_semantics(node->right, st);
        if (error != SUCCESS)
            return error;

        // kill local variables
        for (size_t i = old_var_count; i < st[ST_VAR].count; ++i)
        {
            free(st[ST_VAR].names[i]);
            st[ST_VAR].names[i] = NULL;
        }
        st[ST_VAR].count = old_var_count;

        return SUCCESS;
    }


    if (node->type == STATEMENT && node->value.stmt == OP_CALL)
    {
        const char * name = node->id.name;
        int index = symbol_table_find(name, st, ST_FUNC);
        if (index < 0)
        {
            DEBUG_PRINT("[ERROR] Call of undefined function %s\n", name);
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }
        node->id.id_index = index;

        if (node->left)
        {
            error = check_semantics(node->left, st);
            if (error != SUCCESS)
                return error;
        }
    }


    if (node->type == STATEMENT && node->value.stmt == OP_RETURN)
    {
        if (node->left)
        {
            error = check_semantics(node->left, st);
            if (error != SUCCESS)
                return error;
        }
    }


    if (node->type == OPERATOR && node->value.op == OP_READ)
    {
        if (!node->left || node->left->type != IDENTIFIER)
        {
            DEBUG_PRINT("[ERROR] read statement must have a variable");
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }

        const char * name = node->left->id.name;
        int index = symbol_table_find(name, st, ST_VAR);
        if (index < 0)
        {
            DEBUG_PRINT("[ERROR] read variable '%s' not declared", name);
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }
        node->left->id.id_index = index;
    }

    
    if (node->type == IDENTIFIER)
    {
        const char * name = node->id.name;
        int index = symbol_table_find(name, st, ST_VAR);
        if (index < 0)
        {
            DEBUG_PRINT("[ERROR] undefined variable %s", name);
            ERROR_MESSAGE(SEMANTIC_ERROR, error);
            return error;
        }
        node->id.id_index = index;
    }


    error = check_semantics(node->left, st);
    if (error != SUCCESS)
        return error;

    error = check_semantics(node->right, st);
    return error;
}
