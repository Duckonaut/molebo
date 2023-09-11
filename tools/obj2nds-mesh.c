#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u16 v16;
typedef u16 t16;
typedef float vec3[3];
typedef float vec2[2];
typedef v16 vec3v[3];
typedef t16 vec2t[2];

typedef u16 rgb;

#define ftov16(f) ((v16)((f) * (1 << 12)))
#define itot16(f) ((t16)((f) << 4))
#define ftot16(f) ((t16)((f) * (1 << 4)))
#define RGB(r, g, b) ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))

typedef struct vertex {
    vec3v position;
    rgb color;
    u32 normal;
    vec2t texcoord;
} vertex_t;

int usage() {
    printf("OBJ -> NDS easily readable mesh converter\n"
           "by Duckonaut\n"
           "Usage:\n"
           "    obj2nds-mesh <input.obj> <output.nmsh> [-t/--texture-size SIZE]\n"
           "Options:\n"
           "    -t/--texture-size SIZE: set the texture size (default: 256)\n");
    return EXIT_FAILURE;
}

typedef struct args {
    const char* input;
    const char* output;
    u16 texture_size;
} args_t;

args_t parse_args(int argc, char* argv[]) {
    args_t args = {
        .texture_size = 256,
        .input = NULL,
        .output = NULL,
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--texture-size") == 0) {
            if (i + 1 >= argc) {
                usage();
                exit(EXIT_FAILURE);
            }

            args.texture_size = atoi(argv[i + 1]);
            i++;
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

int main(int argc, char* argv[]) {
    args_t args = parse_args(argc, argv);

    fastObjMesh* mesh = fast_obj_read(args.input);

    if (!mesh) {
        printf("Error: failed to load mesh\n");
        return EXIT_FAILURE;
    }

    FILE* output = fopen(args.output, "wb");

    // write magic: NMSH
    const u8 magic[4] = { 'N', 'M', 'S', 'H' };
    fwrite(magic, 1, 4, output);

    // write vertex count
    u16 vertex_count = mesh->index_count;
    fwrite(&vertex_count, 1, sizeof(u16), output);

    // write index count
    u16 index_count = mesh->index_count;

    fwrite(&index_count, 1, sizeof(u16), output);

    // write mesh data

    // write vertices
    for (size_t i = 0; i < mesh->index_count; i++) {
        fastObjIndex* position = &mesh->indices[i];

        float x = mesh->positions[position->p * 3 + 0];
        float y = mesh->positions[position->p * 3 + 1];
        float z = mesh->positions[position->p * 3 + 2];

        float u = mesh->texcoords[position->t * 2 + 0] * args.texture_size;
        float v = (1.0f - mesh->texcoords[position->t * 2 + 1]) * args.texture_size;

        float nx = mesh->normals[position->n * 3 + 0];
        float ny = mesh->normals[position->n * 3 + 1];
        float nz = mesh->normals[position->n * 3 + 2];

        vertex_t vertex = {
            .position = {
                ftov16(x),
                ftov16(y),
                ftov16(z),
            },
            .color = RGB(255, 255, 255),
            .normal = 0,
            .texcoord = {
                ftot16(u),
                ftot16(v),
            },
        };

        fwrite(&vertex, sizeof(vertex_t), 1, output);
    }

    // write indices
    for (size_t i = 0; i < mesh->index_count; i++) {
        u16 index = (u16)i;
        fwrite(&index, 1, sizeof(u16), output);
    }

    return EXIT_SUCCESS;
}
