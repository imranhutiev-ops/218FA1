#ifndef STATUS_H
#define STATUS_H

typedef enum
{
    STATUS_OK = 0,
    STATUS_NULL_POINTER,
    STATUS_INVALID_ARGUMENT,
    STATUS_OVERFLOW,
    STATUS_NO_MEMORY
} status_t;

const char *status_to_string(status_t status);

#endif