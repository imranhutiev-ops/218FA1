#ifndef BASE_SUMMATION_H
#define BASE_SUMMATION_H

#include <stddef.h>

typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_BAD_BASE,
    STATUS_BAD_DIGIT,
    STATUS_EMPTY_STR,
    STATUS_ALLOC_FAIL,
    STATUS_NULL_PTR
} ResultCode;

int symbol_to_value(char symb);
char value_to_symbol(int val);

ResultCode column_addition(const char *restrict arg_a, const char *restrict arg_b, int radix, char **restrict destination);
ResultCode sum_multiple_numbers(int radix, size_t total_count, char **restrict destination, ...);

#endif // BASE_SUMMATION_H