#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    const size_t size = 30;
    size_t *array = (size_t *)malloc(size * sizeof(size_t));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    memset(array, 0, size * sizeof(size_t));
    array[size - 1] = 1;
    // array[size / 2 - 1] = 1;

    while (true) {
        for (int j = 0; j < size; j++) {
            if (array[j] == 0) {
                printf(" ");
            } else {
                printf("*");
            }
        }
        printf("\n");

        size_t *new_array = (size_t *)malloc(size * sizeof(size_t));
        if (new_array == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            free(array);
            return 1;
        }
        memcpy(new_array, array, size * sizeof(size_t));

        for (int i = 1; i < size - 1; i++) {
            size_t a = array[i - 1];
            size_t b = array[i];
            size_t c = array[i + 1];

            new_array[i] = (110 >> (4 * a + 2 * b + c)) & 1;
        }

        memcpy(array, new_array, size * sizeof(size_t));

        free(new_array);

        sleep(1);
    }

    free(array);
    return 0;
}
