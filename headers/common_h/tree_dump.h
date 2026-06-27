#pragma once

#include <stdio.h>
#include "errors.h"
#include "tree_structure.h"



typedef struct 
{
    const Node_t * node;
    type_t type;
    const char * fillcolor;
    const char * value_str;
    const char * type_str;
} node_dump_t;



void tree_graph_dump_nodes(FILE * dot_fp, const Node_t * node, const char * suffix);
void tree_graph_dump_edges(FILE * dot_fp, const Node_t * node, const char * suffix);
frontend_err tree_graph_dump(Tree_t * tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called);
void define_node_type_for_dump(node_dump_t * nd);
frontend_err tree_graph_optimizer(Tree_t * original_tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called);


#define GRAPH_DUMP(tree, name) do { \
    graph_dump_count++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/pics/graph_tree_%s_%d.png", name, graph_dump_count); \
    sprintf(dot_file, "logger/dots/tree%d.dot", graph_dump_count); \
    tree_graph_dump(tree, dot_file, png_file, __FILE__, __LINE__); \
} while(0)


#define GRAPH_DUMP_OPTIMIZER(source_tree) do { \
    graph_dump_diff++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/pics/graph_middleend_tree%d.png", graph_dump_diff); \
    sprintf(dot_file, "logger/dots/middleend_tree%d.dot", graph_dump_diff); \
    tree_graph_optimizer(source_tree, dot_file, png_file, __FILE__, __LINE__); \
} while(0)


#define GRAPH_DUMP_NODE(node) do { \
    graph_dump_count_node++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/graph_node%d.png", graph_dump_count); \
    sprintf(dot_file, "logger/node%d.dot", graph_dump_count); \
    graph_dump_node(node, dot_file, png_file, __FILE__, __LINE__); \
} while(0)

