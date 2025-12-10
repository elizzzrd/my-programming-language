#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "lexer.h"


SymbolTable symbols_table = {NULL, 0, 0};


int symbol_table_find(const char * name)
{
    for (size_t i = 0; i < symbols_table.count; i++)
    {
        if (strcmp(symbols_table.names[i], name) == 0)
            return (int)i;
    }
    return -1;
}

const char * get_id_name(int id_index)
{
    if (id_index >= 0 && id_index < symbols_table.count)
        return symbols_table.names[id_index];
    else    
        return NULL;
}


int symbol_table_add(const char * name)
{
    if (symbols_table.count == symbols_table.capasity)
    {
        if (!symbol_table_resize(&symbols_table))
            return -1;
    }

    symbols_table.names[symbols_table.count] = strdup(name);
    if (!symbols_table.names[symbols_table.count])
        return -1;

    return (int)symbols_table.count++;
}

int symbol_table_resize(SymbolTable *sb)
{
    size_t new_capacity = (sb->capasity == 0) ? 8 : sb -> capasity * 2;
    char ** new_names = (char **)realloc(sb->names, new_capacity * sizeof(char *));
    if (!new_names)     return 0;

    sb -> names = new_names;
    sb -> capasity = new_capacity;
    return 1;
}

int symbol_table_get_or_add(const char * name)
{
    int index = symbol_table_find(name);
    if (index >= 0)
        return index;
    else
        return symbol_table_add(name);
}

void symbol_table_destroy(SymbolTable * sb)
{
    assert(sb);

    for (int i = 0; i < sb->count; i++)
        free(sb->names[i]);
    free(sb->names);
}