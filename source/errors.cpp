#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "tree_structure.h"
#include "errors.h"

#define HTML_FILE "page.html"


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
        "Graph_dump failed"                             // [11] GRAPH_DUMP_ERROR
    };
    
    if (error < SUCCESS || error > GRAPH_DUMP_ERROR) 
        return "Unknown error";
    
    return tree_error_strings[error];
}


void tree_graph_dump_nodes(FILE * dot_fp, const Node_t * node)
{
    assert(dot_fp);

    if (!node) return;

    type_t type = node->type;
    const char * fillcolor = NULL;
    const char * value_str = NULL;
    const char * type_str = get_string_type(type);

    define_node_type_for_dump(type, &fillcolor, &value_str, (const Node_t *)node);
        

    if (node->left == NULL && node->right == NULL)
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | { <l> %s | <r> %s}}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str, "0", "0");
    else if (type == ROOT)
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str);
    else
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str);
    

    if (node -> left)
        tree_graph_dump_nodes(dot_fp, node -> left);
    if (node -> right)
        tree_graph_dump_nodes(dot_fp, node -> right);    
}


void tree_graph_dump_edges(FILE * dot_fp, const Node_t * node)
{
    assert(dot_fp);

    if (!node) return;

    if (node->left)
        fprintf(dot_fp, 
        "    node_%p -> node_%p [color=\"#0e0246ff\", penwidth=1];\n",
    (const void*)node, (const void*)node->left);

    if (node->right)
        fprintf(dot_fp, 
        "    node_%p -> node_%p [color=\"#0e0246ff\", penwidth=1];\n",
    (const void*)node, (const void*)node->right);

    tree_graph_dump_edges(dot_fp, node->left);
    tree_graph_dump_edges(dot_fp, node->right);
}


ErrorCode tree_graph_dump(Tree_t * tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called)
{
    assert(filename_dot && filename_png && tree); ;

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
        "    rankdir=TB"
        "    bgcolor=\"#ffffff\";\n"
        "    fontname=\"Consolas\";\n"
        "    nodesep=0.6;\n"
        "    node [shape=record, style=filled, fontname=\"Consolas\", margin=0.1, width=1.3, height=0.8];\n"
        "    edge [fontname=\"Consolas\", arrowsize=0.8];\n\n");
    fprintf(dot_fp, "\n");

    if (tree -> root)
    {
        tree_graph_dump_nodes(dot_fp, tree->root);
        tree_graph_dump_edges(dot_fp, tree->root);
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

    return error;
}

void define_node_type_for_dump(type_t type, const char ** fillcolor, const char ** value_str, const Node_t * node)
{
    assert(fillcolor && value_str && node);

    switch (type)
    {
        case OPERATOR:
        {
            *fillcolor = "#d460aeff";
            *value_str = get_string_operator(node->value.op);
            break;
        }
        case VARIABLE:
        {
            *fillcolor = "#f7d598ff";
            *value_str = get_var_name(node->value.var_index);
            break;
        }
        case NUMBER:
        {
            *fillcolor = "#98f7aaff";
            char buffer[64] = {};
            sprintf(buffer, "%lf", node->value.number);
            *value_str = buffer;
            break;
        }
        case ROOT:
        {
            *fillcolor = "#afdeedff";
            *value_str = node->value.root;
            break;
        }
        default:
        {
            *fillcolor = "#fefcfcff";
            break;
        }
    }
}



ErrorCode graph_dump_node(const Node_t * node, const char * filename_dot, const char * filename_png, const char * file_called, int line_called)
{
    assert(node && filename_dot && filename_png);

    ErrorCode error = SUCCESS;
    FILE * dot_fp = fopen(filename_dot, "w");
    if (!dot_fp)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }

    fprintf(dot_fp, "// Graphiz was called from %s:%d\n", file_called, line_called);
    fprintf(dot_fp,
        "digraph NodeGraph {\n"
        "    rankdir=TB"
        "    bgcolor=\"#ffffff\";\n"
        "    fontname=\"Consolas\";\n"
        "    nodesep=0.6;\n"
        "    node [shape=record, style=filled, fontname=\"Consolas\", margin=0.1, width=1.3, height=0.8];\n"
        "    edge [fontname=\"Consolas\", arrowsize=0.8];\n\n");
    fprintf(dot_fp, "\n");

    type_t type = node->type;
    const char * fillcolor = NULL;
    const char * value_str = NULL;
    const char * type_str = get_string_type(type);

    define_node_type_for_dump(type, &fillcolor, &value_str, node);

    if (type == ROOT)
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str);
    else if (node->left == NULL && node->right == NULL)
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | { <l> %s | <r> %s}}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str, "0", "0");
    else
        fprintf(dot_fp, 
        "    node_%p [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}\"];\n", 
        (const void*)node, fillcolor, type_str, value_str);
    
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

    return error;
}


void make_html()
{
    FILE *html_fp = fopen(HTML_FILE, "w");
    if (!html_fp)
    {
        fprintf(stderr, "Cannot open file %s\n", HTML_FILE);
        return;
    }

    fprintf(html_fp,
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "\t<meta charset=\"UTF-8\">\n"
        "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "\t<title>Struct of data: trees</title>\n"
        "\t<style>\n"
        "\tbody {\n"
        "\t\tfont-family: Consolas, monospace;\n"
        "\t\tbackground-color: #f4f4f8;\n"
        "\t\tmargin: 0;\n"
        "\t\tpadding: 20px;\n"
        "\t\tcolor: #333;\n"
        "\t}\n"
        "\th1 {\n"
        "\t\ttext-align: center;\n"
        "\t\tcolor: #222;\n"
        "\t\tmargin-bottom: 30px;\n"
        "\t}\n"
        "\t.container {\n"
		"\tdisplay: flex;\n"
		"flex-direction: column;\n" 
		"align-items: center;\n"    
		"gap: 20px;\n"  
        "\t}\n"
        "\t.card {\n"
        "\t\tbackground: #fff;\n"
        "\t\tborder-radius: 10px;\n"
        "\t\tbox-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);\n"
        "\t\tpadding: 15px;\n"
        "\t\twidth: 100%%;\n"
        "\t\tmax-width: 600px;\n"
        "\t\ttext-align: center;\n"
        "\t\ttransition: transform 0.2s ease-in-out;\n"
        "\t}\n"
        "\t.card:hover { transform: scale(1.02); }\n"
        "\timg {\n"
        "\t\twidth: 100%%;\n"
		"\t\theight: auto;\n"
		"\t\tmax-height: 600px;\n"
        "\t\tobject-fit: contain;\n"
        "\t\tbackground-color: #fafafa;\n"
        "\t\tborder: 1px solid #ddd;\n"
        "\t\tborder-radius: 6px;\n"
        "\t}\n"
        "\tp {\n"
        "\t\tfont-size: 14px;\n"
        "\t\tcolor: #555;\n"
        "\t\tmargin: 10px 0;\n"
        "\t}\n"
        "\t</style>\n"
        "</head>\n"
        "<body>\n"
        "\t<h1>Debug page</h1>\n"
        "\t<div class=\"container\">\n");

    extern int graph_dump_count;        
    for (int i = 1; i <= graph_dump_count; i++)
    {
        fprintf(html_fp,
            "\t\t<div class=\"card\">\n"
            "\t\t\t<p>Dump call: %d</p>\n"
            "\t\t\t<img src=\"logger/graph%d.png\" alt=\"graph%d\">\n"
            "\t\t</div>\n",
            i, i, i);
    }

    fprintf(html_fp,
        "\t</div>\n"
        "</body>\n"
        "</html>\n");

    fclose(html_fp);
}
