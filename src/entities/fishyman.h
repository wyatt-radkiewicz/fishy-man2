#ifndef _FISHYMAN_H_
#define _FISHYMAN_H_
#include <raylib.h>

#include "../entity.h"

typedef struct FishyManData {
    float speed;
    float accel;
    float decel;

    Vector2 camera_transition_target;
    Vector2 transition_movement_dir;
    float rotation;
    float bubble_timer;
} FishyManData;

void fishyman_update(Entity *entity, float delta);
FishyManData *fishyman_data_new(void);

#endif
