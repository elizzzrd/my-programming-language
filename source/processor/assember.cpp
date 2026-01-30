#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>

#include "stack.h"
#include "errors_spu.h"
#include "read_file.h"
#include "spu.h"
#include "assembler.h"
#include "errors.h"
#include "utils.h"



static label_t g_labels[MAX_LABELS];
static size_t labels_count = 0;
static double code_buffer[MAX_CODE_SIZE];
static size_t code_size = 0;


void label_dump(const char * func, int line, const char * filename)
{
    DEBUG_PRINT("\t============LABELS DUMB============\n");
    DEBUG_PRINT("labels dump called from %s %s: %d", filename, func, __LINE__);
    DEBUG_PRINT("labels [%p]", g_labels);
    DEBUG_PRINT("\tSize = %d", MAX_LABELS);
    DEBUG_PRINT("{");
    for (int i = 0; i < MAX_LABELS; i++)
    {
        DEBUG_PRINT("\t*[%d] = %s -- %d",
        i,
        g_labels[i].name ? g_labels[i].name : "(null)",
        g_labels[i].instructor_ptr);
    }
    DEBUG_PRINT("}");
    DEBUG_PRINT("\t============END LABEL DUMP============");
}

int is_label(const char * option, size_t ip)
{
    assert(option);

    size_t len = strlen(option);
    if (option[len - 1] == ':')
    {
        char name[MAX_LABEL_LEN];
        extract_label_name(option, name);

        if (find_label(name) != -1)
        {
            DEBUG_PRINT("[ERROR] label redefinition");
            return -2;
        }

        if (labels_count < MAX_LABELS)
        {
            g_labels[labels_count++] = (label_t){.name = strdup(name), .instructor_ptr = ip};
            return (int)(labels_count - 1);
        }
        else
        {
            DEBUG_PRINT("[ERROR] labels is out of range");
            return -2;
        }
    }
    return -1; 
}

int find_label(const char * name)
{
    assert(name);

    for (int i = 0; i < (int)labels_count; i++)
    {
        if (strcmp(name, g_labels[i].name) == 0)
            return i;
    }
    return -1;
}

char * extract_label_name(const char * option, char * name)
{
    size_t len = strlen(option) - 1;
    if (len >= MAX_LABEL_LEN)
        len = MAX_LABEL_LEN - 1;

    memcpy(name, option, len);
    name[len] = '\0';

    return name;
} 


void init_labels(void) 
{
    labels_count = 0;
    for (int i = 0; i < MAX_LABELS; i++)
    {
        g_labels[i].instructor_ptr = -1;
        g_labels[i].name = NULL;
    }
}

void destroy_labels(label_t * labels)
{
    assert(labels);

    for (size_t i = 0; i < labels_count; i++)
        free(labels[i].name);
}


/*
Spu_Err parse_argument(const char * arg, double * value, type_arg * type) 
{
    assert(value && type);
    Spu_Err errors = SPU_OK;
    *type = UNKNOWN_TYPE;
    if (!arg || !*arg) return SPU_INVALID_COMMAND;

    if (is_number(arg)) 
    {
        *value = atoll(arg);
        *type = NUM;
        return SPU_OK;
    }
    else if (arg[0] == 'r' && isalpha((unsigned char)arg[1])) 
    {
        int reg_num = (toupper(arg[1]) - 'A') + 1;
        if (strlen(arg) != 3 || (reg_num < 1 || reg_num > 16)) 
        {
            DEBUG_PRINT("[ERROR] Invalid register name");
            return SPU_INVALID_COMMAND;
        }
        *value = reg_num;
        *type = REG;
        return errors;
    } 
    else if (arg[0] == '[' && arg[strlen(arg) - 1] == ']')
    {
        char inside[5] = {};
        size_t len = strlen(arg);

        if (len <=2)
        {
            DEBUG_PRINT("[ERROR] Empty brackets in pushm/popm");
            errors |= SPU_INVALID_COMMAND;
            return errors;
        }

        strncpy(inside, arg + 1, len - 2);
        inside[len - 2] = '\0';
        if (is_number(inside))
        {
            int addr = atoi(inside);
            if (addr < 0)
            {
                DEBUG_PRINT("[ERROR] Negative RAM address");
                return SPU_INVALID_COMMAND;
            }
            *value = addr;
            *type = RAM;
            return errors;
        }
    }

    int id = find_label(arg);
    if (id != -1) 
    {
        *value = g_labels[id].instructor_ptr;
        *type = LABEL;
        return errors;
    }

    DEBUG_PRINT("[ERROR] Unrecognized argument type '%s'", get_string_type_arg(*type));
    *type = UNKNOWN_TYPE;
    return SPU_INVALID_COMMAND;
}
*/


Spu_Err parse_argument(const char * arg, double * value, type_arg * type) 
{
    assert(value && type);
    Spu_Err errors = SPU_OK;
    *type = UNKNOWN_TYPE;
    if (!arg || !*arg) return SPU_INVALID_COMMAND;

    // Пробуем распарсить как число (целое или вещественное)
    char *endptr = NULL;
    double num = strtod(arg, &endptr);
    
    if (endptr != arg && *endptr == '\0') 
    {
        // Это число (целое или вещественное)
        *value = num;  // Сохраняем как double с дробной частью
        *type = NUM;
        return SPU_OK;
    }
    else if (arg[0] == 'r' && isalpha((unsigned char)arg[1])) 
    {
        int reg_num = (toupper(arg[1]) - 'A') + 1;
        if (strlen(arg) != 3 || (reg_num < 1 || reg_num > 16)) 
        {
            DEBUG_PRINT("[ERROR] Invalid register name");
            return SPU_INVALID_COMMAND;
        }
        *value = (double)reg_num;  // Регистр тоже как double
        *type = REG;
        return errors;
    } 
    else if (arg[0] == '[' && arg[strlen(arg) - 1] == ']')
    {
        char inside[50] = {};
        size_t len = strlen(arg);

        if (len <= 2)
        {
            DEBUG_PRINT("[ERROR] Empty brackets in pushm/popm");
            errors |= SPU_INVALID_COMMAND;
            return errors;
        }

        strncpy(inside, arg + 1, len - 2);
        inside[len - 2] = '\0';
        
        // Пробуем распарсить как число для адреса
        num = strtod(inside, &endptr);
        if (endptr != inside && *endptr == '\0')
        {
            // Адрес тоже может быть double, но обычно целый
            if (num < 0)
            {
                DEBUG_PRINT("[ERROR] Negative RAM address");
                return SPU_INVALID_COMMAND;
            }
            *value = num;  // Сохраняем как double
            *type = RAM;
            return errors;
        }
    }

    int id = find_label(arg);
    if (id != -1) 
    {
        *value = (double)g_labels[id].instructor_ptr;  // Приведение к double
        *type = LABEL;
        return errors;
    }

    DEBUG_PRINT("[ERROR] Unrecognized argument type '%s'", arg);
    *type = UNKNOWN_TYPE;
    return SPU_INVALID_COMMAND;
}


// узнать, где что находиться
Spu_Err first_pass(const char * txt_filename) 
{
    Spu_Err errors = SPU_OK;
    assert(txt_filename != NULL); 
    
    FILE * file_txt = fopen(txt_filename, "r");
    if (!file_txt) 
    {
        DEBUG_PRINT("[ERROR] Can not open the file %s.\n", txt_filename); 
        errors |= SPU_FILE_ERROR;
        return errors;
    }

    init_labels();
    char cmd_command[NMAX];
    size_t ip = 0;

    while (fgets(cmd_command, NMAX, file_txt))
    {
        cmd_command[strcspn(cmd_command, "\r\n")] = '\0';

        char * comment = strchr(cmd_command, ';'); 
        if (comment) 
            *comment = '\0'; // убираем комментарии
        
        char * line_ptr = cmd_command;
        while (isspace((unsigned char)*line_ptr)) line_ptr++;
        
        char * end = line_ptr + strlen(line_ptr);
        while (end > line_ptr && isspace((unsigned char)*(end - 1))) {
            end--;
        }
        *end = '\0';
        
        //printf("[DEBUG] cmd_command = %s\n", line_ptr);
        if (*line_ptr == '\0') continue; // пустая строка

        char option[NMAX] = {};
        if (sscanf(line_ptr, "%s", option) != 1) continue;

        int label_num = is_label(option, ip); 
        if (label_num != -1)
        {
            continue;
        }
        else if (label_num == -2)
        {
            errors |= SPU_INVALID_COMMAND;
            return errors;
        }

        int cmd = check_option_with_stack_commands(option);
        //printf("assembler 181: option: %s - cmd: %d \n", option, cmd);
        if (cmd == -1) 
        {
            DEBUG_PRINT("[ERROR] Unknown command in text file, option -- %s\n", option);
            errors |= SPU_UNKNOWN_COMMAND;
        }

        if (cmd == PUSH || cmd == PUSHR || cmd == POPR || cmd == PUSHM || cmd == POPM) {ip += 2;}
        else if ((cmd >= JB && cmd <= JMP) || cmd == CALL ) {ip += 2;}
        else {ip++;}
    }  
    fclose(file_txt);
    return errors;
}

//записать, что именно выполнять
Spu_Err second_pass(const char * txt_filename) 
{
    assert(txt_filename);
    Spu_Err errors = SPU_OK;

    FILE *file = fopen(txt_filename, "r");
    if (!file) {
        DEBUG_PRINT("[ERROR] Cannot open input file (second pass)");
        return SPU_FILE_ERROR;
    }

    code_size = 0;
    char cmd_command[NMAX] = {};
    size_t ip = 0;

    while (fgets(cmd_command, NMAX, file)) 
    {
        char * comment = strchr(cmd_command, ';'); 
        if (comment) (*comment) = '\0'; // убираем комментарии

        char * line_ptr = cmd_command;
        while (isspace((unsigned char)*line_ptr)) line_ptr++;
        if (*line_ptr == '\0') continue; // пустая строка

        char option[NMAX] = {};
        if (sscanf(line_ptr, "%s", option) != 1) continue;

        size_t len = strlen(option);
        if (option[len - 1] == ':')
            continue;
    
        int cmd = check_option_with_stack_commands(option);
        if (cmd == -1) 
        {
            DEBUG_PRINT("[ERROR] Unknown command");
            errors |= SPU_UNKNOWN_COMMAND;
            continue;
        }

        code_buffer[code_size++] = cmd;

        char arg[NMAX] = {};
        if (sscanf(cmd_command + strlen(option), "%s", arg) == 1) 
        {
            double value = 0;
            type_arg type = UNKNOWN_TYPE;
            errors |= parse_argument(arg, &value, &type);

            if (errors != SPU_OK)
            {
                DEBUG_PRINT("[ERROR] Error during parsing");
                return errors;
            }
            else if (type == UNKNOWN_TYPE)
            {
                DEBUG_PRINT("[ERROR] Unknown argument type");
                return errors;
            }
            code_buffer[code_size++] = value;
        }

        if (cmd == PUSH || cmd == PUSHR || cmd == POPR|| cmd == PUSHM || cmd == POPM) {ip += 2;}
        else if ((cmd >= JB && cmd <= JMP) || cmd == CALL ) {ip += 2;}
        else {ip++;}
    }

    fclose(file);
    return errors;
}


StackElem * load_bytecode(const char * file_byte, size_t * size)
{
    assert(file_byte != NULL);
    assert(size != NULL);

    FILE * file = fopen(file_byte, "r");
    if (!file) 
    {
        DEBUG_PRINT("[ERROR] Can not open the file %s.\n", file_byte);
        return NULL;
    }

    #define MAX_TEMP 1024
    double temp_buffer[MAX_TEMP];
    size_t count = 0;
    
    while (count < MAX_TEMP && fscanf(file, "%lg", &temp_buffer[count]) == 1) 
        count++;
    
    
    fclose(file);
    
    if (count == 0) {
        *size = 0;
        return NULL;
    }
    
    StackElem * code = (StackElem *) malloc(count * sizeof(StackElem));  
    if (!code) 
    { 
        return NULL; 
    }
    
    for (size_t i = 0; i < count; i++) {
        code[i] = temp_buffer[i];
    }
    
    *size = count;
    return code;
}

Spu_Err assembler(const char * txt_filename, const char * byte_filename) 
{
    assert(txt_filename);
    assert(byte_filename);

    Spu_Err errors = SPU_OK;

    errors |= first_pass(txt_filename);
    label_dump(__func__, __LINE__, __FILE__);
    if (errors != SPU_OK) 
    {
        DEBUG_PRINT("[ERROR] Assembler failed during 1 pass");
        return errors;
    }
    errors |= second_pass(txt_filename);
    label_dump(__func__, __LINE__, __FILE__);
    if (errors != SPU_OK) 
    {
        DEBUG_PRINT("[ERROR] Assembler failed during 2 pass");
        return errors;
    }

    FILE *out = fopen(byte_filename, "w+");
    if (!out) 
    {
        DEBUG_PRINT("[ERROR] Cannot open output file");
        return SPU_FILE_ERROR;
    }

    for (size_t i = 0; i < code_size; i++)
        fprintf(out, "%lg\n", code_buffer[i]);

    destroy_labels(g_labels);
    fclose(out);
    return errors;
}