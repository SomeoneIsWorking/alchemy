#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "igb.h"

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "test_igb_image: check failed at line %d: %s\n",   \
                    __LINE__, #condition);                                      \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_rgba5551(void)
{
    const uint8_t data[] = {
        0x1f, 0x80, 0xe0, 0x83, 0xaa, 0xaa,
        0x00, 0xfc, 0xff, 0x7f,
    };
    const uint8_t expected[] = {
        255, 0, 0, 255, 0, 255, 0, 255,
        0, 0, 255, 255, 255, 255, 255, 0,
    };
    const igb_image image = {
        .width = 2,
        .height = 2,
        .pixel_format = ALCHEMY_IGB_PFMT_RGBA5551,
        .bytes_per_row = 6,
        .data = data,
        .data_len = sizeof(data),
    };
    int length = 0;
    uint8_t *rgba = igb_image_to_rgba(&image, &length);
    CHECK(rgba);
    CHECK(length == (int)sizeof(expected));
    CHECK(memcmp(rgba, expected, sizeof(expected)) == 0);
    free(rgba);
}

static void check_clut_index8(void)
{
    const uint8_t indices[] = {0, 1, 0xee, 2, 3};
    const uint8_t palette[] = {
        1, 2, 3, 4, 0xdd,
        5, 6, 7, 8, 0xdd,
        9, 10, 11, 12, 0xdd,
        13, 14, 15, 16,
    };
    const uint8_t expected[] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16,
    };
    const igb_image image = {
        .width = 2,
        .height = 2,
        .pixel_format = ALCHEMY_IGB_PFMT_CLUT_INDEX8,
        .bytes_per_row = 3,
        .data = indices,
        .data_len = sizeof(indices),
        .palette = {
            .pixel_format = ALCHEMY_IGB_PFMT_RGBA8888,
            .entries = 4,
            .bytes_per_entry = 5,
            .data = palette,
            .data_len = sizeof(palette),
        },
    };
    int length = 0;
    uint8_t *rgba = igb_image_to_rgba(&image, &length);
    CHECK(rgba);
    CHECK(length == (int)sizeof(expected));
    CHECK(memcmp(rgba, expected, sizeof(expected)) == 0);
    free(rgba);
}

int main(void)
{
    check_rgba5551();
    check_clut_index8();
    puts("test_igb_image: OK (2/2 formats through production decode)");
    return 0;
}
