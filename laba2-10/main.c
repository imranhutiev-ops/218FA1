#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

static void process_error_message(ExecutionStatus error_code) {
    switch (error_code) {
        case STATUS_ERROR_DEGREE:
            fprintf(stderr, "Validation Error: Polynomial degree must be greater than or equal to 0.\n");
            break;
        case STATUS_ERROR_EPSILON:
            fprintf(stderr, "Validation Error: Invalid epsilon accuracy or shift parameter.\n");
            break;
        case STATUS_ERROR_ALLOCATION:
            fprintf(stderr, "System Error: Memory allocation failed.\n");
            break;
        case STATUS_ERROR_POINTER_NULL:
            fprintf(stderr, "System Error: Null pointer reference received.\n");
            break;
        default:
            fprintf(stderr, "An unexpected error occurred.\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 5) {
        double eps = atof(argv[1]);
        double a = atof(argv[2]);
        int degree = atoi(argv[3]);

        if (argc != 4 + degree + 1) {
            fprintf(stderr, "CLI Error: Expected %d coefficients, but provided arguments count mismatch.\n", degree + 1);
            return 1;
        }

        double *result_coeffs = NULL;
        ExecutionStatus st = STATUS_SUCCESS;

        /* Поддержка переменного числа аргументов через селективный вызов по степени */
        switch (degree) {
            case 0:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, 0, atof(argv[4]));
                break;
            case 1:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, 1, atof(argv[4]), atof(argv[5]));
                break;
            case 2:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, 2, atof(argv[4]), atof(argv[5]), atof(argv[6]));
                break;
            case 3:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, 3, atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]));
                break;
            case 4:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, 4, atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]));
                break;
            default:
                st = transform_polynomial_expansion(eps, a, &result_coeffs, degree,
                                                    atof(argv[4]), atof(argv[5]), atof(argv[6]), atof(argv[7]), atof(argv[8]));
                break;
        }

        if (st != STATUS_SUCCESS) {
            process_error_message(st);
            return 1;
        }

        printf("Re-expanded coefficients g_0 ... g_%d:\n", degree);
        for (int i = 0; i <= degree; i++) {
            printf("g_%d = %.4f\n", i, result_coeffs[i]);
        }
        free(result_coeffs);
        return 0;
    }

    /* Встроенная демонстрация */
    printf("Original polynomial: f(x) = 3 + 2x + x^2\n");
    printf("Re-expanding around a = 1.0:\n");

    double *demo_g1 = NULL;
    ExecutionStatus demo_st1 = transform_polynomial_expansion(EPSILON_DEFAULT, 1.0, &demo_g1, 2, 3.0, 2.0, 1.0);
    if (demo_st1 == STATUS_SUCCESS) {
        for (int i = 0; i <= 2; i++) {
            printf("g_%d = %.4f\n", i, demo_g1[i]);
        }
        free(demo_g1);
    }

    printf("\nOriginal polynomial: f(x) = 1 - 2x + x^2 = (x - 1)^2\n");
    printf("Re-expanding around a = 1.0:\n");

    double *demo_g2 = NULL;
    ExecutionStatus demo_st2 = transform_polynomial_expansion(EPSILON_DEFAULT, 1.0, &demo_g2, 2, 1.0, -2.0, 1.0);
    if (demo_st2 == STATUS_SUCCESS) {
        for (int i = 0; i <= 2; i++) {
            printf("g_%d = %.4f\n", i, demo_g2[i]);
        }
        free(demo_g2);
    }

    return 0;
}