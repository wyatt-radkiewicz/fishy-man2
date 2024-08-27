#include <stdlib.h>
#include <math.h>

#include "bubble.h"
#include "doughnut.h"
#include "../entity.h"
#include "fishyman.h"
#include "fishyman_guts.h"
#include "../game.h"
#include <raymath.h>
#include "../world.h"

#define SPEED_MULTIPLIER 2.0f
#define MAX_SPEED (8.0f * 4.0f * SPEED_MULTIPLIER * 1.25f)

static void try_transition(Entity *entity, bool vertical, float dir);

void fishyman_update(Entity *entity, float delta) {
    FishyManData *data = entity->custom_data;

    if (death_state > 0) {
        entity->velocity = Vector2Zero();
        entity->scale -= delta * (DEATH_DURATION * 6.0f);
        if (entity->scale < 0.0f) {
            entity->scale = 0.0f;
        }
        return;
    }

    if (in_transition) {
        if (data->is_mega) {
            data->is_mega = false;
            data->base_radius = 3.0f;
            data->speed = MAX_SPEED;
            animation_new(&entity->animation, ANIM_FISHYMAN_NORMAL);
        }
        camera.position = Vector2Add(camera.position, Vector2Scale(Vector2Subtract(data->camera_transition_target, camera.position), 20.0f * delta));
        entity->position = Vector2Add(entity->position, Vector2Scale(data->transition_movement_dir, delta * 32.0f));
        return;
    }

    if (IsKeyDown(KEY_RIGHT)) {
        entity->velocity.x += data->accel * (entity->velocity.x < 0.0f ? 2.0f : 1.0f) * delta;
        entity->flipx = false;
    } else if (IsKeyDown(KEY_LEFT)) {
        entity->velocity.x -= data->accel * (entity->velocity.x > 0.0f ? 2.0f : 1.0f) * delta;
        entity->flipx = true;
    } else {
        if (entity->velocity.x > data->decel * delta) {
            entity->velocity.x -= data->decel * delta;
        } else if (entity->velocity.x < -data->decel * delta) {
            entity->velocity.x += data->decel * delta;
        } else {
            entity->velocity.x = 0.0f;
        }
    }

    if (IsKeyDown(KEY_DOWN)) {
        entity->velocity.y += data->accel * (entity->velocity.y < 0.0f ? 2.0f : 1.0f) * delta;
    } else if (IsKeyDown(KEY_UP)) {
        entity->velocity.y -= data->accel * (entity->velocity.y > 0.0f ? 2.0f : 1.0f) * delta;
    } else {
        if (entity->velocity.y > data->decel * delta) {
            entity->velocity.y -= data->decel * delta;
        } else if (entity->velocity.y < -data->decel * delta) {
            entity->velocity.y += data->decel * delta;
        } else {
            entity->velocity.y = 0.0f;
        }
    }

    float speed_percent = 
        Vector2Length(entity->velocity) /
        Vector2Length((Vector2){ .x = data->speed, .y = data->speed });
    {
        float target = fabsf(entity->velocity.y / data->speed) < 0.2f ? 0.0f : (entity->velocity.y / data->speed);
        data->rotation = (target - data->rotation) * 32.0f * delta + data->rotation;
        if (fabsf(data->rotation - target) < 0.01f)
            data->rotation = target;
        entity->rotation = data->rotation * 10.0f * (entity->flipx ? -1.0f : 1.0f);
    }
    entity->animation.speed_scale = speed_percent * 1.5f + 1.0f;
    entity->velocity.x = fminf(data->speed, fmaxf(entity->velocity.x, -data->speed));
    entity->velocity.y = fminf(data->speed, fmaxf(entity->velocity.y, -data->speed));

    // Spawn bubbles
    data->bubble_timer -= delta;
    if (Vector2Length(entity->velocity) < 8.0f) {
        data->bubble_timer = 1.0f;
    } else if (data->bubble_timer <= 0.0f) {
        data->bubble_timer = fmaxf(1.0f - speed_percent, 0.3f);
        
        // Spawn bubble
        Entity *bubble = game_spawn_entity(ENTITY_PRESET_BUBBLE, entity->position);
        bubble->custom_data = bubble_data_new(
            0.4f, 1.3f,
            1.0f, 8.0f,
            -entity->velocity.x / SPEED_MULTIPLIER, -entity->velocity.x / SPEED_MULTIPLIER,
            30.0f, 60.0f,
            fabsf(entity->velocity.x)
        );
    }

    // Collect doughnuts
    Entity *doughnuts[32];
    int num_doughnuts = game_find_colliding_entities(doughnuts, 32, entity, false);
    for (int i = 0; i < num_doughnuts; i++) {
        Entity *ent = doughnuts[i];

        if (ent->original_preset == ENTITY_PRESET_DOUGHNUT) {
            doughnut_eat(ent);
        }

        if (ent->original_preset == ENTITY_PRESET_MEGA_DOUGHNUT) {
            doughnut_eat(ent);
            data->is_mega = true;
            data->base_radius = 6.0f;
            data->speed = MAX_SPEED * 2.0f;
            animation_new(&entity->animation, ANIM_FISHYMAN_MEGA);
        }
    }

    // Camera
    camera.position = Vector2Add(camera.position, Vector2Scale(Vector2Subtract(entity->position, camera.position), 25.0f * delta));
    camera_clip_to_level(&camera);

    // Die if we are next to shark!!!
    entity->radius = data->base_radius + 0.1f;
    Entity *enemies[32];
    int num_enemies = game_find_colliding_entities(enemies, 32, entity, false);
    for (int i = 0; i < num_enemies; i++) {
        if (enemies[i]->original_preset == ENTITY_PRESET_SHARK ||
            enemies[i]->original_preset == ENTITY_PRESET_GHOST_SHARK) {
            start_death();

            // Spawn guts
            float guts_dir = atan2f(entity->position.y - enemies[i]->position.y, entity->position.x - enemies[i]->position.x);
            float spread = 45.0f / 180.0f * PI;
            for (int j = 0; j < 4; j++) {
                float dir = guts_dir + spread * ((float)rand() / (float)RAND_MAX - 0.5f);
                Entity *gut = game_spawn_entity(ENTITY_PRESET_FISHYMAN_GUTS, entity->position);
                gut->custom_data = fishyman_guts_data_new(50.0f, 720.0f);
                gut->velocity.x = cosf(dir) * 20.0f;
                gut->velocity.y = sinf(dir) * 20.0f;
            }

            break;
        }
    }
    entity->radius = data->base_radius;

    // Be level bound.
    entity_bound_to_level(entity, 0.0f);

    // Start level transitions
    try_transition(entity, false, 1.0f);
    try_transition(entity, false, -1.0f);
    try_transition(entity, true, 1.0f);
    try_transition(entity, true, -1.0f);
}

static void try_transition(Entity *entity, bool vertical, float dir) {
    FishyManData *data = entity->custom_data;

    if (vertical) {
        if (entity->position.y + dir * 5.0f > (float)current_level->worldY + (float)current_level->pxHei ||
            entity->position.y + dir * 5.0f < (float)current_level->worldY) {
            for (int i = 0; i < current_level->numNeighbors; i++) {
                    struct levels *level = getLevelFromUid((float)current_level->neighbors[i].uid);
                    if (current_level_num <= world_get_level_num(level) &&
                        entity->position.x > (float)level->worldX &&
                        entity->position.x < (float)level->worldX + (float)level->pxWid &&
                        entity->position.y + dir * 16.0f > (float)level->worldY &&
                        entity->position.y + dir * 16.0f < (float)level->worldY + (float)level->pxHei) {
                        if (dir > 0.0f) {
                            data->camera_transition_target.y = (float)level->worldY + camera.dimen.y * camera.scale / 2.0f;
                        } else {
                            data->camera_transition_target.y = (float)level->worldY + (float)level->pxHei - camera.dimen.y * camera.scale / 2.0f;
                        }
                        data->camera_transition_target.x = camera.position.x;
                        entity->velocity = Vector2Zero();
                        data->transition_movement_dir = (Vector2){
                            .x = 0.0f, .y = dir,
                        };
                        world_start_transition(level->uid);
                        break;
                    }
            }
        }
    } else {
        if (entity->position.x + dir * 5.0f > (float)current_level->worldX + (float)current_level->pxWid ||
            entity->position.x + dir * 5.0f < (float)current_level->worldX) {
            for (int i = 0; i < current_level->numNeighbors; i++) {
                    struct levels *level = getLevelFromUid((float)current_level->neighbors[i].uid);
                    if (current_level_num <= world_get_level_num(level) &&
                        entity->position.x + dir * 16.0f > (float)level->worldX &&
                        entity->position.x + dir * 16.0f < (float)level->worldX + (float)level->pxWid &&
                        entity->position.y > (float)level->worldY &&
                        entity->position.y < (float)level->worldY + (float)level->pxHei) {
                        if (dir > 0.0f) {
                            data->camera_transition_target.x = (float)level->worldX + camera.dimen.x * camera.scale / 2.0f;
                        } else {
                            data->camera_transition_target.x = (float)level->worldX + (float)level->pxWid - camera.dimen.x * camera.scale / 2.0f;
                        }
                        data->camera_transition_target.y = camera.position.y;
                        entity->velocity = Vector2Zero();
                        data->transition_movement_dir = (Vector2){
                            .x = dir, .y = 0.0f,
                        };
                        world_start_transition(level->uid);
                        break;
                    }
            }
        }
    }
}

FishyManData *fishyman_data_new(void) {
    FishyManData *data = malloc(sizeof(FishyManData));
    data->speed = MAX_SPEED;
    data->accel = data->speed * 8.0f * SPEED_MULTIPLIER;
    data->decel = data->speed * 4.0f * SPEED_MULTIPLIER;
    data->bubble_timer = 0.0f;
    data->rotation = 0.0f;
    data->is_mega = false;
    data->base_radius = 3.0f;

    return data;
}
