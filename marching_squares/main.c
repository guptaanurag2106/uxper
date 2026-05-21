#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FNL_IMPL
#include "FastNoiseLite.h"
#define UTILS_IMPLEMENTATION
#include "../utils/utils.h"
#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRID_SIZE 5

#define GRID_WIDTH (SCREEN_WIDTH / GRID_SIZE + 1)
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_SIZE + 1)
float grid[GRID_HEIGHT][GRID_WIDTH];

#define WALL_COLOUR DARKGRAY
#define BACKGROUND_COLOUR (Color){62, 164, 240, 255}
#define LAND_COLOUR (Color){56, 78, 29, 255}

enum Triangle_Kind { INSIDE, OUTSIDE, __tr_kind_count };
const Color triangle_colours[__tr_kind_count] = {LAND_COLOUR,
                                                 BACKGROUND_COLOUR};

typedef struct Triangle {
    Vector2 a, b, c;
    enum Triangle_Kind kind;
} Triangle;
Vector(Triangle, Triangles);

Triangles triangles = {0};

typedef struct Line {
    Vector2 a, b;
    Color colour;
} Line;
Vector(Line, Lines);

Lines lines = {0};

// int ranges[20];

void make_grid(fnl_state *noise, float xOff, float yOff, float zOff) {
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            const float nx = (float)j * GRID_SIZE + xOff;
            const float ny = (float)i * GRID_SIZE + yOff;
            const float nz = zOff;

            const float v = fnlGetNoise3D(noise, nx, ny, nz);
            // ranges[(int)(v * 10) + 10]++;
            grid[i][j] = v / 2 + 0.5f;
        }
    }
}

static inline bool isWithinThreshold(float value, float threshold) {
    return value >= threshold;
}

static inline float invLerpIso(float a, float b, float iso) {
    const float den = b - a;
    if (fabsf(den) < 1e-8f) return 0.5f;
    return (iso - a) / den;
}

// Call with clockwise
static inline void pushTriangle(Vector2 a, Vector2 b, Vector2 c,
                                enum Triangle_Kind kind) {
#if 1
    vec_push(&triangles, ((Triangle){a, c, b, kind}));  // Counter clockwise
#else
    vec_push(&triangles, ((Triangle){a, b, c, kind}));  // Clockwise
#endif
}

static inline void pushLine(Vector2 a, Vector2 b, Color colour) {
    vec_push(&lines, ((Line){a, b, colour}));
}

void marching(float threshold) {
    for (int i = 0; i < GRID_HEIGHT - 1; i++) {
        for (int j = 0; j < GRID_WIDTH - 1; j++) {
            const float i1 = (float)i, j1 = (float)j;

            const float v0 = grid[i + 1][j];
            const float v1 = grid[i + 1][j + 1];
            const float v2 = grid[i][j + 1];
            const float v3 = grid[i][j];
            const bool b0 = isWithinThreshold(v0, threshold);
            const bool b1 = isWithinThreshold(v1, threshold);
            const bool b2 = isWithinThreshold(v2, threshold);
            const bool b3 = isWithinThreshold(v3, threshold);

            const uint8_t num = (uint8_t)(b3 << 3 | b2 << 2 | b1 << 1 | b0);

            const Vector2 topLeft = {.x = j1 * GRID_SIZE, .y = i1 * GRID_SIZE};
            const Vector2 topRight = {.x = (j1 + 1) * GRID_SIZE,
                                      .y = i1 * GRID_SIZE};
            const Vector2 bottomLeft = {.x = j1 * GRID_SIZE,
                                        .y = (i1 + 1) * GRID_SIZE};
            const Vector2 bottomRight = {.x = (j1 + 1) * GRID_SIZE,
                                         .y = (i1 + 1) * GRID_SIZE};

            const float tBottom = invLerpIso(v0, v1, threshold);
            const float tRight = invLerpIso(v2, v1, threshold);
            const float tTop = invLerpIso(v3, v2, threshold);
            const float tLeft = invLerpIso(v3, v0, threshold);

            const Vector2 bottom = {.x = (j1 + tBottom) * GRID_SIZE,
                                    .y = (i1 + 1.0f) * GRID_SIZE};
            const Vector2 right = {.x = (j1 + 1.0f) * GRID_SIZE,
                                   .y = (i1 + tRight) * GRID_SIZE};
            const Vector2 top = {.x = (j1 + tTop) * GRID_SIZE,
                                 .y = i1 * GRID_SIZE};
            const Vector2 left = {.x = j1 * GRID_SIZE,
                                  .y = (i1 + tLeft) * GRID_SIZE};

            switch (num) {
                case 0:
                    pushTriangle(topLeft, topRight, bottomLeft, OUTSIDE);
                    pushTriangle(topRight, bottomRight, bottomLeft, OUTSIDE);
                    break;
                case 1:
                    pushTriangle(left, bottom, bottomLeft, INSIDE);
                    pushTriangle(topLeft, bottom, left, OUTSIDE);
                    pushTriangle(topLeft, bottomRight, bottom, OUTSIDE);
                    pushTriangle(topLeft, topRight, bottomRight, OUTSIDE);
                    pushLine(bottom, left, WALL_COLOUR);
                    break;
                case 2:
                    pushTriangle(topLeft, topRight, bottomLeft, OUTSIDE);
                    pushTriangle(topRight, bottom, bottomLeft, OUTSIDE);
                    pushTriangle(topRight, right, bottom, OUTSIDE);
                    pushTriangle(right, bottomRight, bottom, INSIDE);
                    pushLine(bottom, right, WALL_COLOUR);
                    break;
                case 3:
                    pushTriangle(topLeft, topRight, left, OUTSIDE);
                    pushTriangle(topRight, right, left, OUTSIDE);
                    pushTriangle(right, bottomRight, bottomLeft, INSIDE);
                    pushTriangle(bottomLeft, left, right, INSIDE);
                    pushLine(left, right, WALL_COLOUR);
                    break;
                case 4:
                    pushTriangle(topLeft, bottomRight, bottomLeft, OUTSIDE);
                    pushTriangle(topLeft, top, bottomRight, OUTSIDE);
                    pushTriangle(top, right, bottomRight, OUTSIDE);
                    pushTriangle(top, topRight, right, INSIDE);
                    pushLine(right, top, WALL_COLOUR);
                    break;
                case 5:
                    pushTriangle(top, topRight, left, INSIDE);
                    pushTriangle(topRight, right, bottomLeft, INSIDE);
                    pushTriangle(right, bottom, bottomLeft, INSIDE);
                    pushTriangle(bottomLeft, left, topRight, INSIDE);
                    pushTriangle(right, bottomRight, bottom, OUTSIDE);
                    pushTriangle(topLeft, top, left, OUTSIDE);
                    pushLine(left, top, WALL_COLOUR);
                    pushLine(bottom, right, WALL_COLOUR);
                    break;
                case 6:
                    pushTriangle(topLeft, top, bottomLeft, OUTSIDE);
                    pushTriangle(top, bottom, bottomLeft, OUTSIDE);
                    pushTriangle(top, topRight, bottom, INSIDE);
                    pushTriangle(topRight, bottomRight, bottom, INSIDE);
                    pushLine(top, bottom, WALL_COLOUR);
                    break;
                case 7:
                    pushTriangle(topLeft, top, left, OUTSIDE);
                    pushTriangle(top, topRight, bottomLeft, INSIDE);
                    pushTriangle(top, bottomLeft, left, INSIDE);
                    pushTriangle(topRight, bottomRight, bottomLeft, INSIDE);
                    pushLine(left, top, WALL_COLOUR);
                    break;
                case 8:
                    pushTriangle(topLeft, top, left, INSIDE);
                    pushTriangle(top, topRight, bottomLeft, OUTSIDE);
                    pushTriangle(top, bottomLeft, left, OUTSIDE);
                    pushTriangle(topRight, bottomRight, bottomLeft, OUTSIDE);
                    pushLine(left, top, WALL_COLOUR);
                    break;
                case 9:
                    pushTriangle(topLeft, top, bottomLeft, INSIDE);
                    pushTriangle(top, bottom, bottomLeft, INSIDE);
                    pushTriangle(top, topRight, bottom, OUTSIDE);
                    pushTriangle(topRight, bottomRight, bottom, OUTSIDE);
                    pushLine(top, bottom, WALL_COLOUR);
                    break;
                case 10:
                    pushTriangle(topLeft, top, right, INSIDE);
                    pushTriangle(topLeft, right, bottomRight, INSIDE);
                    pushTriangle(topLeft, bottomRight, left, INSIDE);
                    pushTriangle(left, bottomRight, bottom, INSIDE);
                    pushTriangle(top, topRight, right, OUTSIDE);
                    pushTriangle(left, bottom, bottomLeft, OUTSIDE);
                    pushLine(top, right, WALL_COLOUR);
                    pushLine(left, bottom, WALL_COLOUR);
                    break;
                case 11:
                    pushTriangle(topLeft, bottomRight, bottomLeft, INSIDE);
                    pushTriangle(topLeft, top, bottomRight, INSIDE);
                    pushTriangle(top, right, bottomRight, INSIDE);
                    pushTriangle(top, topRight, right, OUTSIDE);
                    pushLine(top, right, WALL_COLOUR);
                    break;
                case 12:
                    pushTriangle(topLeft, topRight, left, INSIDE);
                    pushTriangle(topRight, right, left, INSIDE);
                    pushTriangle(right, bottomRight, bottomLeft, OUTSIDE);
                    pushTriangle(bottomLeft, left, right, OUTSIDE);
                    pushLine(left, right, WALL_COLOUR);
                    break;
                case 13:
                    pushTriangle(topLeft, topRight, bottomLeft, INSIDE);
                    pushTriangle(topRight, bottom, bottomLeft, INSIDE);
                    pushTriangle(topRight, right, bottom, INSIDE);
                    pushTriangle(right, bottomRight, bottom, OUTSIDE);
                    pushLine(bottom, right, WALL_COLOUR);
                    break;
                case 14:
                    pushTriangle(left, bottom, bottomLeft, OUTSIDE);
                    pushTriangle(topLeft, bottom, left, INSIDE);
                    pushTriangle(topLeft, bottomRight, bottom, INSIDE);
                    pushTriangle(topLeft, topRight, bottomRight, INSIDE);
                    pushLine(left, bottom, WALL_COLOUR);
                    break;
                case 15:
                    pushTriangle(topLeft, topRight, bottomLeft, INSIDE);
                    pushTriangle(topRight, bottomRight, bottomLeft, INSIDE);
                    break;
                default:
                    fprintf(stderr, "num not 0-15\n");
                    exit(1);
            }
        }
    }
}

int main(void) {
    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    // noise.fractal_type = FNL_FRACTAL_FBM;
    // noise.octaves = 1;
    float xOff = 0, yOff = 0, zOff = 1;

    make_grid(&noise, xOff, yOff, zOff);

    vec_init(&triangles);
    vec_reserve(&triangles, GRID_WIDTH * GRID_HEIGHT * 3);

    vec_init(&lines);
    vec_reserve(&lines, GRID_WIDTH * GRID_HEIGHT / 4);

    float threshold = 0.5f;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Marching Squares");
    SetTargetFPS(60);
    bool showCorners = false;
    bool showLines = false;
    bool showTriangles = true;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_UP)) {
            threshold += 0.05f;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            threshold -= 0.05f;
        }
        if (IsKeyPressed(KEY_C)) {
            showCorners = !showCorners;
        }
        if (IsKeyPressed(KEY_L)) {
            showLines = !showLines;
        }
        if (IsKeyPressed(KEY_T)) {
            showTriangles = !showTriangles;
        }
        if (IsKeyDown(KEY_W)) {
            yOff -= GRID_SIZE;
        }
        if (IsKeyDown(KEY_S)) {
            yOff += GRID_SIZE;
        }
        if (IsKeyDown(KEY_A)) {
            xOff -= GRID_SIZE;
        }
        if (IsKeyDown(KEY_D)) {
            xOff += GRID_SIZE;
        }

        // zOff += 0.1f;

        make_grid(&noise, xOff, yOff, zOff);

        if (showCorners) {
            for (int i = 0; i < GRID_HEIGHT; i++) {
                for (int j = 0; j < GRID_WIDTH; j++) {
                    Vector2 p = {.x = (float)j * GRID_SIZE,
                                 .y = (float)i * GRID_SIZE};

                    if (isWithinThreshold(grid[i][j], threshold)) {
                        DrawCircleV(p, 2, LAND_COLOUR);
                    } else {
                        DrawCircleV(p, 2, BACKGROUND_COLOUR);
                    }
                }
            }
        }

        vec_clear(&lines);
        vec_clear(&triangles);
        marching(threshold);

        if (showLines) {
            vec_foreach(&lines, line) {
                DrawLineV(line->a, line->b, line->colour);
            }
        }

        if (showTriangles) {
            vec_foreach(&triangles, triangle) {
                DrawTriangle(triangle->a, triangle->b, triangle->c,
                             triangle_colours[triangle->kind]);
            }
        }

        EndDrawing();
    }

    // for (int i = 0; i < 20; i++) {
    //     printf("%f ", (i - 10.0f) / 10.0f);
    // }
    // printf("\n");
    //
    // for (int i = 0; i < 20; i++) {
    //     printf("%d ", ranges[i]);
    // }

    return 0;
}
