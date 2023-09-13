#include "texture.h"
#include <stdio.h>

#include "tinf.h"

enum GL_TEXTURE_SIZE_ENUM texture_size_decode(u16 size) {
    switch (size) {
        case 1024:
            return TEXTURE_SIZE_1024;
        case 512:
            return TEXTURE_SIZE_512;
        case 256:
            return TEXTURE_SIZE_256;
        case 128:
            return TEXTURE_SIZE_128;
        case 64:
            return TEXTURE_SIZE_64;
        case 32:
            return TEXTURE_SIZE_32;
        case 16:
            return TEXTURE_SIZE_16;
        case 8:
            return TEXTURE_SIZE_8;
        default:
            return TEXTURE_SIZE_256;
    }
}

texture_t texture_load(const u8* data, size_t datalen) {
    // validate magic (NRGB)
    const u8 magic[4] = { 'N', 'R', 'G', 'B' };
    if (memcmp(data, magic, 4) != 0) {
        iprintf("Error: invalid magic\n");

        texture_t texture = {
            .handle = 0,
            .dyn = NULL,
        };
        return texture;
    }

    u16 width = *(u16*)(data + 4);
    u16 height = *(u16*)(data + 6);

    enum GL_TEXTURE_SIZE_ENUM width_code, height_code;

    width_code = texture_size_decode(width);
    height_code = texture_size_decode(height);

    bool compressed = data[8];

    texture_handle tex;
    glGenTextures(1, &tex);
    glBindTexture(0, tex);

    if (compressed) {
        texture_t texture = {
            .handle = tex,
            .dyn = malloc(sizeof(dyn_texture_t)),
        };

        texture.dyn->width = width;
        texture.dyn->height = height;
        texture.dyn->type = GL_RGBA;
        size_t expected_len = (width * height * 2);

        texture.dyn->data = malloc(expected_len);

        // compressed source
        u8* source = (u8*)data + 16;
        size_t outlen = expected_len;

        iprintf("compressed: %u\n", datalen);

        // decompress using tinf
        int res = tinf_gzip_uncompress(texture.dyn->data, &outlen, source, datalen - 16);

        if ((res != TINF_OK) || (outlen != expected_len)) {
            iprintf("Error: failed to decompress texture\n");
            iprintf("res: %d\n", res);

            texture_t texture = {
                .handle = 0,
                .dyn = NULL,
            };
            return texture;
        }

        iprintf("decompressed: %u\n", outlen);

        rgb* texture_bin = (rgb*)texture.dyn->data;

        glTexImage2D(
            0,
            0,
            GL_RGBA,
            width_code,
            height_code,
            0,
            TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
            (u8*)texture_bin
        );

        return texture;
    } else {
        rgb* texture_bin = (rgb*)(data + 16);

        glTexImage2D(
            0,
            0,
            GL_RGBA,
            width_code,
            height_code,
            0,
            TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
            (u8*)texture_bin
        );

        texture_t texture = {
            .handle = tex,
            .dyn = NULL,
        };

        return texture;
    }
}

void texture_free(texture_t texture) {
    if (texture.dyn) {
        free(texture.dyn->data);
        free(texture.dyn);
    }
    glDeleteTextures(1, &texture.handle);
}
