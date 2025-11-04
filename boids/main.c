#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdlib.h>
#include <time.h>

#include "boid.h"
#define UTILS_IMPLEMENTATION
#include "utils.h"

#define FPS_TARGET 120
#define FPS_CAP 0
#define FPS_AVG_SAMPLES 64

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000

#define BOID_COUNT 30

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
    bool quit;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    uint32_t *pixels;
    uint64_t last_ticks;

    float fps_max;
    float fps_avg;
    float fps_sum;
    int fps_avg_samples;
    int fps_count;

    size_t boid_count;
    Boid *boids;
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
    app->fps_avg = 0;
    app->fps_sum = 0;
    app->fps_avg_samples = FPS_AVG_SAMPLES;
    app->fps_count = 0;
    app->quit = false;

    app->boid_count = BOID_COUNT;
    Log(Log_Info, temp_sprintf("Spawning %d boids", app->boid_count));
    app->boids = (Boid *)malloc(app->boid_count * sizeof(Boid));
    if (app->boids == NULL) {
        Log(Log_Error, "Could not allocate Boids");
        return false;
    }

    srand(time(NULL));

    for (size_t i = 0; i < app->boid_count; i++) {
        init_boid(&app->boids[i], SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    return true;
}

void app_destroy(AppState *app) {
    if (app->window == NULL) return;

    SDL_DestroyTexture(app->texture);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    free(app->pixels);
    free(app->boids);
    SDL_Quit();

    app->window = NULL;
    app->renderer = NULL;
    app->texture = NULL;
    app->pixels = NULL;
    app->boids = NULL;
}

static inline void update_boids(AppState *app, float dt) {
    dt /= 1000;  // ms->s;
    for (size_t i = 0; i < app->boid_count; i++) {
        update_boid(app->boids, i, app->boid_count, SCREEN_WIDTH, SCREEN_HEIGHT,
                    dt);
    }
}

static inline void render(AppState *app) {
    memset(app->pixels, 0, sizeof(*app->pixels) * SCREEN_WIDTH * SCREEN_HEIGHT);
    for (size_t i = 0; i < app->boid_count; i++) {
        render_boid(&app->boids[i], i, app->pixels, SCREEN_WIDTH,
                    SCREEN_HEIGHT);
    }
}

void app_run(AppState *app) {
    SDL_Event e;

#if !defined(FPS_CAP) || FPS_CAP == 0
    const float target_frame_time = 0;  // limit to 500 anyway
#else
    const float target_frame_time = 1000.0f / FPS_TARGET;
#endif

    float frame_time = 0;

    while (!app->quit) {
        uint64_t frame_start = SDL_GetTicks64();

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    app->quit = true;
                    return;
            }
        }

        update_boids(app, frame_time);
        render(app);

        SDL_UpdateTexture(app->texture, NULL, app->pixels, SCREEN_WIDTH * 4);

        SDL_RenderCopy(app->renderer, app->texture, NULL, NULL);
        SDL_RenderPresent(app->renderer);

        app->last_ticks = SDL_GetTicks64();
        frame_time = app->last_ticks - frame_start;

        if (frame_time < target_frame_time) {
            SDL_Delay(target_frame_time - frame_time);
            frame_time = target_frame_time;
        }

        if (frame_time == 0) continue;
        float fps = 1000.0f / frame_time;
        app->fps_sum += fps;
        app->fps_count++;
        if (fps > app->fps_max) app->fps_max = fps;
        if (app->fps_count > app->fps_avg_samples) {
            app->fps_avg = app->fps_sum / app->fps_count;
            app->fps_count = 0;
            app->fps_sum = 0;
        }
    }
}

int main() {
    AppState app = {0};

    if (!app_init(&app)) {
        fprintf(stderr, "App initialization failed\n");
        return 1;
    }

    app_run(&app);

    printf("Max FPS: %.2f\n", app.fps_max);
    printf("Average FPS: %.2f\n", app.fps_avg);

    app_destroy(&app);
    return 0;
}
