#include <stdio.h>
#include "memswap.h"

int main (int argc, char **argv) {
    unsigned long x1[5] = {0, 1, 2, 3, 4};
    unsigned long x2[5] = {5, 6, 7, 8, 9};
    unsigned long a = 10;
    unsigned long b = 256;
    unsigned int i;

    memswap(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    memswap(&x1, &x2, sizeof(x1));
    printf("\nX1: "); for (i = 0; i < 5; ++i) printf("%lu ", x1[i]);
    printf("\nX2: "); for (i = 0; i < 5; ++i) printf("%lu ", x2[i]);
    printf("\n\n");

    memswap8(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    memswap16(&a, &b, sizeof(unsigned long));
    printf("A: %lu B: %lu\n", a, b);

    return(0);
}

