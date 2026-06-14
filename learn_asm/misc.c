#include <stdio.h>

int stars(int d);

int main(void) {
    int ret = stars(10);
    printf("Called stars           with d  = 10, ret = %d\n", ret);
         
    return 0;
}
