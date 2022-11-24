#include <stdlib.h>
#include <string.h>

#include "entity.h"
#include "game.h"
#include "world.h"
#include <raymath.h>

#include "entities/fishyman.h"
#include "entities/bubble.h"
#include "entities/bubble_spawner.h"
#include "entities/doughnut.h"

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

static void entity_process_physics(Entity *entity, float delta);

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
        entity->scale = 0.0f;
        entity->priority = true;
        break;
        case ENTITY_PRESET_BUBBLE_SPAWNER:
        entity_new(entity, bubble_spawner_update, pos, ANIM_NONE, 0.0f, false, bubble_spawner_data_new(), true);
        break;
        case ENTITY_PRESET_DOUGHNUT:
        entity_new(entity, doughnut_update, pos, ANIM_DOUGHNUT, 4.0f, false, doughnut_data_new(pos.y), true);
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
    entity_process_physics(entity, delta);
}
void entity_draw(Entity *entity) {
    float scaled_width = (float)(entity->animation.config->width) * entity->scale;
    float scaled_height = (float)(entity->animation.config->height) * entity->scale;

    Rectangle dest = camera_transform_rect(&camera, (Rectangle) {
        .x = entity->position.x,
        .y = entity->position.y,
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

static void entity_process_physics(Entity *entity, float delta) {
    entity->position = Vector2Add(entity->position, Vector2Scale(entity->velocity, delta));
    
    if (!entity->collide_with_others) {
        return;
    }

    Entity *entity_collisions[32];
    int num_collisions = game_find_colliding_entities(entity_collisions, 32, entity, true);
    for (int i = 0; i < num_collisions; i++) {
        Entity *ent = entity_collisions[i];
        Vector2 dif = Vector2Subtract(ent->position, entity->position);
        float len = ((ent->radius + entity->radius) - Vector2Length(dif)) / 2.0f;
        Vector2 move_dir = Vector2Normalize(dif);
        entity->position = Vector2Add(entity->position, Vector2Scale(move_dir, -len));
        ent->position = Vector2Add(ent->position, Vector2Scale(move_dir, len));
    }

    float radius = entity->radius * entity->scale;
    TileCollision collisions[MAX_TILE_COLLISIONS];
    num_collisions = world_get_colliding_tiles(collisions, entity->position.x, entity->position.y, radius);
    for (TileCollision *col = collisions; col - collisions < num_collisions; col += 1) {
        Rectangle *rect = &col->bounds;
        float left_margin = entity->position.x - (rect->x - radius);
        float right_margin = (rect->x + rect->width + radius) - entity->position.x;
        float top_margin = entity->position.y - (rect->y - radius);
        float bottom_margin = (rect->y + rect->height + radius) - entity->position.y;

#define ROUND_RAD 8.05f
#define R (2.0f * (radius + TILE_PADDING))
        if (col->col_type & ROUND_TOPLEFT && right_margin > R && bottom_margin > R) {
            entity->position = Vector2Add(
                Vector2Scale(Vector2Normalize((Vector2){ .x = -right_margin + R, .y = -bottom_margin + R }), ROUND_RAD),
                (Vector2){ .x = rect->x + rect->width - radius - 2.0f * TILE_PADDING, .y = rect->y + rect->height - radius - 2.0f * TILE_PADDING }
            );
        } else if (col->col_type & ROUND_TOPRIGHT && left_margin > R && bottom_margin > R) {
            entity->position = Vector2Add(
                Vector2Scale(Vector2Normalize((Vector2){ .x = left_margin - R, .y = -bottom_margin + R }), ROUND_RAD),
                (Vector2){ .x = rect->x + radius + 2.0f * TILE_PADDING, .y = rect->y + rect->height - radius - 2.0f * TILE_PADDING }
            );
        } else if (col->col_type & ROUND_BOTTOMLEFT && right_margin > R && top_margin > R) {
            entity->position = Vector2Add(
                Vector2Scale(Vector2Normalize((Vector2){ .x = -right_margin + R, .y = top_margin - R }), ROUND_RAD),
                (Vector2){ .x = rect->x + rect->width - radius - 2.0f * TILE_PADDING, .y = rect->y + radius + 2.0f * TILE_PADDING }
            );
        } else if (col->col_type & ROUND_BOTTOMRIGHT && left_margin > R && top_margin > R) {
            entity->position = Vector2Add(
                Vector2Scale(Vector2Normalize((Vector2){ .x = left_margin - R, .y = top_margin - R }), ROUND_RAD),
                (Vector2){ .x = rect->x + radius + 2.0f * TILE_PADDING, .y = rect->y + radius + 2.0f * TILE_PADDING }
            );
        } else {
            if (left_margin < right_margin && left_margin < top_margin && left_margin < bottom_margin) {
                entity->position.x = rect->x - radius;
                if (entity->velocity.x > 0.0f && left_margin > 0.0f) {
                    entity->velocity.x = 0.0f;
                }
            } else if (right_margin < left_margin && right_margin < top_margin && right_margin < bottom_margin) {
                entity->position.x = rect->x + rect->width + radius;
                if (entity->velocity.x < 0.0f && right_margin > 0.0f) {
                    entity->velocity.x = 0.0f;
                }
            } else if (top_margin < left_margin && top_margin < right_margin && top_margin < bottom_margin) {
                entity->position.y = rect->y - radius;
                if (entity->velocity.y > 0.0f) {
                    entity->velocity.y = 0.0f;
                }
            } else if (bottom_margin < left_margin && bottom_margin < top_margin && bottom_margin < top_margin) {
                entity->position.y = rect->y + rect->height + radius;
                if (entity->velocity.y < 0.0f) {
                    entity->velocity.y = 0.0f;
                }
            }
        }
    }
}
