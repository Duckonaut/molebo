#include "mesh.h"
#include "nds/arm9/videoGL.h"
#include "nds/ndstypes.h"
#include "stdio.h"

#include "tinf.h"

void mesh_load_nmsh(mesh_t* mesh, const u8* data, usize datalen) {
    // validate magic (NMSH)
    const u8 magic[4] = { 'N', 'M', 'S', 'H' };

    if (memcmp(data, magic, 4) != 0) {
        iprintf("Error: invalid magic\n");
        return;
    }

    mesh->mode = GL_TRIANGLES;
    mesh->vert_len = *(u16*)(data + 4);
    mesh->index_len = *(u16*)(data + 6);
    mesh->compressed = *(u8*)(data + 8);

    if (mesh->compressed) {
        size_t expected_len = mesh->vert_len * sizeof(vertex_t) + mesh->index_len * sizeof(u16);

        u8* decompressed_data = malloc(expected_len);

        // compressed source
        u8* source = (u8*)data + 16;
        size_t outlen = expected_len;

        iprintf("compressed: %u\n", datalen);

        // decompress using tinf
        int res = tinf_gzip_uncompress(decompressed_data, &outlen, source, datalen - 16);

        if ((res != TINF_OK) || (outlen != expected_len)) {
            iprintf("Error: failed to decompress texture\n");
            iprintf("res: %d\n", res);

            return;
        }

        iprintf("decompressed: %u\n", outlen);

        mesh->vertices = (vertex_t*)decompressed_data;
        mesh->indices = (u16*)(decompressed_data + mesh->vert_len * sizeof(vertex_t));
    } else {
        mesh->vertices = (vertex_t*)(data + 16);
        mesh->indices = (u16*)(data + 16 + mesh->vert_len * sizeof(vertex_t));
    }
}

void mesh_draw(const mesh_t* mesh) {
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(mesh->mode);
    for (usize i = 0; i < mesh->index_len; i++) {
        vertex_t* vert = &mesh->vertices[mesh->indices[i]];
        glColor(vert->color);
        glTexCoord2t16(vert->texcoord[0], vert->texcoord[1]);
        glVertex3v16(vert->position[0], vert->position[1], vert->position[2]);
    }
    glEnd();
}

mesh_instance_t mesh_instance_create(const mesh_t* mesh, texture_handle texture) {
    return (mesh_instance_t) {
        .mesh = mesh,
        .transform = {
            .position = { 0.0f, 0.0f, 0.0f },
            .rotation = { 0.0f, 0.0f, 0.0f },
            .scale = { 1.0f, 1.0f, 1.0f },
        },
        .texture = texture
    };
}

void mesh_instance_draw(const mesh_instance_t* mesh_instance) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(
        mesh_instance->transform.position[0],
        mesh_instance->transform.position[1],
        mesh_instance->transform.position[2]
    );
    glRotatef(mesh_instance->transform.rotation[0], 1.0f, 0.0f, 0.0f);
    glRotatef(mesh_instance->transform.rotation[1], 0.0f, 1.0f, 0.0f);
    glRotatef(mesh_instance->transform.rotation[2], 0.0f, 0.0f, 1.0f);
    glScalef(
        mesh_instance->transform.scale[0],
        mesh_instance->transform.scale[1],
        mesh_instance->transform.scale[2]
    );
    glBindTexture(0, mesh_instance->texture);
    mesh_draw(mesh_instance->mesh);
}
