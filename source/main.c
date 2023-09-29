#include "content.h"
#include "mesh.h"
#include "strings.h"
#include "texture.h"
#include "types.h"
#include "player.h"
#include "input.h"
#include "util.h"

#include "nds/arm9/input.h"
#include "nds/ndstypes.h"
#include "nds/arm9/console.h"
#include "nds/arm9/video.h"
#include "nds/arm9/videoGL.h"

#include <stddef.h>
#include <math.h>
#include <nds.h>
#include <stdio.h>
#include <string.h>

typedef struct state state_t;

void draw_top_3d_scene(const state_t* state);
void draw_bottom_screen(const state_t* state);
int update(state_t* state);

// clang-format off
vertex_t quad_vertices[4] = {
    {
        { floattov16(-1.0f), floattov16(1.0f), floattov16(0.0f) },
        RGB8(255, 255, 255),
        0,
        { inttot16(0), inttot16(0) }
    },
    {
        { floattov16(1.0f), floattov16(1.0f), floattov16(0.0f) },
        RGB8(255, 255, 255),
        0,
        { inttot16(256), inttot16(0) }
    },
    {
        { floattov16(1.0f), floattov16(-1.0f), floattov16(0.0f) },
        RGB8(255, 255, 255),
        0,
        { inttot16(256), inttot16(256) }
    },
    {
        { floattov16(-1.0f), floattov16(-1.0f), floattov16(0.0f) },
        RGB8(255, 255, 255),
        0,
        { inttot16(0), inttot16(256) }
    },
};
// clang-format on

u16 quad_indices[4] = { 0, 1, 2, 3 };

const mesh_t quad_mesh = {
    .vertices = quad_vertices,
    .indices = quad_indices,
    .vert_len = 4,
    .index_len = 4,
    .mode = GL_QUADS,
};

typedef struct camera {
    vec3 position;
    vec3 rotation;
} camera_t;

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
} state_t;

void state_init(state_t* state) {
    bzero(state, sizeof(state_t));

    content_load(&state->content);

    state->quad_instance = (mesh_instance_t) {
        .mesh = &quad_mesh,
        .transform = {
            .position = { 3.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 2.0f, 2.0f, 2.0f },
        },
        .texture = state->content.molebo_tex.handle,
        .poly_fmt = POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0),
    };

    state->beach_sand_instance = mesh_instance_create(
        &state->content.beach_sand_mesh,
        state->content.sand_tex.handle,
        POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0)
    );

    state->beach_sand_instance.transform.position[2] = 10.0f;

    state->beach_sand_instance.transform.scale[0] = 16.0f;
    state->beach_sand_instance.transform.scale[1] = 16.0f;
    state->beach_sand_instance.transform.scale[2] = 16.0f;

    state->beach_water_instance = mesh_instance_create(
        &state->content.beach_water_mesh,
        state->content.water_tex.handle,
        POLY_ALPHA(15) | POLY_CULL_NONE | POLY_ID(0)
    );

    state->beach_water_instance.transform = state->beach_sand_instance.transform;

    player_init(&state->player, &state->content);
}

state_t state;

int main() {
    glInit();

    videoSetMode(MODE_0_3D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankC(VRAM_C_SUB_BG);

    PrintConsole bottom_screen;
    consoleInit(&bottom_screen, 1, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    consoleSelect(&bottom_screen);

    bgSetPriority(0, 1);

    glEnable(GL_ANTIALIAS);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glEnable(GL_OUTLINE);

    state_init(&state);

    glClearColor(0, 181, 252, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);

    glViewport(0, 0, 255, 191);

    glMatrixMode(GL_MODELVIEW);
    glLight(0, RGB15(10, 10, 10), floattov10(-0.8), 0, floattov10(-0.6));
    glLight(1, RGB15(10, 10, 10), floattov10(0.8), 0, floattov10(0.6));
    glLight(2, RGB15(11, 11, 11), 0, floattov10(-0.99), floattov10(-0.01));
    glMaterialf(GL_AMBIENT, RGB15(8, 8, 8));
    glMaterialf(GL_DIFFUSE, RGB15(24, 24, 24));
    glMaterialf(GL_SPECULAR, RGB15(0, 0, 0));
    glMaterialf(GL_EMISSION, RGB15(0, 0, 0));
    glSetToonTableRange(0, 10, RGB15(18, 16, 16));
    glSetToonTableRange(11, 24, RGB15(28, 26, 26));
    glSetToonTableRange(25, 31, RGB15(31, 31, 31));

    while (1) {
        draw_top_3d_scene(&state);
        draw_bottom_screen(&state);

        // flush to screen
        glFlush(0);

        // wait for the screen to refresh
        swiWaitForVBlank();

        if (update(&state))
            break;
    }

    content_unload(&state.content);

    return 0;
}

void draw_top_3d_scene(const state_t* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, 256.0 / 192.0, 0.1, 100);
    glRotateX(state->camera.rotation[0]);
    glRotateY(state->camera.rotation[1]);
    glRotateZ(state->camera.rotation[2]);
    glTranslatef(
        -state->camera.position[0],
        -state->camera.position[1],
        -state->camera.position[2]
    );

    mesh_instance_draw(&state->beach_sand_instance);
    mesh_instance_draw(&state->quad_instance);

    // draw molebo
    player_draw(&state->player);

    glPolyFmt(POLY_ALPHA(20) | POLY_CULL_NONE);
    mesh_instance_draw(&state->beach_water_instance);
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
}

void draw_bottom_screen(const state_t* state) {}

int update(state_t* state) {
    input_update(&state->input);

    if (state->input.held & KEY_START)
        return 1;

    player_update(&state->player, &state->input);

    state->camera.position[0] =
        lerpf(state->camera.position[0], state->player.transform.position[0], 0.1f);
    state->camera.position[1] =
        lerpf(state->camera.position[1], state->player.transform.position[1] + 9.0f, 0.1f);
    state->camera.position[2] =
        lerpf(state->camera.position[2], state->player.transform.position[2] + 8.0f, 0.1f);

    state->camera.rotation[0] = 30.0f;

    return 0;
}
