#include "functions.h"
#include <stdlib.h>
#include <string.h>

static char *clone_string(const char *src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char *dest = malloc(len + 1);
    if (!dest) return NULL;
    strcpy(dest, src);
    return dest;
}

ErrorCode check_names(const File *files, size_t n) {
    if (!files && n > 0) return ERR_POINTER;
    
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (strcmp(files[i].name, files[j].name) == 0) {
                return ERR_DUPLICATE;
            }
        }
    }
    return SUCCESS;
}

void free_files(File *files, size_t n) {
    if (!files) return;
    for (size_t i = 0; i < n; i++) {
        free(files[i].name);
    }
    free(files);
}

void free_backup(Backup *b) {
    if (!b) return;
    if (b->filenames) {
        for (size_t i = 0; i < b->count; i++) {
            free(b->filenames[i]);
        }
        free(b->filenames);
        b->filenames = NULL;
    }
    b->count = 0;
    b->total_size = 0;
    b->total_importance = 0;
}

ErrorCode solve_knapsack(const File *files, size_t n, size_t capacity, Backup *out) {
    if (!files && n > 0) return ERR_POINTER;
    if (!out) return ERR_POINTER;

    out->filenames = NULL;
    out->count = 0;
    out->total_size = 0;
    out->total_importance = 0;

    if (n == 0 || capacity == 0) return SUCCESS;

    size_t cols = capacity + 1;
    unsigned long long *table = calloc((n + 1) * cols, sizeof(unsigned long long));
    if (!table) return ERR_MEMORY;

    for (size_t i = 1; i <= n; i++) {
        size_t f_size = files[i - 1].size;
        unsigned long long f_imp = files[i - 1].importance;
        
        for (size_t w = 0; w <= capacity; w++) {
            unsigned long long val_without = table[(i - 1) * cols + w];
            if (f_size <= w) {
                unsigned long long val_with = table[(i - 1) * cols + (w - f_size)] + f_imp;
                table[i * cols + w] = (val_with > val_without) ? val_with : val_without;
            } else {
                table[i * cols + w] = val_without;
            }
        }
    }

    int *taken = calloc(n, sizeof(int));
    if (!taken) {
        free(table);
        return ERR_MEMORY;
    }

    size_t curr_w = capacity;
    size_t selected_num = 0;
    size_t sum_size = 0;

    for (size_t i = n; i > 0; i--) {
        size_t f_size = files[i - 1].size;
        unsigned long long f_imp = files[i - 1].importance;

        if (curr_w >= f_size && table[i * cols + curr_w] == table[(i - 1) * cols + (curr_w - f_size)] + f_imp) {
            taken[i - 1] = 1;
            curr_w -= f_size;
            selected_num++;
            sum_size += f_size;
        }
    }

    out->total_importance = table[n * cols + capacity];
    out->total_size = sum_size;
    out->count = selected_num;

    if (selected_num > 0) {
        out->filenames = malloc(selected_num * sizeof(char *));
        if (!out->filenames) {
            free(taken);
            free(table);
            return ERR_MEMORY;
        }

        size_t idx = 0;
        for (size_t i = 0; i < n; i++) {
            if (taken[i]) {
                out->filenames[idx] = clone_string(files[i].name);
                if (!out->filenames[idx]) {
                    for (size_t k = 0; k < idx; k++) {
                        free(out->filenames[k]);
                    }
                    free(out->filenames);
                    out->filenames = NULL;
                    free(taken);
                    free(table);
                    return ERR_MEMORY;
                }
                idx++;
            }
        }
    }

    free(taken);
    free(table);
    return SUCCESS;
}