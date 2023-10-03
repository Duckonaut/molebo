#pragma once

#include "mesh.h"
typedef struct content {
    mesh_t molebo_mesh;
    mesh_t molebo_eye_mesh;
    mesh_t beach_sand_mesh;
    mesh_t beach_water_mesh;
    mesh_t rocket_mesh;

    texture_t blank_tex;
    texture_t molebo_tex;
    texture_t molebo_eye_tex;
    texture_t molebo_gun_tex;
    texture_t sand_tex;
    texture_t water_tex;
} content_t;

void content_load(content_t* content);
void content_unload(content_t* content);
