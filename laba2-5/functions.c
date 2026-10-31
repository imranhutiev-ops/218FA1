#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    FILE *dest_file;
    char *dest_str;
    size_t written_chars;
} RenderContext;

static void emit_char(RenderContext *ctx, char ch) {
    if (ctx->dest_file) {
        fputc(ch, ctx->dest_file);
    } else if (ctx->dest_str) {
        ctx->dest_str[ctx->written_chars] = ch;
    }
    ctx->written_chars++;
}

static void emit_str(RenderContext *ctx, const char *s) {
    if (!s) return;
    while (*s) {
        emit_char(ctx, *s);
        s++;
    }
}

static void render_roman(RenderContext *ctx, int val) {
    if (val <= 0) {
        emit_str(ctx, "0");
        return;
    }
    static const struct {
        int weight;
        const char *glyph;
    } mapping[] = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"},
        {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
    };
    for (size_t i = 0; i < sizeof(mapping) / sizeof(mapping[0]); i++) {
        while (val >= mapping[i].weight) {
            emit_str(ctx, mapping[i].glyph);
            val -= mapping[i].weight;
        }
    }
}

static void render_zeckendorf(RenderContext *ctx, unsigned int val) {
    if (val == 0) {
        emit_str(ctx, "01");
        return;
    }
    unsigned int fib[64];
    fib[0] = 1;
    fib[1] = 2;
    size_t count = 2;
    while (1) {
        unsigned int next_val = fib[count - 1] + fib[count - 2];
        if (next_val > val || next_val < fib[count - 1]) {
            break;
        }
        fib[count++] = next_val;
    }

    char *bits = calloc(count, sizeof(char));
    if (!bits) return;

    unsigned int remainder = val;
    for (size_t i = count; i > 0; i--) {
        size_t idx = i - 1;
        if (remainder >= fib[idx]) {
            bits[idx] = 1;
            remainder -= fib[idx];
        }
    }

    for (size_t i = 0; i < count; i++) {
        emit_char(ctx, bits[i] ? '1' : '0');
    }
    emit_char(ctx, '1');
    free(bits);
}

static void render_base(RenderContext *ctx, int number, int base_val, int upper) {
    if (base_val < 2 || base_val > 36) {
        base_val = 10;
    }

    unsigned int magnitude = 0;
    if (number < 0) {
        emit_char(ctx, '-');
        magnitude = (unsigned int)(-(long long)number);
    } else {
        magnitude = (unsigned int)number;
    }

    if (magnitude == 0) {
        emit_char(ctx, '0');
        return;
    }

    const char *set = upper ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            : "0123456789abcdefghijklmnopqrstuvwxyz";

    char local_buf[64];
    size_t idx = 0;
    while (magnitude > 0) {
        local_buf[idx++] = set[magnitude % (unsigned int)base_val];
        magnitude /= (unsigned int)base_val;
    }

    for (size_t i = idx; i > 0; i--) {
        emit_char(ctx, local_buf[i - 1]);
    }
}

static void parse_and_render_base10(RenderContext *ctx, const char *s, int base_val, int upper) {
    if (base_val < 2 || base_val > 36) {
        base_val = 10;
    }
    if (!s) return;

    long long sign_factor = 1;
    if (*s == '-') {
        sign_factor = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    long long acc = 0;
    while (*s) {
        char ch = *s++;
        int digit_val = -1;
        if (ch >= '0' && ch <= '9') {
            digit_val = ch - '0';
        } else if (upper && (ch >= 'A' && ch <= 'Z')) {
            digit_val = ch - 'A' + 10;
        } else if (!upper && (ch >= 'a' && ch <= 'z')) {
            digit_val = ch - 'a' + 10;
        } else {
            return;
        }

        if (digit_val >= base_val) {
            return;
        }
        acc = acc * base_val + digit_val;
    }

    acc *= sign_factor;
    char num_str[64];
    snprintf(num_str, sizeof(num_str), "%lld", acc);
    emit_str(ctx, num_str);
}

static void render_memory(RenderContext *ctx, const void *ptr, size_t length) {
    if (!ptr) return;
    const unsigned char *bytes = ptr;
    for (size_t i = 0; i < length; i++) {
        for (int b = 7; b >= 0; b--) {
            emit_char(ctx, ((bytes[i] >> b) & 1) ? '1' : '0');
        }
        if (i + 1 < length) {
            emit_char(ctx, ' ');
        }
    }
}

static int execute_formatting(RenderContext *ctx, const char *fmt, va_list ap) {
    const char *p = fmt;
    while (*p) {
        if (*p != '%') {
            emit_char(ctx, *p++);
            continue;
        }

        p++;
        if (*p == '%') {
            emit_char(ctx, '%');
            p++;
            continue;
        }

        if (*p == 'R' && *(p + 1) == 'o') {
            int val = va_arg(ap, int);
            render_roman(ctx, val);
            p += 2;
        } else if (*p == 'Z' && *(p + 1) == 'r') {
            unsigned int val = va_arg(ap, unsigned int);
            render_zeckendorf(ctx, val);
            p += 2;
        } else if (*p == 'C' && (*(p + 1) == 'v' || *(p + 1) == 'V')) {
            int upper = (*(p + 1) == 'V');
            int num = va_arg(ap, int);
            int base = va_arg(ap, int);
            render_base(ctx, num, base, upper);
            p += 2;
        }   else if (*p == 't' && *(p + 1) == 'o') {
            const char *s = va_arg(ap, const char *);
            int base = va_arg(ap, int);
            parse_and_render_base10(ctx, s, base, 0);
            p += 2;
        } else if (*p == 'T' && *(p + 1) == 'O') {
            const char *s = va_arg(ap, const char *);
            int base = va_arg(ap, int);
            parse_and_render_base10(ctx, s, base, 1);
            p += 2;
        } else if (*p == 'm' && *(p + 1) == 'i') {
            int val = va_arg(ap, int);
            render_memory(ctx, &val, sizeof(int));
            p += 2;
        } else if (*p == 'm' && *(p + 1) == 'u') {
            unsigned int val = va_arg(ap, unsigned int);
            render_memory(ctx, &val, sizeof(unsigned int));
            p += 2;
        } else if (*p == 'm' && *(p + 1) == 'd') {
            double val = va_arg(ap, double);
            render_memory(ctx, &val, sizeof(double));
            p += 2;
        } else if (*p == 'm' && *(p + 1) == 'f') {
            float val = (float)va_arg(ap, double);
            render_memory(ctx, &val, sizeof(float));
            p += 2;
        } else {
            // fallback for other %md vs %mf check
            if (*p == 'm' && *(p + 1) == 'd') {
                double val = va_arg(ap, double);
                render_memory(ctx, &val, sizeof(double));
                p += 2;
                continue;
            }
            if (*p == 'm' && *(p + 1) == 'f') {
                float val = (float)va_arg(ap, double);
                render_memory(ctx, &val, sizeof(float));
                p += 2;
                continue;
            }

            char spec_buf[64];
            size_t s_idx = 0;
            spec_buf[s_idx++] = '%';
            while (*p && !strchr("diuoxXfFeEgGaAcspn", *p)) {
                if (s_idx < 62) {
                    spec_buf[s_idx++] = *p;
                }
                p++;
            }
            if (*p) {
                spec_buf[s_idx++] = *p++;
            }
            spec_buf[s_idx] = '\0';

            char tmp[1024];
            int len = 0;
            char last_char = spec_buf[s_idx - 1];

            if (strchr("di", last_char)) {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, int));
            } else if (strchr("uoxX", last_char)) {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, unsigned int));
            } else if (strchr("fFeEgGaA", last_char)) {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, double));
            } else if (last_char == 'c') {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, int));
            } else if (last_char == 's') {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, char *));
            } else if (last_char == 'p') {
                len = snprintf(tmp, sizeof(tmp), spec_buf, va_arg(ap, void *));
            }

            if (len > 0) {
                emit_str(ctx, tmp);
            }
        }
    }

    if (ctx->dest_str) {
        ctx->dest_str[ctx->written_chars] = '\0';
    }
    return (int)ctx->written_chars;
}

int overfprintf(FILE *stream, const char *format, ...) {
    if (!stream || !format) return -1;
    va_list ap;
    va_start(ap, format);
    RenderContext ctx = { .dest_file = stream, .dest_str = NULL, .written_chars = 0 };
    int res = execute_formatting(&ctx, format, ap);
    va_end(ap);
    return res;
}

int oversprintf(char *str, const char *format, ...) {
    if (!str || !format) return -1;
    va_list ap;
    va_start(ap, format);
    RenderContext ctx = { .dest_file = NULL, .dest_str = str, .written_chars = 0 };
    int res = execute_formatting(&ctx, format, ap);
    va_end(ap);
    return res;
}