#include <stdlib.h>

#include "../game.h"
#include "shark.h"
#include "../world.h"
#include <raymath.h>

void accel_to_point(Entity *entity, float delta, Vector2 target, float accel, float max_speed) {
    if (entity->position.x < target.x) {
        entity->velocity.x += accel * delta;
    } else if (entity->position.x > target.x) {
        entity->velocity.x -= accel * delta;
    }
    entity->velocity.x = fmaxf(fminf(entity->velocity.x, max_speed), -max_speed);

    if (entity->position.y < target.y) {
        entity->velocity.y += accel * delta;
    } else if (entity->position.y > target.y) {
        entity->velocity.y -= accel * delta;
    }
    entity->velocity.y = fmaxf(fminf(entity->velocity.y, max_speed), -max_speed);

}

void shark_update(Entity *entity, float delta) {
    SharkData *data = entity->custom_data;
    float max_speed = 8.0f;

    switch (data->state) {
        case SHARK_STATE_ROAM:
        data->timer -= delta;
        if (data->timer <= 0.0f) {
            data->timer = 3.0f;

            float dir = (float)rand() / (float)RAND_MAX * 360.0f;
            float len = (float)rand() / (float)RAND_MAX * data->home_size;

            data->target.x = data->home.x + cosf(dir) * len;
            data->target.y = data->home.y + sinf(dir) * len;
        }
        if (entity_lineofsight(entity, global_player)) {
            data->state = SHARK_STATE_CHASE;
            data->speed = -35.0f;
        }
        accel_to_point(entity, delta, data->target, 4.0f, 8.0f);
        break;
        case SHARK_STATE_CHASE:
        max_speed = data->max_chase_speed / 2.0f;
        data->speed += data->max_chase_speed * 1.75f * delta * (data->speed < 0.0f ? 2.0f : 1.0f);
        if (data->speed > data->max_chase_speed) {
            data->speed = data->max_chase_speed;
        }
        float dir = atan2f(global_player->position.y - entity->position.y, global_player->position.x - entity->position.x);
        entity->velocity.x = cosf(dir) * data->speed;
        entity->velocity.y = sinf(dir) * data->speed;

        if (!entity_lineofsight(entity, global_player)) {
            data->seen_timer += delta;
            if (data->seen_timer > 0.5f) {
                data->state = SHARK_STATE_FOLLOW;
                data->timer = 5.0f;
                data->target = global_player->position;
            }
        } else {
            data->seen_timer = 0.0f;
        }
        break;
        case SHARK_STATE_FOLLOW:
        data->timer -= delta;
        max_speed = Vector2Length((Vector2){ .x = data->max_chase_speed / 1.33f, .y = data->max_chase_speed / 1.33f });
        accel_to_point(entity, delta, data->target, data->max_chase_speed / 1.5f, data->max_chase_speed / 1.33f);
        
        if (data->stationary_timer > 0.5f) {
            float xdist = data->target.x - entity->position.x;
            float ydist = data->target.y - entity->position.y;
            float detecty = (entity->radius + 4.0f) * (ydist > 0.0f ? 1.0f : -1.0f);
            float detectx = (entity->radius + 4.0f) * (xdist > 0.0f ? 1.0f : -1.0f);
            if (fabsf(ydist) > fabsf(xdist) && world_point_colliding(Vector2Add(entity->position, (Vector2){ .x = 0.0f, .y = detecty }))) {
                if (xdist > 0.0f) {
                    entity->velocity.x -= data->max_chase_speed * 2.0f;
                } else {
                    entity->velocity.x += data->max_chase_speed * 2.0f;
                }
            } else if (fabsf(xdist) > fabsf(ydist) && world_point_colliding(Vector2Add(entity->position, (Vector2){ .x = detectx, .y = 0.0f }))) {
                if (ydist > 0.0f) {
                    entity->velocity.y -= data->max_chase_speed * 2.0f;
                } else {
                    entity->velocity.y += data->max_chase_speed * 2.0f;
                }
            }
        }
        
        if (data->timer < 0.0f || Vector2Distance(entity->position, data->target) < entity->radius + 4.0f ||
            data->stationary_timer > 2.0f) {
            data->state = SHARK_STATE_ROAM;
            data->home = entity->position;
            data->timer = 0.0f;
        }
        if (entity_lineofsight(entity, global_player)) {
            data->state = SHARK_STATE_CHASE;
            data->speed = -35.0f;
        }
        break;
    }

    if (Vector2Length(entity->velocity) < max_speed / 3.0f) {
        data->stationary_timer += delta;
    } else {
        data->stationary_timer = 0.0f;
    }

    // Clip to bounds
    entity_bound_to_level(entity, 4.0f);

    // Rotate and flip sprite
    if (data->state != SHARK_STATE_CHASE) {
        entity->flipx = (entity->velocity.x < 0.0f) ? true : false;
    } else {
        entity->flipx = (global_player->position.x < entity->position.x) ? true : false;
    }
    float target = fabsf(entity->velocity.y / max_speed) < 0.2f ? 0.0f : (entity->velocity.y / max_speed);
    data->rotation = (target - data->rotation) * 32.0f * delta + data->rotation;
    if (fabsf(data->rotation - target) < 0.01f)
        data->rotation = target;
    entity->rotation = data->rotation * 20.0f * (entity->flipx ? -1.0f : 1.0f);

    if (data->state != SHARK_STATE_CHASE || data->speed < data->max_chase_speed / 3.0f) {
        float speed_percent = 
            Vector2Length(entity->velocity) /
            Vector2Length((Vector2){ .x = data->max_chase_speed, .y = data->max_chase_speed });
        entity->animation.speed_scale = speed_percent * 4.0f + 1.0f;
    } else {
        entity->animation.speed_scale = 10.0f;
    }
}
SharkData *shark_data_new(Vector2 spawn_pos) {
    SharkData *data = malloc(sizeof(SharkData));
    data->home = spawn_pos;
    data->last_seen_pos = spawn_pos;
    data->last_seen_dir = Vector2Zero();
    data->home_size = 12.0f;
    data->max_chase_speed = 64.0f;// * 1.04f;
    data->speed = 0.0f;
    data->timer = 0.0f;
    data->target = spawn_pos;
    data->state = SHARK_STATE_ROAM;
    data->rotation = 0.0f;
    data->seen_timer = 0.0f;
    return data;
}
