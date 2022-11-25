#ifndef _WORLD_H_
#define _WORLD_H_
#include <raylib.h>

#include "cLDtk.h"

#define TILE_PADDING 0.1f
#define TILE_PADDING2 0.2f
#define MAX_TILE_COLLISIONS 16
#define IS_SOLID 1
#define ROUND_TOPLEFT 2
#define ROUND_TOPRIGHT 4
#define ROUND_BOTTOMLEFT 8
#define ROUND_BOTTOMRIGHT 16
#define IS_LOCK 32

typedef struct TileCollision {
    Rectangle bounds;
    int col_type;
} TileCollision;

extern int last_level_id, current_level_uid, transition_level_uid;
extern struct levels *current_level, *transition_level;
extern bool in_transition;
extern float transition_timer;
extern int current_doughnuts_left;
extern int current_level_num;

void world_setup(void);
void world_free(void);
void world_draw_background(void);
void world_draw_foreground(void);
void world_start_transition(int levelUid);
void world_end_transition(void);
void world_force_create_tiles_for_level(int levelUid);
void world_spawn_entities_for_level(int levelUid);
void world_despawn_entities_for_level(int levelUid);
int world_get_colliding_tiles(TileCollision collisions[MAX_TILE_COLLISIONS], float x, float y, float r);
void world_unlock_doors(void);
int world_get_level_num(struct levels *level);
bool world_line_colliding(Vector2 start, Vector2 end);
bool world_point_colliding(Vector2 pt);

#endif
