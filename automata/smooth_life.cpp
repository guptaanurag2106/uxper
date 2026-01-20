#include <time.h>
#include <unistd.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

#define level_string                                                           \
    " `.-':_,^=;><+!rc*/"                                                      \
    "z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&" \
    "@"
#define levels (sizeof(level_string) / sizeof(level_string[0]) - 1)

#define WIDTH 50
#define HEIGHT 50

float grid[HEIGHT][WIDTH] = {0.0};
float ra = 11;
float ri = ra / 3.0;
float alpha_n = 0.028;
float alpha_m = 0.147;
float b1 = 0.278;
float b2 = 0.365;
float d1 = 0.267;
float d2 = 0.445;
float dt = 0.05f;

float rand_float() { return (float)rand() / (float)RAND_MAX; }

void init_state() {
    int cy = HEIGHT / 2;
    int cx = WIDTH / 2;
    for (int y = -cy / 2; y < cy / 2; y++) {
        for (int x = -cx / 2; x < cx / 2; x++) {
            grid[y + HEIGHT / 2][x + WIDTH / 2] = rand_float();
        }
    }
}

void printDebug() {
    std::cout << "-------------------------------------------------------------"
                 "-------------------------------------------------------------"
                 "------------------------------------------"
              << std::endl;
    for (const auto &row : grid) {
        for (auto c : row) {
            char x = level_string[int((levels - 1) * c)];
            std::cout << x << x;
        }
        std::cout << std::endl;
    }
}

int emod(int a, int b) { return (a % b + b) % b; }

float sigma(float x, float a, float alpha) {
    return 1.0f / (1.0f + exp(-(x - a) * 4 / alpha));
}

float sigma_n(float x, float a, float b) {
    return sigma(x, a, alpha_n) * (1 - sigma(x, b, alpha_n));
}

float sigma_m(float x, float y, float m) {
    return x * (1 - sigma(m, 0.5f, alpha_m)) + y * sigma(m, 0.5f, alpha_m);
}

float s(float n, float m) {
    return sigma_n(n, sigma_m(b1, d1, m), sigma_m(b2, d2, m));
}

void compute_grid_new(float (&grid_new)[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float m = 0, M = 0;
            float n = 0, N = 0;

            for (int i = -(ra - 1); i <= (ra - 1); i++) {
                for (int j = -(ra - 1); j <= (ra - 1); j++) {
                    int cx = emod(y + i, HEIGHT);
                    int cy = emod(x + j, WIDTH);

                    if ((i * i + j * j) <= ri * ri) {
                        m += grid[cy][cx];
                        M += 1;
                    } else if ((i * i + j * j) <= ra * ra) {
                        n += grid[cy][cx];
                        N += 1;
                    }
                }
            }
            m /= M;
            n /= N;
            float q = s(n, m);
            grid_new[y][x] = 2 * q - 1;
        }
    }
}

void clamp(float *x, float a, float b) {
    if (*x < a) *x = a;
    if (*x > b) *x = b;
}

void update() {
    float grid_new[HEIGHT][WIDTH] = {0.0};

    compute_grid_new(grid_new);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            grid[y][x] += dt * grid_new[y][x];
            clamp(&grid[y][x], 0, 1);
        }
    }
}

int main() {
    srand(time(0));
    init_state();

    for (;;) {
        usleep(200000);
        update();
        printDebug();
    }

    return 0;
}
