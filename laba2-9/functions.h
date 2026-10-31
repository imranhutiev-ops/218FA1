#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

#define EPSILON_DEFAULT 1e-9

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_RADIX,
    STATUS_ERROR_DOMAIN,
    STATUS_ERROR_POINTER
} ExecutionStatus;

unsigned long long compute_greatest_common_divisor(unsigned long long first, unsigned long long second);
ExecutionStatus verify_fraction_termination(double fraction_value, int radix, int *const destination_flag, double epsilon);
ExecutionStatus analyze_base_terminations(int radix, size_t total_items, int *const destination_array, ...);

#endif