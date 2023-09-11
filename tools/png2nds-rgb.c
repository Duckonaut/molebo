#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u16 rgb;

#define ARGB(a, r, g, b) (((a > 0) ? 1 << 15 : 0) | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))

int usage() {
    printf("Usage: png2nds-rgb <input.png> <output.nrgb>\n");
    return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return usage();
    }

    int width, height, channels;
    u8* data = stbi_load(argv[1], &width, &height, &channels, 4);

    if (!data) {
        printf("Error: could not load image\n");
        return EXIT_FAILURE;
    }

    const int valid_sizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };

    bool valid_size_x = false;

    for (int i = 0; i < sizeof(valid_sizes) / sizeof(int); i++) {
        if (width == valid_sizes[i]) {
            valid_size_x = true;
            break;
        }
    }

    if (!valid_size_x) {
        printf("Error: invalid width\n");
        return EXIT_FAILURE;
    }

    bool valid_size_y = false;

    for (int i = 0; i < sizeof(valid_sizes) / sizeof(int); i++) {
        if (height == valid_sizes[i]) {
            valid_size_y = true;
            break;
        }
    }

    if (!valid_size_y) {
        printf("Error: invalid height\n");
        return EXIT_FAILURE;
    }

    FILE* output = fopen(argv[2], "wb");

    if (!output) {
        printf("Error: could not open output file\n");
        return EXIT_FAILURE;
    }

    // magic: NRGB
    fwrite("NRGB", 1, 4, output);
    // width
    fwrite(&width, 1, 2, output);
    // height
    fwrite(&height, 1, 2, output);

    for (int i = 0; i < width * height; i++) {
        u8 r = data[i * 4];
        u8 g = data[i * 4 + 1];
        u8 b = data[i * 4 + 2];
        u8 a = data[i * 4 + 3];

        rgb color = ARGB(a, r, g, b);
        fwrite(&color, 1, 2, output);
    }

    fclose(output);

    stbi_image_free(data);

    return EXIT_SUCCESS;
}
