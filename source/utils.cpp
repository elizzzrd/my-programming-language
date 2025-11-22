#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

#include "utils.h"
#include "tree_structure.h"
#include "errors.h"



char * read_line(void)
{
    char * line = NULL;
    size_t bufsize = 0;

    ssize_t len = getline(&line, &bufsize, stdin);
    if (len < 0)
    {
        free(line);
        return NULL;
    }

    if (len > 0 && line[len - 1] == '\n')   line[len-1] = '\0'; 
    return line;
}


void to_upper_str(char * input)
{
    int i = 0;
    for (i = 0; input[i] != '\0'; i++)
    {
        input[i] = (char)toupper((unsigned char) input[i]);
    }
    input[i] = '\0';
}


size_t get_file_size(const char * filename) 
{
    assert(filename != NULL);
    
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) 
    {
        return (size_t)-1;
    }

    return (size_t)(file_stat.st_size);
}


char * initialize_buffer(size_t file_size)
{
    assert(file_size > 0);

    char * buffer = (char *)calloc(file_size + 1, sizeof(char));
    if (buffer == NULL) 
    {
        ErrorCode error;
        ERROR_MESSAGE(TREE_MEMORY_ALLOCATION_ERROR, error);
        return NULL;
    }
    return buffer;
}


ErrorCode load_to_buffer(const char * filename, char ** buffer)
{
    assert(filename);
    assert(buffer);

    ErrorCode error = SUCCESS;

    size_t file_size = get_file_size(filename);
    if (file_size == (size_t)-1) 
    {
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }

    *buffer = initialize_buffer(file_size);
    if (*buffer == NULL) 
    {
        ERROR_MESSAGE(TREE_MEMORY_ALLOCATION_ERROR, error);
        return error;
    }

    FILE * file_ptr = fopen(filename, "r");
    if (!file_ptr) 
    {
        free(*buffer);
        *buffer = NULL;
        ERROR_MESSAGE(OPENING_FILE_ERROR, error);
        return error;
    }

    size_t read_symbols = fread(*buffer, sizeof(char), file_size, file_ptr);
    (*buffer)[read_symbols] = '\0';
    fclose(file_ptr);

    return SUCCESS;
}

