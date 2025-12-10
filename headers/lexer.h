#pragma once


typedef struct 
{
    char **names;
    size_t count;
    size_t capasity;
} SymbolTable;

extern SymbolTable symbols_table;

int symbol_table_find(const char * name);
int symbol_table_add(const char * name);
int symbol_table_resize(SymbolTable *sb);
int symbol_table_get_or_add(const char * name);
void symbol_table_destroy(SymbolTable * sb);
const char * get_id_name(int id_index);