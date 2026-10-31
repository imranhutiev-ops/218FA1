#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_err(ErrorCode err) {
    if (err == ERR_INPUT) {
        fprintf(stderr, "Error: Invalid input data format or values.\n");
    } else if (err == ERR_DUPLICATE) {
        fprintf(stderr, "Error: Duplicate file names detected in input.\n");
    } else if (err == ERR_MEMORY) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
    } else if (err == ERR_POINTER) {
        fprintf(stderr, "Error: Null pointer encountered.\n");
    }
}

int main(void) {
    long long n_in = 0, cap_in = 0;

    if (scanf("%lld %lld", &n_in, &cap_in) != 2 || n_in <= 0 || cap_in < 0) {
        print_err(ERR_INPUT);
        return 1;
    }

    size_t n = (size_t)n_in;
    size_t capacity = (size_t)cap_in;

    File *files = malloc(n * sizeof(File));
    if (!files) {
        print_err(ERR_MEMORY);
        return 1;
    }

    for (size_t i = 0; i < n; i++) {
        files[i].name = NULL;
        files[i].size = 0;
        files[i].importance = 0;
    }

    char buffer[512];
    long long sz_in = 0;
    unsigned long long imp_in = 0;

    for (size_t i = 0; i < n; i++) {
        if (scanf("%511s %lld %llu", buffer, &sz_in, &imp_in) != 3 || sz_in <= 0) {
            free_files(files, n);
            print_err(ERR_INPUT);
            return 1;
        }

        size_t len = strlen(buffer);
        files[i].name = malloc(len + 1);
        if (!files[i].name) {
            free_files(files, n);
            print_err(ERR_MEMORY);
            return 1;
        }
        strcpy(files[i].name, buffer);
        files[i].size = (size_t)sz_in;
        files[i].importance = imp_in;
    }

    ErrorCode err = check_names(files, n);
    if (err != SUCCESS) {
        free_files(files, n);
        print_err(err);
        return 1;
    }

    Backup result;
    err = solve_knapsack(files, n, capacity, &result);
    free_files(files, n);

    if (err != SUCCESS) {
        print_err(err);
        return 1;
    }

    printf("TOTAL_SIZE %zu\n", result.total_size);
    printf("TOTAL_IMPORTANCE %llu\n", result.total_importance);
    printf("FILES %zu\n", result.count);
    for (size_t i = 0; i < result.count; i++) {
        printf("%s\n", result.filenames[i]);
    }

    free_backup(&result);
    return 0;
}
