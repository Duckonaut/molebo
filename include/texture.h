#pragma once
#include "types.h"

typedef int texture_handle;

typedef struct dyn_texture {
    u16 width;
    u16 height;
    GL_TEXTURE_TYPE_ENUM type;
    u8* data;
} dyn_texture_t;

typedef struct texture {
    dyn_texture_t* dyn;
    texture_handle handle;
} texture_t;

enum GL_TEXTURE_SIZE_ENUM texture_size_decode(u16 size);

texture_t texture_load(const u8* data, size_t datalen);
void texture_free(texture_t texture);
