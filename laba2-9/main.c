#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

static void handle_error_output(ExecutionStatus status_code) {
    switch (status_code) {
        case STATUS_ERROR_RADIX:
            fprintf(stderr, "Validation Error: Radix must be greater than or equal to 2.\n");
            break;
        case STATUS_ERROR_DOMAIN:
            fprintf(stderr, "Validation Error: Fraction value must strictly reside within (0, 1).\n");
            break;
        case STATUS_ERROR_POINTER:
            fprintf(stderr, "System Error: Null pointer reference detected.\n");
            break;
        default:
            fprintf(stderr, "Unknown Error occurred.\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        int radix = atoi(argv[1]);
        size_t count = (size_t)(argc - 2);
        int *results_buffer = (int *)malloc(count * sizeof(int));
        if (results_buffer == NULL) {
            fprintf(stderr, "Critical Error: Failed to allocate dynamic memory buffer.\n");
            return 1;
        }

        ExecutionStatus execution_st = STATUS_SUCCESS;
        for (size_t i = 0; i < count; i++) {
            double parsed_fraction = atof(argv[2 + i]);
            execution_st = verify_fraction_termination(parsed_fraction, radix, &results_buffer[i], EPSILON_DEFAULT);
            if (execution_st != STATUS_SUCCESS) {
                free(results_buffer);
                handle_error_output(execution_st);
                return 1;
            }
        }

        for (size_t i = 0; i < count; i++) {
            printf("Fraction %s in radix %d: %s\n", 
                   argv[2 + i], 
                   radix, 
                   results_buffer[i] ? "has finite representation" : "has infinite representation");
        }

        free(results_buffer);
        return 0;
    }

    /* Демонстрация работы вариативной функции */
    int sample_results_radix2[3];
    ExecutionStatus demo_st = analyze_base_terminations(2, 3, sample_results_radix2, 0.5, 0.25, 0.2);
    if (demo_st == STATUS_SUCCESS) {
        printf("Radix 2 Demonstration:\n");
        printf("  0.5:  %s\n", sample_results_radix2[0] ? "finite" : "infinite");
        printf("  0.25: %s\n", sample_results_radix2[1] ? "finite" : "infinite");
        printf("  0.2:  %s\n", sample_results_radix2[2] ? "finite" : "infinite");
    }

    int sample_results_radix10[3];
    demo_st = analyze_base_terminations(10, 3, sample_results_radix10, 0.5, 0.25, 0.2);
    if (demo_st == STATUS_SUCCESS) {
        printf("Radix 10 Demonstration:\n");
        printf("  0.5:  %s\n", sample_results_radix10[0] ? "finite" : "infinite");
        printf("  0.25: %s\n", sample_results_radix10[1] ? "finite" : "infinite");
        printf("  0.2:  %s\n", sample_results_radix10[2] ? "finite" : "infinite");
    }

    return 0;
}