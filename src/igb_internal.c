#include "igb_internal.h"

#include <stdlib.h>
#include <string.h>

FILE *igb_file_open(const char *path, const char *mode) {
#ifdef _MSC_VER
  FILE *file = NULL;
  if (fopen_s(&file, path, mode) != 0) {
    return NULL;
  }
  return file;
#else
  return fopen(path, mode);
#endif
}

char *igb_duplicate_string(const char *source) {
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
