#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define UTILS_IMPLEMENTATION
#include "utils.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000

#define FPS_TARGET 60
#define FPS_CAP 0

#define SDL_CHECK(x)                                                    \
    do {                                                                \
        if ((x) < 0) {                                                  \
            Log(Log_Error, temp_sprintf("%s: %s", #x, SDL_GetError())); \
            exit(1);                                                    \
        }                                                               \
    } while (0)

#define SDL_PTR(x)                                                      \
    ({                                                                  \
        __auto_type _r = (x);                                           \
        if (!_r) {                                                      \
            Log(Log_Error, temp_sprintf("%s: %s", #x, SDL_GetError())); \
            exit(1);                                                    \
        }                                                               \
        _r;                                                             \
    })

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixels;
    uint64_t last_ticks;
    float fps_max;
    float fps_sum;
    int fps_count;
    bool quit;
} AppState;

bool app_init(AppState *app) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        Log(Log_Error, temp_sprintf("SDL_Init: %s", SDL_GetError()));
        return false;
    }

    app->window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, 0);
    if (!app->window) return false;

    app->renderer =
        SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) return false;

    app->texture = SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                     SCREEN_HEIGHT);
    if (!app->texture) return false;

    app->pixels =
        aligned_alloc(64, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    if (!app->pixels) return false;

    app->fps_max = 0;
    app->fps_sum = 0;
    app->fps_count = 0;
    app->quit = false;

    return true;
}

void app_destroy(AppState *app) {
    if (app->texture) SDL_DestroyTexture(app->texture);
    if (app->renderer) SDL_DestroyRenderer(app->renderer);
    if (app->window) SDL_DestroyWindow(app->window);
    free(app->pixels);
    SDL_Quit();
}

static inline void fill_screen(uint32_t *pixels, uint32_t color) {
    const size_t n = (size_t)SCREEN_WIDTH * SCREEN_HEIGHT;
    for (size_t i = 0; i < n; i++) pixels[i] = color;
}

void app_run(AppState *app) {
    SDL_Event e;
    const float target_frame_time = 1000.0f / FPS_TARGET;
    app->last_ticks = SDL_GetTicks64();

    while (!app->quit) {
        uint64_t frame_start = SDL_GetTicks64();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) app->quit = true;
        }

        fill_screen(app->pixels, 0xFF0020FF);
        SDL_UpdateTexture(app->texture, NULL, app->pixels, SCREEN_WIDTH * 4);

        SDL_RenderCopy(app->renderer, app->texture, NULL, NULL);
        SDL_RenderPresent(app->renderer);

        uint64_t frame_end = SDL_GetTicks64();
        float frame_time = frame_end - frame_start;

#if FPS_CAP
        if (frame_time < target_frame_time) {
            SDL_Delay(target_frame_time - frame_time);
            frame_time = target_frame_time;  // assume perfect sleep
        }
#endif

        float fps = 1000.0f / frame_time;
        app->fps_sum += fps;
        app->fps_count++;
        if (fps > app->fps_max) app->fps_max = fps;
    }
}

int main() {
    AppState app = {0};

    if (!app_init(&app)) {
        fprintf(stderr, "App initialization failed\n");
        return 1;
    }

    app_run(&app);
    app_destroy(&app);

    printf("Max FPS: %.2f\n", app.fps_max);
    printf("Average FPS: %.2f\n", app.fps_sum / app.fps_count);
    return 0;
}
