#include "player.h"
#include "input.h"
#include "nds/input.h"
#include <string.h>

void player_init(player_t* player, const content_t* content) {}

void player_update(player_t* player, const input_t* input) {
    bool left = input->held & KEY_LEFT;
    bool right = input->held & KEY_RIGHT;
    bool up = input->held & KEY_UP;
    bool down = input->held & KEY_DOWN;
    bool a = input->held & KEY_A;
    bool b = input->held & KEY_B;
    bool x = input->held & KEY_X;
    bool y = input->held & KEY_Y;

    bool l = input->held & KEY_L;
    bool r = input->held & KEY_R;

    player->body.transform.position[0] += (right - left) * 0.1f;
    player->body.transform.position[2] += (down - up) * 0.1f;

    memcpy(&player->eyes.transform, &player->body.transform,
           sizeof(transform_t));
}

void player_draw(const player_t* player) {
    mesh_instance_draw(&player->body);
    mesh_instance_draw(&player->eyes);
}
