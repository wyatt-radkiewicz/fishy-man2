#ifndef _ANIMATION_H_
#define _ANIMATION_H_
#include <raylib.h>
#include <stdint.h>

typedef enum Animations {
    ANIM_NONE,
    ANIM_FISHYMAN_NORMAL,
    ANIM_DOUGHNUT,
    ANIM_SHARK,
    ANIM_BUBBLE,
    ANIM_BUBBLE_POP,
    ANIM_BUBBLE_SMALL,
    ANIM_SHARK_GHOST,
    ANIM_DOUGHNUT_MEGA,
    ANIM_CHECKPOINT,
    ANIM_FISHYMAN_MEGA,
    ANIM_MAX,
} Animations;

typedef enum AnimationDir {
    ANIMDIR_FORWARD,
    ANIMDIR_PINGPONG,
} AnimationDir;

typedef struct AnimationConfig {
    uint32_t base_x, base_y;
    uint32_t width, height;
    uint32_t frames;
    AnimationDir dir;
    float frame_time;
} AnimationConfig;

typedef struct Animation {
    float timer;
    float speed_scale;

    uint32_t times_completed;
    uint32_t frame;
    int32_t pingpong_dir;
    const AnimationConfig *config;
} Animation;

extern const AnimationConfig animations[ANIM_MAX];

void animation_new(Animation *anim, Animations anim_config);
void animation_update(Animation *anim, float delta);
Rectangle animation_get_rect(Animation *anim);

#endif
