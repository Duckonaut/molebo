#pragma once
#include "nds/arm9/videoGL.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <nds/ndstypes.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float vec2[2];
typedef float vec3[3];
typedef i16 vec3i[3];
typedef v16 vec3v[3];
typedef u8 vec3b[3];
typedef t16 vec2t[2];

typedef size_t usize;

#define v16tofloat(v) ((float)(v) / (float)(1 << 12))
