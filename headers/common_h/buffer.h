#pragma once

#include <stdio.h>

typedef enum
{
    BUFFER_SUCCESS,
    BUFFER_FILE_OPEN_ERROR,
    BUFFER_FILE_CLOSE_ERROR,
    BUFFER_ALLOCATION_ERROR,
} buffer_err;


char * buffer_ctor(buffer_err * error, const char * filename);
buffer_err buffer_dtor(char * buffer);
ssize_t get_file_size(const char * filename);
void skip_spaces_buffer(const char ** s);