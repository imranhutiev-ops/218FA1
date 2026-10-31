#include "functions.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int symbol_to_value(char symb) {
    if (symb >= '0' && symb <= '9') {
        return symb - '0';
    }
    if (symb >= 'a' && symb <= 'z') {
        return symb - 'a' + 10;
    }
    if (symb >= 'A' && symb <= 'Z') {
        return symb - 'A' + 10;
    }
    return -1;
}

char value_to_symbol(int val) {
    if (val >= 0 && val <= 9) {
        return (char)('0' + val);
    }
    if (val >= 10 && val <= 35) {
        return (char)('A' + (val - 10));
    }
    return '?';
}

static char *allocate_string_copy(const char *src_str) {
    if (!src_str) {
        return NULL;
    }
    size_t length = strlen(src_str);
    char *duplicate = malloc(length + 1);
    if (!duplicate) {
        return NULL;
    }
    memcpy(duplicate, src_str, length + 1);
    return duplicate;
}

ResultCode column_addition(const char *restrict arg_a, const char *restrict arg_b, int radix, char **restrict destination) {
    if (!arg_a || !arg_b || !destination) {
        return STATUS_NULL_PTR;
    }
    if (radix < 2 || radix > 36) {
        return STATUS_BAD_BASE;
    }

    size_t len_a = strlen(arg_a);
    size_t len_b = strlen(arg_b);
    if (len_a == 0 || len_b == 0) {
        return STATUS_EMPTY_STR;
    }

    for (size_t i = 0; i < len_a; ++i) {
        int d = symbol_to_value(arg_a[i]);
        if (d < 0 || d >= radix) {
            return STATUS_BAD_DIGIT;
        }
    }
    for (size_t i = 0; i < len_b; ++i) {
        int d = symbol_to_value(arg_b[i]);
        if (d < 0 || d >= radix) {
            return STATUS_BAD_DIGIT;
        }
    }

    size_t buffer_size = (len_a > len_b ? len_a : len_b) + 2;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        return STATUS_ALLOC_FAIL;
    }

    long long idx_a = (long long)len_a - 1;
    long long idx_b = (long long)len_b - 1;
    int carry_value = 0;
    size_t write_pos = 0;

    while (idx_a >= 0 || idx_b >= 0 || carry_value > 0) {
        int val_a = (idx_a >= 0) ? symbol_to_value(arg_a[idx_a]) : 0;
        int val_b = (idx_b >= 0) ? symbol_to_value(arg_b[idx_b]) : 0;
        
        int summation = val_a + val_b + carry_value;
        carry_value = summation / radix;
        int remainder = summation % radix;
        
        buffer[write_pos++] = value_to_symbol(remainder);
        idx_a--;
        idx_b--;
    }

    // Реверс буфера
    for (size_t i = 0; i < write_pos / 2; ++i) {
        char temp_char = buffer[i];
        buffer[i] = buffer[write_pos - 1 - i];
        buffer[write_pos - 1 - i] = temp_char;
    }
    buffer[write_pos] = '\0';

    // Удаление ведущих нулей
    size_t leading_zeros = 0;
    while (leading_zeros + 1 < write_pos && buffer[leading_zeros] == '0') {
        leading_zeros++;
    }

    size_t final_length = write_pos - leading_zeros;
    char *result_str = malloc(final_length + 1);
    if (!result_str) {
        free(buffer);
        return STATUS_ALLOC_FAIL;
    }

    memcpy(result_str, buffer + leading_zeros, final_length);
    result_str[final_length] = '\0';
    free(buffer);

    *destination = result_str;
    return STATUS_SUCCESS;
}

ResultCode sum_multiple_numbers(int radix, size_t total_count, char **restrict destination, ...) {
    if (!destination) {
        return STATUS_NULL_PTR;
    }
    if (radix < 2 || radix > 36) {
        return STATUS_BAD_BASE;
    }
    if (total_count == 0) {
        char *zero_str = allocate_string_copy("0");
        if (!zero_str) {
            return STATUS_ALLOC_FAIL;
        }
        *destination = zero_str;
        return STATUS_SUCCESS;
    }

    va_list arguments_list;
    va_start(arguments_list, destination);

    char *accumulator = allocate_string_copy("0");
    if (!accumulator) {
        va_end(arguments_list);
        return STATUS_ALLOC_FAIL;
    }

    for (size_t i = 0; i < total_count; ++i) {
        const char *next_operand = va_arg(arguments_list, const char *);
        char *updated_accumulator = NULL;
        
        ResultCode status = column_addition(accumulator, next_operand, radix, &updated_accumulator);
        free(accumulator);
        
        if (status != STATUS_SUCCESS) {
            va_end(arguments_list);
            return status;
        }
        accumulator = updated_accumulator;
    }

    va_end(arguments_list);
    *destination = accumulator;
    return STATUS_SUCCESS;
}