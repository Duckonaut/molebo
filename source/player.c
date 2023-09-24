#include "player.h"

void player_init(player_t* player, const content_t* content) {}

void player_update(player_t* player) {}

void player_draw(const player_t* player) {
    mesh_instance_draw(&player->body);
    mesh_instance_draw(&player->eyes);
}
