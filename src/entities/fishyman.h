#ifndef _FISHYMAN_H_
#define _FISHYMAN_H_

typedef struct FishyManData {
    float speed;
    float accel;
    float decel;
} FishyManData;

void fishyman_update(Entity *entity, float delta);
FishyManData *fishyman_data_new(void);

#endif
