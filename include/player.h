#pragma once

#include "content.h"
#include "input.h"
#include "mesh.h"
#include "texture.h"

typedef enum player_anim_state {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_WALK,
} player_anim_state_t;

typedef struct player {
    transform_t transform;
    texture_handle body_texture;
    texture_handle eyes_texture;

    float speed;
    u32 timer;
} player_t;

void player_init(player_t* player, const content_t* content);
void player_update(player_t* player, const input_t* input);
void player_draw(const player_t* player);
