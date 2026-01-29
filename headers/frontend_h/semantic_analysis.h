#include <stdio.h>

typedef enum
{
    ST_VAR  = 0,
    ST_FUNC = 1
} st_mode_t;


typedef struct 
{
    char **names;
    size_t count;
    size_t capasity;
} SymbolTable;




int symbol_table_find(const char * name, SymbolTable * st, st_mode_t mode);
int symbol_table_add(const char * name, SymbolTable * st, st_mode_t mode);
int symbol_table_resize(SymbolTable *st, st_mode_t mode);
void symbol_table_destroy(SymbolTable * st);
const char * get_id_name(size_t id_index, SymbolTable * st, st_mode_t mode);
SymbolTable * st_init(void);

ErrorCode collect_definitions(Node_t * node, SymbolTable * st);
ErrorCode check_semantics(Node_t * node, SymbolTable * st);
ErrorCode semantic_analysis(Tree_t * tree);