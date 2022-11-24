#include <math.h>
#include <stdlib.h>
#include <raylib.h>

#include "doughnut.h"
#include "../game.h"
#include "../world.h"

static float doughnut_offset = 0.0f;

void doughnut_update(Entity *entity, float delta) {
    DoughnutData *data = entity->custom_data;
    entity->position.y = data->original_y + sinf(data->timer * 3.0f);
    data->timer += delta;

    if (data->eaten) {
        data->eaten_timer += delta;
        entity->scale -= delta * 1.0f;
        entity->tint.a = (uint8_t)fminf((1.1f - data->eaten_timer * 2.0f) * 255.0f, 255.0f);

        if (data->eaten_timer >= 0.5f) {
            game_despawn_entity(entity);
        }
    }
}
DoughnutData *doughnut_data_new(float y_pos) {
    DoughnutData *data = calloc(1, sizeof(DoughnutData));
    data->original_y = y_pos;
    data->eaten_timer = 0.0f;
    data->timer = doughnut_offset;
    doughnut_offset += 0.5f;
}
void doughnut_eat(Entity *doughnut) {
    DoughnutData *data = doughnut->custom_data;
    if (!data->eaten) {
        current_doughnuts_left -= 1;
        if (current_doughnuts_left == 0) {
            world_unlock_doors();
        }
    }
    data->eaten = true;
}
