#include "boid.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils.h"

void init_boid(Boid *boid, int screen_width, int screen_height) {
    boid->x = rand() % screen_width;
    boid->y = rand() % screen_height;
    boid->theta = (rand() / (double)RAND_MAX) * 2 * PI;
    boid->sint = sinf(boid->theta);
    boid->cost = cosf(boid->theta);
    boid->speed = BOID_SPEED;
}

static inline void place_pixel(uint32_t *restrict pixels, int width, int height, int x,
                               int y, uint32_t colour) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    pixels[y * width + x] = colour;
}

void render_boid(Boid *boid, size_t index, uint32_t *pixels, int screen_width,
                 int screen_height) {
    (void)index;
    /* top = (a,0), topleft = (-a/2, 3a/2tan(x/2)), topright = (-a/2,
     * -3a/2tan(x/2))
     * x' = xcos + ysin y' = xsin - ycos
     */
    const float hh = BOID_SIZE / 2;
    const float sint = boid->sint;
    const float cost = boid->cost;
    const float x1 = hh * cost;
    const float y1 = hh * sint;
    const float x2 = -hh * cost / 2 + 3 * hh * BOID_TOP_ANGLE * sint / 2;
    const float y2 = -hh * sint / 2 - 3 * hh * BOID_TOP_ANGLE * cost / 2;
    const float x3 = -hh * cost / 2 - 3 * hh * BOID_TOP_ANGLE * sint / 2;
    const float y3 = -hh * sint / 2 + 3 * hh * BOID_TOP_ANGLE * cost / 2;

    const uint32_t colour = 0xFF0000FF;
    for (float i = -BOID_SIZE / 2; i <= BOID_SIZE / 2; i += 0.3) {
        for (float j = -BOID_SIZE / 2; j <= BOID_SIZE / 2; j += 0.3) {
            if (triangle_is_inside(x1, y1, x2, y2, x3, y3, i, j)) {
                place_pixel(pixels, screen_width, screen_height, boid->x + i,
                            boid->y + j, colour);
            }
        }
    }
}

void update_boid(Boid *boids, size_t index, size_t count, int screen_width,
                 int screen_height, float dt) {
    Boid *boid = &boids[index];

    float separation_force_x = 0, separation_force_y = 0;

    float sin_sum = 0, cos_sum = 0;
    int neighbours = 0;

    float x_sum = 0, y_sum = 0;
    float cohesion_weight_sum = 0;

    for (size_t j = 0; j < count; j++) {
        if (j == index) continue;
        Boid *b2 = &boids[j];

        const float epsilon = 1e-6;
        float dx = b2->x - boid->x;
        float dy = b2->y - boid->y;
        float dist2 = dx * dx + dy * dy + epsilon;

        if (dist2 > BOID_NEIGHBOUR_DIST * BOID_NEIGHBOUR_DIST) continue;

        float dist = sqrtf(dist2);
        float inv = 1.0f / dist;
        float cos_diff = boid->cost * dx + boid->sint * dy;
        if (cos_diff < BOID_FOV * dist) continue;
        // printf("asdf");

        if (dist < MIN_SEPARATION && dist > 1e-6f) {
            float overlap = MIN_SEPARATION - dist;
            boid->x -= (dx * (overlap * 0.20f) * inv);
            boid->y -= (dy * (overlap * 0.20f) * inv);
        }

        float sep_weight = 1.0f / dist2;
        if (sep_weight > 100.0f) sep_weight = 100.0f;

        separation_force_x -= dx * sep_weight;
        separation_force_y -= dy * sep_weight;

        sin_sum += b2->sint;
        cos_sum += b2->cost;

        // float cohesion_weight = dist / (BOID_NEIGHBOUR_DIST);
        float cohesion_weight = 1;
        x_sum += b2->x + dt * b2->speed * b2->cost;
        y_sum += b2->y + dt * b2->speed * b2->sint;
        cohesion_weight_sum += cohesion_weight;

        neighbours++;
    }

    if (neighbours > 0) {
        float separation_theta = atan2f(separation_force_y, separation_force_x);
        if (separation_theta < 0) separation_theta += 2 * PI;
        float sdiff = wrap_float(separation_theta - boid->theta, -PI, PI);

        float avg_theta = atan2f(sin_sum, cos_sum);
        if (avg_theta < 0) avg_theta += 2 * PI;
        float adiff = wrap_float(avg_theta - boid->theta, -PI, PI);

        float centre_x = (cohesion_weight_sum > 0.0f)
                             ? (x_sum / cohesion_weight_sum)
                             : boid->x;
        float centre_y = (cohesion_weight_sum > 0.0f)
                             ? (y_sum / cohesion_weight_sum)
                             : boid->y;
        float centre_theta = atan2f(centre_y - boid->y, centre_x - boid->x);
        if (centre_theta < 0) centre_theta += 2 * PI;
        float cdiff = wrap_float(centre_theta - boid->theta, -PI, PI);

        float turn = (sdiff * SEPARATION_STRENGTH + adiff * ALIGNMENT_STRENGTH +
                      cdiff * COHESION_STRENGTH);
        const float max_turn = 2.0f;  // rad/s
        turn = clamp_float(turn, -max_turn, max_turn);
        boid->theta = wrap_float(boid->theta + dt * turn, 0, 2 * PI);
        // printf("%f \n", turn);

        boid->sint = sinf(boid->theta);
        boid->cost = cosf(boid->theta);
    }

    boid->x =
        wrap_float(boid->x + dt * boid->speed * boid->cost, 0, screen_width);
    boid->y =
        wrap_float(boid->y + dt * boid->speed * boid->sint, 0, screen_height);
}
