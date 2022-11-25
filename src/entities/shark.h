#ifndef _SHARK_H_
#define _SHARK_H_

#include "../entity.h"

typedef enum SharkState {
    SHARK_STATE_ROAM,
    SHARK_STATE_CHASE,
    SHARK_STATE_FOLLOW,
} SharkState;

typedef struct SharkData {
    Vector2 home, target;
    Vector2 last_seen_pos, last_seen_dir;

    float timer;
    float max_chase_speed;
    float home_size;
    float rotation;
    float speed;
    float seen_timer;
    float stationary_timer;
    SharkState state;
} SharkData;

void shark_update(Entity *entity, float delta);
SharkData *shark_data_new(Vector2 spawn_pos);

#endif
