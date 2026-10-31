#ifndef FILE_SEARCH_H
#define FILE_SEARCH_H

#include <stddef.h>
#include "status.h"

typedef struct
{
    size_t line;
    size_t column;
} occurrence_t;

typedef struct
{
    const char *path;     
    status_t status;      
    size_t count;          
    occurrence_t *items;   
} file_result_t;

status_t search_in_files(file_result_t **results, const char *substring,
                         size_t file_count, ...);

void free_results(file_result_t *results, size_t file_count);

#endif