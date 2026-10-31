#include <ctype.h>
#include <stdlib.h>
#include "str_ops.h"

/* Выделяет буфер под строку длиной length (+ завершающий '\0'). */
static status_t alloc_string(const size_t length, char **buffer)
{
    if (length == (size_t)-1)
        return STATUS_OVERFLOW;
    *buffer = (char *)malloc(length + 1);
    return (*buffer == NULL) ? STATUS_NO_MEMORY : STATUS_OK;
}

status_t str_length(const char *src, size_t *length)
{
    size_t n = 0;

    if (src == NULL || length == NULL)
        return STATUS_NULL_POINTER;

    while (src[n] != '\0')
        ++n;
    *length = n;
    return STATUS_OK;
}

status_t str_reverse(const char *src, char **result)
{
    size_t n = 0, i = 0;
    char *out = NULL;
    status_t status = STATUS_OK;

    if (src == NULL || result == NULL)
        return STATUS_NULL_POINTER;

    status = str_length(src, &n);
    if (status == STATUS_OK)
        status = alloc_string(n, &out);
    if (status != STATUS_OK)
        return status;

    for (i = 0; i < n; ++i)
        out[i] = src[n - 1 - i];
    out[n] = '\0';

    *result = out;
    return STATUS_OK;
}

status_t str_upper_odd(const char *src, char **result)
{
    size_t n = 0, i = 0;
    char *out = NULL;
    status_t status = STATUS_OK;

    if (src == NULL || result == NULL)
        return STATUS_NULL_POINTER;

    status = str_length(src, &n);
    if (status == STATUS_OK)
        status = alloc_string(n, &out);
    if (status != STATUS_OK)
        return status;

    for (i = 0; i < n; ++i)
        out[i] = (i % 2 == 1) ? (char)toupper((unsigned char)src[i]) : src[i];
    out[n] = '\0';

    *result = out;
    return STATUS_OK;
}

status_t str_digits_letters_others(const char *src, char **result)
{
    size_t n = 0, i = 0, pos = 0;
    char *out = NULL;
    status_t status = STATUS_OK;

    if (src == NULL || result == NULL)
        return STATUS_NULL_POINTER;

    status = str_length(src, &n);
    if (status == STATUS_OK)
        status = alloc_string(n, &out);
    if (status != STATUS_OK)
        return status;

    /* Три прохода: цифры, затем буквы, затем всё остальное. */
    for (i = 0; i < n; ++i)
        if (isdigit((unsigned char)src[i]))
            out[pos++] = src[i];
    for (i = 0; i < n; ++i)
        if (isalpha((unsigned char)src[i]))
            out[pos++] = src[i];
    for (i = 0; i < n; ++i)
        if (!isdigit((unsigned char)src[i]) && !isalpha((unsigned char)src[i]))
            out[pos++] = src[i];
    out[pos] = '\0';

    *result = out;
    return STATUS_OK;
}

/* Перемешивание индексов (Фишер - Йетс). Вызывает rand(), состояние задаёт srand в вызывающей функции. */
static void shuffle_indices(size_t *order, const size_t count)
{
    size_t i = 0;

    for (i = count; i > 1; --i)
    {
        const size_t j = (size_t)rand() % i;
        const size_t tmp = order[i - 1];
        order[i - 1] = order[j];
        order[j] = tmp;
    }
}

status_t str_concat_random(const char *const *strings, const size_t count,
                           const unsigned int seed, char **result)
{
    size_t *order = NULL;
    size_t *lengths = NULL;
    size_t total = 0, i = 0, pos = 0, k = 0;
    char *out = NULL;
    status_t status = STATUS_OK;

    if (strings == NULL || result == NULL)
        return STATUS_NULL_POINTER;
    if (count == 0 || count > (size_t)-1 / sizeof *order)
        return STATUS_INVALID_ARGUMENT;

    for (i = 0; i < count; ++i)
        if (strings[i] == NULL)
            return STATUS_NULL_POINTER;

    order = (size_t *)malloc(count * sizeof *order);
    lengths = (size_t *)malloc(count * sizeof *lengths);
    if (order == NULL || lengths == NULL)
        status = STATUS_NO_MEMORY;

    for (i = 0; i < count && status == STATUS_OK; ++i)
    {
        status = str_length(strings[i], &lengths[i]);
        if (status == STATUS_OK && lengths[i] > (size_t)-1 - 1 - total)
            status = STATUS_OVERFLOW;
        if (status == STATUS_OK)
        {
            total += lengths[i];
            order[i] = i;
        }
    }

    if (status == STATUS_OK)
        status = alloc_string(total, &out);

    if (status == STATUS_OK)
    {
        srand(seed);
        shuffle_indices(order, count);

        for (i = 0; i < count; ++i)
        {
            const size_t idx = order[i];
            for (k = 0; k < lengths[idx]; ++k)
                out[pos++] = strings[idx][k];
        }
        out[pos] = '\0';
        *result = out;
    }

    free(order);
    free(lengths);
    return status;
}