#include "functions.h"
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

ExecutionStatus transform_polynomial_expansion(double epsilon, double shift_value, double **const destination_array, int polynomial_degree, ...) {
    if (destination_array == NULL) {
        return STATUS_ERROR_POINTER_NULL;
    }
    if (polynomial_degree < 0) {
        return STATUS_ERROR_DEGREE;
    }
    if (epsilon <= 0.0 || isnan(epsilon) || isinf(epsilon) || isnan(shift_value) || isinf(shift_value)) {
        return STATUS_ERROR_EPSILON;
    }

    const size_t total_elements = (size_t)polynomial_degree + 1U;
    double *coefficients_buffer = (double *)malloc(total_elements * sizeof(double));
    if (coefficients_buffer == NULL) {
        return STATUS_ERROR_ALLOCATION;
    }

    va_list argument_pointer;
    va_start(argument_pointer, polynomial_degree);
    for (size_t index = 0U; index < total_elements; index++) {
        coefficients_buffer[index] = va_arg(argument_pointer, double);
    }
    va_end(argument_pointer);

    /* Метод Горнера для переразложения многочлена по степеням (x - a) */
    for (int iteration_step = 0; iteration_step < polynomial_degree; iteration_step++) {
        for (int current_idx = polynomial_degree - 1; current_idx >= iteration_step; current_idx--) {
            coefficients_buffer[current_idx] += coefficients_buffer[current_idx + 1] * shift_value;
        }
    }

    /* Обнуление коэффициентов, меньших заданного эпсилон */
    for (size_t index = 0U; index < total_elements; index++) {
        if (fabs(coefficients_buffer[index]) < epsilon) {
            coefficients_buffer[index] = 0.0;
        }
    }

    *destination_array = coefficients_buffer;
    return STATUS_SUCCESS;
}