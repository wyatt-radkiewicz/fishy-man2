#include <stdlib.h>
#include <string.h>

#include "entity.h"
#include "game.h"
#include <raymath.h>

#include "entities/fishyman.h"
#include "entities/bubble.h"
#include "entities/bubble_spawner.h"

static const char *ident_to_preset[ENTITY_PRESET_MAX] = {
    "ENTITY_PRESET_UNKOWN_IDENTIFIER",
    "ENTITY_PRESET_FISHYMAN",
    "ENTITY_PRESET_BUBBLE",
    "ENTITY_PRESET_BUBBLE_SPAWNER",
    "ENTITY_PRESET_SHARK",
    "ENTITY_PRESET_DOUGHNUT",
    "ENTITY_PRESET_GHOST_SHARK",
    "ENTITY_PRESET_MEGA_DOUGHNUT",
    "ENTITY_PRESET_CHECKPOINT",
};

EntityPreset entity_preset_from_identifier(const char *ident) {
    for (int i = 0; i < ENTITY_PRESET_MAX; i++) {
        if (strcmp(ident_to_preset[i], ident) == 0) {
            return i;
        }
    }

    return ENTITY_PRESET_UNKOWN_IDENTIFIER;
}
Entity *entity_preset(EntityPreset preset, Vector2 pos) {
    Entity *entity = malloc(sizeof(Entity));
    
    switch (preset) {
        case ENTITY_PRESET_FISHYMAN:
        entity_new(entity, fishyman_update, pos, ANIM_FISHYMAN_NORMAL, 3.0f, true, fishyman_data_new(), true);
        break;
        case ENTITY_PRESET_BUBBLE:
        entity_new(entity, bubble_update, pos, (rand() & 4) ? ANIM_BUBBLE : ANIM_BUBBLE_SMALL, 0.0f, false, NULL, true);
        entity->tint.a = 0;
        entity->priority = true;
        break;
        case ENTITY_PRESET_BUBBLE_SPAWNER:
        entity_new(entity, bubble_spawner_update, pos, ANIM_NONE, 0.0f, false, bubble_spawner_data_new(), true);
        break;
        default:
        entity_new(entity, NULL, pos, ANIM_NONE, 0.0f, false, NULL, false);
        break;
    }

    entity->original_preset = preset;

    return entity;
}
void entity_new(Entity *entity, EntityUpdateFunc func, Vector2 pos, Animations anim_config, float radius, bool collide_with_others, void *custom_data, bool free_data) {
    entity->position = pos;
    entity->velocity = Vector2Zero();
    entity->scale = 1.0f;
    animation_new(&entity->animation, anim_config);
    entity->radius = radius;
    entity->rotation = 0.0f;
    entity->collide_with_others = collide_with_others;
    entity->update_func = func;
    entity->custom_data = custom_data;
    entity->free_custom_data = free_data;
    entity->flipx = false;
    entity->original_preset = ENTITY_PRESET_UNKOWN_IDENTIFIER;
    entity->level_uid = -1;
    entity->tint = WHITE;
    entity->priority = false;
}
void entity_drop(Entity *entity) {
    if (entity->free_custom_data && entity->custom_data) {
        free(entity->custom_data);
    }
}
void entity_update(Entity *entity, float delta) {
    if (entity->update_func) {
        entity->update_func(entity, delta);
    }

    // Update entity animation
    animation_update(&entity->animation, delta);

    // Run physics and move
    entity->position = Vector2Add(entity->position, Vector2Scale(entity->velocity, delta));
    
}
void entity_draw(Entity *entity) {
    float scaled_width = (float)(entity->animation.config->width) * entity->scale;
    float scaled_height = (float)(entity->animation.config->height) * entity->scale;

    Rectangle dest = camera_transform_rect(&camera, (Rectangle) {
        .x = entity->position.x - scaled_width / 2.0f,
        .y = entity->position.y - scaled_height / 2.0f,
        .width = scaled_width,
        .height = scaled_height,
    });

    Rectangle source = animation_get_rect(&entity->animation);
    source.width *= entity->flipx ? -1.0f : 1.0f;

    DrawTexturePro(
        entity_texture,
        source,
        dest,
        (Vector2) {
            .x = dest.width / 2,
            .y = dest.height / 2,
        },
        entity->rotation,
        entity->tint
    );
}
