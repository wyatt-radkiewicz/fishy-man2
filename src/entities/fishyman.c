#include <stdlib.h>
#include <math.h>

#include "bubble.h"
#include "../entity.h"
#include "fishyman.h"
#include "../game.h"
#include <raymath.h>
#include "../world.h"

void fishyman_update(Entity *entity, float delta) {
    FishyManData *data = entity->custom_data;

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
    entity->rotation = entity->velocity.y / data->speed * 10.0f * (entity->flipx ? -1.0f : 1.0f);
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
            -entity->velocity.x, -entity->velocity.x,
            30.0f, 60.0f,
            fabsf(entity->velocity.x)
        );
    }

    camera.position = Vector2Add(camera.position, Vector2Scale(Vector2Subtract(entity->position, camera.position), 0.8f));

    // Clip the camera bounds
    if (camera.position.x - camera.dimen.x * camera.scale / 2.0f < (float)current_level->worldX) {
        camera.position.x = (float)current_level->worldX + camera.dimen.x * camera.scale / 2.0f;
    }
    if (camera.position.x + camera.dimen.x * camera.scale / 2.0f > (float)current_level->worldX + (float)current_level->pxWid) {
        camera.position.x = (float)current_level->worldX + (float)current_level->pxWid - camera.dimen.x * camera.scale / 2.0f;
    }
    if (camera.position.y - camera.dimen.y * camera.scale / 2.0f < (float)current_level->worldY) {
        camera.position.y = (float)current_level->worldY + camera.dimen.y * camera.scale / 2.0f;
    }
    if (camera.position.y + camera.dimen.y * camera.scale / 2.0f > (float)current_level->worldY + (float)current_level->pxHei) {
        camera.position.y = (float)current_level->worldY + (float)current_level->pxHei - camera.dimen.y * camera.scale / 2.0f;
    }
    
}
FishyManData *fishyman_data_new(void) {
    FishyManData *data = malloc(sizeof(FishyManData));
    data->speed = 8.0f * 4.0f;
    data->accel = data->speed * 8.0f;
    data->decel = data->speed * 4.0f;

    return data;
}
