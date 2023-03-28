//
// -- warning.c
//
#include "warning.h"

#include <stdio.h>

extern int line_count;
static int warning_count;
static const int max_warnings = 10;

//
// -- display a warning message on stderr
void
warning(const char *message) {
    if (warning_count < max_warnings) {
        fprintf(stderr, "line %d: %s\n", line_count, message);
    }
    if (warning_count == max_warnings) {
        fprintf(stderr, "too many warnings, will no longer report\n");
    }
    ++warning_count;
}
