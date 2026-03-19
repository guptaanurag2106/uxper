#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

int test(int x) { return (int)((x*x)/2 + 1); }

int main(void) {
    int a = 2;
    InitWindow(800, 600, "Flood Fill");
    Vector2 x = GetMousePosition();
    printf("%f", x.x);
    if (rand() == 2) {
        test(2);
    } else {
        test(5);
    }
    printf("Hello World\n");
    return 0;
}
