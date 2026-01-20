#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum CellStates { EMPTY = 0, CONDUCTOR, HEAD, TAIL, STATE_COUNT };
#include "./render.h"

const unsigned int Colours[STATE_COUNT] = {[EMPTY] = DEAD_COLOUR,
                                           [CONDUCTOR] = STATE_1_COLOUR,
                                           [HEAD] = STATE_3_COLOUR,
                                           [TAIL] = STATE_2_COLOUR};

void printDebug() {
    printf(
        "-------------------------------------------------------------"
        "-------------------------------------------------------------"
        "------------------------------------------\n");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            enum CellStates c = grid[i][j];
            if (c == EMPTY)
                printf("@");
            else
                printf(" ");
        }
        printf("\n");
    }
}

unsigned int update(int row, int col, unsigned int curr_state) {
    int head_count = 0;
    switch (curr_state) {
        case EMPTY:
            return EMPTY;
        case HEAD:
            return TAIL;
        case TAIL:
            return CONDUCTOR;
        case CONDUCTOR:
            if (grid[(row - 1 + ROWS) % ROWS][(col) % COLUMNS] == HEAD)
                head_count++;
            if (grid[(row - 1 + ROWS) % ROWS][(col - 1 + COLUMNS) % COLUMNS] ==
                HEAD)
                head_count++;
            if (grid[(row - 1 + ROWS) % ROWS][(col + 1) % COLUMNS] == HEAD)
                head_count++;
            if (grid[(row) % ROWS][(col - 1 + COLUMNS) % COLUMNS] == HEAD)
                head_count++;
            if (grid[(row) % ROWS][(col + 1) % COLUMNS] == HEAD) head_count++;
            if (grid[(row + 1) % ROWS][(col) % COLUMNS] == HEAD) head_count++;
            if (grid[(row + 1) % ROWS][(col - 1 + COLUMNS) % COLUMNS] == HEAD)
                head_count++;
            if (grid[(row + 1) % ROWS][(col + 1 + COLUMNS) % COLUMNS] == HEAD)
                head_count++;

            if (head_count == 2 || head_count == 1) return HEAD;
            return CONDUCTOR;
        default:
            return EMPTY;
    }
}

int main() {
    srand(time(0));
    init_state();

    render_init("Wireworld");

    while (should_continue()) {
        handle_input();
        next_state(update);
        // printDebug();
        render(Colours);
    }

    clean();

    return 0;
}
