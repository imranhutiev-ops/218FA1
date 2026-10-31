#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    FILE *stream_ptr;
    const char *string_ptr;
    size_t offset;
} StreamContext;

static int get_next_character(StreamContext *ctx) {
    if (ctx->stream_ptr) {
        return fgetc(ctx->stream_ptr);
    }
    if (ctx->string_ptr) {
        if (ctx->string_ptr[ctx->offset] != '\0') {
            return (unsigned char)ctx->string_ptr[ctx->offset++];
        }
    }
    return EOF;
}

static void unget_character(StreamContext *ctx, int c) {
    if (c == EOF) {
        return;
    }
    if (ctx->stream_ptr) {
        ungetc(c, ctx->stream_ptr);
    } else if (ctx->string_ptr) {
        if (ctx->offset > 0) {
            ctx->offset--;
        }
    }
}

static void skip_spaces(StreamContext *ctx) {
    int ch;
    while ((ch = get_next_character(ctx)) != EOF) {
        if (!isspace((unsigned char)ch)) {
            unget_character(ctx, ch);
            break;
        }
    }
}

static int get_roman_val(char symbol) {
    switch (symbol) {
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default:  return 0;
    }
}

static int parse_roman(StreamContext *ctx, int *result) {
    skip_spaces(ctx);
    int numerals[128];
    size_t len = 0;
    int ch;

    while ((ch = get_next_character(ctx)) != EOF) {
        int val = get_roman_val((char)ch);
        if (val == 0) {
            unget_character(ctx, ch);
            break;
        }
        if (len < sizeof(numerals) / sizeof(numerals[0])) {
            numerals[len++] = val;
        }
    }

    if (len == 0) {
        return 0;
    }

    int accumulated = 0;
    for (size_t i = 0; i < len; ++i) {
        if (i + 1 < len && numerals[i] < numerals[i + 1]) {
            accumulated -= numerals[i];
        } else {
            accumulated += numerals[i];
        }
    }

    *result = accumulated;
    return 1;
}

static int parse_zeckendorf(StreamContext *ctx, unsigned int *result) {
    skip_spaces(ctx);

    unsigned int fib_cache[64];
    fib_cache[0] = 1;
    fib_cache[1] = 2;
    for (size_t i = 2; i < 64; ++i) {
        fib_cache[i] = fib_cache[i - 1] + fib_cache[i - 2];
    }

    char symbols[128];
    size_t count = 0;
    int ch;

    while ((ch = get_next_character(ctx)) != EOF) {
        if (ch == '0' || ch == '1') {
            if (count < sizeof(symbols)) {
                symbols[count++] = (char)ch;
            }
        } else {
            unget_character(ctx, ch);
            break;
        }
    }

    if (count < 2) {
        return 0;
    }

    if (symbols[count - 1] == '1') {
        count--;
    }

    unsigned int total_sum = 0;
    for (size_t i = 0; i < count; ++i) {
        if (symbols[i] == '1') {
            total_sum += fib_cache[i];
        }
    }

    *result = total_sum;
    return 1;
}

static int parse_base_number(StreamContext *ctx, int radix, int uppercase_flag, int *result) {
    if (radix < 2 || radix > 36) {
        radix = 10;
    }

    skip_spaces(ctx);
    int ch = get_next_character(ctx);
    if (ch == EOF) {
        return 0;
    }

    int sign_multiplier = 1;
    if (ch == '-') {
        sign_multiplier = -1;
    } else if (ch == '+') {
        sign_multiplier = 1;
    } else {
        unget_character(ctx, ch);
    }

    long long value_accumulator = 0;
    int parsed_digits = 0;

    while ((ch = get_next_character(ctx)) != EOF) {
        int digit_val = -1;
        if (ch >= '0' && ch <= '9') {
            digit_val = ch - '0';
        } else if (uppercase_flag && (ch >= 'A' && ch <= 'Z')) {
            digit_val = ch - 'A' + 10;
        } else if (!uppercase_flag && (ch >= 'a' && ch <= 'z')) {
            digit_val = ch - 'a' + 10;
        } else {
            unget_character(ctx, ch);
            break;
        }

        if (digit_val >= radix) {
            unget_character(ctx, ch);
            break;
        }

        value_accumulator = value_accumulator * radix + digit_val;
        parsed_digits++;
    }

    if (parsed_digits == 0) {
        return 0;
    }

    *result = (int)(value_accumulator * sign_multiplier);
    return 1;
}

static int execute_vscan(StreamContext *ctx, const char *format_str, va_list ap) {
    const char *ptr = format_str;
    int successful_matches = 0;

    while (*ptr != '\0') {
        if (isspace((unsigned char)*ptr)) {
            skip_spaces(ctx);
            ptr++;
            continue;
        }

        if (*ptr != '%') {
            int ch = get_next_character(ctx);
            if (ch != *ptr) {
                unget_character(ctx, ch);
                return successful_matches;
            }
            ptr++;
            continue;
        }

        ptr++;
        if (*ptr == '%') {
            int ch = get_next_character(ctx);
            if (ch != '%') {
                unget_character(ctx, ch);
                return successful_matches;
            }
            ptr++;
            continue;
        }

        if (*ptr == 'R' && *(ptr + 1) == 'o') {
            int *dest = va_arg(ap, int *);
            if (parse_roman(ctx, dest)) {
                successful_matches++;
            } else {
                return successful_matches;
            }
            ptr += 2;
        } else if (*ptr == 'Z' && *(ptr + 1) == 'r') {
            unsigned int *dest = va_arg(ap, unsigned int *);
            if (parse_zeckendorf(ctx, dest)) {
                successful_matches++;
            } else {
                return successful_matches;
            }
            ptr += 2;
        } else if (*ptr == 'C' && (*(ptr + 1) == 'v' || *(ptr + 1) == 'V')) {
            int is_upper = (*(ptr + 1) == 'V');
            int *dest = va_arg(ap, int *);
            int radix = va_arg(ap, int);
            if (parse_base_number(ctx, radix, is_upper, dest)) {
                successful_matches++;
            } else {
                return successful_matches;
            }
            ptr += 2;
        } else {
            char specifier_buf[64];
            size_t s_idx = 0;
            specifier_buf[s_idx++] = '%';
            while (*ptr && !strchr("diuoxXfFeEgGaAcspn", *ptr)) {
                if (s_idx < 62) {
                    specifier_buf[s_idx++] = *ptr;
                }
                ptr++;
            }
            if (*ptr) {
                specifier_buf[s_idx++] = *ptr++;
            }
            specifier_buf[s_idx] = '\0';

            skip_spaces(ctx);

            char token_buf[512];
            size_t t_idx = 0;
            int ch;
            while ((ch = get_next_character(ctx)) != EOF) {
                if (isspace(ch)) {
                    unget_character(ctx, ch);
                    break;
                }
                if (t_idx < sizeof(token_buf) - 1) {
                    token_buf[t_idx++] = (char)ch;
                }
            }
            token_buf[t_idx] = '\0';

            if (t_idx == 0) {
                return successful_matches;
            }

            void *target_ptr = va_arg(ap, void *);
            if (sscanf(token_buf, specifier_buf, target_ptr) > 0) {
                successful_matches++;
            } else {
                return successful_matches;
            }
        }
    }

    return successful_matches;
}

int overfscanf(FILE *restrict stream, const char *restrict format, ...) {
    if (!stream || !format) {
        return -1;
    }
    va_list ap;
    va_start(ap, format);
    StreamContext ctx = { .stream_ptr = stream, .string_ptr = NULL, .offset = 0 };
    int res = execute_vscan(&ctx, format, ap);
    va_end(ap);
    return res;
}

int oversscanf(const char *restrict str, const char *restrict format, ...) {
    if (!str || !format) {
        return -1;
    }
    va_list ap;
    va_start(ap, format);
    StreamContext ctx = { .stream_ptr = NULL, .string_ptr = str, .offset = 0 };
    int res = execute_vscan(&ctx, format, ap);
    va_end(ap);
    return res;
}