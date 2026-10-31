#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

static void handle_error_code(ResultCode code) {
    switch (code) {
        case STATUS_BAD_BASE:
            fprintf(stderr, "[ERR] Base must be within the interval [2..36].\n");
            break;
        case STATUS_BAD_DIGIT:
            fprintf(stderr, "[ERR] Number contains a digit invalid for this base.\n");
            break;
        case STATUS_EMPTY_STR:
            fprintf(stderr, "[ERR] One of the number strings is empty.\n");
            break;
        case STATUS_ALLOC_FAIL:
            fprintf(stderr, "[ERR] Heap memory allocation error.\n");
            break;
        case STATUS_NULL_PTR:
            fprintf(stderr, "[ERR] Null pointer exception.\n");
            break;
        default:
            fprintf(stderr, "[ERR] Unknown execution error.\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        int radix = atoi(argv[1]);
        size_t items_count = (size_t)(argc - 2);
        char *final_output = NULL;
        ResultCode res_code = STATUS_SUCCESS;

        if (items_count <= 4) {
            // Вызов через variadic функцию для небольшого количества аргументов
            if (items_count == 1) {
                res_code = sum_multiple_numbers(radix, 1, &final_output, argv[2]);
            } else if (items_count == 2) {
                res_code = sum_multiple_numbers(radix, 2, &final_output, argv[2], argv[3]);
            } else if (items_count == 3) {
                res_code = sum_multiple_numbers(radix, 3, &final_output, argv[2], argv[3], argv[4]);
            } else if (items_count == 4) {
                res_code = sum_multiple_numbers(radix, 4, &final_output, argv[2], argv[3], argv[4], argv[5]);
            }
        } else {
            // Последовательное сложение в столбик для большего числа аргументов
            char *current_acc = NULL;
            res_code = column_addition(argv[2], argv[3], radix, &current_acc);
            if (res_code == STATUS_SUCCESS) {
                for (size_t i = 4; i < (size_t)argc; ++i) {
                    char *next_acc = NULL;
                    res_code = column_addition(current_acc, argv[i], radix, &next_acc);
                    free(current_acc);
                    if (res_code != STATUS_SUCCESS) {
                        break;
                    }
                    current_acc = next_acc;
                }
                final_output = current_acc;
            }
        }

        if (res_code != STATUS_SUCCESS) {
            handle_error_code(res_code);
            return 1;
        }

        printf("%s\n", final_output);
        free(final_output);
        return 0;
    }

    printf("=== Demonstration Mode ===\n");

    char *res_one = NULL;
    ResultCode code_one = sum_multiple_numbers(16, 3, &res_one, "FF", "1", "A0");
    if (code_one == STATUS_SUCCESS) {
        printf("Hex Sum (FF + 1 + A0) = %s\n", res_one);
        free(res_one);
    }

    char *res_two = NULL;
    ResultCode code_two = sum_multiple_numbers(10, 2, &res_two, "99999999999999999999999999999999", "1");
    if (code_two == STATUS_SUCCESS) {
        printf("Decimal Huge Sum (999...999 + 1) = %s\n", res_two);
        free(res_two);
    }

    char *res_three = NULL;
    ResultCode code_three = sum_multiple_numbers(2, 3, &res_three, "1011", "1101", "1");
    if (code_three == STATUS_SUCCESS) {
        printf("Binary Sum (1011 + 1101 + 1) = %s\n", res_three);
        free(res_three);
    }

    return 0;
}