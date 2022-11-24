#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <raylib.h>
#include <stdbool.h>

#include "animation.h"

typedef enum EntityPreset {
    ENTITY_PRESET_UNKOWN_IDENTIFIER,
    ENTITY_PRESET_FISHYMAN_SPAWN,
    ENTITY_PRESET_FISHYMAN,
    ENTITY_PRESET_BUBBLE,
    ENTITY_PRESET_BUBBLE_SPAWNER,
    ENTITY_PRESET_SHARK,
    ENTITY_PRESET_DOUGHNUT,
    ENTITY_PRESET_GHOST_SHARK,
    ENTITY_PRESET_MEGA_DOUGHNUT,
    ENTITY_PRESET_CHECKPOINT,
    ENTITY_PRESET_MAX,
} EntityPreset;

typedef struct Entity Entity;
typedef void(*EntityUpdateFunc)(struct Entity *, float);
struct Entity {
    Vector2 position;
    Vector2 velocity;
    Vector2 scale;
    Animation animation;
    float radius;
    float rotation;
    EntityUpdateFunc update_func;
    void *custom_data;

    bool collide_with_others;
    bool free_custom_data;
};

EntityPreset entity_preset_from_identifier(const char *ident);
Entity *entity_preset(EntityPreset preset, Vector2 pos);
void entity_new(Entity *entity, EntityUpdateFunc func, Vector2 pos, Animations anim_config, float radius, bool collide_with_others, void *custom_data, bool free_data);
void entity_drop(Entity *entity);
void entity_update(Entity *entity, float delta);
void entity_draw(Entity *entity);

#endif
