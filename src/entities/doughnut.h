#ifndef _DOUGHNUT_H_
#define _DOUGHNUT_H_
#include <stdbool.h>

#include "../entity.h"

typedef struct DoughnutData {
    float timer;
    float original_y;
    bool eaten;
    float eaten_timer;

    bool inited_frame;

    Vector2 inherited_velocity;
    bool inherit_velocity;
    float inherit_timer;

    bool is_mega_doughnut;
} DoughnutData;

void doughnut_update(Entity *entity, float delta);
DoughnutData *doughnut_data_new(float y_pos, bool mega_doughnut);
void doughnut_eat(Entity *doughnut);

#endif
