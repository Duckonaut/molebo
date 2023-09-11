#include "texture.h"
#include <stdio.h>

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

texture_handle texture_load(const u8* data) {
    rgb* texture_bin = (rgb*)data;

    // validate magic (NRGB)
    const u8 magic[4] = { 'N', 'R', 'G', 'B' };
    if (memcmp(texture_bin, magic, 4) != 0) {
        iprintf("Error: invalid magic\n");
        return 0;
    }

    enum GL_TEXTURE_SIZE_ENUM width, height;

    width = texture_size_decode(texture_bin[4] | (texture_bin[5] << 8));
    height = texture_size_decode(texture_bin[6] | (texture_bin[7] << 8));

    texture_handle tex;
    glGenTextures(1, &tex);
    glBindTexture(0, tex);
    glLight(0, RGB15(31, 31, 31), floattov10(0.0f), floattov10(0.0f), floattov10(-1.0f));


    glTexImage2D(
        0,
        0,
        GL_RGB,
        width,
        height,
        0,
        TEXGEN_TEXCOORD,
        (u8*)texture_bin + 8
    );

    glBindTexture(0, 0);

    return tex;
}

