#include "mesh.h"
#include "nds/ndstypes.h"
#include "strings.h"
#include "texture.h"
#include "types.h"

#include "nds/arm9/console.h"
#include "nds/arm9/video.h"
#include "nds/arm9/videoGL.h"

#include <stddef.h>
#include <math.h>
#include <nds.h>
#include <stdio.h>
#include <string.h>

#include "molebo_nrgb.h"
#include "molebo_eyes_nrgb.h"
#include "mole_nmsh.h"
#include "mole_eyes_nmsh.h"
#include "quad_nmsh.h"

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

struct state {
    mesh_instance_t quad_instance;
    mesh_t molebo_mesh;
    mesh_t molebo_eye_mesh;
    mesh_instance_t molebo_instance;
    mesh_instance_t molebo_eye_instance;
    texture_t molebo_tex;
    texture_t molebo_eye_tex;
    texture_t blank_tex;
};

void state_init(state_t* state) {
    bzero(state, sizeof(state_t));

    u8* blank_tex_data = malloc(8 * 8 * 2 + 16);
    memcpy(blank_tex_data, "NRGB", 4);
    *(u16*)(blank_tex_data + 4) = 8;
    *(u16*)(blank_tex_data + 6) = 8;
    *(u8*)(blank_tex_data + 8) = 0;
    memset(blank_tex_data + 16, 0xFF, 8 * 8 * 2);

    state->molebo_tex = texture_load(molebo_nrgb, molebo_nrgb_size);

    if (!state->molebo_tex.handle) {
        iprintf("Error: could not load molebo texture\n");
        return;
    }

    state->molebo_eye_tex = texture_load(molebo_eyes_nrgb, molebo_eyes_nrgb_size);

    if (!state->molebo_eye_tex.handle) {
        iprintf("Error: could not load molebo eyes texture\n");
        return;
    }

    state->blank_tex = texture_load(blank_tex_data, 8 * 8 * 2 + 16);

    if (!state->blank_tex.handle) {
        iprintf("Error: could not load blank texture\n");
        return;
    }

    mesh_load_nmsh(&state->molebo_mesh, mole_nmsh, mole_nmsh_size);
    mesh_load_nmsh(&state->molebo_eye_mesh, mole_eyes_nmsh, mole_eyes_nmsh_size);

    state->molebo_instance = (mesh_instance_t) {
        .mesh = &state->molebo_mesh,
        .transform = {
            .position = { -2.0f, -1.5f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = state->molebo_tex.handle,
    };

    state->molebo_eye_instance = (mesh_instance_t) {
        .mesh = &state->molebo_eye_mesh,
        .transform = state->molebo_instance.transform,
        .texture = state->molebo_eye_tex.handle,
    };

    state->quad_instance = (mesh_instance_t) {
        .mesh = &quad_mesh,
        .transform = {
            .position = { 3.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 2.0f, 2.0f, 2.0f },
        },
        .texture = state->molebo_tex.handle,
    };
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

    state_init(&state);

    glClearColor(0, 0, 0, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);

    glViewport(0, 0, 255, 191);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, 256.0 / 192.0, 0.1, 100);

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
    // glMaterialf(GL_AMBIENT, RGB15(16, 16, 16));
    // glMaterialf(GL_DIFFUSE, RGB15(16, 16, 16));
    // glMaterialf(GL_SPECULAR, BIT(15) | RGB15(8, 8, 8));
    // glMaterialf(GL_EMISSION, RGB15(16, 16, 16));

    // ds uses a table for shinyness..this generates a half-ass one
    // glMaterialShinyness();

    glMatrixMode(GL_MODELVIEW);

    iprintf("sizeof(molebo_nmsh): %d\n", mole_nmsh_size);
    iprintf("sizeof(molebo_tex): %d\n", molebo_nrgb_size);

    iprintf("\ntexture handles:\n");
    iprintf(" molebo:      %d\n", state.molebo_tex.handle);
    iprintf(" molebo eyes: %d\n", state.molebo_eye_tex.handle);
    iprintf(" blank:       %d\n", state.blank_tex.handle);

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

    texture_free(state.molebo_tex);
    texture_free(state.molebo_eye_tex);
    texture_free(state.blank_tex);

    return 0;
}

void draw_top_3d_scene(const state_t* state) {
    mesh_instance_draw(&state->quad_instance);
    mesh_instance_draw(&state->molebo_instance);
    mesh_instance_draw(&state->molebo_eye_instance);
}

void draw_bottom_screen(const state_t* state) {}

int update(state_t* state) {
    scanKeys();

    int keys = keysDown();

    if (keys & KEY_START)
        return 1;

    if (keys & KEY_UP)
        state->quad_instance.texture++;

    if (keys & KEY_DOWN)
        state->quad_instance.texture--;

    if (state->quad_instance.texture > state->blank_tex.handle)
        state->quad_instance.texture = state->molebo_tex.handle;

    if (state->quad_instance.texture < state->molebo_tex.handle)
        state->quad_instance.texture = state->blank_tex.handle;

    state->molebo_instance.transform.rotation[1] += 0.5f;

    state->molebo_instance.transform.rotation[1] =
        fmodf(state->molebo_instance.transform.rotation[1], 360);

    state->molebo_eye_instance.transform.rotation[1] =
        state->molebo_instance.transform.rotation[1];

    return 0;
}
