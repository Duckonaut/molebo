#pragma once

#include "content.h"
#include "input.h"
#include "mesh.h"

typedef struct player {
    mesh_instance_t body;
    mesh_instance_t head;
    mesh_instance_t eyes;
    mesh_instance_t left_arm;
    mesh_instance_t right_arm;
    mesh_instance_t left_leg;
    mesh_instance_t right_leg;

    vec3 position;
    vec3 rotation;

    float speed;
} player_t;

void player_init(player_t* player, const content_t* content);
void player_update(player_t* player, const input_t* input);
void player_draw(const player_t* player);
