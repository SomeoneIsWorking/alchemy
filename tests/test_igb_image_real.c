#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "igb.h"
#include "igb_internal.h"

#define CHECK(condition)                                                                           \
  do {                                                                                             \
    if (!(condition)) {                                                                            \
      fprintf(stderr, "test_igb_image_real: check failed at line %d: %s\n", __LINE__, #condition); \
      exit(1);                                                                                     \
    }                                                                                              \
  } while (0)

typedef struct {
  size_t exact;
  size_t pixels;
  uint64_t sad;
} image_diff;

static void asset_path(char out[4096], const char *root, const char *relative) {
  int length = snprintf(out, 4096, "%s/%s", root, relative);
  CHECK(length > 0 && length < 4096);
}

static int path_exists(const char *path) {
  FILE *file = igb_file_open(path, "rb");
  if (!file) {
    return 0;
  }
  fclose(file);
  return 1;
}

static uint8_t *load_first_image(const char *path, igb *file, igb_image *image, int *rgba_len) {
  if (igb_open(file, path) != 0) {
    return NULL;
  }
  if (igb_find_images(file, image, 1) != 1) {
    igb_close(file);
    return NULL;
  }
  uint8_t *rgba = igb_image_to_rgba(image, rgba_len);
  if (!rgba) {
    igb_close(file);
  }
  return rgba;
}

static image_diff compare(const uint8_t *actual, const uint8_t *oracle, size_t pixels) {
  image_diff diff = {.pixels = pixels};
  for (size_t pixel = 0; pixel < pixels; ++pixel) {
    unsigned delta = 0;
    for (int channel = 0; channel < 4; ++channel) {
      uint8_t a = actual[pixel * 4 + (size_t)channel];
      uint8_t b = oracle[pixel * 4 + (size_t)channel];
      delta += a > b ? a - b : b - a;
    }
    diff.sad += delta;
    diff.exact += delta == 0;
  }
  return diff;
}

static void verify_pair(const char *ps2_path, const char *x360_path, int expected_format,
                        size_t expected_exact, uint64_t expected_sad) {
  igb ps2_file;
  igb x360_file;
  igb_image ps2_image;
  igb_image x360_image;
  int ps2_len = 0;
  int x360_len = 0;
  uint8_t *ps2_rgba = load_first_image(ps2_path, &ps2_file, &ps2_image, &ps2_len);
  uint8_t *x360_rgba = load_first_image(x360_path, &x360_file, &x360_image, &x360_len);
  CHECK(ps2_rgba && x360_rgba);
  CHECK(ps2_image.pixel_format == expected_format);
  CHECK(ps2_image.width == x360_image.width);
  CHECK(ps2_image.height == x360_image.height);
  CHECK(ps2_len == x360_len);

  size_t pixels = (size_t)ps2_image.width * (size_t)ps2_image.height;
  image_diff diff = compare(ps2_rgba, x360_rgba, pixels);
  printf("real format=%d exact=%zu/%zu rgba_sad=%llu/%zu\n", expected_format, diff.exact,
         diff.pixels, (unsigned long long)diff.sad, diff.pixels * 4);
  CHECK(diff.exact == expected_exact);
  CHECK(diff.sad == expected_sad);

  free(x360_rgba);
  free(ps2_rgba);
  igb_close(&x360_file);
  igb_close(&ps2_file);
}

int main(int argc, char **argv) {
#ifdef _MSC_VER
  char *ps2_environment = NULL;
  char *x360_environment = NULL;
  size_t environment_size = 0;
  CHECK(_dupenv_s(&ps2_environment, &environment_size, "ALCHEMY_MUA_PS2_ASSETS") == 0);
  CHECK(_dupenv_s(&x360_environment, &environment_size, "ALCHEMY_MUA_X360_ASSETS") == 0);
  const char *ps2_root = ps2_environment;
  const char *x360_root = x360_environment;
#else
  const char *ps2_root = getenv("ALCHEMY_MUA_PS2_ASSETS");
  const char *x360_root = getenv("ALCHEMY_MUA_X360_ASSETS");
#endif
  if (!ps2_root) {
    ps2_root = "scratch/mua-ps2/assets";
  }
  if (!x360_root) {
    x360_root = "scratch/mua-x360/assets";
  }
  char ps2_font[4096];
  char x360_font[4096];
  char ps2_default[4096];
  char x360_default[4096];
  asset_path(ps2_font, ps2_root, "textures/fonts/360_big.igb");
  asset_path(x360_font, x360_root, "textures/fonts/360_big.igb");
  asset_path(ps2_default, ps2_root, "textures/default.igb");
  asset_path(x360_default, x360_root, "textures/default.igb");
#ifdef _MSC_VER
  free(ps2_environment);
  free(x360_environment);
#endif
  int present = path_exists(ps2_font) + path_exists(x360_font) + path_exists(ps2_default) +
                path_exists(x360_default);
  if (present == 0) {
    puts("test_igb_image_real: SKIP (MUA PS2/Xbox 360 corpus absent)");
    return 77;
  }
  CHECK(present == 4);

  uint64_t font_sad = 165402;
  if (argc == 2 && strcmp(argv[1], "--falsify") == 0) {
    ++font_sad;
  }
  verify_pair(ps2_font, x360_font, ALCHEMY_IGB_PFMT_CLUT_INDEX8, 40656, font_sad);
  verify_pair(ps2_default, x360_default, ALCHEMY_IGB_PFMT_RGBA5551, 0, 5760);
  puts("test_igb_image_real: OK (2/2 assets, 65792/65792 pixels compared)");
  return 0;
}
