#include "projectile.h"

#include "mesh.h"
#include "nds/ndstypes.h"
#include "util.h"
#include "state.h"

#include "nds/arm9/trig_lut.h"

void projectile_init(
    projectile_t* projectile,
    const mesh_t* mesh,
    texture_handle texture,
    float radius,
    float gravity,
    bool friendly,
    const vec3 position,
    const vec3 velocity
) {
    projectile->mesh = mesh;
    projectile->texture = texture;
    projectile->radius = radius;
    projectile->gravity = gravity;
    projectile->friendly = friendly;
    projectile->transform.scale[0] = 1.0f;
    projectile->transform.scale[1] = 1.0f;
    projectile->transform.scale[2] = 1.0f;
    memcpy(projectile->transform.position, position, sizeof(float) * 3);
    memcpy(projectile->velocity, velocity, sizeof(float) * 3);
}

void projectile_fire(
    state_t* state,
    const mesh_t* mesh,
    texture_handle texture,
    float radius,
    float gravity,
    bool friendly,
    const vec3 position,
    const vec3 velocity
) {
    projectile_t* slot = NULL;

    for (u16 i = 0; i < MAX_PROJECTILES; ++i) {
        if (!state->projectiles[i].active) {
            slot = &state->projectiles[i];
            break;
        }
    }

    if (slot) {
        projectile_init(slot, mesh, texture, radius, gravity, friendly, position, velocity);
        slot->active = true;
    }
}

void projectile_update(projectile_t* p) {
    p->transform.position[0] += p->velocity[0];
    p->transform.position[1] += p->velocity[1];
    p->transform.position[2] += p->velocity[2];

    p->velocity[1] -= p->gravity;

    // rotate all 3 axes to face velocity
    // y
    float angle = atan2f(p->velocity[1], p->velocity[0]);
    p->transform.rotation[1] = (i16)(angle * (32768.0f / 2.0f / M_PI));
    // x
    angle = atan2f(p->velocity[2], p->velocity[0]);
    p->transform.rotation[0] = (i16)(angle * (32768.0f / 2.0f / M_PI));
    // z
    angle = atan2f(p->velocity[2], p->velocity[1]);
    p->transform.rotation[2] = (i16)(angle * (32768.0f / 2.0f / M_PI));

    if (p->transform.position[1] < ground_z_to_y(p->transform.position[2])) {
        p->active = false;
    }
}

void projectile_draw(const projectile_t *projectile) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(
        projectile->transform.position[0],
        projectile->transform.position[1],
        projectile->transform.position[2]
    );
    glRotateXi(projectile->transform.rotation[0]);
    glRotateYi(projectile->transform.rotation[1]);
    glRotateZi(projectile->transform.rotation[2]);
    glScalef(
        projectile->transform.scale[0],
        projectile->transform.scale[1],
        projectile->transform.scale[2]
    );

    glBindTexture(GL_TEXTURE_2D, projectile->texture);
    mesh_draw(projectile->mesh);
}
