#ifndef CHTTP_ERROR_H
#define CHTTP_ERROR_H

#include <stdbool.h>

typedef struct {
    bool occurred;
    char msg[256];
} chttp_error_t;

void chttp_report(chttp_error_t *err, const char *fmt, ...);

#endif