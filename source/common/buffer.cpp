#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "buffer.h"


char * buffer_ctor(buffer_err * error, const char * filename)
{
    assert(filename);

    size_t file_size = get_file_size(filename);
    if (file_size == (size_t)-1) 
    {
        *error =  BUFFER_FILE_OPEN_ERROR;
        return NULL;
    }

    char * buffer = (char *)calloc(file_size + 1, sizeof(char));
    if (buffer == NULL) 
    {
        *error = BUFFER_ALLOCATION_ERROR;
        return NULL;
    }

    FILE * file_ptr = fopen(filename, "r");
    if (!file_ptr) 
    {
        buffer_dtor(buffer);
        *error = BUFFER_FILE_OPEN_ERROR;
        return NULL;
    }

    size_t read_symbols = fread(buffer, sizeof(char), file_size, file_ptr);
    buffer[read_symbols] = '\0';
    fclose(file_ptr);

    *error = BUFFER_SUCCESS;
    return buffer;
}

buffer_err buffer_dtor(char * buffer)
{
    assert(buffer);

    free(buffer);
    buffer = NULL;

    return BUFFER_SUCCESS;
}


ssize_t get_file_size(const char * filename) 
{
    assert(filename != NULL);
    
    struct stat file_stat = {};
    if (stat(filename, &file_stat) == -1) 
    {
        return -1;
    }

    return (file_stat.st_size);
}


void skip_spaces_buffer(const char ** s)
{
    assert(s);

    while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
        (*s)++;

    return;
}