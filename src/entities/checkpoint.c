#include "../game.h"
#include "../world.h"
#include <raymath.h>

void checkpoint_update(Entity *entity, float delta) {
    Entity **iter = entities;
    Entity **player = game_find_next_entity_of_preset(iter, ENTITY_PRESET_FISHYMAN);

    if (player && *player && Vector2Distance(entity->position, (*player)->position) <
        entity->radius * entity->scale + (*player)->radius * (*player)->scale) {
        last_level_uid_checkpoint = current_level_uid;
    }
}
