#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "cLDtk/cLDtk.h"

GameState current_state = GAMESTATE_LEVEL;
Entity **entities = NULL;
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
static void draw_entities(void);

int main(int argc, char **argv) {
    InitWindow(500, 500, "Fishy Man 2");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    load_assets();
    entity_list_new();

    camera_new(&camera, (Vector2){ .x = 128.0f, .y = 128.0f });
    game_spawn_entity(ENTITY_PRESET_FISHYMAN, Vector2Zero());

    while (!WindowShouldClose()) {
        update_entities();

        BeginDrawing();
        ClearBackground((Color){ .r = 205, .g = 212, .b = 165, .a = 255 });
        camera_update(&camera, GetScreenWidth(), GetScreenHeight());
        draw_entities();
        EndDrawing();
    }

    entity_list_drop();
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
        }
    }
}

static void load_assets(void) {
    entity_texture = LoadTexture("res/entities.png");
    terrain_texture = LoadTexture("res/terrain.png");
    loadJSONFile("{\"jsonVersion\":\"1.1.3\"}","res/world.ldtk");
    importMapData();
}
static void unload_assets(void) {
    UnloadTexture(entity_texture);
    UnloadTexture(terrain_texture);

    freeMapData();
    json_value_free(schema);
    json_value_free(user_data);
}
static void entity_list_new(void) {
    entity_capacity = 64;
    entities = calloc(entity_capacity, sizeof(Entity *));
}
static void entity_list_drop(void) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i]) {
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
            }
        }
    }

    current_entity = -1;
}
static void draw_entities(void) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i]) {
            entity_draw(entities[i]);
        }
    }
}