#pragma once

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include "errors.h"



#define DEBUG_LOG_FILE "logger/debug.log"
#define DEBUG_PRINT(fmt, ...)                                               \
    do {                                                                    \
        FILE *dbg_fp = fopen(DEBUG_LOG_FILE, "a");                         \
        if (dbg_fp) {                                                       \
            fprintf(dbg_fp, "%s:%d: %s: " fmt "\n",                       \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);          \
            fclose(dbg_fp);                                                 \
        }                                                                   \
    } while (0)



bool IS_ZERO(Node_t * node); 
bool IS_ONE(Node_t * node); 


char * read_line(void);
void to_upper_str(char * input);

char * initialize_buffer(size_t file_size);
ErrorCode load_to_buffer(const char * filename, char ** buffer);
