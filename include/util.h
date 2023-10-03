#pragma once

#include "nds/arm9/trig_lut.h"
#include "types.h"
#include <math.h>
inline static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

static inline float ground_z_to_y(float z) {
    if (z > -8.0f) {
        return 0.0f;
    } else if (z < -28.0f) {
        return -4.0f;
    } else {
        return lerpf(0.0f, -4.0f, (z + 8.0f) / -20.0f);
    }
}

/// lerps between two angles from -32768 to 32767 (ds angles)
/// loops around and uses the shortest path
static inline i16 nds_anglelerp(i16 a, i16 b, float by) {
    i32 diff = b - a;
    if (diff > 16384) {
        diff -= 32768;
    } else if (diff < -16384) {
        diff += 32768;
    }
    return a + (i16)(diff) * by;
}

static inline i16 nds_atan2i(i16 y, i16 x) {
    // ds angles are from -32768 to 32767
    float angle = atan2f(y, x) * 32767.0f / 2 / M_PI;
    if (angle < 0.0f) {
        angle += 65536.0f;
    }
    return (i16)(angle);
}

static inline void vec3_rotate_y(vec3 v, i16 angle) {
    float x = v[0];
    float z = v[2];
    float cos_angle = (cosLerp(-angle) / (float)(1 << 12));
    float sin_angle = (sinLerp(-angle) / (float)(1 << 12));
    v[0] = x * cos_angle - z * sin_angle;
    v[2] = x * sin_angle + z * cos_angle;
}

static inline void vec3_add(vec3 a, vec3 b) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}
