#include <stdbool.h>
#include <raylib.h>

#include "game.h"
#include "world.h"

int last_level_id = -1, current_level_uid = -1, transition_level_uid = -1;
struct levels *current_level, *transition_level;
bool in_transition = false;

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y);

void world_setup(void) {
    loadJSONFile("{\"jsonVersion\":\"1.1.3\"}","res/world.ldtk");
    importMapData();

    current_level = getLevel("Level_0");
    current_level_uid = current_level->uid;
}
void world_free(void) {
    freeMapData();
    json_value_free(schema);
    json_value_free(user_data);
}
void world_draw_background(void) {
    draw_autolayer(getLayer("Background", current_level->uid), (float)current_level->worldX, (float)current_level->worldY);
}
void world_draw_foreground(void) {
    float x = (float)current_level->worldX;
    float y = (float)current_level->worldY;

    draw_autolayer(getLayer("Locks", current_level->uid), x, y);
    draw_autolayer(getLayer("Collisions", current_level->uid), x, y);
}
void world_start_transition(int levelUid) {
    in_transition = true;
    transition_level_uid = levelUid;
    transition_level = getLevelFromUid(levelUid);
}
void world_end_transition(void) {
    in_transition = false;
    last_level_id = current_level_uid;
    current_level_uid = transition_level_uid;
    current_level = transition_level;
    transition_level = NULL;
    transition_level_uid = 0;
}

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y) {
    for (int i = layer->autoTiles_data_ptr->count - 1; i >= 0; i--) {
        struct autoTiles *tile = &layer->autoTiles_data_ptr[i];

        int x = tile->SRCx;
        int y = tile->SRCy;

        if ((x == 0 || x == 48) && (y == 64 || y == 72)) {
            x += ((int)(GetTime() * 3.0) % 6) * 8;
        }

        Rectangle dest = camera_transform_rect(&camera, (Rectangle){
            .x = offset_x + (float)tile->x - 0.001f,
            .y = offset_y + (float)tile->y - 0.001f,
            .width = 8.0f + 0.002f,
            .height = 8.0f + 0.002f,
        });
        
        DrawTexturePro(
            terrain_texture,
            (Rectangle){
                .x = (float)x + 0.0001f,
                .y = (float)y + 0.0001f,
                .width = 8.0f * (tile->f & 1 ? -1.0f : 1.0f) - 0.0002f,
                .height = 8.0f * (tile->f & 2 ? -1.0f : 1.0f) - 0.0002f,
            },
            dest,
            (Vector2){
                .x = 0.0f,
                .y = 0.0f,
            },
            0.0f,
            WHITE
        );
    }
}
void world_spawn_entities_for_current_level(void) {
    struct layerInstances *layer = getLayer("Entities", current_level_uid);
    struct entityInstances *ents = layer->entityInstances_data_ptr;
    for (int i = 0; i < layer->numEntityInstancesDataPtr; i++) {
        EntityPreset preset = entity_preset_from_identifier(ents[i].identifier);
        if (preset == ENTITY_PRESET_FISHYMAN && game_find_next_entity_of_preset(entities, ENTITY_PRESET_FISHYMAN)) {
            break;
        }

        Entity *ent = game_spawn_entity(preset, (Vector2){
            .x = (float)ents[i].x + (float)current_level->worldX,
            .y = (float)ents[i].y + (float)current_level->worldY,
        });
        if (ent->original_preset != ENTITY_PRESET_FISHYMAN) {
            ent->level_uid = current_level_uid;
        }
    }
}
void world_despawn_entities_for_last_level(void) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i]->level_uid == last_level_id) {
            game_despawn_entity(entities[i]);
        }
    }
}
