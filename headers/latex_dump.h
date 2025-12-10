#pragma once

#include <stdio.h>
#include "tree_structure.h"

#define LATEX_DUMP_FILE "logger/dump.tex"

FILE * init_latex_dump(const char * filename);
void latex_dump_step(const char * filename, const Tree_t * tree);
void node_to_latex(const Node_t * node, FILE * fp);
void close_latex_dump(FILE * fp);