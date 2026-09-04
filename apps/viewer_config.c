#include "viewer_config.h"

#include <stddef.h>
#include <string.h>

int alchemy_viewer_config_parse(int argc, char **argv, unsigned int allowed_options,
                                alchemy_viewer_config *config) {
  if (!config) {
    return 0;
  }
  *config = (alchemy_viewer_config){0};
  for (int index = 1; index < argc; ++index) {
    const char *argument = argv[index];
    if (strcmp(argument, "--screenshot") == 0) {
      if (++index >= argc) {
        config->error = "--screenshot requires a path";
        return 0;
      }
      config->screenshot_path = argv[index];
      continue;
    }
    if (strcmp(argument, "--compose-transforms") == 0) {
      if ((allowed_options & ALCHEMY_VIEWER_ALLOW_TRANSFORMS) == 0) {
        config->error = "--compose-transforms is not supported by this viewer";
        return 0;
      }
      config->compose_transforms = 1;
      continue;
    }
    if (argument[0] == '-') {
      config->error = "unknown option";
      return 0;
    }
    config->input_path = argument;
    config->first_extra_argument = index + 1;
    return 1;
  }
  config->error = "an input file is required";
  return 0;
}
