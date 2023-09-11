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
#include "mole_nmsh.h"
#include "quad_nmsh.h"

typedef struct state state_t;

void draw_top_3d_scene(const state_t* state);
void draw_bottom_screen(const state_t* state);
int update(state_t* state);

// clang-format off
vertex_t tri_vertices[3] = {
    {
        { floattov16(0.0f), floattov16(1.0f), floattov16(0.0f) },
        RGB8(255, 0, 0),
        0,
        { inttot16(4), inttot16(8) }
    },
    {
        { floattov16(-1.0f), floattov16(-1.0f), floattov16(0.0f) },
        RGB8(0, 255, 0),
        0,
        { inttot16(0), inttot16(0) }
    },
    {
        { floattov16(1.0f), floattov16(-1.0f), floattov16(0.0f) },
        RGB8(0, 0, 255),
        0,
        { inttot16(8), inttot16(0) }
    },
};
// clang-format on

u16 tri_indices[3] = { 0, 1, 2 };

const mesh_t tri_mesh = {
    .vertices = tri_vertices,
    .indices = tri_indices,
    .vert_len = 3,
    .index_len = 3,
    .mode = GL_TRIANGLES,
};

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
    mesh_instance_t tri_instance;
    mesh_instance_t quad_instance;
    mesh_t molebo_mesh;
    mesh_instance_t molebo_instance;
    texture_handle molebo_tex;
    texture_handle blank_tex;
};

void state_init(state_t* state) {
    bzero(state, sizeof(state_t));

    u8* blank_tex_data = malloc(8 * 8 * 2 + 8);
    memcpy(blank_tex_data, "NRGB", 4);
    *(u16*)(blank_tex_data + 4) = 8;
    *(u16*)(blank_tex_data + 6) = 8;
    memset(blank_tex_data + 8, 0xFF, 8 * 8 * 2);

    texture_handle tex = texture_load(molebo_nrgb);

    if (!tex) {
        printf("Error: could not load texture\n");
        return;
    }

    state->molebo_tex = tex;

    texture_handle blank_tex = texture_load(blank_tex_data);

    if (!blank_tex) {
        printf("Error: could not load texture\n");
        return;
    }

    state->blank_tex = blank_tex;

    mesh_load_nmsh(&state->molebo_mesh, mole_nmsh);

    state->molebo_instance = (mesh_instance_t) {
        .mesh = &state->molebo_mesh,
        .transform = {
            .position = { 0.0f, -1.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = tex,
    };

    state->tri_instance = (mesh_instance_t) {
        .mesh = &tri_mesh,
        .transform = {
            .position = { -4.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = tex,
    };
    state->quad_instance = (mesh_instance_t) {
        .mesh = &quad_mesh,
        .transform = {
            .position = { 4.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = tex,
    };
}

state_t state;

int main() {
    glInit();

    videoSetMode(MODE_0_3D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankC(VRAM_C_SUB_BG);

    PrintConsole bottom_screen;
    consoleInit(&bottom_screen, 1, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
    consoleSelect(&bottom_screen);

    bgSetPriority(0, 1);

    glEnable(GL_ANTIALIAS);
    glEnable(GL_TEXTURE_2D);

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

    iprintf("sizeof(vertex_t): %d", sizeof(vertex_t));

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

    return 0;
}

void draw_top_3d_scene(const state_t* state) {
    mesh_instance_draw(&state->tri_instance);
    mesh_instance_draw(&state->quad_instance);
    mesh_instance_draw(&state->molebo_instance);
}

void draw_bottom_screen(const state_t* state) {}

int update(state_t* state) {
    scanKeys();

    int keys = keysDown();

    if (keys & KEY_START)
        return 1;

    printf("\x1b[14;5H rtri  = %f     \n", state->tri_instance.transform.rotation[1]);
    printf(
        "\x1b[15;5H rquad = %f,\x1b[16;14H%f \n",
        state->quad_instance.transform.rotation[0],
        state->quad_instance.transform.rotation[2]
    );
    state->tri_instance.transform.rotation[1] += 0.9f;

    state->tri_instance.transform.rotation[1] =
        fmodf(state->tri_instance.transform.rotation[1], 360);

    state->quad_instance.transform.rotation[0] -= 0.5f;
    state->quad_instance.transform.rotation[2] -= 0.5f;

    state->quad_instance.transform.rotation[0] =
        fmodf(state->quad_instance.transform.rotation[0], 360);

    state->quad_instance.transform.rotation[2] =
        fmodf(state->quad_instance.transform.rotation[2], 360);

    state->molebo_instance.transform.rotation[1] += 0.5f;

    state->molebo_instance.transform.rotation[1] =
        fmodf(state->molebo_instance.transform.rotation[1], 360);

    return 0;
}
