#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "translate_to_nasm.h"

variables_t vars = {};

void init_variables(void)
{
    vars.var_count = 0;
    vars.var_capacity = 16;
    vars.current_func_id = -1;
    vars.func_counter = 0;
    vars.var_list = (var_info_t *) calloc(vars.var_capacity, sizeof(var_info_t));
}


int find_variable_in_current_func(const char * name)
{
    for (int i = 0; i < vars.var_count; i++)
    {
        if (vars.var_list[i].func_id == vars.current_func_id &&
             strcmp(vars.var_list[i].name, name) == 0)
            return i;
    }
    return -1;
}

int add_variable(const char * name, bool is_parameter)
{
    int idx = find_variable_in_current_func(name);
    if (idx != -1)      return idx;

    if (vars.var_count >= vars.var_capacity)
    {
        vars.var_capacity *= 2;
        vars.var_list = (var_info_t *) realloc(vars.var_list, vars.var_capacity * sizeof(var_info_t));
    }
    
    int cur_count = vars.var_count; 
    int offset = 0;

    vars.var_list[cur_count].name = strdup(name);
    vars.var_list[cur_count].offset = offset;
    vars.var_list[cur_count].initialized = false;
    vars.var_list[cur_count].is_parametr = is_parameter;
    vars.var_list[cur_count].func_id = vars.current_func_id;
    vars.var_list[cur_count].param_index = -1;

    vars.var_count++;
    return cur_count;
}


var_info_t * get_variable_by_index(int index)
{
    if (index < 0 || index >= vars.var_count)
        return NULL;
    return &vars.var_list[index];
}


void clear_variables_for_func(int func_id)
{
    int i = 0;
    while (i < vars.var_count)
    {
        if (vars.var_list[i].func_id == func_id)
        {
            free(vars.var_list[i].name);

            for (int j = i; j < vars.var_count - 1; j++)
                vars.var_list[j] = vars.var_list[j+1];
            
            vars.var_count--;
        }
        else {
            i++;
        }
    }
}

void clear_all_variables(void)
{
    for (int i = 0; i < vars.var_count; i++) {
        if (vars.var_list[i].name) {
            free(vars.var_list[i].name);
            vars.var_list[i].name = NULL;
        }
    }
    vars.var_count = 0;
}


void destroy_variables(void)
{
    clear_all_variables();
    if (vars.var_list) {
        free(vars.var_list);
        vars.var_list = NULL;
    }
    vars.var_capacity = 0;
}


int get_frame_size_for_func(int func_id)
{
    int size = 0;
    for (int i = 0; i < vars.var_count; i++)
    {
        if (vars.var_list[i].func_id == func_id && !vars.var_list[i].is_parametr)
            size += 8;
    }
    return size;
}

void clear_variables(void)
{
    for (int i = 0; i < vars.var_count; i++)
        free(vars.var_list[i].name);

    free(vars.var_list);
    vars.var_list = NULL;
    vars.var_count = 0;
    vars.var_capacity = 0;
}


void assign_offset_for_function(int func_id)
{
    int param_count = 0;
    int local_count = 0;

    for (int i = 0; i < vars.var_count; i++)
    {
        if (vars.var_list[i].func_id == func_id && vars.var_list[i].is_parametr)
            param_count++;
    }

    int param_index = 0;
    for (int i = 0; i < vars.var_count; i++)
    {
        if (vars.var_list[i].func_id == func_id && vars.var_list[i].is_parametr)
        {
            vars.var_list[i].offset = - (param_index + 1) * 8;
            vars.var_list[i].param_index = param_index;
            param_index++;
        }
    }

    for (int i = 0; i < vars.var_count; i++) 
    {
        if (vars.var_list[i].func_id == func_id && !vars.var_list[i].is_parametr) 
            {
            vars.var_list[i].offset = -(param_count + local_count + 1) * 8;
            local_count++;
        }
    }
}

//--------------------------------------------------------------

ErrorCode collect_variables(Node_t * node)
{
    if (!node)      return SUCCESS;
    ErrorCode error = SUCCESS;

    if (node->type == STATEMENT)
    {
        switch (node->value.stmt)
        {
            case OP_ASSIGNMENT:
            case OP_VAR_DEF:
            {
                if (node->left && node->left->type == IDENTIFIER)
                {
                    int idx = add_variable(node->left->id.name, false);
                    if (idx == -1)
                        return SEMANTIC_ERROR;
                    
                    node->left->id.id_index = idx;
                }
                break;
            }
            case OP_FUNC_DEF:
            {
                int old_func = vars.current_func_id;
                vars.current_func_id = ++vars.func_counter;
                
                Node_t * params = node -> left;
                int param_count = 0;
    
                if (params && params->type == STATEMENT && params->value.stmt == OP_PARAMS)
                {
                    Node_t * param = params->left;
                    while (param)
                    {
                        if (param->type == IDENTIFIER) 
                        {     
                            int idx = add_variable(param->id.name, true);
                            param->id.id_index = idx;
                        }
                        param = param -> left;
                    }
                }
    
                error = collect_variables(node->right);
                if (error != SUCCESS)
                    return error;

                assign_offset_for_function(vars.current_func_id);
    
                vars.current_func_id = old_func;
                return SUCCESS;
            }
            default: break;
        }
    }

    error = collect_variables(node->left);
    if (error != SUCCESS)   return error;

    error = collect_variables(node->right);
    if (error != SUCCESS)   return error;

    return SUCCESS;
}

