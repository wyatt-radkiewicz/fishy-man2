#ifndef _FISHYMAN_GUTS_
#define _FISHYMAN_GUTS_

#include "../entity.h"

typedef struct FishyManGutsData {
    float angular_velocity;
} FishyManGutsData;

void fishyman_guts_update(Entity *entity, float delta);
FishyManGutsData *fishyman_guts_data_new(float min_vel, float max_vel);

#endif
