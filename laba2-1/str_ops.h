#ifndef STR_OPS_H
#define STR_OPS_H

#include <stddef.h>
#include "status.h"

/* Во всех функциях, возвращающих строку, результат лежит в куче:
   вызывающий код обязан освободить его через free(). */

/* -l: длина строки. */
status_t str_length(const char *src, size_t *length);

/* -r: перевёрнутая строка. */
status_t str_reverse(const char *src, char **result);

/* -u: символы на нечётных позициях (счёт с 0) переводятся в верхний регистр. */
status_t str_upper_odd(const char *src, char **result);

/* -n: сначала цифры, затем буквы, затем остальные символы (порядок сохраняется). */
status_t str_digits_letters_others(const char *src, char **result);

/* -c: конкатенация строк strings[0..count-1] в псевдослучайном порядке.
   Генератор засевается через srand(seed). */
status_t str_concat_random(const char *const *strings, size_t count,
                           unsigned int seed, char **result);

#endif