#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "viewer_config.h"

static void test_complete_config(void) {
  char *argv[] = {"viewer", "--compose-transforms", "--screenshot", "frame.bmp", "scene.igb", "45"};
  alchemy_viewer_config config;
  assert(alchemy_viewer_config_parse(6, argv, ALCHEMY_VIEWER_ALLOW_TRANSFORMS, &config));
  assert(strcmp(config.input_path, "scene.igb") == 0);
  assert(strcmp(config.screenshot_path, "frame.bmp") == 0);
  assert(config.compose_transforms == 1);
  assert(config.first_extra_argument == 5);
}

static void test_refusals(void) {
  alchemy_viewer_config config;
  char *missing_path[] = {"viewer", "--screenshot"};
  assert(!alchemy_viewer_config_parse(2, missing_path, 0, &config));
  assert(strstr(config.error, "requires a path"));

  char *unsupported[] = {"viewer", "--compose-transforms", "scene.igb"};
  assert(!alchemy_viewer_config_parse(3, unsupported, 0, &config));
  assert(strstr(config.error, "not supported"));

  char *unknown[] = {"viewer", "--mystery", "scene.igb"};
  assert(!alchemy_viewer_config_parse(3, unknown, 0, &config));
  assert(strstr(config.error, "unknown option"));

  char *missing_input[] = {"viewer"};
  assert(!alchemy_viewer_config_parse(1, missing_input, 0, &config));
  assert(strstr(config.error, "input file"));
}

int main(void) {
  test_complete_config();
  test_refusals();
  printf("viewer config: typed CLI values and negative cases passed\n");
  return 0;
}
