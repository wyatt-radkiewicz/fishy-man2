#include <stdbool.h>
#include <raylib.h>

#include "game.h"
#include "world.h"

int current_level_uid, transition_level_uid;
struct levels *current_level, *transition_level;
static bool in_transition = false;

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y);

void world_setup(void) {
    loadJSONFile("{\"jsonVersion\":\"1.1.3\"}","res/world.ldtk");
    importMapData();

    current_level = getLevel("Level_0");
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
    current_level_uid = transition_level_uid;
    current_level = transition_level;
    transition_level = NULL;
    transition_level_uid = 0;
}

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y) {
    offset_x -= 4.0f;
    offset_y -= 4.0f;

    for (int i = layer->autoTiles_data_ptr->count - 1; i >= 0; i--) {
        struct autoTiles *tile = &layer->autoTiles_data_ptr[i];

        Rectangle dest = camera_transform_rect(&camera, (Rectangle){
            .x = offset_x + (float)tile->x,
            .y = offset_y + (float)tile->y,
            .width = 8.0f,
            .height = 8.0f,
        });
        
        DrawTexturePro(
            terrain_texture,
            (Rectangle){
                .x = (float)tile->SRCx,
                .y = (float)tile->SRCy,
                .width = 8.0f * (tile->f & 1 ? -1.0f : 1.0f),
                .height = 8.0f * (tile->f & 2 ? -1.0f : 1.0f),
            },
            dest,
            (Vector2){
                .x = dest.width / 2.0f,
                .y = dest.height / 2.0f,
            },
            0.0f,
            WHITE
        );
    }
}
