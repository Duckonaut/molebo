#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <zlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u16 rgb;

#define ARGB(a, r, g, b)                                                                       \
    (((a > 0) ? 1 << 15 : 0) | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))

int usage() {
    printf("PNG -> NDS easily readable texture converter\n"
           "by Duckonaut\n"
           "Usage:\n"
           "    png2nds-rgb <input.obj> <output.nmsh> [-g/--gzip]\n"
           "Options:\n"
           "    -g/--gzip: compress the output file\n");
    return EXIT_FAILURE;
}

typedef struct args {
    const char* input;
    const char* output;
    bool gzip;
} args_t;

args_t parse_args(int argc, char* argv[]) {
    args_t args = {
        .input = NULL,
        .output = NULL,
        .gzip = false,
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--gzip") == 0) {
            args.gzip = true;
        } else if (!args.input) {
            args.input = argv[i];
        } else if (!args.output) {
            args.output = argv[i];
        } else {
            usage();
        }
    }

    if (!args.input) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (!args.output) {
        usage();
        exit(EXIT_FAILURE);
    }

    return args;
}

void nrgb_write(bool compressed, const void* data, size_t size, void* output) {
    if (compressed) {
        gzwrite(output, data, size);
    } else {
        fwrite(data, size, 1, output);
    }
}

int main(int argc, char* argv[]) {
    args_t args = parse_args(argc, argv);

    int width, height, channels;
    u8* data = stbi_load(args.input, &width, &height, &channels, 4);

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

    void* output = fopen(args.output, "wb");

    if (!output) {
        printf("Error: could not open output file\n");
        return EXIT_FAILURE;
    }

    // magic: NRGB
    fwrite("NRGB", 1, 4, output);

    u16 w = width;
    u16 h = height;

    // width
    fwrite(&w, 1, 2, output);
    // height
    fwrite(&h, 1, 2, output);

    // if compressed
    u8 compressed = args.gzip;

    fwrite(&compressed, 1, 1, output);

    // pad to 16 bytes
    const u8 padding[7] = { 0 };
    fwrite(padding, 1, 7, output);

    if (args.gzip) {
        fclose(output);

        output = gzopen(args.output, "ab");

        if (!output) {
            printf("Error: could not open output file\n");
            return EXIT_FAILURE;
        }
    }


    for (int i = 0; i < width * height; i++) {
        u8 r = data[i * 4];
        u8 g = data[i * 4 + 1];
        u8 b = data[i * 4 + 2];
        u8 a = data[i * 4 + 3];

        rgb color = ARGB(a, r, g, b);
        nrgb_write(args.gzip, &color, sizeof(rgb), output);
    }

    if (args.gzip) {
        gzclose(output);
    } else {
        fclose(output);
    }

    stbi_image_free(data);

    return EXIT_SUCCESS;
}
