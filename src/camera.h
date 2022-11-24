#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <raylib.h>

typedef struct GameCamera {
    Vector2 position;
    float scale;

    Vector2 max_dimen;
    Vector2 dimen;
    float window_width, window_height;
} GameCamera;

void camera_new(GameCamera *camera, Vector2 max_dimen);
void camera_update(GameCamera *camera, int window_width, int window_height);
Rectangle camera_transform_rect(GameCamera *camera, Rectangle rect);

#endif
