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
int current_doughnuts_left = 0;
float transition_timer = 0.0f;
int current_level_num;

static int *collisions = NULL;

static float next_floor_anim = 3.0f;
static float next_rock_anim = 6.0f;
static int floor_anim_frame = 0;
static int rock_anim_frame = 0;
static float lock_anim = 0.0f;

static void draw_autolayer(struct layerInstances *layer, float offset_x, float offset_y);
static void free_collisions(void);
static void build_collisions(void);

void world_setup(void) {
    loadJSONFile("{\"jsonVersion\":\"1.1.3\"}","res/world.ldtk");
    importMapData();

    world_start_transition(getLevel("Level_0")->uid);
    world_end_transition();
}
void world_free(void) {
    free_collisions();
    freeMapData();
    json_value_free(schema);
    json_value_free(user_data);
}
void world_draw_background(void) {
    draw_autolayer(getLayer("Background", current_level->uid), (float)current_level->worldX, (float)current_level->worldY);

    if (in_transition) {
        draw_autolayer(getLayer("Background", transition_level_uid), (float)transition_level->worldX, (float)transition_level->worldY);
    }
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
    if (!in_transition && current_doughnuts_left == 0) {
        lock_anim += GetFrameTime();
    }

    draw_autolayer(getLayer("Locks", current_level->uid), x, y);
    draw_autolayer(getLayer("Collisions", current_level->uid), x, y);

    if (in_transition) {
        transition_timer -= GetFrameTime();
        x = (float)transition_level->worldX;
        y = (float)transition_level->worldY;
        draw_autolayer(getLayer("Locks", transition_level->uid), x, y);
        draw_autolayer(getLayer("Collisions", transition_level->uid), x, y);
    }
}
void world_start_transition(int levelUid) {
    if (levelUid != current_level_uid && levelUid != transition_level_uid) {
        in_transition = true;
        transition_level_uid = levelUid;
        transition_level = getLevelFromUid(levelUid);
        transition_timer = 0.5f;
        world_spawn_entities_for_level(transition_level_uid);
    }
}
void world_end_transition(void) {
    in_transition = false;
    last_level_id = current_level_uid;
    current_level_uid = transition_level_uid;
    current_level = transition_level;
    current_intGrid = getLayer("Collisions", current_level_uid);
    current_level_num = world_get_level_num(current_level);
    free_collisions();
    build_collisions();
    world_despawn_entities_for_level(last_level_id);
    transition_level = NULL;
    transition_level_uid = -1;
    lock_anim = 0.0f;
}
int world_get_level_num(struct levels *level) {
    return level->firstIntFieldInst;
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
        if ((x == 80 || x == 88) && y == 40) {
            int frame = (int)(lock_anim * 8.0f);
            if (frame > 6) {
                frame = 6;
            }
            x += frame * 8;
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
void world_spawn_entities_for_level(int levelUid) {
    struct levels *level = getLevelFromUid(levelUid);
    struct layerInstances *layer = getLayer("Entities", levelUid);
    struct entityInstances *ents = layer->entityInstances_data_ptr;
    current_doughnuts_left = 0;
    for (int i = 0; i < layer->numEntityInstancesDataPtr; i++) {
        EntityPreset preset = entity_preset_from_identifier(ents[i].identifier);
        if (preset == ENTITY_PRESET_FISHYMAN && game_find_next_entity_of_preset(entities, ENTITY_PRESET_FISHYMAN)) {
            break;
        }

        if (preset == ENTITY_PRESET_DOUGHNUT) {
            current_doughnuts_left += 1;
        } else if (preset == ENTITY_PRESET_MEGA_DOUGHNUT) {
            current_doughnuts_left += 2;
        }

        Entity *ent = game_spawn_entity(preset, (Vector2){
            .x = (float)ents[i].x + (float)level->worldX,
            .y = (float)ents[i].y + (float)level->worldY,
        });
        if (ent->original_preset != ENTITY_PRESET_FISHYMAN) {
            ent->level_uid = levelUid;
        } else {
            global_player = ent;
        }
    }
}
void world_despawn_entities_for_level(int levelUid) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] && entities[i]->level_uid == levelUid && entities[i]->original_preset != ENTITY_PRESET_FISHYMAN) {
            game_despawn_entity(entities[i]);
        }
    }
}
void world_unlock_doors(void) {
    for (int i = 0; i < current_intGrid->cWid * current_intGrid->cHei; i++) {
        if (collisions[i] & IS_LOCK) {
            collisions[i] = 0;
        }
    }
}

static void free_collisions(void) {
    free(collisions);
}
static void build_collisions(void) {
    collisions = calloc(current_intGrid->cWid * current_intGrid->cHei, sizeof(int));

    struct layerInstances *locks_layer = getLayer("Locks", current_level_uid);

    for (int y = 0; y < current_intGrid->cHei; y++) {
        for (int x = 0; x < current_intGrid->cWid; x++) {
            int *col = &collisions[y * current_intGrid->cWid + x];
            *col = current_intGrid->intGrid[y * current_intGrid->cWid + x] ? IS_SOLID : 0;

            if (!(*col) && locks_layer->intGrid[y * locks_layer->cWid + x]) {
                *col = IS_SOLID | IS_LOCK;
            }

            // Check neighbors
            if (*col && ~(*col) & IS_LOCK) {
                bool left = x > 0 ? current_intGrid->intGrid[y * current_intGrid->cWid + (x - 1)] : true;
                bool right = x < current_intGrid->cWid - 1 ? current_intGrid->intGrid[y * current_intGrid->cWid + (x + 1)] : true;
                bool top = y > 0 ? current_intGrid->intGrid[(y - 1) * current_intGrid->cWid + x] : true;
                bool bottom = y < current_intGrid->cHei - 1 ? current_intGrid->intGrid[(y + 1) * current_intGrid->cWid + x] : true;
            
                *col |= (!left && !top) ? ROUND_TOPLEFT : 0;
                *col |= (!right && !top) ? ROUND_TOPRIGHT : 0;
                *col |= (!left && !bottom) ? ROUND_BOTTOMLEFT : 0;
                *col |= (!right && !bottom) ? ROUND_BOTTOMRIGHT : 0;
            }
        }
    }
}

static Rectangle world_get_bound_from_grid(int px, int py, int *col) {
    int localx = (px - current_level->worldX) / 8;
    int localy = (py - current_level->worldY) / 8;

    if (localx >= 0 && localx < current_intGrid->cWid &&
        localy >= 0 && localy < current_intGrid->cHei) {
        int c = collisions[localy * current_intGrid->cWid + localx];
        if (c) {
            *col = c;
            return (Rectangle){
                .x = (float)(localx * 8 + current_level->worldX) - TILE_PADDING,
                .y = (float)(localy * 8 + current_level->worldY) - TILE_PADDING,
                .width = 8.0f + TILE_PADDING2,
                .height = 8.0f + TILE_PADDING2,
            };
        } else {
            *col = 0;
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
int sort_rect(const TileCollision *a, const TileCollision *b) {
    if (Vector2Distance((Vector2){ .x = a->bounds.x + 4.0f, .y = a->bounds.y + 4.0f }, origin) < 
        Vector2Distance((Vector2){ .x = b->bounds.x + 4.0f, .y = b->bounds.y + 4.0f }, origin)) {
        return -1;
    } else {
        return 1;
    }
}

int world_get_colliding_tiles(TileCollision collisions[MAX_TILE_COLLISIONS], float x, float y, float r) {
    int curr_rect = 0, col;
    memset(collisions, 0, sizeof(TileCollision) * MAX_TILE_COLLISIONS);

    for (int xx = (int)(x - r) - 8; xx <= (int)(x + r) + 8; xx += 8) {
        for (int yy = (int)(y - r) - 8; yy <= (int)(y + r) + 8; yy += 8) {
            Rectangle rect = world_get_bound_from_grid(xx, yy, &col);
            float left_margin = x - (rect.x - r);
            float right_margin = rect.x + rect.width + r - x;
            float top_margin = y - (rect.y - r);
            float bottom_margin = rect.y + rect.height + r - y;
#define R (2.0f * (r + TILE_PADDING))

            if (col && left_margin > 0 && right_margin > 0 &&
                top_margin > 0 && bottom_margin > 0) {
                if (col & ROUND_TOPLEFT && right_margin > R && bottom_margin > R &&
                    Vector2Length((Vector2){ right_margin - R, bottom_margin - R }) > 8.0f) {
                    continue;
                } else if (col & ROUND_TOPRIGHT && left_margin > R && bottom_margin > R &&
                     Vector2Length((Vector2){ left_margin - R, bottom_margin - R }) > 8.0f) {
                    continue;
                } else if (col & ROUND_BOTTOMLEFT && right_margin > R && top_margin > R &&
                     Vector2Length((Vector2){ right_margin - R, top_margin - R }) > 8.0f) {
                    continue;
                } else if (col & ROUND_BOTTOMRIGHT && left_margin > R && top_margin > R &&
                     Vector2Length((Vector2){ left_margin - R, top_margin - R }) > 8.0f) {
                    continue;
                }

                collisions[curr_rect].bounds = rect;
                collisions[curr_rect++].col_type = col;
                if (curr_rect == MAX_TILE_COLLISIONS) {
                    break;
                }
            }
        }
    }

    origin = (Vector2){ .x = x, .y = y };
    qsort(collisions, 8, sizeof(TileCollision), (int(*)(const void *, const void *))sort_rect);
    return curr_rect;
}

