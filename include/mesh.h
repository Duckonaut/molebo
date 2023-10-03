#pragma once

#include "types.h"
#include "texture.h"

typedef struct vertex {
    vec3v position;
    rgb color;
    u32 normal;
    vec2t texcoord;
} vertex_t;

typedef struct mesh {
    vertex_t* vertices;
    u16* indices;
    usize vert_len;
    usize index_len;
    GL_GLBEGIN_ENUM mode;
    bool compressed;
    bool lit;
} mesh_t;

typedef struct transform {
    vec3 position;
    vec3i rotation;
    vec3 scale;
} transform_t;

typedef struct mesh_instance {
    const mesh_t* mesh;
    transform_t transform;
    texture_handle texture;
    u32 poly_fmt;
} mesh_instance_t;

void mesh_load_nmsh(mesh_t* mesh, const u8* data, usize datalen);
void mesh_draw(const mesh_t* mesh);
void mesh_free(mesh_t* mesh);

mesh_instance_t
mesh_instance_create(const mesh_t* mesh, texture_handle texture, u32 poly_fmt);
void mesh_instance_draw(const mesh_instance_t* mesh_instance);
