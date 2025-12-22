#include <stdio.h>
#include <stdlib.h>

#include "tree_structure.h"
#include "errors.h"
#include "tree_operations.h"
#include "load_expression.h"
#include "utils.h"
#include "math_functions.h"
#include "latex_dump.h"
#include "build_tree.h"
#include "lexer.h"
#include "syntax_analysis.h"

#define AST_OUTPUT "ast_output.txt"
// void savenode(Node_t * node, FILE * file_ptr);
// ErrorCode save_tree(Tree_t * tree);

void generate_code(Node_t* node, FILE* file);
ErrorCode generate_source_code(Tree_t* tree, const char* filename) ;

int main(void)
{
    Tree_t tree = {};
    ErrorCode error = SUCCESS;
    error = init_tree(&tree);
    if (error != SUCCESS)
    {
        destroy_tree(&tree);
        return 1;
    }

    
    TokenList token_list = {};
    DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS START");
    error = lexicalAnalysis(&token_list);
    if (error != SUCCESS)
    {
        DEBUG_PRINT("error during lexical analysis");
        destroy_tokens(&token_list);
        return -1;
    }
    else
    {
        DEBUG_PRINT("[INFO] LEXICAL_ANALYSIS END");
        lexer_dump(&token_list);
    }


    DEBUG_PRINT("\n[INFO] SYNTAX_ANALYSIS START");
    size_t pos = 0;
    Node_t * tree_root = GetProgram_tokens(&token_list, &pos, &tree);
    if (!tree_root)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        free(tree_root);
        return -1;
    }
    tree.root->right = tree_root;
    error = build_parent_links(&tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(LOADING_EXPRESSION_ERROR, error);
        free(tree_root);
        return -1;
    }
    GRAPH_DUMP(&tree);
    DEBUG_PRINT("[INFO] SYNTAX_ANALYSIS END");
    destroy_tokens(&token_list);
    
    DEBUG_PRINT("expression has been loaded successfully\n");
    save_tree(&tree, AST_OUTPUT);
   
                   
    DEBUG_PRINT("Everything is good before deletion");
    if (tree.root)
        destroy_tree(&tree);
    //destroy_tree(&diff_tree);
    //make_html();
    //symbol_table_destroy(&symbols_table);
    DEBUG_PRINT("tree deleted succefully");
    printf("Programm is finished\n");
    return 0;
}



void generate_code(Node_t* node, FILE* file) 
{
    if (!node) return;
    
    switch(node->type) {
        case STATEMENT: {
            statement_t stmt = node->value.stmt;
            switch(stmt) {
                case OP_PRINT: {
                    fprintf(file, "print ");
                    generate_code(node->right, file);  // выражение для печати
                    fprintf(file, ";\n");
                    break;
                }
                case OP_ASSIGNMENT: {
                    generate_code(node->left, file);   // идентификатор
                    fprintf(file, " = ");
                    generate_code(node->right, file);  // выражение
                    fprintf(file, ";\n");
                    break;
                }
                case OP_IF: {
                    fprintf(file, "if (");
                    generate_code(node->left, file);   // условие
                    fprintf(file, ") ");
                    generate_code(node->right, file);  // блок
                    break;
                }
                case OP_WHILE: {
                    fprintf(file, "while (");
                    generate_code(node->left, file);   // условие
                    fprintf(file, ") ");
                    generate_code(node->right, file);  // блок
                    break;
                }
                case OP_BLOCK: {
                    fprintf(file, "{\n");
                    // Проходим по всем statement'ам в блоке
                    Node_t* current = node->right;
                    while (current) {
                        if (current->type == STATEMENT && current->value.stmt == OP_END) {
                            current = current->right;  // пропускаем разделитель
                        } else {
                            generate_code(current, file);
                            current = current->right;
                        }
                    }
                    fprintf(file, "}\n");
                    break;
                }
                default:{
                    // Просто выражение
                    generate_code(node->left, file);
                    fprintf(file, ";\n");
                    break;}
            }
            break;
        }
            
        case OPERATOR: {
            char op_char = '?';
            switch(node->value.op) {
                case OP_ADD: op_char = '+'; break;
                case OP_SUB: op_char = '-'; break;
                case OP_MUL: op_char = '*'; break;
                case OP_DIV: op_char = '/'; break;
                default: op_char = '?';
            }
            fprintf(file, "(");
            generate_code(node->left, file);
            fprintf(file, " %c ", op_char);
            generate_code(node->right, file);
            fprintf(file, ")");
            break;
        }
            
        case IDENTIFIER:
            fprintf(file, "%s", get_id_name(node->value.id_index));
            break;
            
        case NUMBER:
            fprintf(file, "%.2lf", node->value.number);
            break;
            
        case STRING:
            fprintf(file, "\"%s\"", node->value.string_value);
            break;
            
        default:
            fprintf(file, "/* unknown node type %d */", node->type);
    }
}

ErrorCode generate_source_code(Tree_t* tree, const char* filename) 
{
    FILE* file = fopen(filename, "w");
    if (!file) return OPENING_FILE_ERROR;
    
    if (tree->root && tree->root->right) {
        generate_code(tree->root->right, file);
    }
    
    fprintf(file, "$\n");  // конец программы
    fclose(file);
    return SUCCESS;
}

