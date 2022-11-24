#ifndef _GAME_H_
#define _GAME_H_
#include "entity.h"
#include "camera.h"

typedef enum GameState {
    GAMESTATE_LEVEL,
    GAMESTATE_LEVEL_TRANSITION,
} GameState;

extern GameState current_state;
extern Entity **entities;
extern size_t entity_capacity;
extern GameCamera camera;

extern Texture2D entity_texture;
extern Texture2D terrain_texture;

void game_run(void);

Entity *game_spawn_entity(EntityPreset preset, Vector2 pos);
void game_despawn_entity(Entity *entity);
// Warning: the iter here won't work if the entity array has been reallocated
Entity **game_find_next_entity_of_preset(Entity **iter, EntityPreset preset);

#endif
