#include "mesh.h"
#include "nds/arm9/videoGL.h"
#include "nds/ndstypes.h"
#include "stdio.h"

void mesh_load_nmsh(mesh_t* mesh, const u8* data) {
    // validate magic (NMSH)
    const u8 magic[4] = { 'N', 'M', 'S', 'H' };

    if (memcmp(data, magic, 4) != 0) {
        iprintf("Error: invalid magic\n");
        return;
    }

    mesh->mode = GL_TRIANGLES;
    mesh->vert_len = *(u16*)(data + 4);
    mesh->index_len = *(u16*)(data + 6);
    mesh->vertices = (vertex_t*)(data + 8);
    mesh->indices = (u16*)(data + 8 + mesh->vert_len * sizeof(vertex_t));
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
