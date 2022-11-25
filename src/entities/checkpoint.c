#include <stdlib.h>

#include "checkpoint.h"
#include "doughnut.h"
#include "../game.h"
#include "../world.h"
#include <raymath.h>

void checkpoint_update(Entity *entity, float delta) {
    CheckpointData *data = entity->custom_data;

    Entity **iter = entities;
    Entity **player = game_find_next_entity_of_preset(iter, ENTITY_PRESET_FISHYMAN);

    if (player && *player && Vector2Distance(entity->position, (*player)->position) <
        entity->radius * entity->scale + (*player)->radius * (*player)->scale) {
        if (last_level_uid_checkpoint != current_level_uid) {
            data->happy_timer = 3.5f;
            data->spawn_timer = 0.15f;
        }
        last_level_uid_checkpoint = current_level_uid;
    }

    if (data->happy_timer > 0.0f) {
        data->happy_timer -= delta;
        data->spawn_timer -= delta;
        if (data->spawn_timer < 0.0f) {
            data->spawn_timer = 0.15f;

            float x = entity->position.x + ((float)rand() / (float)RAND_MAX - 0.5f) * 80.0f;
            Entity *doughnut = game_spawn_entity(ENTITY_PRESET_DOUGHNUT, (Vector2){ .x = x, .y = entity->position.y - 12.0f });
            DoughnutData *doughnut_data = doughnut->custom_data;
            doughnut_data->inherit_velocity = true;
            doughnut_data->inherited_velocity = (Vector2) {
                .x = ((float)rand() / (float)RAND_MAX - 0.5f) * 8.0f,
                .y = -(((float)rand() / (float)RAND_MAX) * 10.0f + 10.0f)
            };
        }
        entity->animation.speed_scale = 10.0f;
    } else {
        entity->animation.speed_scale = 1.0f;
    }
}
CheckpointData *checkpoint_data_new(void) {
    CheckpointData *data = malloc(sizeof(CheckpointData));
    data->happy_timer = 0.0f;
    data->spawn_timer = 0.0f;
    return data;
}
