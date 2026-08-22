#include "igb_image.h"

#include <stdlib.h>
#include <string.h>

static const igb_object *referenced_object(const igb *file,
                                           const igb_fieldval *reference)
{
    return reference ? igb_object_by_index(file, reference->i32) : NULL;
}

static void read_palette(const igb *file, const igb_object *image_object,
                         igb_palette *palette)
{
    int slot_offset = file->slot_offset;
    const igb_fieldval *clut_reference =
        igb_object_field(image_object, (uint16_t)(17 + slot_offset));
    const igb_object *clut = referenced_object(file, clut_reference);
    if (!clut || clut->is_mem || !clut->type_name ||
        strcmp(clut->type_name, "igClut") != 0) {
        return;
    }

    const igb_fieldval *format =
        igb_object_field(clut, (uint16_t)(2 + slot_offset));
    const igb_fieldval *entries =
        igb_object_field(clut, (uint16_t)(3 + slot_offset));
    const igb_fieldval *bytes_per_entry =
        igb_object_field(clut, (uint16_t)(4 + slot_offset));
    const igb_fieldval *data_reference =
        igb_object_field(clut, (uint16_t)(5 + slot_offset));
    const igb_object *data = referenced_object(file, data_reference);
    if (!format || !entries || !bytes_per_entry || !data || !data->is_mem) {
        return;
    }

    palette->pixel_format = format->i32;
    palette->entries = entries->i32;
    palette->bytes_per_entry = bytes_per_entry->i32;
    palette->data = data->mem;
    palette->data_len = data->mem_size;
}

int igb_find_images(const igb *file, igb_image *out, int max)
{
    if (!file || !out || max <= 0) {
        return 0;
    }
    int count = 0;
    int slot_offset = file->slot_offset;
    for (int object_index = 0; object_index < file->n_objects; ++object_index) {
        const igb_object *object = &file->objects[object_index];
        if (object->is_mem || !object->type_name ||
            strcmp(object->type_name, "igImage") != 0) {
            continue;
        }
        const igb_fieldval *width =
            igb_object_field(object, (uint16_t)(2 + slot_offset));
        const igb_fieldval *height =
            igb_object_field(object, (uint16_t)(3 + slot_offset));
        const igb_fieldval *components =
            igb_object_field(object, (uint16_t)(4 + slot_offset));
        const igb_fieldval *format =
            igb_object_field(object, (uint16_t)(11 + slot_offset));
        const igb_fieldval *image_size =
            igb_object_field(object, (uint16_t)(12 + slot_offset));
        const igb_fieldval *data_reference =
            igb_object_field(object, (uint16_t)(13 + slot_offset));
        const igb_fieldval *bytes_per_row =
            igb_object_field(object, (uint16_t)(19 + slot_offset));
        const igb_fieldval *compressed =
            igb_object_field(object, (uint16_t)(20 + slot_offset));
        const igb_fieldval *name =
            igb_object_field(object, (uint16_t)(22 + slot_offset));
        const igb_object *data = referenced_object(file, data_reference);
        if (!width || !height || !format || !data_reference ||
            data_reference->short_name[0] == 0 || !data || !data->is_mem) {
            continue;
        }

        igb_image *image = &out[count];
        memset(image, 0, sizeof(*image));
        image->width = width->i32;
        image->height = height->i32;
        image->num_components = components ? components->i32 : 0;
        image->pixel_format = format->i32;
        image->image_size = image_size ? image_size->i32 : 0;
        image->bytes_per_row = bytes_per_row ? bytes_per_row->i32 : 0;
        image->compressed = compressed ? (compressed->i32 != 0) : 1;
        image->data = data->mem;
        image->data_len = data->mem_size;
        image->name = (name && name->blob) ? (char *)name->blob : NULL;
        read_palette(file, object, &image->palette);

        ++count;
        if (count >= max) {
            break;
        }
    }
    return count;
}

static size_t row_stride(const igb_image *image, size_t packed_stride)
{
    return image->bytes_per_row > 0 ?
        (size_t)image->bytes_per_row : packed_stride;
}

static int rows_fit(size_t data_len, int height, size_t stride,
                    size_t packed_stride)
{
    return data_len >= packed_stride && stride >= packed_stride &&
           (size_t)(height - 1) <= (data_len - packed_stride) / stride;
}

static uint8_t *alloc_rgba(const igb_image *image, int *out_len)
{
    size_t size = (size_t)image->width * (size_t)image->height * 4;
    uint8_t *rgba = malloc(size);
    if (rgba) {
        *out_len = (int)size;
    }
    return rgba;
}

uint8_t *igb_image_decode_rgba5551(const igb_image *image, int *out_len)
{
    size_t packed_stride = (size_t)image->width * 2;
    size_t stride = row_stride(image, packed_stride);
    if (!rows_fit(image->data_len, image->height, stride, packed_stride)) {
        return NULL;
    }
    uint8_t *rgba = alloc_rgba(image, out_len);
    if (!rgba) {
        return NULL;
    }

    for (int y = 0; y < image->height; ++y) {
        const uint8_t *row = image->data + (size_t)y * stride;
        for (int x = 0; x < image->width; ++x) {
            uint16_t value =
                (uint16_t)(row[x * 2] | (row[x * 2 + 1] << 8));
            size_t pixel = (size_t)y * (size_t)image->width + (size_t)x;
            uint8_t *out = rgba + pixel * 4;
            out[0] = (uint8_t)(((value & 0x1F) * 255) / 31);
            out[1] = (uint8_t)((((value >> 5) & 0x1F) * 255) / 31);
            out[2] = (uint8_t)((((value >> 10) & 0x1F) * 255) / 31);
            out[3] = (value & 0x8000) ? 255 : 0;
        }
    }
    return rgba;
}

uint8_t *igb_image_decode_clut_index8(const igb_image *image, int *out_len)
{
    size_t stride = row_stride(image, (size_t)image->width);
    const igb_palette *palette = &image->palette;
    if (!rows_fit(image->data_len, image->height, stride,
                  (size_t)image->width) ||
        palette->pixel_format != ALCHEMY_IGB_PFMT_RGBA8888 ||
        palette->entries <= 0 || palette->bytes_per_entry < 4 ||
        palette->data_len < 4 ||
        (size_t)(palette->entries - 1) >
            (palette->data_len - 4) / (size_t)palette->bytes_per_entry) {
        return NULL;
    }
    uint8_t *rgba = alloc_rgba(image, out_len);
    if (!rgba) {
        return NULL;
    }

    for (int y = 0; y < image->height; ++y) {
        const uint8_t *row = image->data + (size_t)y * stride;
        for (int x = 0; x < image->width; ++x) {
            unsigned index = row[x];
            if (index >= (unsigned)palette->entries) {
                free(rgba);
                *out_len = 0;
                return NULL;
            }
            const uint8_t *entry = palette->data +
                (size_t)index * (size_t)palette->bytes_per_entry;
            size_t pixel = (size_t)y * (size_t)image->width + (size_t)x;
            memcpy(rgba + pixel * 4, entry, 4);
        }
    }
    return rgba;
}
