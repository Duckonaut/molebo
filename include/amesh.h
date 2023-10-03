#pragma once

#include "nds/ndstypes.h"
#include "texture.h"
#define MAX_ANIMATIONS 8
#define MAX_LAYERS 4

typedef struct animation {
    const void* animation_data;
    u32 num_frames;
} animation_t;

typedef struct amesh {
    const void* mesh_layers[MAX_LAYERS];
    const animation_t animations[MAX_ANIMATIONS];
    u8 num_layers;
    u8 num_animations;
} amesh_t;

typedef struct amesh_instance {
    const amesh_t* mesh;
    texture_handle texture_handles[MAX_LAYERS];

    u8 current_animation;
    u32 current_frame; // 20.12 fixed point

    u8 last_animation;
    u32 last_frame; // 20.12 fixed point

    float blend;
} amesh_instance_t;

void amesh_instance_init(
    amesh_instance_t* instance,
    const amesh_t* mesh,
    texture_handle* texture_handles,
    u8 num_layers
);

bool amesh_instance_update(
    amesh_instance_t* instance,
    u32 dt
);

void amesh_instance_set_animation(
    amesh_instance_t* instance,
    u8 animation_index
);

void amesh_instance_render(
    const amesh_instance_t* instance
);
