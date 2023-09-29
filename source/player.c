#include "player.h"
#include "input.h"
#include "stdio.h"
#include "util.h"

#include "nds/input.h"
#include "nds/arm9/trig_lut.h"

#include <string.h>

#include <dsma.h>

#include "mole_rig_dsm.h"
#include "mole_rig_eyes_dsm.h"
#include "mole_rig_gun_dsm.h"
#include "anim_mole_rig_base_dsa.h"
#include "anim_mole_rig_clap_dsa.h"

void player_init(player_t* player, const content_t* content) {
    player->body_texture = content->molebo_tex.handle;
    player->eyes_texture = content->molebo_eye_tex.handle;
    player->gun_texture = content->molebo_gun_tex.handle;

    player->transform = (transform_t){
        .position = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 2.0f, 2.0f, 2.0f },
    };
}

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

    player->transform.position[0] += (right - left) * 0.1f;
    player->transform.position[2] += (down - up) * 0.1f;

    player->transform.position[1] = ground_z_to_y(player->transform.position[2]);

    if (left || right || up || down) {
        i16 current_angle = player->transform.rotation[1];
        i16 target_angle = nds_atan2i((left - right), (up - down));
        iprintf("\x1b[16;0H        \n");
        iprintf("\x1b[16;0H%d\n", target_angle);
        player->transform.rotation[1] = nds_anglelerp(current_angle, target_angle, 0.1f);
    }

    player->timer = (((player->timer >> 12) + 1) % 16) << 12;
}

void player_draw(const player_t* player) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(
        player->transform.position[0],
        player->transform.position[1],
        player->transform.position[2]
    );
    glRotateYi(player->transform.rotation[1]);
    glRotateZi(player->transform.rotation[2]);
    glRotateXi(player->transform.rotation[0] + 0x6000);
    glScalef(
        player->transform.scale[0],
        player->transform.scale[1],
        player->transform.scale[2]
    );
    glBindTexture(0, player->body_texture);
    glPolyFmt(
        POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1 |
        POLY_FORMAT_LIGHT2 | POLY_TOON_HIGHLIGHT | POLY_ID(0b001000)
    );
    // mesh_draw(player->body.mesh);

    DSMA_DrawModel(mole_rig_dsm, anim_mole_rig_clap_dsa, player->timer);
    glBindTexture(0, player->eyes_texture);
    DSMA_DrawModel(mole_rig_eyes_dsm, anim_mole_rig_clap_dsa, player->timer);
    glBindTexture(0, player->gun_texture);
    DSMA_DrawModel(mole_rig_gun_dsm, anim_mole_rig_clap_dsa, player->timer);

    // mesh_instance_draw(&player->body);
    // mesh_instance_draw(&player->eyes);
}
