#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "tree_structure.h"
#include "errors.h"
#include "lexer.h"
#include "utils.h"
#include "math_functions.h"

#define HTML_FILE "logger/page.html"
#define DISABLE_DEBUG_PRINT

const char * tree_error_string(ErrorCode error) 
{
    static const char * tree_error_strings[] = 
    {
        "Success",                                      // [0] SUCCESS

        "Null pointer provided",                        // [1] TREE_NULL_POINTER
        "Memory allocation failed",                     // [2] TREE_MEMORY_ALLOCATION_ERROR
        "Deletion failed",                              // [3] TREE_DELETION_ERROR
        "Tree is empty",                                // [4] TREE_EMPTY_TREE
        "Invalid node_type",                            // [5] TREE_INVALID_NODE_TYPE
        "Invalid operator type",                        // [6] TREE_INVALID_OPERATOR
        "Error during creating a node",                 // [7] TREE_CREATING_NODE_ERROR,

        "Can not open the file",                        // [8] OPENING_FILE_ERROR
        "Error during loading expression",              // [9] LOADING_EXPRESSION_ERROR
        "Error during saving expression to latex",      // [10] SAVING_LATEX_ERROR
        "Graph_dump failed",                            // [11] GRAPH_DUMP_ERROR
        "Error during differentiation",                 // [12] DIFFERENTIATION_ERROR   
         
        "Error during lexical analysis",                // [13] LEXER_ERROR
        "Error during syntax analysis"                  // [14] PARSER_ERROR
        "Error during traslating to asm"                // [15] TRANSLATING_TO_ASM_ERROR
        "Syntax error",                                 // [16] SYNTAX_ERROR
        "Semantic error"                                // [17] SEMANTIC_ERROR
    };
    
    if (error < SUCCESS || error > SEMANTIC_ERROR) 
        return "Unknown error";
    
    return tree_error_strings[error];
}


void tree_graph_dump_nodes(FILE * dot_fp, const Node_t * node, const char * suffix)
{
    assert(dot_fp);

    if (!node) return;

    type_t type = node->type;
    const char * fillcolor = NULL;
    const char * value_str = NULL;
    const char * type_str = get_string_type(type);

    if (!type_str) 
    {
        DEBUG_PRINT("[WARNING]: Unknown node type: %d\n", type);
        type_str = "UNKNOWN";
    }

    define_node_type_for_dump(type, &fillcolor, &value_str, (const Node_t *)node);
    if (!fillcolor) fillcolor = "#ffffff";
    if (!value_str) value_str = "NULL";


    if (node->left == NULL && node->right == NULL && type == NUMBER)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | { <l> %s | <r> %s}}\"];\n", 
        (const void*)node, suffix, fillcolor, type_str, value_str, "0", "0");
    else if (type == ROOT)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s }\"];\n", 
        (const void*)node, suffix, fillcolor, get_statement_name(OP_PROGRAM));
    else if (type == STATEMENT && node->id.name)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | <i> %s}\"];\n", 
        (const void*)node, suffix, fillcolor, type_str, value_str, node->id.name);
    else if (type == IDENTIFIER)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <i> %s | <n> %d}\"];\n", 
        (const void*)node, suffix, fillcolor, type_str,  node->id.name, node->id.id_index);
    else if (type == OPERATOR && (node->value.op >= OP_BELOW || node->value.op >= OP_ABOVE_EQUAL))
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> \\%s}\"];\n", 
        (const void*)node, suffix, fillcolor, type_str, value_str);
    else
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}\"];\n", 
        (const void*)node, suffix, fillcolor, type_str, value_str);
    

    if (node -> left)
        tree_graph_dump_nodes(dot_fp, node -> left, suffix);
    if (node -> right)
        tree_graph_dump_nodes(dot_fp, node -> right, suffix);    
}


void tree_graph_dump_edges(FILE * dot_fp, const Node_t * node, const char * suffix)
{
    assert(dot_fp);

    if (!node) return;

    if (node->left)
        fprintf(dot_fp, 
        "    node_%p%s -> node_%p%s [color=\"#da1616\", penwidth=1];\n",
    (const void*)node, suffix, (const void*)node->left, suffix);

    if (node->right)
        fprintf(dot_fp, 
        "    node_%p%s -> node_%p%s [color=\"#6110d3\", penwidth=1];\n",
    (const void*)node, suffix, (const void*)node->right, suffix);

    tree_graph_dump_edges(dot_fp, node->left, suffix);
    tree_graph_dump_edges(dot_fp, node->right, suffix);
}



ErrorCode tree_graph_dump(Tree_t * tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called)
{
    assert(filename_dot && filename_png && tree); ;

    DEBUG_PRINT("[INFO] Starting graph dump to %s and %s\n", filename_dot, filename_png);

    ErrorCode error = SUCCESS;
    if (!tree)
    {
        ERROR_MESSAGE(TREE_NULL_POINTER, error);
        return error;
    }

    FILE * dot_fp = fopen(filename_dot, "w");
    if (!dot_fp)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }
    fprintf(dot_fp, "// Graphiz was called from %s:%d\n", file_called, line_called);
    fprintf(dot_fp,
        "digraph TreeGraph {\n"
        "    rankdir=TB;"
        "    bgcolor=\"#ffffff\";\n"
        "    fontname=\"Consolas\";\n"
        "    nodesep=0.6;\n"
        "    node [shape=record, style=filled, fontname=\"Consolas\", margin=0.1, width=1.3, height=0.8];\n"
        "    edge [fontname=\"Consolas\", arrowsize=0.8];\n\n");
    fprintf(dot_fp, "\n");

    if (tree -> root)
    {
        tree_graph_dump_nodes(dot_fp, tree->root, "");
        tree_graph_dump_edges(dot_fp, tree->root, "");
    }

    fprintf(dot_fp, "}\n");
    fclose(dot_fp);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "dot -Tpng \"%s\" -o \"%s\"", filename_dot, filename_png);
    int ret = system(cmd);
    if (ret != 0) 
    {
        ERROR_MESSAGE(GRAPH_DUMP_ERROR, error);
        return error;
    }

    DEBUG_PRINT("Graph dump completed: %s and %s have been created\n", filename_dot, filename_png);

    return error;
}


ErrorCode tree_graph_middleend(Tree_t * original_tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called)
{
    assert(filename_dot && filename_png && original_tree); ;

    DEBUG_PRINT("[INFO] Starting graph dump in middleend to %s and %s\n", filename_dot, filename_png);
    ErrorCode error = SUCCESS;
    
    FILE * dot_fp = fopen(filename_dot, "w");
    if (!dot_fp)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }

    fprintf(dot_fp, "// Graphiz was called from %s:%d\n", file_called, line_called);
    fprintf(dot_fp,
        "digraph MiddleendTreeGraph {\n"
        "    bgcolor=\"#ffffff\";\n"
        "    fontname=\"Consolas\";\n"
        "    nodesep=0.6;\n"
        "    node [shape=record, style=filled, fontname=\"Consolas\", margin=0.1, width=1.3, height=0.8];\n"
        "    edge [fontname=\"Consolas\", arrowsize=0.8];\n\n");
    fprintf(dot_fp, "\n");

    fprintf(dot_fp, "subgraph cluster_original_tree {\n");
    fprintf(dot_fp, "label = \"Original Tree\";\n");
    if (original_tree -> root)
    {
        tree_graph_dump_nodes(dot_fp, original_tree->root, "");
        tree_graph_dump_edges(dot_fp, original_tree->root, "");
    }
    fprintf(dot_fp, "}\n");

    
    fprintf(dot_fp, "subgraph cluster_optimized_tree {\n");
    fprintf(dot_fp, "label = \"Optimized Tree\";\n");
    error = optimize_tree_recursive(original_tree);
    if (error != SUCCESS)
    {
        ERROR_MESSAGE(GRAPH_DUMP_ERROR, error);
        fprintf(dot_fp, "node_* [label = \"No optimization\"];\n");
    }
    if (original_tree -> root)
    {
        tree_graph_dump_nodes(dot_fp, original_tree->root, "_opt");
        tree_graph_dump_edges(dot_fp, original_tree->root, "_opt");
    }
    fprintf(dot_fp, "}\n");
    fprintf(dot_fp, "}\n");


    fclose(dot_fp);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "dot -Tpng \"%s\" -o \"%s\"", filename_dot, filename_png);
    int ret = system(cmd);
    if (ret != 0) 
    {
        ERROR_MESSAGE(GRAPH_DUMP_ERROR, error);
        return error;
    }

    DEBUG_PRINT("[INFO] Graph dump of middleend tree completed: %s and %s have been created\n", filename_dot, filename_png);

    return error;
}



void define_node_type_for_dump(type_t type, const char ** fillcolor, const char ** value_str, const Node_t * node)
{
    assert(fillcolor && value_str && node);
    * fillcolor = "#ffffff";
    * value_str = "ERROR";

    switch (type)
    {
        case OPERATOR:
        {
            *fillcolor = "#d460aeff";
            *value_str = get_string_operator(node->value.op);
            break;
        }
        case IDENTIFIER:
        {
            *fillcolor = "#f7d598ff";
            *value_str = node->id.name;
            break;
        }
        case NUMBER:
        {
            *fillcolor = "#98f7aaff";

            static char buf[64];  

            snprintf(buf, sizeof(buf), "%g", node->value.number);
            *value_str = buf;
            break;
        }
        case ROOT:
        {
            *fillcolor = "#afdeedff";
            *value_str = node->value.root ? node->value.root : "ROOT_NULL";
            break;
        }
        case STATEMENT:
        {
            *fillcolor = "#d29bdaff";
            if ((node->value.stmt == OP_FUNC_DEF || node->value.stmt == OP_CALL) && node->id.name)
            {
                *value_str = get_statement_name(node->value.stmt); 
                break;
            }

            if (node->id.name)
                *value_str = node->id.name;
            else
                *value_str = get_statement_name(node->value.stmt);
            break;
        }
        case STRING:
        {
            *fillcolor = "#f7d598ff";
            *value_str = node->value.string_value ? node->value.string_value : "STRING_NULL";
            break;
        }
        default:
        {
            *fillcolor = "#fefcfcff";
            DEBUG_PRINT("WARNING: Unknown node type in define_node_type_for_dump: %d\n", type);
            *fillcolor = "#fefcfcff";
            *value_str = "UNKNOWN_TYPE";
            break;
        }
    }
}


