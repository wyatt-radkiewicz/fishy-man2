#ifndef _DOUGHNUT_H_
#define _DOUGHNUT_H_

#include "../entity.h"

typedef struct DoughnutData {
    float timer;
    float original_y;
    bool eaten;
    float eaten_timer;
} DoughnutData;

void doughnut_update(Entity *entity, float delta);
DoughnutData *doughnut_data_new(float y_pos);
void doughnut_eat(Entity *doughnut);

#endif
