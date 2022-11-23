#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>

int main(int argc, char **argv) {
    InitWindow(500, 500, "Fishy Man 2");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){ .r = 205, .g = 212, .b = 165, .a = 255 });
        EndDrawing();
    }

    return EXIT_SUCCESS;
}
