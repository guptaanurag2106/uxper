#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 100
#define ITERATIONS 100

enum CellState { DEAD = 0, ALIVE = 1 };

struct {
    enum CellState rows[ROWS];
} state = {.rows = {DEAD}};

static int newState[8] = {
    [0] = 0, [1] = 1, [2] = 1, [3] = 1, [4] = 0, [5] = 1, [6] = 1, [7] = 0,
};

void init() {
    for (int i = 0; i < ROWS; i++) {
        state.rows[i] = rand() % 2;
    }
}

void update() {
    enum CellState copy[ROWS] = {DEAD};
    for (int i = 0; i < ROWS; i++) copy[i] = state.rows[i];

    for (int i = 0; i < ROWS; i++) {
        state.rows[i] = newState[4 * copy[(i - 1) % ROWS] + 2 * copy[i] +
                                 copy[(i + 1) % ROWS]];
    }
}

void print() {
    // for (int i = 0; i < ROWS; i++) {
    //     putc('-', stdout);
    // }
    // putc('\n', stdout);

    for (int i = 0; i < ROWS; i++) {
        if (state.rows[i] == DEAD) {
            putc(' ', stdout);
        } else if (state.rows[i] == ALIVE) {
            putc('*', stdout);
        }
    }

    putc('\n', stdout);
}

int main() {
    srand(time(NULL));
    init();
    for (int i = 0; i < ITERATIONS; i++) {
        print();
        update();
    }
    return 0;
}
