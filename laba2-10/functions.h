#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define EPSILON_DEFAULT 1e-9

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_DEGREE,
    STATUS_ERROR_EPSILON,
    STATUS_ERROR_ALLOCATION,
    STATUS_ERROR_POINTER_NULL
} ExecutionStatus;

ExecutionStatus transform_polynomial_expansion(double epsilon, double shift_value, double **const destination_array, int polynomial_degree, ...);

#endif