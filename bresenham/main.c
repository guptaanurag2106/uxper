#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIMIT_FPS 0
#define TARGET_FPS 60

#define SCP(expr)                                               \
    ({                                                          \
        void *_scp_tmp = (expr);                                \
        if (!_scp_tmp) {                                        \
            fprintf(stderr, "SDL error: %s\n", SDL_GetError()); \
            exit(EXIT_FAILURE);                                 \
        }                                                       \
        _scp_tmp;                                               \
    })

#define SCC(val)                                                \
    ({                                                          \
        int _scc_tmp = (val);                                   \
        if (_scc_tmp < 0) {                                     \
            fprintf(stderr, "SDL error: %s\n", SDL_GetError()); \
            exit(EXIT_FAILURE);                                 \
        };                                                      \
    })

#define CLAMP(x, min, max)                                    \
    ({                                                        \
        __typeof__(x) _x = (x), _mi = (min), _ma = (max);     \
        (_x) < (_mi) ? (_mi) : ((_x) > (_ma) ? (_ma) : (_x)); \
    })

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1200
#define LINE_COLOUR1 0xFFFF2937
#define LINE_COLOUR2 0xFFFFFE00
#define LINE_COLOUR3 0xFF66BFFF

static inline uint32_t blend_toward(float t, uint32_t colour) {
    uint8_t r = (uint8_t)(((colour >> 16) & 0xFF) * t);
    uint8_t g = (uint8_t)(((colour >> 8) & 0xFF) * t);
    uint8_t b = (uint8_t)(((colour) & 0xFF) * t);
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

void draw_simple_line(uint32_t *image, int startx, int starty, int endx,
                      int endy) {
    // y - starty = m*(x - startx);
    float theta = atan2(endy - starty, endx - startx);
    if (theta < 0) theta += 2 * M_PI;
    const float slope = tanf(theta);
    const float step = 0.1 * ((endx > startx) ? 1 : -1);

    float y = starty;
    float x = startx;
    while (step * (endx - x) >= 0) {
        y += step * slope;
        if (y < 0 || y >= SCREEN_HEIGHT) break;
        image[(int)y * SCREEN_WIDTH + (int)x] = LINE_COLOUR1;
        x += step;
    }
}

void draw_bresenham_line(uint32_t *image, int startx, int starty, int endx,
                         int endy) {
    int dx = endx - startx;
    int dy = endy - starty;
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;
    dx = abs(dx);
    dy = abs(dy);
    const bool stepx = (dx >= dy);

    if (dx == 0 && dy == 0) {  // point
        if (startx >= 0 && startx < SCREEN_WIDTH && starty >= 0 &&
            starty < SCREEN_HEIGHT) {
            image[starty * SCREEN_WIDTH + startx] = LINE_COLOUR2;
        }
    }

    int x = startx;
    int y = starty;

    int d = stepx ? (2 * dy - dx) : (2 * dx - dy);

    int steps = stepx ? dx : dy;

    for (int i = 0; i <= steps; i++) {
        if (y >= 0 && y < SCREEN_HEIGHT && x >= 0 && x < SCREEN_WIDTH)
            image[y * SCREEN_WIDTH + x] = LINE_COLOUR2;

        if (stepx) {
            x += sx;
            if (d <= 0) {
                d += 2 * dy;
            } else {
                d += 2 * (dy - dx);
                y += sy;
            }
        } else {
            y += sy;
            if (d <= 0) {
                d += 2 * dx;
            } else {
                d += 2 * (dx - dy);
                x += sx;
            }
        }
    }
}

void draw_gupta_sproull_line(uint32_t *image, int startx, int starty, int endx,
                             int endy) {
    int dx = endx - startx;
    int dy = endy - starty;

    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    dx = abs(dx);
    dy = abs(dy);

    const bool stepx = (dx >= dy);

    if (dx == 0 && dy == 0) {
        if (startx >= 0 && startx < SCREEN_WIDTH && starty >= 0 &&
            starty < SCREEN_HEIGHT)
            image[starty * SCREEN_WIDTH + startx] = LINE_COLOUR2;
        return;
    }

    int x = startx;
    int y = starty;

    int d = stepx ? (2 * dy - dx) : (2 * dx - dy);
    int steps = stepx ? dx : dy;

    float inv_len = 1.0f / sqrtf((float)(dx * dx + dy * dy));
    float dD = (stepx ? dy : dx) * inv_len;
    float D = 0.0f;

    for (int i = 0; i <= steps; i++) {
        float w0 = CLAMP(1.0f - fabsf(D), 0.0f, 1.0f);
        float w1 = CLAMP(1.0f - fabsf(D + dD), 0.0f, 1.0f);
        float w_1 = CLAMP(1.0f - fabsf(D - dD), 0.0f, 1.0f);

        w0 = w0 * w0 * w0;
        w1 = w1 * w1 * w1;
        w_1 = w_1 * w_1 * w_1;

        if (stepx) {
            if (x >= 0 && x < SCREEN_WIDTH) {
                if (y - 1 >= 0 && y - 1 < SCREEN_HEIGHT)
                    image[(y - 1) * SCREEN_WIDTH + x] =
                        blend_toward(w_1, LINE_COLOUR3);

                if (y >= 0 && y < SCREEN_HEIGHT)
                    image[y * SCREEN_WIDTH + x] = LINE_COLOUR3;

                if (y + 1 >= 0 && y + 1 < SCREEN_HEIGHT)
                    image[(y + 1) * SCREEN_WIDTH + x] =
                        blend_toward(w1, LINE_COLOUR3);
            }
        } else {
            if (y >= 0 && y < SCREEN_HEIGHT) {
                if (x - 1 >= 0 && x - 1 < SCREEN_WIDTH)
                    image[y * SCREEN_WIDTH + (x - 1)] =
                        blend_toward(w_1, LINE_COLOUR3);

                if (x >= 0 && x < SCREEN_WIDTH)
                    image[y * SCREEN_WIDTH + x] = LINE_COLOUR3;

                if (x + 1 >= 0 && x + 1 < SCREEN_WIDTH)
                    image[y * SCREEN_WIDTH + (x + 1)] =
                        blend_toward(w1, LINE_COLOUR3);
            }
        }

        if (stepx) {
            x += sx;
            if (d <= 0) {
                d += 2 * dy;
                D += dD;
            } else {
                d += 2 * (dy - dx);
                y += sy;
                D -= inv_len;
            }
        } else {
            y += sy;
            if (d <= 0) {
                d += 2 * dx;
                D += dD;
            } else {
                d += 2 * (dx - dy);
                x += sx;
                D -= inv_len;
            }
        }
    }
}

void clear_and_render(uint32_t *image, float theta) {
    const int sx = SCREEN_WIDTH / 2;
    const int sy = SCREEN_HEIGHT / 2;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            image[y * SCREEN_WIDTH + x] = 0xFF000000;
        }
    }

    const int length = SCREEN_WIDTH / 3;

    draw_simple_line(image, sx, sy, sx + length * cosf(theta),
                     sy - length * sinf(theta));

    draw_bresenham_line(image, sx, sy, sx + length * cosf(theta + M_PI / 3),
                        sy - length * sinf(theta + M_PI / 3));

    draw_gupta_sproull_line(image, sx, sy,
                            sx + length * cosf(theta + 2 * M_PI / 3),
                            sy - length * sinf(theta + 2 * M_PI / 3));

    draw_simple_line(image, sx, sy, sx + length * cosf(theta + M_PI),
                     sy - length * sinf(theta + M_PI));

    draw_bresenham_line(image, sx, sy, sx + length * cosf(theta + 4 * M_PI / 3),
                        sy - length * sinf(theta + 4 * M_PI / 3));
    draw_gupta_sproull_line(image, sx, sy,
                            sx + length * cosf(theta + 5 * M_PI / 3),
                            sy - length * sinf(theta + 5 * M_PI / 3));
}

void run_perf_comparison(SDL_Renderer *renderer, uint32_t *image) {
    printf("Running Performance Comparison\n");
    const int num_lines = 10000;
    uint64_t start, end;
    double seconds;
    uint64_t freq = SDL_GetPerformanceFrequency();

    srand(time(NULL));

    start = SDL_GetPerformanceCounter();
    for (int i = 0; i < num_lines; ++i) {
        draw_simple_line(image, rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT,
                         rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
    }
    end = SDL_GetPerformanceCounter();
    seconds = (double)(end - start) / freq;
    printf("1. Simple Line:\t%d lines in %.6f seconds\n", num_lines, seconds);

    start = SDL_GetPerformanceCounter();
    for (int i = 0; i < num_lines; ++i) {
        draw_bresenham_line(image, rand() % SCREEN_WIDTH,
                            rand() % SCREEN_HEIGHT, rand() % SCREEN_WIDTH,
                            rand() % SCREEN_HEIGHT);
    }
    end = SDL_GetPerformanceCounter();
    seconds = (double)(end - start) / freq;
    printf("2. Bresenham:\t%d lines in %.6f seconds\n", num_lines, seconds);

    start = SDL_GetPerformanceCounter();
    for (int i = 0; i < num_lines; ++i) {
        draw_gupta_sproull_line(image, rand() % SCREEN_WIDTH,
                                rand() % SCREEN_HEIGHT, rand() % SCREEN_WIDTH,
                                rand() % SCREEN_HEIGHT);
    }
    end = SDL_GetPerformanceCounter();
    seconds = (double)(end - start) / freq;
    printf("3. Gupta-Sproull:\t%d lines in %.6f seconds\n", num_lines, seconds);

    start = SDL_GetPerformanceCounter();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < num_lines; ++i) {
        SDL_RenderDrawLine(renderer, rand() % SCREEN_WIDTH,
                           rand() % SCREEN_HEIGHT, rand() % SCREEN_WIDTH,
                           rand() % SCREEN_HEIGHT);
    }
    SDL_RenderPresent(renderer);
    end = SDL_GetPerformanceCounter();
    seconds = (double)(end - start) / freq;
    printf("4. SDL_RenderDrawLine:\t%d lines in %.6f seconds\n", num_lines,
           seconds);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SCP(SDL_CreateWindow("Lines", SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED,
                                              SCREEN_WIDTH, SCREEN_HEIGHT, 0));

    SDL_Renderer *renderer =
        SCP(SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE));

    SDL_Texture *texture = SCP(SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT));

    uint32_t *image = malloc(sizeof(uint32_t) * SCREEN_WIDTH * SCREEN_HEIGHT);

    run_perf_comparison(renderer, image);

    float theta = 0;

    char window_title[100];

    bool quit = false;
    SDL_Event ev;
    while (!quit) {
        uint32_t frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (ev.key.keysym.sym) {
                        case SDLK_q:
                            quit = true;
                            break;
                        case SDLK_UP:
                            theta = fmodf(theta + 0.1, 2 * M_PI);
                            break;
                        case SDLK_DOWN:
                            theta = fmodf(theta - 0.1, 2 * M_PI);
                            break;
                    }
            }
        }

        clear_and_render(image, theta);
        SDL_UpdateTexture(texture, NULL, image, SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        uint32_t frame_end = SDL_GetTicks();
        uint32_t frame_duration = frame_end - frame_start;
        float seconds = (frame_duration) / 1000.0f;
        float fps = 1 / seconds;
#if LIMIT_FPS
        const uint32_t frame_delay = 1000 / TARGET_FPS;
        if (frame_delay > frame_duration) {
            SDL_Delay(frame_delay - frame_duration);
        }
        fps = TARGET_FPS;
#endif
        snprintf(window_title, 100, "Lines | FPS: %.2f", fps);

        SDL_SetWindowTitle(window, window_title);
    }

    free(image);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
