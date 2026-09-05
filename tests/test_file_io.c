#include "igb_internal.h"

#include <assert.h>
#include <string.h>

int main(int argc, char **argv) {
  assert(argc == 3);
  assert(igb_file_open(argv[2], "rb") == NULL);
  FILE *output = igb_file_open(argv[1], "wb");
  assert(output != NULL);
  const char bytes[] = "Alchemy file boundary";
  assert(fwrite(bytes, 1, sizeof(bytes), output) == sizeof(bytes));
  assert(fclose(output) == 0);
  FILE *input = igb_file_open(argv[1], "rb");
  assert(input != NULL);
  char actual[sizeof(bytes)] = {0};
  assert(fread(actual, 1, sizeof(actual), input) == sizeof(actual));
  assert(fclose(input) == 0);
  assert(memcmp(actual, bytes, sizeof(bytes)) == 0);
  assert(remove(argv[1]) == 0);
  puts("File boundary: refused missing input; wrote and reopened exact bytes");
  return 0;
}
