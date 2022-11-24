#ifndef _WORLD_H_
#define _WORLD_H_

#include "cLDtk.h"

extern int last_level_id, current_level_uid, transition_level_uid;
extern struct levels *current_level, *transition_level;
extern bool in_transition;

void world_setup(void);
void world_free(void);
void world_draw_background(void);
void world_draw_foreground(void);
void world_start_transition(int levelUid);
void world_end_transition(void);
void world_spawn_entities_for_current_level(void);
void world_despawn_entities_for_last_level(void);

#endif
