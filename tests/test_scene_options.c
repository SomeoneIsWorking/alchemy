#include <assert.h>
#include <stdio.h>

#include "igb_mesh.h"

typedef struct {
  int load_started;
  int roots;
} diagnostic_counts;

static void record_diagnostic(void *context, const igb_scene_diagnostic *diagnostic) {
  diagnostic_counts *counts = context;
  assert(diagnostic);
  switch (diagnostic->kind) {
  case IGB_SCENE_DIAGNOSTIC_LOAD_STARTED:
    ++counts->load_started;
    assert(diagnostic->object_index == -1);
    break;
  case IGB_SCENE_DIAGNOSTIC_ROOT_DISCOVERED:
    ++counts->roots;
    assert(diagnostic->object_index == 0);
    assert(diagnostic->object_type);
    break;
  case IGB_SCENE_DIAGNOSTIC_GEOMETRY_DECODED:
    assert(0 && "empty group must not produce geometry");
    break;
  }
}

int main(void) {
  char group_type[] = "igGroup";
  igb_object object = {.type_name = group_type};
  igb file = {.objects = &object, .n_objects = 1};
  diagnostic_counts counts = {0};
  const igb_scene_options options = {
      .compose_transforms = 1,
      .diagnostic = record_diagnostic,
      .diagnostic_context = &counts,
  };
  igb_scene scene;

  assert(igb_scene_load(&file, &options, &scene) == 0);
  assert(scene.n_meshes == 0);
  assert(counts.load_started == 1);
  assert(counts.roots == 1);
  igb_scene_free(&scene);
  printf("scene options: typed immutable configuration and diagnostics passed\n");
  return 0;
}
