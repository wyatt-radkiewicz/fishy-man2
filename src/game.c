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
static float death_timer = 0.0f;
int death_state = 0;
int last_level_uid_checkpoint = 0;
GameCamera camera;

Texture2D entity_texture;
Texture2D terrain_texture;
Texture2D wipe_texture;

static void load_assets(void);
static void unload_assets(void);
static void entity_list_new(void);
static void entity_list_drop(void);
static void update_entities(void);
static void draw_entities(int priority);

int main(int argc, char **argv) {
    InitWindow(500, 500, "Fishy Man 2");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    load_assets();
    entity_list_new();

    //camera_new(&camera, (Vector2){ 256.0f, 256.0f });// .x = 192.0f, .y = 128.0f });
    camera_new(&camera, (Vector2){ .x = 192.0f, .y = 128.0f });
    world_setup();
    last_level_uid_checkpoint = current_level_uid;
    camera.position = global_player->position;

    while (!WindowShouldClose()) {
        if (death_state > 0) {
            death_timer -= GetFrameTime();
        }
        if (death_state == 1) {
            if (death_timer <= 0.0f) {
                death_state = 2;
            }
        } else if (death_state == 2) {
            if (in_transition) {
                world_end_transition();
            }
            for (int i = 0; i < entity_capacity; i++) {
                game_despawn_entity(entities[i]);
            }
            world_force_create_tiles_for_level(last_level_uid_checkpoint);
            world_spawn_entities_for_level(last_level_uid_checkpoint);
            camera.position = global_player->position;
            camera_clip_to_level(&camera);
            //update_entities();

            death_state = 3;
        } else if (death_state == 3) {
            if (death_timer <= DEATH_DURATION / -2.0f) {
                death_state = 0;
            }
        } else if (in_transition && transition_timer <= 0.0f) {
            world_end_transition();
        }

        if (death_state < 2) {
            update_entities();
        }

        if (IsKeyPressed(KEY_SPACE)) {
            start_death();
        }

        BeginDrawing();
        ClearBackground((Color){ .r = 205, .g = 212, .b = 165, .a = 255 });
        camera_update(&camera, GetScreenWidth(), GetScreenHeight());
        world_draw_background();
        draw_entities(PRIORITY_LOW);
        draw_entities(PRIORITY_MID);
        world_draw_foreground();
        draw_entities(PRIORITY_HI);

        if (death_state > 0) {
            float y = ((death_timer / DEATH_DURATION) + 0.5f) * (384.0f + camera.dimen.y * camera.scale);

            DrawTextureTiled(wipe_texture, (Rectangle){
                .x = 0.0f, .y = 0.0f, .width = 16.0f, .height = 384.0f
            }, camera_transform_rect(&camera, (Rectangle) {
                .x = camera.position.x - camera.dimen.x * camera.scale / 2.0f,
                .y = camera.position.y + camera.dimen.y * camera.scale / 2.0f - y,
                .width = camera.dimen.x * camera.scale,
                .height = 384.0f,
            }), (Vector2){ .x = 0.0f, .y = 0.0f }, 0.0f, (float)GetScreenHeight() / (camera.dimen.y * camera.scale), WHITE);
        }
        
        EndDrawing();
    }

    entity_list_drop();
    world_free();
    unload_assets();
    CloseWindow();
}
void start_death(void) {
    if (!death_state) {
        death_state = 1;
        death_timer = DEATH_DURATION / 2.0f;
    }
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
        entity_slot = &entities[entity_capacity / 2];
    }

    // Make the entity
    *entity_slot = entity_preset(preset, pos);
    return *entity_slot;
}
void game_despawn_entity(Entity *entity) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] == entity && entity) {
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
    wipe_texture = LoadTexture("res/wipe.png");
}
static void unload_assets(void) {
    UnloadTexture(entity_texture);
    UnloadTexture(terrain_texture);
    UnloadTexture(wipe_texture);
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
static void draw_entities(int priority) {
    for (int i = 0; i < entity_capacity; i++) {
        if (entities[i] && entities[i]->priority == priority) {
            entity_draw(entities[i]);
        }
    }
}

// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest.
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;

    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                    (Rectangle){dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, (Rectangle){dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }

        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                        (Rectangle){dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, (Rectangle){dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                        (Rectangle){dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, (Rectangle){dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }

            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }

        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                        (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }

            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}

