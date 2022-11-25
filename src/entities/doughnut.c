#include <math.h>
#include <stdlib.h>
#include <raylib.h>

#include "../animation.h"
#include "doughnut.h"
#include "../game.h"
#include "../world.h"
#include <raymath.h>

static float doughnut_offset = 0.0f;
static int doughnut_frame_offset = 0;

void doughnut_update(Entity *entity, float delta) {
    DoughnutData *data = entity->custom_data;

    if (data->inherit_velocity) {
        data->inherited_velocity.y += 20.0f * delta;
        entity->position = Vector2Add(entity->position, Vector2Scale(data->inherited_velocity, delta));
        data->inherit_timer -= delta;
    } else {
        entity->position.y = data->original_y + sinf(data->timer * 3.0f);
        data->timer += delta;
    }

    if (!data->inited_frame) {
        if (data->is_mega_doughnut) {
            animation_new(&entity->animation, ANIM_DOUGHNUT_MEGA);
            entity->radius = 8.0f;
        }

        entity->animation.frame = doughnut_frame_offset % entity->animation.config->frames;
        doughnut_frame_offset += 1;
        data->inited_frame = true;
    }

    if (data->eaten || (data->inherit_velocity && data->inherit_timer < 0.0f)) {
        data->eaten_timer += delta;
        entity->scale -= delta * 1.0f;
        entity->tint.a = (uint8_t)fminf((1.1f - data->eaten_timer * 2.0f) * 255.0f, 255.0f);

        if (data->eaten_timer >= 0.5f) {
            game_despawn_entity(entity);
        }
    }
}
DoughnutData *doughnut_data_new(float y_pos, bool mega_doughnut) {
    DoughnutData *data = calloc(1, sizeof(DoughnutData));
    data->original_y = y_pos;
    data->eaten_timer = 0.0f;
    data->timer = doughnut_offset;
    data->inited_frame = false;
    data->inherit_velocity = false;
    data->inherited_velocity = Vector2Zero();
    data->inherit_timer = 3.0f;
    data->is_mega_doughnut = mega_doughnut;
    doughnut_offset += 0.5f;
    return data;
}
void doughnut_eat(Entity *doughnut) {
    DoughnutData *data = doughnut->custom_data;
    if (!data->eaten) {
        current_doughnuts_left -= 1;
        if (data->is_mega_doughnut) {
            current_doughnuts_left = 0;
        }
        if (current_doughnuts_left <= 0) {
            current_doughnuts_left = 0;
            world_unlock_doors();
        }
    }
    data->eaten = true;
}
