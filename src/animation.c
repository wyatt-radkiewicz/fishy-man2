#include <math.h>
#include "animation.h"

const AnimationConfig animations[ANIM_MAX] = {
    (AnimationConfig){  // ANIM_NONE
        .base_x = 72, .base_y = 4,
        .width = 0, .height = 0,
        .frames = 0,
        .dir = ANIMDIR_FORWARD,
        .frame_time = -INFINITY,
    },
    (AnimationConfig){  // ANIM_FISHYMAN_NORMAL
        .base_x = 0, .base_y = 0,
        .width = 8, .height = 8,
        .frames = 8,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_DOUGHNUT
        .base_x = 0, .base_y = 8,
        .width = 8, .height = 8,
        .frames = 12,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 7.0,
    },
    (AnimationConfig){  // ANIM_SHARK
        .base_x = 0, .base_y = 16,
        .width = 16, .height = 8,
        .frames = 10,
        .dir = ANIMDIR_PINGPONG,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_BUBBLE
        .base_x = 0, .base_y = 24,
        .width = 8, .height = 8,
        .frames = 9,
        .dir = ANIMDIR_PINGPONG,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_BUBBLE_POP
        .base_x = 72, .base_y = 24,
        .width = 8, .height = 8,
        .frames = 3,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_BUBBLE_SMALL
        .base_x = 0, .base_y = 32,
        .width = 8, .height = 8,
        .frames = 3,
        .dir = ANIMDIR_PINGPONG,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_SHARK_GHOST
        .base_x = 0, .base_y = 40,
        .width = 16, .height = 8,
        .frames = 10,
        .dir = ANIMDIR_PINGPONG,
        .frame_time = 1.0 / 16.0,
    },
    (AnimationConfig){  // ANIM_DOUGHNUT_MEGA
        .base_x = 0, .base_y = 48,
        .width = 16, .height = 16,
        .frames = 8,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_CHECKPOINT
        .base_x = 0, .base_y = 64,
        .width = 40, .height = 24,
        .frames = 1,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 5.0,
    },
    (AnimationConfig){  // ANIM_FISHYMAN_MEGA
        .base_x = 0, .base_y = 88,
        .width = 16, .height = 16,
        .frames = 8,
        .dir = ANIMDIR_FORWARD,
        .frame_time = 1.0 / 5.0,
    }
};

void animation_new(Animation *anim, Animations anim_config) {
    anim->timer = 0.0f;
    anim->speed_scale = 1.0f;
    anim->times_completed = 0;
    anim->frame = 0;
    anim->pingpong_dir = 1;
    anim->config = &animations[anim_config];
}
void animation_update(Animation *anim, float delta) {
    anim->timer += delta;
    if (anim->timer >= anim->config->frame_time / anim->speed_scale) {
        anim->timer = 0.0f;
        switch (anim->config->dir) {
            case ANIMDIR_FORWARD:
            anim->frame += 1;
            if (anim->frame >= anim->config->frames) {
                anim->times_completed += 1;
                anim->frame = 0;
            }
            break;
            case ANIMDIR_PINGPONG:
            anim->frame += anim->pingpong_dir;
            if (anim->frame == anim->config->frames - 1) {
                anim->pingpong_dir = -1;
                anim->times_completed += 1;
            } else if (anim->frame == 0) {
                anim->pingpong_dir = 1;
            }
            break;
        }
    }
}
Rectangle animation_get_rect(Animation *anim) {
    return (Rectangle){
        .x = anim->config->base_x + anim->config->width * anim->frame + 0.0001f,
        .y = anim->config->base_y + 0.0001f,
        .width = anim->config->width - 0.0002f,
        .height = anim->config->height - 0.0002f,
    };
}
