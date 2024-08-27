#include <stdlib.h>

#include "../game.h"
#include "ghost_shark.h"
#include <raymath.h>

void ghost_shark_update(Entity *entity, float delta) {
    GhostSharkData *data = entity->custom_data;

    if (data->speed < data->max_speed) {
        data->speed += delta * data->max_speed;
    }
    float dir = atan2f(global_player->position.y - entity->position.y, global_player->position.x - entity->position.x);
    entity->velocity.x = cosf(dir) * data->speed;
    entity->velocity.y = sinf(dir) * data->speed;

    entity_bound_to_level(entity, 4.0f);

    // Rotate and flip sprite
    entity->flipy = (entity->velocity.x < 0.0f) ? true : false;
    entity->rotation = dir / PI * 180.0f;
}

GhostSharkData *ghost_shark_data_new(void) {
    GhostSharkData *data = malloc(sizeof(GhostSharkData));
    data->max_speed = 42.0f;
    data->speed = 0.0f;
    return data;
}
