#include <stdlib.h>

#include "bubble.h"
#include "bubble_spawner.h"
#include "../game.h"
#include <raymath.h>

void bubble_spawner_update(Entity *entity, float delta) {
    BubbleSpawnerData *data = entity->custom_data;

    data->timeleft -= delta;
    if (data->timeleft <= 0.0f) {
        data->timeleft = 8.0f * ((float)rand() / (float)RAND_MAX);
        
        // Spawn a bubble
        Entity *bubble = game_spawn_entity(ENTITY_PRESET_BUBBLE, Vector2AddValue(entity->position, 4.0f));
        bubble->custom_data = bubble_data_new(
            2.5f, 5.0f,
            1.0f, 8.0f,
            -8.0f, 8.0f,
            8.0f, 24.0f,
            1.0f
        );
    }
}
BubbleSpawnerData *bubble_spawner_data_new(void) {
    BubbleSpawnerData *data = calloc(1, sizeof(BubbleSpawnerData));
    data->timeleft = 1.0f;
}
