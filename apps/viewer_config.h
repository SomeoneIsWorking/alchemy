#ifndef ALCHEMY_VIEWER_CONFIG_H
#define ALCHEMY_VIEWER_CONFIG_H

enum {
  ALCHEMY_VIEWER_ALLOW_TRANSFORMS = 1U << 0,
};

typedef struct {
  const char *input_path;
  const char *screenshot_path;
  int compose_transforms;
  int first_extra_argument;
  const char *error;
} alchemy_viewer_config;

int alchemy_viewer_config_parse(int argc, char **argv, unsigned int allowed_options,
                                alchemy_viewer_config *config);

#endif
