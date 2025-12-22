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


static int labels[MAX_LABELS];
static int code_buffer[MAX_CODE_SIZE];
static size_t code_size = 0;


void label_dump(const char * func, int line, const char * filename)
{
    fprintf(log_file, "============LABELS DUMB============\n");
    fprintf(log_file, "\nlabels dump called from %s %s: %d\n", filename, func, __LINE__);
    fprintf(log_file, "labels [%p]\n", labels);
    fprintf(log_file, "\n\tSize = %d\n", MAX_LABELS);
    fprintf(log_file, "{\n");
    for (int i = 0; i < MAX_LABELS; i++)
    {
        fprintf(log_file, "\t*[%d] = %d\n", i, labels[i]);
    }
    fprintf(log_file, "}\n");
    fprintf(log_file, "============END LABEL DUMP============\n");
}


void init_labels(void) 
{
    for (int i = 0; i < MAX_LABELS; i++)
        labels[i] = -1;
}


Spu_Err parse_argument(const char * arg, int * value, type_arg * type) 
{
    assert(value && type);
    Spu_Err errors = SPU_OK;
    if (!arg || !*arg) return SPU_INVALID_COMMAND;

    if (is_number(arg)) 
    {
        *value = atoi(arg);
        *type = NUM;
        return errors;
    }
    else if (arg[0] == 'r' && isalpha((unsigned char)arg[1])) 
    {
        int reg_num = (toupper(arg[1]) - 'A') + 1;
        if (strlen(arg) != 3 || (reg_num < 1 || reg_num > 16)) 
        {
            log_message("Invalid register name", __FILE__, __LINE__);
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
            log_message("Empty brackets in pushm/popm", __FILE__, __LINE__);
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
                log_message("Negative RAM address", __FILE__, __LINE__);
                return SPU_INVALID_COMMAND;
            }
            *value = addr;
            *type = RAM;
            return errors;
        }
    }
    else if (is_label(arg)) 
    {
        int id = atoi(arg + 1);
        if (id < 0 || id >= MAX_LABELS) 
        {
            log_message("Label ID out of range", __FILE__, __LINE__);
            return SPU_INVALID_COMMAND;
        }
        if (labels[id] == -1) {
            log_message("Label used but not defined", __FILE__, __LINE__);
            return SPU_INVALID_COMMAND;
        }
        *value = labels[id];
        *type = LABEL;
        return errors;
    }

    log_message("Unrecognized argument type", __FILE__, __LINE__);
    *type = UNKNOWN_TYPE;
    return SPU_INVALID_COMMAND;
}



// узнать, где что находиться
Spu_Err first_pass(const char * txt_filename) 
{
    Spu_Err errors = SPU_OK;
    assert(txt_filename != NULL); 
    
    printf("[DEBUG] Opening file: %s\n", txt_filename);
    FILE * file_txt = fopen(txt_filename, "r");
    if (!file_txt) 
    {
        fprintf(log_file, "Can not open the file %s.\n", txt_filename); 
        errors |= SPU_FILE_ERROR;
        return errors;
    }
    printf("[DEBUG] fopen returned: %p\n", (void*)file_txt);

    if (!log_file) printf("[WARN] log_file == NULL\n");


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

        int label_num = is_label(option); 
        if (label_num != 0 && label_num != -1)
        {
            if (labels[label_num] != -1)
            {
                log_message("label has already been defined", __FILE__, __LINE__);
                errors |= SPU_INVALID_COMMAND;
                return errors;
            }
            labels[label_num] = (int)ip;
            continue;
        }
        else if (label_num == 0)
        {
            errors |= SPU_INVALID_COMMAND;
            log_message("Label index out of range", __FILE__, __LINE__);
        }

        int cmd = check_option_with_stack_commands(option);
        //printf("assembler 181: option: %s - cmd: %d \n", option, cmd);
        if (cmd == -1) 
        {
            log_message("Unknown command in text file\n", __FILE__, __LINE__);
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
        log_message("Cannot open input file (second pass)", __FILE__, __LINE__);
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

        int label_num = is_label(option);
        if (label_num > 0)
            continue;
        else if (label_num == 0)
        {
            log_message("Label index out of range", __FILE__, __LINE__);
            errors |= SPU_INVALID_COMMAND;
            continue;
        }
    
        int cmd = check_option_with_stack_commands(option);
        //printf("[DEBUG] cmd = %d, option = %s\n", cmd, option);
        if (cmd == -1) 
        {
            log_message("Unknown command", __FILE__, __LINE__);
            errors |= SPU_UNKNOWN_COMMAND;
            continue;
        }

        code_buffer[code_size++] = cmd;
        // printf("[DEBUG] code_size = %lu", code_size);
        // printf("[DEBUG] code_buffer ");
        // for (size_t i = 0; i < code_size; i++)
        //     printf("%d ", code_buffer[i]);
        // printf("\n");

        char arg[NMAX] = {};
        if (sscanf(cmd_command + strlen(option), "%s", arg) == 1) 
        {
            int value = 0;
            type_arg type = UNKNOWN_TYPE;
            errors |= parse_argument(arg, &value, &type);

            if (errors != SPU_OK)
            {
                log_message("Error during parsing", __FILE__, __LINE__);
                return errors;
            }
            else if (type == UNKNOWN_TYPE)
            {
                log_message("Unknown argument type", __FILE__, __LINE__);
                return errors;
            }
            // else if (type == LABEL && labels[value] == -1)
            // {
            //     log_message("Undefined label", __FILE__, __LINE__);
            //     printf("[DEBUG] label_num = %d, labels[label_num] = %d\n", value, labels[value]);
            //     errors |= SPU_INVALID_COMMAND;
            //     return errors;
            // }
            code_buffer[code_size++] = value;
        }

        if (cmd == PUSH || cmd == PUSHR || cmd == POPR|| cmd == PUSHM || cmd == POPM) {ip += 2;}
        else if ((cmd >= JB && cmd <= JMP) || cmd == CALL ) {ip += 2;}
        else {ip++;}
    }

    fclose(file);
    return errors;
}


int * load_bytecode(const char * file_byte, size_t * size)
{
    assert(file_byte != NULL);
    assert(size != NULL);

    FILE * file = fopen(file_byte, "r");
    if (!file) 
    {
        fprintf(log_file, "Can not open the file %s.\n", file_byte);
        return NULL;
    }

    
    #define MAX_TEMP 1024
    int temp_buffer[MAX_TEMP];
    size_t count = 0;
    
    while (count < MAX_TEMP && fscanf(file, "%d", &temp_buffer[count]) == 1) {
        count++;
    }
    
    fclose(file);
    
    if (count == 0) {
        *size = 0;
        return NULL;
    }
    
    int * code = (int *) malloc(count * sizeof(int));  
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
        log_message("Assembler failed during 1 pass", __FILE__, __LINE__);
        return errors;
    }
    errors |= second_pass(txt_filename);
    label_dump(__func__, __LINE__, __FILE__);
    if (errors != SPU_OK) 
    {
        log_message("Assembler failed during 2 pass", __FILE__, __LINE__);
        return errors;
    }

    FILE *out = fopen(byte_filename, "w+");
    if (!out) 
    {
        log_message("Cannot open output file", __FILE__, __LINE__);
        return SPU_FILE_ERROR;
    }

    for (size_t i = 0; i < code_size; i++)
        fprintf(out, "%d\n", code_buffer[i]);

    fclose(out);
    return errors;
}