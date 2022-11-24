#define RAYMATH_IMPLEMENTATION
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "world.h"

GameState current_state = GAMESTATE_LEVEL;
Entity **entities = NULL;
Entity *global_player = NULL;
size_t entity_capacity = 0;
static int64_t current_entity = -1;
static bool despawn_last_entity = false;
GameCamera camera;

Texture2D entity_texture;
Texture2D terrain_texture;

static void load_assets(void);
static void unload_assets(void);
static void entity_list_new(void);
static void entity_list_drop(void);
static void update_entities(void);
static void draw_entities(bool priority);

int main(int argc, char **argv) {
    InitWindow(500, 500, "Fishy Man 2");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    load_assets();
    entity_list_new();

    camera_new(&camera, (Vector2){ .x = 192.0f, .y = 128.0f });
    world_setup();

    while (!WindowShouldClose()) {
        if (in_transition && transition_timer <= 0.0f) {
            world_end_transition();
        }
        update_entities();

        BeginDrawing();
        ClearBackground((Color){ .r = 205, .g = 212, .b = 165, .a = 255 });
        camera_update(&camera, GetScreenWidth(), GetScreenHeight());
        world_draw_background();
        draw_entities(false);
        world_draw_foreground();
        draw_entities(true);
        EndDrawing();
    }

    entity_list_drop();
    world_free();
    unload_assets();
    CloseWindow();
}

Entity *game_spawn_entity(EntityPreset preset, Vector2 pos) {
    // Find an empty entity slot
    Entity **entity_slot = NULL;

    for (int i = 0; i < entity_capacity; i++) {
        if (!entities[i]) {
            entity_slot = &entities[i];
        }
    }

    if (!entity_slot) {
        // Grow the list
        entity_capacity *= 2;
        entities = realloc(entities, sizeof(Entity *) * entity_capacity);
        memset(entities + entity_capacity / 2, 0, sizeof(Entity *) * (entity_capacity / 2));
    }

    // Make the entity
    *entity_slot = entity_preset(preset, pos);
    return *entity_slot;
}
void game_despawn_entity(Entity *entity) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] == entity) {
            if (i != current_entity) {
                entities[i] = NULL;
                entity_drop(entity);
                free(entity);
            } else {
                despawn_last_entity = true;
            }
            return;
        }
    }
}
Entity **game_find_next_entity_of_preset(Entity **iter, EntityPreset preset) {
    for (int i = iter - entities; i < entity_capacity; i++) {
        if (entities[i] && entities[i]->original_preset == preset) {
            return &entities[i];
        }
    }

    return NULL;
}
int game_find_colliding_entities(Entity **buffer, int length, Entity *a, bool only_colliders) {
    int curr_ent = 0;

    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] && Vector2Distance(entities[i]->position, a->position) < entities[i]->radius + a->radius &&
            (!only_colliders || (only_colliders && entities[i]->collide_with_others)) && entities[i] != a) {
            buffer[curr_ent++] = entities[i];
            if (curr_ent == length) {
                break;
            }
        }
    }

    return curr_ent;
}

static void load_assets(void) {
    entity_texture = LoadTexture("res/entities.png");
    terrain_texture = LoadTexture("res/terrain.png");
}
static void unload_assets(void) {
    UnloadTexture(entity_texture);
    UnloadTexture(terrain_texture);
}
static void entity_list_new(void) {
    entity_capacity = 64;
    entities = calloc(entity_capacity, sizeof(Entity *));
}
static void entity_list_drop(void) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i]) {
            entity_drop(entities[i]);
            free(entities[i]);
            entities[i] = NULL;
        }
    }

    free(entities);
}
static void update_entities(void) {
    float delta = GetFrameTime();

    for (current_entity = 0; current_entity < entity_capacity; current_entity++) {
        if (entities[current_entity]) {
            entity_update(entities[current_entity], delta);

            if (despawn_last_entity) {
                entity_drop(entities[current_entity]);
                free(entities[current_entity]);
                entities[current_entity] = NULL;
                despawn_last_entity = false;
            }
        }
    }

    current_entity = -1;
}
static void draw_entities(bool priority) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] && entities[i]->priority == priority) {
            entity_draw(entities[i]);
        }
    }
}
