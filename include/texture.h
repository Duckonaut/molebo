#pragma once
#include "types.h"

typedef int texture_handle;

enum GL_TEXTURE_SIZE_ENUM texture_size_decode(u16 size);

texture_handle texture_load(const u8* data);
