#include <stdlib.h>
#include <math.h>

#include "bubble.h"
#include "../game.h"
#include <raymath.h>

void bubble_update(Entity *entity, float delta) {
    BubbleData *data = entity->custom_data;

    data->time_left -= delta;
    if (data->time_left > 0.0f) {
        if (entity->scale < 1.0f) {
            float addition = fminf(data->lifetime * 5.0f, 5.0f);
            entity->scale += addition * delta;
            entity->tint.a = (uint8_t)((data->lifetime - data->time_left) * addition * 255.0f);
        } else {
            entity->scale = 1.0f;
            entity->tint.a = 255;
        }

        if (data->base_vel_x > data->decel * delta) {
            data->base_vel_x -= data->decel * delta;
        } else if (data->base_vel_x < -data->decel * delta) {
            data->base_vel_x += data->decel * delta;
        } else {
            data->base_vel_x = 0.0f;
        }

        entity->velocity.x = data->base_vel_x + (float)sin(GetTime() * 90.0) * data->wiggle;
        entity->velocity.y = fmaxf(entity->velocity.y + data->accel * delta, data->max_vely);
    } else if ((entity->animation.config - animations) != ANIM_BUBBLE_POP) {
        entity->velocity = Vector2Zero();
        animation_new(&entity->animation, ANIM_BUBBLE_POP);
    } else if (entity->animation.times_completed > 0) {
        game_despawn_entity(entity);
    }
}
BubbleData *bubble_data_new(
    float min_lifetime, float max_lifetime,
    float min_wiggle, float max_wiggle,
    float min_velx, float max_velx,
    float min_vely, float max_vely,
    float decel
) {
#define CALC_RAND (float)rand() / (float)RAND_MAX
#define GET_RAND(name) ((max_##name - min_##name) * rand_##name + min_##name)
    float rand_lifetime = CALC_RAND;
    float rand_wiggle = CALC_RAND;
    float rand_velx = CALC_RAND;
    float rand_vely = CALC_RAND;
    float rand_accel = CALC_RAND;
    BubbleData *data = calloc(1, sizeof(BubbleData));

    data->lifetime = GET_RAND(lifetime);
    data->wiggle = GET_RAND(wiggle);
    data->base_vel_x = GET_RAND(velx);
    data->max_vely = -GET_RAND(vely);
    data->accel = -(16.0f * rand_accel + 8.0f);
    data->time_left = data->lifetime;
    data->decel = decel;
}
