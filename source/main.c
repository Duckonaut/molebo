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
#include "molebo_eye_nrgb.h"
#include "mole_nmsh.h"
#include "mole_eyes_nmsh.h"
#include "quad_nmsh.h"
#include "beach_sand_nmsh.h"
#include "sand_nrgb.h"
#include "beach_water_nmsh.h"
#include "water_nrgb.h"

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

struct state {
    // content
    mesh_t molebo_mesh;
    mesh_t molebo_eye_mesh;
    mesh_t beach_sand_mesh;
    mesh_t beach_water_mesh;
    texture_t blank_tex;
    texture_t molebo_tex;
    texture_t molebo_eye_tex;
    texture_t sand_tex;
    texture_t water_tex;

    // systems
    camera_t camera;

    // instances
    mesh_instance_t quad_instance;
    mesh_instance_t molebo_instance;
    mesh_instance_t molebo_eye_instance;
    mesh_instance_t beach_sand_instance;
    mesh_instance_t beach_water_instance;
};

void state_init(state_t* state) {
    bzero(state, sizeof(state_t));

    u8* blank_tex_data = malloc(8 * 8 * 2 + 16);
    memcpy(blank_tex_data, "NRGB", 4);
    *(u16*)(blank_tex_data + 4) = 8;
    *(u16*)(blank_tex_data + 6) = 8;
    *(u8*)(blank_tex_data + 8) = 0;
    memset(blank_tex_data + 16, 0xFF, 8 * 8 * 2);

    state->blank_tex = texture_load(blank_tex_data, 8 * 8 * 2 + 16);

    if (!state->blank_tex.handle) {
        iprintf("Error: could not load blank texture\n");
        return;
    }

#define STANDARD_TEXTURE_LOAD(name)                                                            \
    state->name##_tex = texture_load(name##_nrgb, name##_nrgb_size);                           \
                                                                                               \
    if (!state->name##_tex.handle) {                                                           \
        iprintf("Error: could not load " #name " texture\n");                                  \
        return;                                                                                \
    }

    STANDARD_TEXTURE_LOAD(molebo);
    STANDARD_TEXTURE_LOAD(molebo_eye);
    STANDARD_TEXTURE_LOAD(sand);
    STANDARD_TEXTURE_LOAD(water);

#undef STANDARD_TEXTURE_LOAD

    mesh_load_nmsh(&state->molebo_mesh, mole_nmsh, mole_nmsh_size);
    state->molebo_mesh.lit = true;
    mesh_load_nmsh(&state->molebo_eye_mesh, mole_eyes_nmsh, mole_eyes_nmsh_size);
    state->molebo_eye_mesh.lit = true;
    mesh_load_nmsh(&state->beach_sand_mesh, beach_sand_nmsh, beach_sand_nmsh_size);
    mesh_load_nmsh(&state->beach_water_mesh, beach_water_nmsh, beach_water_nmsh_size);

    state->molebo_instance = (mesh_instance_t) {
        .mesh = &state->molebo_mesh,
        .transform = {
            .position = { -2.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = state->molebo_tex.handle,
        .poly_fmt = POLY_ALPHA(31)
            | POLY_CULL_BACK
            | POLY_ID(0b001000)
            | POLY_FORMAT_LIGHT0
            | POLY_FORMAT_LIGHT1
            | POLY_TOON_HIGHLIGHT,
    };

    state->molebo_eye_instance = (mesh_instance_t){
        .mesh = &state->molebo_eye_mesh,
        .transform = state->molebo_instance.transform,
        .texture = state->molebo_eye_tex.handle,
        .poly_fmt = state->molebo_instance.poly_fmt,
    };

    state->quad_instance = (mesh_instance_t) {
        .mesh = &quad_mesh,
        .transform = {
            .position = { 3.0f, 0.0f, -6.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 2.0f, 2.0f, 2.0f },
        },
        .texture = state->molebo_tex.handle,
        .poly_fmt = POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0),
    };

    state->beach_sand_instance = mesh_instance_create(
        &state->beach_sand_mesh,
        state->sand_tex.handle,
        POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0)
    );

    state->beach_sand_instance.transform.position[2] = 10.0f;

    state->beach_sand_instance.transform.scale[0] = 16.0f;
    state->beach_sand_instance.transform.scale[1] = 16.0f;
    state->beach_sand_instance.transform.scale[2] = 16.0f;

    state->beach_water_instance = mesh_instance_create(
        &state->beach_water_mesh,
        state->water_tex.handle,
        POLY_ALPHA(15) | POLY_CULL_NONE | POLY_ID(0)
    );

    state->beach_water_instance.transform = state->beach_sand_instance.transform;
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
    glLight(0, RGB15(16, 16, 16), floattov10(-0.8), 0, floattov10(-0.6));
    glLight(1, RGB15(16, 16, 16), 0, floattov10(-1.0), 0);
    glMaterialf(GL_AMBIENT, RGB15(8, 8, 8));
    glMaterialf(GL_DIFFUSE, RGB15(24, 24, 24));
    glMaterialf(GL_SPECULAR, RGB15(0, 0, 0));
    glMaterialf(GL_EMISSION, RGB15(0, 0, 0));
    glSetToonTableRange(0, 16, RGB15(18, 16, 16));
    glSetToonTableRange(17, 24, RGB15(28, 26, 26));
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

    texture_free(state.molebo_tex);
    texture_free(state.molebo_eye_tex);
    texture_free(state.blank_tex);

    return 0;
}

void draw_top_3d_scene(const state_t* state) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, 256.0 / 192.0, 0.1, 100);
    glRotateX(state->camera.rotation[0]);
    glRotateY(state->camera.rotation[1]);
    glRotateZ(state->camera.rotation[2]);
    glTranslatef(
        state->camera.position[0],
        state->camera.position[1],
        state->camera.position[2]
    );

    mesh_instance_draw(&state->beach_sand_instance);
    mesh_instance_draw(&state->quad_instance);
    mesh_instance_draw(&state->molebo_instance);
    mesh_instance_draw(&state->molebo_eye_instance);
    glPolyFmt(POLY_ALPHA(20) | POLY_CULL_NONE);
    mesh_instance_draw(&state->beach_water_instance);
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
}

void draw_bottom_screen(const state_t* state) {}

int update(state_t* state) {
    scanKeys();

    int keys = keysHeld();

    if (keys & KEY_START)
        return 1;

    bool left = keys & KEY_LEFT;
    bool right = keys & KEY_RIGHT;
    bool up = keys & KEY_UP;
    bool down = keys & KEY_DOWN;
    bool a = keys & KEY_A;
    bool b = keys & KEY_B;
    bool x = keys & KEY_X;
    bool y = keys & KEY_Y;

    bool l = keys & KEY_L;
    bool r = keys & KEY_R;

    if (left)
        state->camera.position[0] -= 0.1f;
    if (right)
        state->camera.position[0] += 0.1f;
    if (up)
        state->camera.position[2] += 0.1f;
    if (down)
        state->camera.position[2] -= 0.1f;

    if (l)
        state->camera.position[1] += 0.1f;
    if (r)
        state->camera.position[1] -= 0.1f;

    if (a) // look right
        state->camera.rotation[1] -= 2.0f;
    if (b) // look down
        state->camera.rotation[0] += 2.0f;
    if (x) // look up
        state->camera.rotation[0] -= 2.0f;
    if (y) // look left
        state->camera.rotation[1] += 2.0f;

    if (state->camera.rotation[0] > 87.0f)
        state->camera.rotation[0] = 87.0f;
    if (state->camera.rotation[0] < -87.0f)
        state->camera.rotation[0] = -87.0f;
    if (state->camera.rotation[1] > 360.0f)
        state->camera.rotation[1] -= 360.0f;
    if (state->camera.rotation[1] < 0.0f)
        state->camera.rotation[1] += 360.0f;

    if (state->quad_instance.texture > state->blank_tex.handle)
        state->quad_instance.texture = state->molebo_tex.handle;

    if (state->quad_instance.texture < state->molebo_tex.handle)
        state->quad_instance.texture = state->blank_tex.handle;

    state->molebo_instance.transform.rotation[1] += 1.0f;

    state->molebo_eye_instance.transform = state->molebo_instance.transform;

    return 0;
}
