#include "camera.h"
#include "world.h"
#include <raymath.h>

void camera_new(GameCamera *camera, Vector2 max_dimen) {
    camera->position = Vector2Zero();
    camera->max_dimen = max_dimen;
    camera->dimen = max_dimen;
    camera->scale = 1.0f;
}
void camera_update(GameCamera *camera, int window_width, int window_height) {
    camera->window_width = (float)window_width;
    camera->window_height = (float)window_height;

    float window_ratio = (float)window_width / (float)window_height;
    float camera_ratio = camera->max_dimen.x / camera->max_dimen.y;

    if (window_ratio > camera_ratio) {
        camera->dimen.x = camera->max_dimen.x;
        camera->dimen.y = camera->max_dimen.x * (1.0f / window_ratio);
    } else {
        camera->dimen.y = camera->max_dimen.y;
        camera->dimen.x = camera->max_dimen.y * window_ratio;
    }
}
Rectangle camera_transform_rect(GameCamera *camera, Rectangle rect) {
    rect.x = (rect.x - camera->position.x) / camera->scale / camera->dimen.x * camera->window_width + camera->window_width / 2.0f;
    rect.y = (rect.y - camera->position.y) / camera->scale / camera->dimen.y * camera->window_height + camera->window_height / 2.0f;
    rect.width = rect.width / camera->scale / camera->dimen.x * camera->window_width;
    rect.height = rect.height / camera->scale / camera->dimen.y * camera->window_height;

    return rect;
}
void camera_clip_to_level(GameCamera *camera) {
    if (camera->position.x - camera->dimen.x * camera->scale / 2.0f < (float)current_level->worldX) {
        camera->position.x = (float)current_level->worldX + camera->dimen.x * camera->scale / 2.0f;
    }
    if (camera->position.x + camera->dimen.x * camera->scale / 2.0f > (float)current_level->worldX + (float)current_level->pxWid) {
        camera->position.x = (float)current_level->worldX + (float)current_level->pxWid - camera->dimen.x * camera->scale / 2.0f;
    }
    if (camera->position.y - camera->dimen.y * camera->scale / 2.0f < (float)current_level->worldY) {
        camera->position.y = (float)current_level->worldY + camera->dimen.y * camera->scale / 2.0f;
    }
    if (camera->position.y + camera->dimen.y * camera->scale / 2.0f > (float)current_level->worldY + (float)current_level->pxHei) {
        camera->position.y = (float)current_level->worldY + (float)current_level->pxHei - camera->dimen.y * camera->scale / 2.0f;
    }
}
