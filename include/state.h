#pragma once

#include "content.h"
#include "input.h"
#include "player.h"
#include "types.h"
#include "projectile.h"

typedef struct camera {
    vec3 position;
    vec3 rotation;
} camera_t;

#define MAX_PROJECTILES 32

typedef struct state {
    // content
    content_t content;

    // systems
    camera_t camera;
    input_t input;

    // instances
    mesh_instance_t quad_instance;
    mesh_instance_t beach_sand_instance;
    mesh_instance_t beach_water_instance;

    // molebo
    player_t player;
    projectile_t projectiles[MAX_PROJECTILES];
} state_t;

extern state_t state;
