#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "str_ops.h"

typedef enum
{
    FLAG_LENGTH,
    FLAG_REVERSE,
    FLAG_UPPER_ODD,
    FLAG_DIGITS_LETTERS,
    FLAG_CONCAT
} flag_t;

static status_t parse_flag(const char *arg, flag_t *flag)
{
    if (arg == NULL || flag == NULL)
        return STATUS_NULL_POINTER;
    if (arg[0] != '-' || arg[1] == '\0' || arg[2] != '\0')
        return STATUS_INVALID_ARGUMENT;

    switch (arg[1])
    {
    case 'l': *flag = FLAG_LENGTH;         return STATUS_OK;
    case 'r': *flag = FLAG_REVERSE;        return STATUS_OK;
    case 'u': *flag = FLAG_UPPER_ODD;      return STATUS_OK;
    case 'n': *flag = FLAG_DIGITS_LETTERS; return STATUS_OK;
    case 'c': *flag = FLAG_CONCAT;         return STATUS_OK;
    default:  return STATUS_INVALID_ARGUMENT;
    }
}

/* unsigned int из строки: только десятичные цифры, без errno/strtol. */
static status_t parse_uint(const char *text, unsigned int *out)
{
    unsigned long long value = 0;
    size_t i = 0;

    if (text == NULL || out == NULL)
        return STATUS_NULL_POINTER;
    if (text[0] == '\0')
        return STATUS_INVALID_ARGUMENT;

    for (i = 0; text[i] != '\0'; ++i)
    {
        const unsigned int digit = (unsigned int)(text[i] - '0');
        if (text[i] < '0' || text[i] > '9')
            return STATUS_INVALID_ARGUMENT;
        value = value * 10 + digit;
        if (value > UINT_MAX)
            return STATUS_OVERFLOW;
    }

    *out = (unsigned int)value;
    return STATUS_OK;
}

/* Обработка -c: собирает список {argv[2], argv[4], argv[5], ...} и вызывает str_concat_random. */
static status_t run_concat(const int argc, const char *const argv[], char **result)
{
    unsigned int seed = 0;
    const char **list = NULL;
    size_t count = 0, i = 0;
    status_t status = parse_uint(argv[3], &seed);

    if (status != STATUS_OK)
        return status;

    count = (size_t)(argc - 3); /* argv[2] + argv[4..argc-1] */
    list = (const char **)malloc(count * sizeof *list);
    if (list == NULL)
        return STATUS_NO_MEMORY;

    list[0] = argv[2];
    for (i = 1; i < count; ++i)
        list[i] = argv[i + 3];

    status = str_concat_random(list, count, seed, result);
    free(list);
    return status;
}

/* Выполняет действие, не печатая ничего. Для -l кладёт длину в *length, для остальных - строку в *text. */
static status_t execute(const flag_t flag, const int argc, const char *const argv[],
                        size_t *length, char **text)
{
    switch (flag)
    {
    case FLAG_LENGTH:         return str_length(argv[2], length);
    case FLAG_REVERSE:        return str_reverse(argv[2], text);
    case FLAG_UPPER_ODD:      return str_upper_odd(argv[2], text);
    case FLAG_DIGITS_LETTERS: return str_digits_letters_others(argv[2], text);
    case FLAG_CONCAT:         return run_concat(argc, argv, text);
    default:                  return STATUS_INVALID_ARGUMENT;
    }
}

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s -l <string>\n"
            "  %s -r <string>\n"
            "  %s -u <string>\n"
            "  %s -n <string>\n"
            "  %s -c <string> <seed> [string ...]\n",
            program, program, program, program, program);
}

int main(const int argc, char *argv[])
{
    const char *program = (argc > 0 && argv[0] != NULL) ? argv[0] : "lab1";
    const char *const *args = (const char *const *)argv;
    flag_t flag = FLAG_LENGTH;
    size_t length = 0;
    char *text = NULL;
    status_t status = STATUS_OK;

    if (argc < 3)
        status = STATUS_INVALID_ARGUMENT;
    if (status == STATUS_OK)
        status = parse_flag(args[1], &flag);
    if (status == STATUS_OK && flag != FLAG_CONCAT && argc != 3)
        status = STATUS_INVALID_ARGUMENT;
    if (status == STATUS_OK && flag == FLAG_CONCAT && argc < 4)
        status = STATUS_INVALID_ARGUMENT;

    if (status != STATUS_OK)
    {
        fprintf(stderr, "Error: %s\n", status_to_string(status));
        print_usage(program);
        return 1;
    }

    status = execute(flag, argc, args, &length, &text);
    if (status != STATUS_OK)
    {
        fprintf(stderr, "Error: %s\n", status_to_string(status));
        return 1;
    }

    if (flag == FLAG_LENGTH)
        printf("%llu\n", (unsigned long long)length);
    else
        printf("%s\n", text);

    free(text);
    return 0;
}