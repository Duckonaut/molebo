#pragma once
#include "nds/arm9/videoGL.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <nds/ndstypes.h>

typedef float vec2[2];
typedef float vec3[3];
typedef v16 vec3v[3];
typedef u8 vec3b[3];
typedef t16 vec2t[2];

typedef size_t usize;

#define v16tofloat(v) ((float)(v) / (float)(1 << 12))
