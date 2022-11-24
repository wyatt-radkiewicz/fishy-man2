#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>

#include "game.h"
#include "world.h"

int last_level_id = -1, current_level_uid = -1, transition_level_uid = -1;
struct levels *current_level, *transition_level;
bool in_transition = false;
struct layerInstances *current_intGrid;

static float next_floor_anim = 3.0f;
static float next_rock_anim = 6.0f;
static int floor_anim_frame = 0;
static int rock_anim_frame = 0;

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y);

void world_setup(void) {
    loadJSONFile("{\"jsonVersion\":\"1.1.3\"}","res/world.ldtk");
    importMapData();

    world_start_transition(getLevel("Level_0")->uid);
    world_end_transition();
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
    next_floor_anim = (next_floor_anim < 0.0f) ? 10.0f : (next_floor_anim - GetFrameTime());
    next_rock_anim = (next_rock_anim < 0.0f) ? 12.0f : (next_rock_anim - GetFrameTime());

    int floor_frame = (int)(next_floor_anim * 8.0f);
    int rock_frame = (int)(next_floor_anim * 4.0f);
    floor_anim_frame = (floor_frame < 12) ? floor_frame : 0;
    rock_anim_frame = (rock_frame < 5) ? rock_frame : 0;

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
    current_intGrid = getLayer("Collisions", current_level_uid);
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
        if (x == 0 && y == 56) {
            x += rock_anim_frame * 8;
        }
        if (x == 0 && y == 48) {
            x += floor_anim_frame * 8;
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
static Rectangle world_get_bound_from_grid(int px, int py) {
    int localx = (px - current_level->worldX) / 8;
    int localy = (py - current_level->worldY) / 8;

    if (localx >= 0 && localx < current_intGrid->cWid &&
        localy >= 0 && localy < current_intGrid->cHei) {
        if (current_intGrid->intGrid[localy * current_intGrid->cWid + localx]) {
            return (Rectangle){
                .x = (float)(localx * 8 + current_level->worldX) - 0.1f,
                .y = (float)(localy * 8 + current_level->worldY) - 0.1f,
                .width = 8.0f + 0.2f,
                .height = 8.0f + 0.2f,
            };
        } else {
            return (Rectangle){
                .x = 0.0f,
                .y = 0.0f,
                .width = 0.0f,
                .height = 0.0f,
            };
        }
    }
}

static Vector2 origin;
int sort_rect(const Rectangle *a, const Rectangle *b) {
    if (Vector2Distance((Vector2){ .x = a->x + 4.0f, .y = a->y + 4.0f }, origin) < 
        Vector2Distance((Vector2){ .x = b->x + 4.0f, .y = b->y + 4.0f }, origin)) {
        return -1;
    } else {
        return 1;
    }
}

void world_get_colliding_tiles(Rectangle rects[MAX_COLLISION_TILES], float x, float y, float r) {
    int curr_rect = 0;
    memset(rects, 0, sizeof(Rectangle) * MAX_COLLISION_TILES);

    for (int xx = (int)(x - r) - 8; xx <= (int)(x + r) + 8; xx += 8) {
        for (int yy = (int)(y - r) - 8; yy <= (int)(y + r) + 8; yy += 8) {
            Rectangle rect = world_get_bound_from_grid(xx, yy);
            if (x > rect.x - r &&
                x < rect.x + rect.width + r &&
                y > rect.y - r &&
                y < rect.y + rect.height + r) {
                rects[curr_rect++] = rect;
                if (curr_rect == MAX_COLLISION_TILES - 1) {
                    break;
                }
            }
        }
    }

    origin = (Vector2){ .x = x, .y = y };
    qsort(rects, 8, sizeof(Rectangle), (int(*)(const void *, const void *))sort_rect);
}

