#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "errors.h"
#include "tree_structure.h"
#include "tree_dump.h"
#include "optimize_tree.h"




void tree_graph_dump_nodes(FILE * dot_fp, const Node_t * node, const char * suffix)
{
    assert(dot_fp);
    if (!node) return;

    node_dump_t nd = {};
    nd.node = node;
    nd.type = node->type; 
    nd.fillcolor = NULL;
    nd.value_str = NULL;
    nd.type_str = get_string_type(nd.type); 


    define_node_type_for_dump(&nd);
    if (!nd.fillcolor)  nd.fillcolor = "#ffffff";
    if (!nd.value_str)  nd.value_str = "NULL";


    if (node->left == NULL && node->right == NULL && nd.type == NUMBER)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | { <l> %s | <r> %s}}\"];\n", 
        (const void*)node, suffix, nd.fillcolor, nd.type_str, nd.value_str, "0", "0");
    else if (nd.type == ROOT)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s }\"];\n", 
        (const void*)node, suffix, nd.fillcolor, get_statement_name(OP_PROGRAM));
    else if (nd.type == STATEMENT && node->id.name)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s | <i> %s}\"];\n", 
        (const void*)node, suffix, nd.fillcolor, nd.type_str, nd.value_str, node->id.name);
    else if (nd.type == IDENTIFIER)
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <i> %s | <n> %d}\"];\n", 
        (const void*)node, suffix, nd.fillcolor, nd.type_str,  node->id.name, node->id.id_index);
    else if (nd.type == OPERATOR && (node->value.op >= OP_BELOW || node->value.op >= OP_ABOVE_EQUAL))
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> \\%s}\"];\n", 
        (const void*)node, suffix, nd.fillcolor, nd.type_str, nd.value_str);
    else
        fprintf(dot_fp, 
        "    node_%p%s [fillcolor = \"%s\", label=\"{ <t> %s | <v> %s}\"];\n", 
        (const void*)node, suffix, nd.fillcolor, nd.type_str, nd.value_str);
    

    if (node -> left)
        tree_graph_dump_nodes(dot_fp, node -> left, suffix);
    if (node -> right)
        tree_graph_dump_nodes(dot_fp, node -> right, suffix);
        
    return;
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

    return;
}


frontend_err tree_graph_dump(Tree_t * tree, const char * filename_dot, 
                             const char * filename_png, const char * file_called, 
                             int line_called)
{
    assert(filename_dot && filename_png && tree); 
    DEBUG_PRINT("[INFO] Starting graph dump to %s and %s\n", filename_dot, filename_png);
    frontend_err error = FRONTEND_SUCCESS;

    if (!tree)
    {
        error = NULL_POINTER;
        ERROR_MESSAGE_FRONTEND(error);
        return error;
    }

    FILE * dot_fp = fopen(filename_dot, "w");
    if (!dot_fp)
    {
        error = OPENING_FILE_ERROR;
        ERROR_MESSAGE_FRONTEND(error);
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
        error = GRAPH_DUMP_ERROR;
        ERROR_MESSAGE_FRONTEND(error);
        return error;
    }

    DEBUG_PRINT("Graph dump completed: %s and %s have been created\n", filename_dot, filename_png);
    return error;
}


frontend_err tree_graph_optimizer(Tree_t * original_tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called)
{
    assert(filename_dot && filename_png && original_tree); 

    DEBUG_PRINT("[INFO] Starting graph dump in middleend to %s and %s\n", filename_dot, filename_png);
    
    FILE * dot_fp = fopen(filename_dot, "w");
    if (!dot_fp)
    {
        ERROR_MESSAGE_FRONTEND(OPENING_FILE_ERROR);
        return OPENING_FILE_ERROR;
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
    optimize_err error = optimize_tree_recursive(original_tree);
    if (error != OPTIMIZER_SUCCESS)
    {
        ERROR_MESSAGE_FRONTEND(GRAPH_DUMP_ERROR);
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
        ERROR_MESSAGE_FRONTEND(GRAPH_DUMP_ERROR);
        return GRAPH_DUMP_ERROR;
    }

    DEBUG_PRINT("[INFO] Graph dump of middleend tree completed: %s and %s have been created\n", filename_dot, filename_png);

    return FRONTEND_SUCCESS;
}



void define_node_type_for_dump(node_dump_t * nd)
{
    assert(nd);

    switch (nd->type)
    {
        case OPERATOR:
        {
            nd->fillcolor = "#d460aeff";
            nd->value_str = get_string_operator(nd->node->value.op);
            break;
        }
        case IDENTIFIER:
        {
            nd->fillcolor = "#f7d598ff";
            nd->value_str = nd->node->id.name;
            break;
        }
        case NUMBER:
        {
            nd->fillcolor = "#98f7aaff";

            static char buf[64];  

            snprintf(buf, sizeof(buf), "%g", nd->node->value.number);
            nd->value_str = buf;
            break;
        }
        case ROOT:
        {
            nd->fillcolor = "#afdeedff";
            nd->value_str = (nd->node->value.root) ? (nd->node->value.root) : "ROOT_NULL";
            break;
        }
        case STATEMENT:
        {
            nd->fillcolor = "#d29bdaff";
            if ((nd->node->value.stmt == OP_FUNC_DEF || nd->node->value.stmt == OP_CALL) && nd->node->id.name)
            {
                nd->value_str = get_statement_name(nd->node->value.stmt); 
                break;
            }

            if (nd->node->id.name)
                nd->value_str = nd->node->id.name;
            else
                nd->value_str = get_statement_name(nd->node->value.stmt);
            break;
        }
        case STRING:
        {
            nd->fillcolor = "#f7d598ff";
            nd->value_str = (nd->node->value.string_value) ? (nd->node->value.string_value) : "STRING_NULL";
            break;
        }
        default:
        {
            nd->fillcolor = "#fefcfcff";
            nd->fillcolor = "#fefcfcff";
            nd->value_str = "UNKNOWN_TYPE";
            DEBUG_PRINT("WARNING: Unknown node type in define_node_type_for_dump: %d\n", nd->type);
            break;
        }
    }
}


