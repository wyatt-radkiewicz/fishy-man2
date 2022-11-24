#ifndef _GHOST_SHARK_H_
#define _GHOST_SHARK_H_

#include "../entity.h"

typedef struct GhostSharkData {
    float speed;
    float max_speed;
} GhostSharkData;

void ghost_shark_update(Entity *entity, float delta);
GhostSharkData *ghost_shark_data_new(void);

#endif
