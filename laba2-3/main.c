#include <stdio.h>
#include "file_search.h"
#include "status.h"

static void print_substring(const char *sub)
{
    size_t i = 0, length = 0;

    printf("\"");
    for (i = 0; sub[i] != '\0'; ++i)
    {
        if (sub[i] == '\n')
            printf("\\n");
        else if (sub[i] == '\t')
            printf("\\t");
        else
            printf("%c", sub[i]);
        ++length;
    }
    printf("\" (length %llu)\n", (unsigned long long)length);
}

static void report(const status_t status, const char *sub,
                   file_result_t *results, const size_t file_count)
{
    size_t i = 0, k = 0;

    printf("=== Search for ");
    print_substring(sub);

    if (status != STATUS_OK)
    {
        printf("Error: %s\n\n", status_to_string(status));
        return;
    }

    for (i = 0; i < file_count; ++i)
    {
        printf("File: %s\n", results[i].path != NULL ? results[i].path : "(null)");
        if (results[i].status != STATUS_OK)
            printf("  error: %s\n", status_to_string(results[i].status));
        else if (results[i].count == 0)
            printf("  no occurrences\n");
        else
        {
            printf("  occurrences: %llu\n", (unsigned long long)results[i].count);
            for (k = 0; k < results[i].count; ++k)
                printf("  line %llu, column %llu\n",
                       (unsigned long long)results[i].items[k].line,
                       (unsigned long long)results[i].items[k].column);
        }
    }
    printf("\n");
    free_results(results, file_count);
}

int main(void)
{
    file_result_t *results = NULL;
    status_t status = STATUS_OK;

    status = search_in_files(&results, "ab", 3, "sample1.txt", "sample2.txt", "missing.txt");
    report(status, "ab", results, 3);

    status = search_in_files(&results, "ab\n\tt", 1, "sample1.txt");
    report(status, "ab\n\tt", results, 1);

    status = search_in_files(&results, "  ", 2, "sample1.txt", "sample2.txt");
    report(status, "  ", results, 2);

    status = search_in_files(&results, "aa", 1, "sample2.txt");
    report(status, "aa", results, 1);

    status = search_in_files(&results, "", 1, "sample1.txt");
    report(status, "", results, 1);

    return 0;
}