#ifndef _CHECKPOINT_H_
#define _CHECKPOINT_H_

#include "../entity.h"

typedef struct CheckpointData {
    float happy_timer;
    float spawn_timer;
} CheckpointData;

void checkpoint_update(Entity *entity, float delta);
CheckpointData *checkpoint_data_new(void);

#endif
