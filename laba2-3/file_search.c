#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "file_search.h"

static size_t text_length(const char *text)
{
    size_t n = 0;

    while (text[n] != '\0')
        ++n;
    return n;
}

static void build_lps(const char *sub, const size_t sub_len, size_t *lps)
{
    size_t len = 0; 
    size_t i = 1;

    lps[0] = 0;
    while (i < sub_len)
    {
        if (sub[i] == sub[len])
        {
            ++len;
            lps[i] = len;
            ++i;
        }
        else if (len > 0)
        {
            len = lps[len - 1];
        }
        else
        {
            lps[i] = 0;
            ++i;
        }
    }
}


static void scan_file(FILE *file, const char *sub, const size_t sub_len,
                      const size_t *lps, size_t *line_hist, size_t *col_hist,
                      occurrence_t *items, const size_t capacity, size_t *count)
{
    size_t matched = 0, index = 0, found = 0;
    size_t line = 1, column = 1;
    int c = 0;

    while ((c = fgetc(file)) != EOF)
    {
        const char ch = (char)c;

        line_hist[index % sub_len] = line;
        col_hist[index % sub_len] = column;

        while (matched > 0 && ch != sub[matched])
            matched = lps[matched - 1];
        if (ch == sub[matched])
            ++matched;

        if (matched == sub_len)
        {
            if (items != NULL && found < capacity)
            {
                const size_t slot = (index + 1) % sub_len;
                items[found].line = line_hist[slot];
                items[found].column = col_hist[slot];
            }
            ++found;
            matched = lps[matched - 1]; 
        }

        if (ch == '\n')
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
        ++index;
    }
    *count = found;
}

static status_t search_file(const char *path, const char *sub, const size_t sub_len,
                            const size_t *lps, size_t *line_hist, size_t *col_hist,
                            file_result_t *result)
{
    FILE *file = fopen(path, "r");
    occurrence_t *items = NULL;
    size_t count = 0, second_count = 0;
    status_t status = STATUS_OK;

    if (file == NULL)
        return STATUS_FILE_OPEN;

    scan_file(file, sub, sub_len, lps, line_hist, col_hist, NULL, 0, &count);
    if (ferror(file))
        status = STATUS_FILE_READ;

    if (status == STATUS_OK && count > 0)
    {
        if (count > (size_t)-1 / sizeof *items)
            status = STATUS_OVERFLOW;
        else
        {
            items = (occurrence_t *)malloc(count * sizeof *items);
            if (items == NULL)
                status = STATUS_NO_MEMORY;
        }
    }

    if (status == STATUS_OK && count > 0)
    {
        if (fseek(file, 0, SEEK_SET) != 0)
            status = STATUS_FILE_READ;
        else
        {
            scan_file(file, sub, sub_len, lps, line_hist, col_hist,
                      items, count, &second_count);
            if (ferror(file) || second_count != count)
                status = STATUS_FILE_READ; 
        }
    }

    fclose(file);

    if (status == STATUS_OK)
    {
        result->count = count;
        result->items = items;
    }
    else
    {
        free(items);
    }
    return status;
}

status_t search_in_files(file_result_t **results, const char *substring,
                         const size_t file_count, ...)
{
    va_list args;
    file_result_t *out = NULL;
    size_t *lps = NULL, *line_hist = NULL, *col_hist = NULL;
    size_t sub_len = 0, i = 0;

    if (results == NULL || substring == NULL)
        return STATUS_NULL_POINTER;

    sub_len = text_length(substring);
    if (sub_len == 0 || file_count == 0)
        return STATUS_INVALID_ARGUMENT;
    if (file_count > (size_t)-1 / sizeof *out || sub_len > (size_t)-1 / sizeof *lps)
        return STATUS_OVERFLOW;

    out = (file_result_t *)calloc(file_count, sizeof *out);
    lps = (size_t *)malloc(sub_len * sizeof *lps);
    line_hist = (size_t *)malloc(sub_len * sizeof *line_hist);
    col_hist = (size_t *)malloc(sub_len * sizeof *col_hist);
    if (out == NULL || lps == NULL || line_hist == NULL || col_hist == NULL)
    {
        free(out);
        free(lps);
        free(line_hist);
        free(col_hist);
        return STATUS_NO_MEMORY;
    }

    build_lps(substring, sub_len, lps);

    va_start(args, file_count);
    for (i = 0; i < file_count; ++i)
    {
        const char *path = va_arg(args, const char *);

        out[i].path = path;
        if (path == NULL)
            out[i].status = STATUS_NULL_POINTER;
        else
            out[i].status = search_file(path, substring, sub_len, lps,
                                        line_hist, col_hist, &out[i]);
    }
    va_end(args);

    free(lps);
    free(line_hist);
    free(col_hist);
    *results = out;
    return STATUS_OK;
}

void free_results(file_result_t *results, const size_t file_count)
{
    size_t i = 0;

    if (results == NULL)
        return;
    for (i = 0; i < file_count; ++i)
        free(results[i].items);
    free(results);
}