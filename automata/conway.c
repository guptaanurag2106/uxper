#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum CellStates { DEAD = 0, ALIVE, STATE_COUNT };
#include "./render.h"

const unsigned int Colours[STATE_COUNT] = {
    [DEAD] = DEAD_COLOUR, [ALIVE] = STATE_1_COLOUR};

void printDebug() {
    printf(
        "-------------------------------------------------------------"
        "-------------------------------------------------------------"
        "------------------------------------------\n");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            enum CellStates c = grid[i][j];
            if (c == ALIVE)
                printf("@");
            else
                printf(" ");
        }
        printf("\n");
    }
}

unsigned int update(int row, int col, unsigned int curr_state) {
    int neighbour_count =
        grid[(row - 1 + ROWS) % ROWS][(col - 1 + COLUMNS) % COLUMNS] +
        grid[(row - 1 + ROWS) % ROWS][col] +
        grid[(row - 1 + ROWS) % ROWS][(col + 1) % COLUMNS] +
        grid[row][(col - 1 + COLUMNS) % COLUMNS] +
        grid[row][(col + 1) % COLUMNS] +
        grid[(row + 1) % ROWS][(col - 1 + COLUMNS) % COLUMNS] +
        grid[(row + 1) % ROWS][col] +
        grid[(row + 1) % ROWS][(col + 1) % COLUMNS];

    if (curr_state == DEAD && neighbour_count == 3) return ALIVE;
    if (curr_state == ALIVE && (neighbour_count >= 4 || neighbour_count <= 1))
        return DEAD;
    return (enum CellStates)curr_state;
}

int main() {
    srand(time(0));
    init_state();

    render_init("Conway");

    while (should_continue()) {
        handle_input();
        next_state(update);
        // printDebug();
        render(Colours);
    }

    clean();

    return 0;
}
