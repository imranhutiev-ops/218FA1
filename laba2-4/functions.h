#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

typedef enum {
    SUCCESS = 0,
    ERR_INPUT,
    ERR_DUPLICATE,
    ERR_MEMORY,
    ERR_POINTER
} ErrorCode;

typedef struct {
    char *name;
    size_t size;
    unsigned long long importance;
} File;

typedef struct {
    char **filenames;
    size_t count;
    size_t total_size;
    unsigned long long total_importance;
} Backup;

ErrorCode check_names(const File *files, size_t n);
ErrorCode solve_knapsack(const File *files, size_t n, size_t capacity, Backup *out);
void free_backup(Backup *b);
void free_files(File *files, size_t n);

#endif