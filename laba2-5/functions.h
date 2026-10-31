#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>

int overfprintf(FILE *stream, const char *format, ...);
int oversprintf(char *str, const char *format, ...);

#endif