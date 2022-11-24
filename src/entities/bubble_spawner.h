#ifndef _BUBBLESPAWNER_H_
#define _BUBBLESPAWNER_H_

#include "../entity.h"

typedef struct BubbleSpawnerData {
    float timeleft;
} BubbleSpawnerData;

void bubble_spawner_update(Entity *entity, float delta);
BubbleSpawnerData *bubble_spawner_data_new(void);

#endif
