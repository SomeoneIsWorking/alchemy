#ifndef IGB_IMAGE_H
#define IGB_IMAGE_H

#include <stdint.h>

#include "igb.h"

uint8_t *igb_image_decode_rgba5551(const igb_image *img, int *out_len);
uint8_t *igb_image_decode_clut_index8(const igb_image *img, int *out_len);

#endif
