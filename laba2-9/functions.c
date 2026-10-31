#include "functions.h"
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

unsigned long long compute_greatest_common_divisor(unsigned long long first, unsigned long long second) {
    unsigned long long remainder;
    while (second != 0ULL) {
        remainder = first % second;
        first = second;
        second = remainder;
    }
    return first;
}

ExecutionStatus verify_fraction_termination(double fraction_value, int radix, int *const destination_flag, double epsilon) {
    if (destination_flag == NULL) {
        return STATUS_ERROR_POINTER;
    }
    if (radix < 2) {
        return STATUS_ERROR_RADIX;
    }
    if (fraction_value <= 0.0 + epsilon || fraction_value >= 1.0 - epsilon || isnan(fraction_value) || isinf(fraction_value)) {
        return STATUS_ERROR_DOMAIN;
    }

    const unsigned long long scaling_factor = 1000000000ULL;
    unsigned long long numerator = (unsigned long long)llround(fraction_value * (double)scaling_factor);
    unsigned long long denominator = scaling_factor;

    unsigned long long common_divisor = compute_greatest_common_divisor(numerator, denominator);
    denominator /= common_divisor;

    unsigned long long base_val = (unsigned long long)radix;
    unsigned long long factor_gcd = compute_greatest_common_divisor(denominator, base_val);

    while (factor_gcd > 1ULL) {
        while (denominator % factor_gcd == 0ULL) {
            denominator /= factor_gcd;
        }
        factor_gcd = compute_greatest_common_divisor(denominator, base_val);
    }

    *destination_flag = (denominator == 1ULL) ? 1 : 0;
    return STATUS_SUCCESS;
}

ExecutionStatus analyze_base_terminations(int radix, size_t total_items, int *const destination_array, ...) {
    if (destination_array == NULL && total_items > 0U) {
        return STATUS_ERROR_POINTER;
    }
    if (radix < 2) {
        return STATUS_ERROR_RADIX;
    }

    va_list argument_list;
    va_start(argument_list, destination_array);

    for (size_t index = 0U; index < total_items; index++) {
        double current_val = va_arg(argument_list, double);
        int is_finite = 0;
        ExecutionStatus status = verify_fraction_termination(current_val, radix, &is_finite, EPSILON_DEFAULT);
        if (status != STATUS_SUCCESS) {
            va_end(argument_list);
            return status;
        }
        destination_array[index] = is_finite;
    }

    va_end(argument_list);
    return STATUS_SUCCESS;
}