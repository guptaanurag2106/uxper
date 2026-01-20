#include <string.h>
#include <sys/time.h>

#include "raylib.h"

#define ROWS 20
#define COLUMNS 20
#define GRID_SIZE 50
#define UPDATE_TIME 200  // 1 per UPDATE_TIMEms

#define SCREEN_WIDTH COLUMNS *GRID_SIZE
#define SCREEN_HEIGHT ROWS *GRID_SIZE

static inline Color unpack_colour(unsigned int colour) {
    return (Color){
        .r = (unsigned char)((colour >> 16) & 0xFF),
        .g = (unsigned char)((colour >> 8) & 0xFF),
        .b = (unsigned char)((colour >> 0) & 0xFF),
        .a = (unsigned char)((colour >> 24) & 0xFF),
    };
}

#define BACKGROUND 0xFF282828
#define GRID_COLOUR 0xFF3c3836
#define DEAD_COLOUR 0xFF32302f
#define PAUSE_COLOUR 0xFFAA1111

#define STATE_1_COLOUR 0xFFfabd2f
#define STATE_2_COLOUR 0xFFfe8019
#define STATE_3_COLOUR 0xFF458588
#define STATE_4_COLOUR 0xFF83a598

unsigned int grid[ROWS][COLUMNS] = {{0}};

inline static void init_state() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            grid[y][x] = 0;
        }
    }
}

inline static void render_init(const char *name) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, name);
    SetTargetFPS(60);
}

typedef unsigned int (*cell_update)(int row, int col, unsigned int curr_state);

bool is_paused = true;

inline static void next_state(cell_update update_func) {
    static struct timeval last_run;
    struct timeval now;
    gettimeofday(&now, NULL);
    if ((now.tv_sec - last_run.tv_sec) * 1000 +
            (now.tv_usec - last_run.tv_usec) / 1000 <
        UPDATE_TIME)
        return;
    last_run = now;
    if (is_paused) return;
    unsigned int grid_new[ROWS][COLUMNS] = {{0}};

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            grid_new[y][x] = update_func(y, x, grid[y][x]);
        }
    }
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLUMNS; x++) {
            grid[y][x] = grid_new[y][x];
        }
    }
}

inline static bool should_continue() { return !WindowShouldClose(); }

inline static void render(const unsigned int colours[STATE_COUNT]) {
    BeginDrawing();

    ClearBackground(unpack_colour(BACKGROUND));

    for (int row = 1; row < ROWS; row++) {
        DrawLine(0, row * GRID_SIZE, SCREEN_WIDTH, row * GRID_SIZE,
                 unpack_colour(GRID_COLOUR));
    }
    for (int col = 1; col < COLUMNS; col++) {
        DrawLine(col * GRID_SIZE, 0, col * GRID_SIZE, SCREEN_HEIGHT,
                 unpack_colour(GRID_COLOUR));
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            DrawRectangle(j * GRID_SIZE + 1, i * GRID_SIZE + 1, GRID_SIZE - 2,
                          GRID_SIZE - 2, unpack_colour(colours[grid[i][j]]));
        }
    }

    if (is_paused) {
        DrawText("Paused", GRID_SIZE, GRID_SIZE, 30,
                 unpack_colour(PAUSE_COLOUR));
    }

    EndDrawing();
}

inline static void handle_input() {
    if (IsKeyPressed(KEY_SPACE)) is_paused = !is_paused;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_pos = GetMousePosition();
        const int grid_x = (int)(mouse_pos.x / GRID_SIZE);
        const int grid_y = (int)(mouse_pos.y / GRID_SIZE);
        grid[grid_y][grid_x] = (grid[grid_y][grid_x] + 1) % STATE_COUNT;
    }
    if (IsKeyPressed(KEY_F5)) {
        memset(grid, 0, ROWS * COLUMNS * sizeof(unsigned int));
    }
}

inline static void clean() { CloseWindow(); }
