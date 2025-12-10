#include <stdio.h>
#include <assert.h>

#include "tree_structure.h"
#include "latex_dump.h"
#include "errors.h"



FILE * init_latex_dump(const char * filename)
{
    assert(filename);
    ErrorCode error = SUCCESS;

    FILE * fp = fopen(filename, "w");
    if (!fp)
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return NULL;
    }

    fprintf(fp,
            "\\documentclass{article}\n"
            "\\usepackage{amsmath}\n"
            "\\usepackage{amssymb}\n"
            "\\usepackage{geometry}\n"
            "\\geometry{margin=1in}\n"
            "\\begin{document}\n"
            "\\[\n");
    
    return fp;
}


void close_latex_dump(FILE * fp)
{
    fprintf(fp,
        "\n\\]\n"
        "\\end{document}\n");

    fclose(fp);
}

/*
void latex_dump_step(const char * filename, const Tree_t * tree)
{
    assert(filename);

    FILE * fp = fopen(filename, "a");
    if (!fp)
    {
        ErrorCode error = SUCCESS;
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return;
    }

    fprintf(fp, "\\[ ");
    node_to_latex(tree->root->right, fp);
    fprintf(fp, "\\] ");

    fclose(fp);
}

void node_to_latex(const Node_t * node, FILE * fp)
{
    if (!node)  return;

    type_t type = node->type;

    switch (type)
    {
    case NUMBER:
        fprintf(fp, "%g", node->value.number);
        return;
    case VARIABLE:
        fprintf(fp, "%s", get_var_name(node->value.var_index));
        return;
    case OPERATOR:
    {
        operator_t op = node->value.op;
        Node_t * u = node->left;
        Node_t * v = node->right;

        switch (op)
        {
            case OP_ADD:
                fprintf(fp, "("); node_to_latex(u, fp); fprintf(fp, " + "); node_to_latex(v, fp); fprintf(fp, ")");
                return;

            case OP_SUB:
                fprintf(fp, "("); node_to_latex(u, fp); fprintf(fp, " - "); node_to_latex(v, fp); fprintf(fp, ")");
                return;

            case OP_MUL:
                fprintf(fp, "("); node_to_latex(u, fp); fprintf(fp, "\\cdot "); node_to_latex(v, fp); fprintf(fp, ")");
                return;

            case OP_DIV:
                fprintf(fp, "\\frac{"); node_to_latex(u, fp); fprintf(fp, "}{"); node_to_latex(v, fp); fprintf(fp, "}");
                return;

            case OP_POW:
                fprintf(fp, "{("); node_to_latex(u, fp); fprintf(fp, ")^{"); node_to_latex(v, fp); fprintf(fp, "}}");
                return;

            case OP_SIN:  fprintf(fp, "\\sin(");  node_to_latex(u, fp); fprintf(fp, ")"); return;
            case OP_COS:  fprintf(fp, "\\cos(");  node_to_latex(u, fp); fprintf(fp, ")"); return;
            case OP_TAN:  fprintf(fp, "\\tan(");  node_to_latex(u, fp); fprintf(fp, ")"); return;
            case OP_LN:   fprintf(fp, "\\ln(");   node_to_latex(u, fp); fprintf(fp, ")"); return;
            case OP_EXP:  fprintf(fp, "e^{");     node_to_latex(u, fp); fprintf(fp, "}"); return;
            case OP_SQRT: fprintf(fp, "\\sqrt{"); node_to_latex(u, fp); fprintf(fp, "}"); return;

            case OP_UNARY_MINUS:
                fprintf(fp, "-("); node_to_latex(u, fp); fprintf(fp, ")");
                return;

            default:
                fprintf(fp, "??");
                return;
        }
    }
     
    default:
        fprintf(fp, "??");
    }
    fclose(fp);
}
*/