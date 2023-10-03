#include "amesh.h"
#include "dsma.h"
#include <stdio.h>

void amesh_instance_init(
    amesh_instance_t* instance,
    const amesh_t* mesh,
    texture_handle* texture_handles,
    u8 num_layers
) {
    instance->mesh = mesh;
    if (num_layers != mesh->num_layers) {
        iprintf(
            "animated_mesh_instance_init: num_layers (%d) != mesh->num_layers (%d)\n",
            num_layers,
            mesh->num_layers
        );
        return;
    }
    for (u8 i = 0; i < num_layers; ++i) {
        instance->texture_handles[i] = texture_handles[i];
    }
    instance->current_animation = 0;
    instance->current_frame = 0;
}

bool amesh_instance_update(amesh_instance_t* instance, u32 dt) {
    const amesh_t* mesh = instance->mesh;
    const animation_t* animation = &mesh->animations[instance->current_animation];
    instance->current_frame += dt;

    if (instance->blend > 0) {
        instance->blend -= 1 << 9;
    }

    if (instance->current_frame >= animation->num_frames << 12) {
        instance->current_frame = 0;
        return true;
    }
    return false;
}

void amesh_instance_set_animation(amesh_instance_t* instance, u8 animation_index) {
    instance->last_animation = instance->current_animation;
    instance->last_frame = instance->current_frame;

    instance->current_animation = animation_index;
    instance->current_frame = 0;

    instance->blend = 1 << 12;
}

void amesh_instance_render(const amesh_instance_t* instance) {
    for (int i = 0; i < instance->mesh->num_layers; i++) {
        glBindTexture(0, instance->texture_handles[i]);
        if (instance->blend > 0) {
            DSMA_DrawModelBlendAnimation(
                instance->mesh->mesh_layers[i],
                instance->mesh->animations[instance->current_animation].animation_data,
                instance->current_frame,
                instance->mesh->animations[instance->last_animation].animation_data,
                instance->last_frame,
                instance->blend
            );
        } else {
            DSMA_DrawModel(
                instance->mesh->mesh_layers[i],
                instance->mesh->animations[instance->current_animation].animation_data,
                instance->current_frame
            );
        }
    }
}
