#pragma once

#include <stdio.h>
#include "tree_structure.h"

extern int graph_dump_count;
extern int graph_dump_count_node;
extern int graph_dump_diff;



#define GRAPH_DUMP(tree, name) do { \
    graph_dump_count++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/pics/graph_tree_%s_%d.png", name, graph_dump_count); \
    sprintf(dot_file, "logger/dots/tree%d.dot", graph_dump_count); \
    tree_graph_dump(tree, dot_file, png_file, __FILE__, __LINE__); \
} while(0)


#define GRAPH_DUMP_MIDDLEEND(source_tree) do { \
    graph_dump_diff++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/pics/graph_middleend_tree%d.png", graph_dump_diff); \
    sprintf(dot_file, "logger/dots/middleend_tree%d.dot", graph_dump_diff); \
    tree_graph_middleend(source_tree, dot_file, png_file, __FILE__, __LINE__); \
} while(0)



#define GRAPH_DUMP_NODE(node) do { \
    graph_dump_count_node++; \
    char png_file[100], dot_file[100]; \
    sprintf(png_file, "logger/graph_node%d.png", graph_dump_count); \
    sprintf(dot_file, "logger/node%d.dot", graph_dump_count); \
    graph_dump_node(node, dot_file, png_file, __FILE__, __LINE__); \
} while(0)


#define ERROR_MESSAGE(enum_error, error) \
    error = enum_error; \
    printf("\n\n%s:%d --- %s\n\n", __FILE__, __LINE__, tree_error_string(error));     \
    
// ErrorCode node_verify(Node_t * node)
// ErrorCode tree_verify(Tree_t * tree);

const char * tree_error_string(ErrorCode error); 

ErrorCode tree_graph_dump(Tree_t * tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called);
ErrorCode graph_dump_node(const Node_t * node, const char * filename_dot, const char * filename_png, const char * file_called, int line_called);
ErrorCode tree_graph_middleend(Tree_t * original_tree, const char * filename_dot, const char * filename_png, const char * file_called, int line_called);
void tree_graph_dump_nodes(FILE * dot_fp, const Node_t * node, const char * suffix);
void tree_graph_dump_edges(FILE * dot_fp, const Node_t * node, const char * suffix);


void define_node_type_for_dump(type_t type, const char ** fillcolor, const char ** value_str, const Node_t * node);
void make_html(void);