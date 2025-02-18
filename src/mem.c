#include <stdlib.h>
#include <stddef.h>

#include "mem.h"
#include "utils.h"

void *s_malloc(size_t bytes) {
    void *p = malloc(bytes);
    if (!p)
        err_wargs("Could not allocate %zu bytes", bytes);
    return p;
}
