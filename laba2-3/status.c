#include "status.h"

const char *status_to_string(const status_t status)
{
    switch (status)
    {
    case STATUS_OK:               return "ok";
    case STATUS_NULL_POINTER:     return "null pointer";
    case STATUS_INVALID_ARGUMENT: return "invalid argument";
    case STATUS_OVERFLOW:         return "value overflow";
    case STATUS_NO_MEMORY:        return "not enough memory";
    case STATUS_FILE_OPEN:        return "cannot open file";
    case STATUS_FILE_READ:        return "cannot read file";
    default:                      return "unknown error";
    }
}