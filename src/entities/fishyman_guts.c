#include <stdlib.h>

#include "fishyman_guts.h"
#include <raymath.h>

void fishyman_guts_update(Entity *entity, float delta) {
    FishyManGutsData *data = entity->custom_data;
    if (data->angular_velocity > 0.0f) {
        data->angular_velocity -= 3.0f * delta;
    } else {
        data->angular_velocity += 3.0f * delta;
    }
    entity->velocity = Vector2Subtract(entity->velocity, Vector2Scale(Vector2One(), delta * 40.0f));
    entity->rotation += data->angular_velocity * delta;
    entity->position = Vector2Add(entity->position, Vector2Scale(entity->velocity, delta));
}
FishyManGutsData *fishyman_guts_data_new(float min_vel, float max_vel) {
    FishyManGutsData *data = malloc(sizeof(FishyManGutsData));
    data->angular_velocity = (max_vel - min_vel) * ((float)rand() / (float)RAND_MAX) + min_vel;
    data->angular_velocity *= (rand() & 1) ? -1.0f : 1.0f;
    return data;
}
