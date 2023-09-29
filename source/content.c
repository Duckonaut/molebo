#include "content.h"

#include "molebo_nrgb.h"
#include "molebo_eye_nrgb.h"
#include "molebo_gun_nrgb.h"
#include "mole_nmsh.h"
#include "mole_eyes_nmsh.h"
#include "quad_nmsh.h"
#include "beach_sand_nmsh.h"
#include "sand_nrgb.h"
#include "beach_water_nmsh.h"
#include "stdio.h"
#include "water_nrgb.h"

#include "mesh.h"
#include "texture.h"

void content_load(content_t* content) {
    u8* blank_tex_data = malloc(8 * 8 * 2 + 16);
    memcpy(blank_tex_data, "NRGB", 4);
    *(u16*)(blank_tex_data + 4) = 8;
    *(u16*)(blank_tex_data + 6) = 8;
    *(u8*)(blank_tex_data + 8) = 0;
    memset(blank_tex_data + 16, 0xFF, 8 * 8 * 2);

    content->blank_tex = texture_load(blank_tex_data, 8 * 8 * 2 + 16);

    if (!content->blank_tex.handle) {
        iprintf("Error: could not load blank texture\n");
        return;
    }

#define STANDARD_TEXTURE_LOAD(name)                                                            \
    content->name##_tex = texture_load(name##_nrgb, name##_nrgb_size);                         \
                                                                                               \
    if (!content->name##_tex.handle) {                                                         \
        iprintf("Error: could not load " #name " texture\n");                                  \
        return;                                                                                \
    }

    STANDARD_TEXTURE_LOAD(molebo);
    STANDARD_TEXTURE_LOAD(molebo_eye);
    STANDARD_TEXTURE_LOAD(molebo_gun);
    STANDARD_TEXTURE_LOAD(sand);
    STANDARD_TEXTURE_LOAD(water);

#undef STANDARD_TEXTURE_LOAD

    mesh_load_nmsh(&content->molebo_mesh, mole_nmsh, mole_nmsh_size);
    content->molebo_mesh.lit = true;
    mesh_load_nmsh(&content->molebo_eye_mesh, mole_eyes_nmsh, mole_eyes_nmsh_size);
    content->molebo_eye_mesh.lit = true;
    mesh_load_nmsh(&content->beach_sand_mesh, beach_sand_nmsh, beach_sand_nmsh_size);
    mesh_load_nmsh(&content->beach_water_mesh, beach_water_nmsh, beach_water_nmsh_size);
}

void content_unload(content_t* content) {
    texture_free(content->blank_tex);
    texture_free(content->molebo_tex);
    texture_free(content->molebo_eye_tex);
    texture_free(content->sand_tex);
    texture_free(content->water_tex);

    mesh_free(&content->molebo_mesh);
    mesh_free(&content->molebo_eye_mesh);
    mesh_free(&content->beach_sand_mesh);
    mesh_free(&content->beach_water_mesh);
}
