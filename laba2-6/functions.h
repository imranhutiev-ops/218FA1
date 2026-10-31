#ifndef OVER_SCANF_H
#define OVER_SCANF_H

#include <stdio.h>

int overfscanf(FILE *restrict stream, const char *restrict format, ...);
int oversscanf(const char *restrict str, const char *restrict format, ...);

#endif // OVER_SCANF_H