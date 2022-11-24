#ifndef _BUBBLE_H_
#define _BUBBLE_H_

#include "../entity.h"

typedef struct BubbleData {
    float lifetime;
    float time_left;
    float wiggle;
    float base_vel_x;
    float decel;
    float max_vely;
    float accel;
} BubbleData;

void bubble_update(Entity *entity, float delta);
BubbleData *bubble_data_new(
    float min_lifetime, float max_lifetime,
    float min_wiggle, float max_wiggle,
    float min_velx, float max_velx,
    float min_vely, float max_vely,
    float decel
);

#endif
