#include "igb_mesh.h"

#include <stddef.h>
#include <string.h>

int igb_obj_is(const igb *f, int idx, const char *type) {
  if (idx < 0 || idx >= f->n_objects) {
    return 0;
  }
  const igb_object *object = &f->objects[idx];
  return object->type_name && strcmp(object->type_name, type) == 0;
}

int igb_obj_slot_i32(const igb *f, int idx, uint16_t slot) {
  if (idx < 0 || idx >= f->n_objects) {
    return -1;
  }
  const igb_fieldval *field = igb_object_field(&f->objects[idx], slot);
  if (!field) {
    return -1;
  }
  if (field->blob && field->blob_len >= 4) {
    return (int)field->blob[0] | ((int)field->blob[1] << 8) | ((int)field->blob[2] << 16) |
           ((int)field->blob[3] << 24);
  }
  return field->i32;
}

const uint8_t *igb_obj_slot_blob(const igb *f, int idx, uint16_t slot, int *out_len) {
  if (idx < 0 || idx >= f->n_objects) {
    return NULL;
  }
  const igb_fieldval *field = igb_object_field(&f->objects[idx], slot);
  if (!field || !field->blob) {
    return NULL;
  }
  *out_len = (int)field->blob_len;
  return field->blob;
}
