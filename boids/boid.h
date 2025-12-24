#pragma once

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils.h"

#define BOID_SIZE 40.0f
#define BOID_SPEED 200
#define BOID_NEIGHBOUR_DIST BOID_SIZE * 3
#define MIN_SEPARATION BOID_SIZE * 0.7f
#define BOID_FOV cosf(PI_2_3 / 2)

#define SEPARATION_STRENGTH 0.6f
#define ALIGNMENT_STRENGTH 1.0f
#define COHESION_STRENGTH 1.4f

// isosceles triangle with top angle = PI/6
#define BOID_TOP_ANGLE tanf(PI / 12)

__attribute__((aligned(64)))
typedef struct {
    float x, y;
    float speed;
    float theta;  // 0 means facing along X -PI to PI
    float sint;
    float cost;
} Boid;

void init_boid(Boid *boid, int screen_width, int screen_height);

void render_boid(Boid *boid, size_t index, uint32_t *pixels, int screen_width,
                 int screen_height);

void update_boid(Boid *boid, size_t index, size_t count, int screen_width,
                 int screen_height, float dt);
