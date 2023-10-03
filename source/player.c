#include "player.h"
#include "input.h"
#include "projectile.h"
#include "state.h"
#include "stdio.h"
#include "types.h"
#include "util.h"
#include "amesh.h"

#include "nds/input.h"
#include "nds/arm9/trig_lut.h"

#include <string.h>

#include <dsma.h>

#include "mole_rig_dsm.h"
#include "mole_rig_eyes_dsm.h"
#include "mole_rig_gun_dsm.h"
#include "anim_mole_rig_base_dsa.h"
#include "anim_mole_rig_walk_dsa.h"
#include "anim_mole_rig_clap_dsa.h"

const static amesh_t s_mole_rig = {
    .num_layers = 3,
    .mesh_layers = { mole_rig_dsm, mole_rig_eyes_dsm, mole_rig_gun_dsm },
    .num_animations = 2,
    .animations = { { .animation_data = anim_mole_rig_base_dsa, .num_frames = 40 },
                    { .animation_data = anim_mole_rig_walk_dsa, .num_frames = 20 },
                    { .animation_data = anim_mole_rig_clap_dsa, .num_frames = 16 } },
};

void player_init(player_t* player, const content_t* content) {
    amesh_instance_init(
        &player->mesh_instance,
        &s_mole_rig,
        (texture_handle[]){ content->molebo_tex.handle,
                            content->molebo_eye_tex.handle,
                            content->molebo_gun_tex.handle },
        3
    );
    amesh_instance_set_animation(&player->mesh_instance, 1);

    player->transform = (transform_t){
        .position = { 0.0f, 0.0f, 0.0f },
        .rotation = { 0.0f, 0.0f, 0.0f },
        .scale = { 1.0f, 1.0f, 1.0f },
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
        if (player->mesh_instance.current_animation != 1) {
            amesh_instance_set_animation(&player->mesh_instance, 1);
        }
    } else {
        if (player->mesh_instance.current_animation != 0) {
            amesh_instance_set_animation(&player->mesh_instance, 0);
        }
    }

    if (input->just_pressed & KEY_R) {
        vec3 rocket_pos;
        memcpy(rocket_pos, player->transform.position, sizeof(vec3));

        vec3 rocket_launcher_offset = { 1.5f, 2.0f, -0.8f };
        // add the offset to the player's position, rotated by the player's rotation
        vec3_rotate_y(rocket_launcher_offset, player->transform.rotation[1]);
        vec3_add(rocket_pos, rocket_launcher_offset);

        vec3 rocket_vel;
        rocket_vel[0] = (sinLerp(player->transform.rotation[1]) / (float)(1 << 12)) * -0.2f;
        rocket_vel[1] = 0.5f;
        rocket_vel[2] = (cosLerp(player->transform.rotation[1]) / (float)(1 << 12)) * -0.2f;

        projectile_fire(
            &state,
            &state.content.rocket_mesh,
            state.content.molebo_gun_tex.handle,
            0.5f,
            0.02f,
            true,
            rocket_pos,
            rocket_vel
        );
    }

    amesh_instance_update(&player->mesh_instance, 1 << 11);
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
    glPolyFmt(
        POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1 |
        POLY_FORMAT_LIGHT2 | POLY_TOON_HIGHLIGHT | POLY_ID(8)
    );

    amesh_instance_draw(&player->mesh_instance);
}
