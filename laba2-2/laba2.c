#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

#define EPSILON 1e-6

typedef enum {
    SUCCESS = 0,
    ERR_ARGS,
    ERR_FILE_OPEN,
    ERR_FILE_READ,
    ERR_FILE_WRITE,
    ERR_MEMORY,
    ERR_INVALID_DATA,
    ERR_INVALID_FLAG
} Status;

typedef enum { SORT_ASCENDING = 1, SORT_DESCENDING = 2 } SortDirection;

typedef struct {
    unsigned int id;
    char *name;
    char *surname;
    double salary;
} Employee;

typedef struct {
    SortDirection direction;
    double epsilon;
} SortContext;

static void free_employees(Employee *employees, size_t count) {
    if (employees == NULL) return;
    for (size_t i = 0; i < count; ++i) {
        free(employees[i].name);
        free(employees[i].surname);
    }
    free(employees);
}

static Status parse_flag(const char *arg, SortDirection *direction) {
    if (arg == NULL || direction == NULL) return ERR_INVALID_FLAG;
    if ((arg[0] != '-' && arg[0] != '/') || arg[1] == '\0' || arg[2] != '\0')
        return ERR_INVALID_FLAG;
    if (arg[1] == 'a') *direction = SORT_ASCENDING;
    else if (arg[1] == 'd') *direction = SORT_DESCENDING;
    else return ERR_INVALID_FLAG;
    return SUCCESS;
}

/* Every nonempty physical line is returned, without its newline. */
static Status read_line(FILE *file, char **out_line, int *out_eof) {
    if (file == NULL || out_line == NULL || out_eof == NULL) return ERR_INVALID_DATA;
    *out_line = NULL;
    *out_eof = 0;
    size_t capacity = 128, length = 0;
    char *line = malloc(capacity);
    if (line == NULL) return ERR_MEMORY;

    int ch;
    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (ch == '\0') {
            free(line);
            return ERR_INVALID_DATA;
        }
        if (length >= capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                free(line);
                return ERR_MEMORY;
            }
            size_t next_capacity = capacity * 2;
            char *tmp = realloc(line, next_capacity);
            if (tmp == NULL) {
                free(line);
                return ERR_MEMORY;
            }
            line = tmp;
            capacity = next_capacity;
        }
        line[length++] = (char)ch;
    }
    if (ferror(file)) {
        free(line);
        return ERR_FILE_READ;
    }
    if (ch == EOF && length == 0) {
        free(line);
        *out_eof = 1;
        return SUCCESS;
    }
    if (length > 0 && line[length - 1] == '\r') --length;
    line[length] = '\0';
    *out_line = line;
    return SUCCESS;
}

static Status next_token(char **cursor, char **token) {
    if (cursor == NULL || *cursor == NULL || token == NULL) return ERR_INVALID_DATA;
    char *p = *cursor;
    while (*p != '\0' && isspace((unsigned char)*p)) ++p;
    if (*p == '\0') {
        *token = NULL;
        *cursor = p;
        return SUCCESS;
    }
    *token = p;
    while (*p != '\0' && !isspace((unsigned char)*p)) ++p;
    if (*p != '\0') *p++ = '\0';
    *cursor = p;
    return SUCCESS;
}

static Status parse_id(const char *s, unsigned int *out_id) {
    if (s == NULL || out_id == NULL || *s == '\0') return ERR_INVALID_DATA;
    unsigned int number = 0;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (s[i] < '0' || s[i] > '9') return ERR_INVALID_DATA;
        unsigned int digit = (unsigned int)(s[i] - '0');
        if (number > (UINT_MAX - digit) / 10U) return ERR_INVALID_DATA;
        number = number * 10U + digit;
    }
    *out_id = number;
    return SUCCESS;
}

static Status validate_name(const char *s) {
    if (s == NULL || *s == '\0') return ERR_INVALID_DATA;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        if (!((s[i] >= 'A' && s[i] <= 'Z') ||
              (s[i] >= 'a' && s[i] <= 'z'))) return ERR_INVALID_DATA;
    }
    return SUCCESS;
}

/* Restrict strtod to ordinary decimal notation, optionally with exponent. */
static Status parse_salary(const char *s, double *out_salary) {
    if (s == NULL || out_salary == NULL || *s == '\0') return ERR_INVALID_DATA;
    const char *p = s;
    if (*p == '+') ++p;
    int digits = 0;
    while (*p >= '0' && *p <= '9') { ++p; ++digits; }
    if (*p == '.') {
        ++p;
        while (*p >= '0' && *p <= '9') { ++p; ++digits; }
    }
    if (digits == 0) return ERR_INVALID_DATA;
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') ++p;
        if (*p < '0' || *p > '9') return ERR_INVALID_DATA;
        while (*p >= '0' && *p <= '9') ++p;
    }
    if (*p != '\0') return ERR_INVALID_DATA;
    char *end = NULL;
    double value = strtod(s, &end);
    if (end == s || *end != '\0' || !isfinite(value) || value < 0.0)
        return ERR_INVALID_DATA;
    *out_salary = value;
    return SUCCESS;
}

static Status duplicate_string(const char *src, char **out) {
    if (src == NULL || out == NULL) return ERR_INVALID_DATA;
    size_t length = strlen(src);
    if (length == SIZE_MAX) return ERR_MEMORY;
    char *copy = malloc(length + 1);
    if (copy == NULL) return ERR_MEMORY;
    memcpy(copy, src, length + 1);
    *out = copy;
    return SUCCESS;
}

static Status parse_employee_line(char *line, Employee *out_employee, int *is_blank) {
    if (line == NULL || out_employee == NULL || is_blank == NULL) return ERR_INVALID_DATA;
    *is_blank = 0;
    char *cursor = line;
    char *parts[5] = {NULL, NULL, NULL, NULL, NULL};
    for (size_t i = 0; i < 5; ++i) {
        Status st = next_token(&cursor, &parts[i]);
        if (st != SUCCESS) return st;
    }
    if (parts[0] == NULL) {
        *is_blank = 1;
        return SUCCESS;
    }
    if (parts[1] == NULL || parts[2] == NULL || parts[3] == NULL || parts[4] != NULL)
        return ERR_INVALID_DATA;

    Employee temp = {0, NULL, NULL, 0.0};
    Status st = parse_id(parts[0], &temp.id);
    if (st != SUCCESS) return st;
    st = validate_name(parts[1]);
    if (st != SUCCESS) return st;
    st = validate_name(parts[2]);
    if (st != SUCCESS) return st;
    st = parse_salary(parts[3], &temp.salary);
    if (st != SUCCESS) return st;
    st = duplicate_string(parts[1], &temp.name);
    if (st != SUCCESS) return st;
    st = duplicate_string(parts[2], &temp.surname);
    if (st != SUCCESS) { free(temp.name); return st; }
    *out_employee = temp;
    return SUCCESS;
}

static Status read_employees_from_file(const char *filename,
                                       Employee **out_employees, size_t *out_count) {
    if (filename == NULL || filename[0] == '\0' ||
        out_employees == NULL || out_count == NULL) return ERR_INVALID_DATA;
    *out_employees = NULL;
    *out_count = 0;
    FILE *file = fopen(filename, "r");
    if (file == NULL) return ERR_FILE_OPEN;

    Employee *employees = NULL;
    size_t count = 0, capacity = 0;
    Status st = SUCCESS;
    int eof = 0;
    while (!eof && st == SUCCESS) {
        char *line = NULL;
        st = read_line(file, &line, &eof);
        if (st != SUCCESS || eof) { free(line); break; }
        Employee emp = {0, NULL, NULL, 0.0};
        int blank = 0;
        st = parse_employee_line(line, &emp, &blank);
        free(line);
        if (st != SUCCESS) break;
        if (blank) continue;
        if (count == capacity) {
            size_t next_capacity = capacity == 0 ? 8 : capacity * 2;
            if (capacity > SIZE_MAX / 2 ||
                next_capacity > SIZE_MAX / sizeof(Employee)) {
                free(emp.name);
                free(emp.surname);
                st = ERR_MEMORY;
                break;
            }
            Employee *tmp = realloc(employees, next_capacity * sizeof(Employee));
            if (tmp == NULL) {
                free(emp.name);
                free(emp.surname);
                st = ERR_MEMORY;
                break;
            }
            employees = tmp;
            capacity = next_capacity;
        }
        employees[count++] = emp;
    }
    if (fclose(file) != 0 && st == SUCCESS) st = ERR_FILE_READ;
    if (st != SUCCESS) {
        free_employees(employees, count);
        return st;
    }
    *out_employees = employees;
    *out_count = count;
    return SUCCESS;
}

/* Salary classes use fixed epsilon-width intervals, making order transitive. */
static Status compare_employees(const Employee *a, const Employee *b,
                                const SortContext *ctx, int *out_result) {
    if (a == NULL || b == NULL || ctx == NULL || out_result == NULL ||
        a->name == NULL || a->surname == NULL ||
        b->name == NULL || b->surname == NULL ||
        !isfinite(a->salary) || !isfinite(b->salary)) return ERR_INVALID_DATA;
    if (!isfinite(ctx->epsilon) || ctx->epsilon <= 0.0 ||
        (ctx->direction != SORT_ASCENDING && ctx->direction != SORT_DESCENDING))
        return ERR_INVALID_DATA;

    /* floorl avoids overflow of double ratio on typical C99 platforms. */
    long double class_a = floorl((long double)a->salary / (long double)ctx->epsilon);
    long double class_b = floorl((long double)b->salary / (long double)ctx->epsilon);
    int cmp;
    if (!isfinite(class_a) || !isfinite(class_b)) {
        cmp = (a->salary > b->salary) - (a->salary < b->salary);
    } else {
        cmp = (class_a > class_b) - (class_a < class_b);
    }
    if (cmp != 0) {
        *out_result = ctx->direction == SORT_ASCENDING ? cmp : -cmp;
        return SUCCESS;
    }
    cmp = strcmp(a->surname, b->surname);
    if (cmp == 0) cmp = strcmp(a->name, b->name);
    if (cmp == 0) cmp = (a->id > b->id) - (a->id < b->id);
    *out_result = cmp;
    return SUCCESS;
}

static Status sort_employees(Employee *employees, size_t count,
                             SortDirection direction, double eps) {
    if ((employees == NULL && count != 0) || !isfinite(eps) || eps <= 0.0 ||
        (direction != SORT_ASCENDING && direction != SORT_DESCENDING))
        return ERR_INVALID_DATA;
    SortContext ctx = {direction, eps};
    for (size_t i = 1; i < count; ++i) {
        Employee key = employees[i];
        size_t j = i;
        int cmp = 0;
        while (j > 0) {
            Status st = compare_employees(&employees[j - 1], &key, &ctx, &cmp);
            if (st != SUCCESS) return st;
            if (cmp <= 0) break;
            employees[j] = employees[j - 1];
            --j;
        }
        employees[j] = key;
    }
    return SUCCESS;
}

static Status write_employees_to_file(const char *filename,
                                      const Employee *employees, size_t count) {
    if (filename == NULL || filename[0] == '\0' ||
        (employees == NULL && count != 0)) return ERR_INVALID_DATA;
    FILE *file = fopen(filename, "w");
    if (file == NULL) return ERR_FILE_OPEN;
    Status st = SUCCESS;
    for (size_t i = 0; i < count; ++i) {
        if (employees[i].name == NULL || employees[i].surname == NULL ||
            !isfinite(employees[i].salary) || employees[i].salary < 0.0) {
            st = ERR_INVALID_DATA;
            break;
        }
        if (fprintf(file, "%u %s %s %.2f\n", employees[i].id,
                    employees[i].name, employees[i].surname, employees[i].salary) < 0) {
            st = ERR_FILE_WRITE;
            break;
        }
    }
    if (fclose(file) != 0 && st == SUCCESS) st = ERR_FILE_WRITE;
    return st;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <-a|-d|/a|/d> <output_file>\n", argv[0]);
        return ERR_ARGS;
    }
    SortDirection direction;
    Status st = parse_flag(argv[2], &direction);
    if (st != SUCCESS) {
        fprintf(stderr, "Invalid flag: %s\n", argv[2]);
        return st;
    }
    Employee *employees = NULL;
    size_t count = 0;
    st = read_employees_from_file(argv[1], &employees, &count);
    if (st == SUCCESS) st = sort_employees(employees, count, direction, EPSILON);
    if (st == SUCCESS) st = write_employees_to_file(argv[3], employees, count);
    if (st != SUCCESS) fprintf(stderr, "Error: status code %d\n", (int)st);
    free_employees(employees, count);
    return st;
}
