#include "igb_internal.h"

#include <stdlib.h>
#include <string.h>

char *igb_duplicate_string(const char *source)
{
    if (!source) {
        return NULL;
    }
    size_t size = strlen(source) + 1;
    char *copy = malloc(size);
    if (copy) {
        memcpy(copy, source, size);
    }
    return copy;
}
