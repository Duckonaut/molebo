#pragma once

#include "mesh.h"
#include "texture.h"
#include "types.h"

#include "nds/ndstypes.h"

typedef struct state state_t;

typedef struct projectile {
    transform_t transform;
    const mesh_t* mesh;
    texture_handle texture;
    float radius;
    vec3 velocity;
    float gravity;
    bool active;
    bool friendly;
} projectile_t;

void projectile_init(
    projectile_t* projectile,
    const mesh_t* mesh,
    texture_handle texture,
    float radius,
    float gravity,
    bool friendly,
    const vec3 position,
    const vec3 velocity
);

void projectile_fire(
    state_t* state,
    const mesh_t* mesh,
    texture_handle texture,
    float radius,
    float gravity,
    bool friendly,
    const vec3 position,
    const vec3 velocity
);

void projectile_update(projectile_t* projectile);

void projectile_draw(const projectile_t* projectile);
