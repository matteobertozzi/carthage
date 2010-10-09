#include <stdio.h>
#include "uintx.h"

UINTX_DEFINE(128)

#define uintx128_init(x)                 uintx_init(x, 128U)
#define uintx128_from_u64(x, v)          uintx_from_u64(x, 128U, v)

#define uintx128_inc(x)                  uintx_inc(x, 128U)
#define uintx128_dec(x)                  uintx_dec(x, 128U)
#define uintx128_add64(x, v)             uintx_add64(x, 128U, v)

#define uintx128_compare64(x, v)         uintx_compare64(x, 128U, v)

#define uintx128_and64(x, v)             uintx_and64(x, 128U, v)
#define uintx128_or64(x, v)              uintx_or64(x, 128U, v)
#define uintx128_xor64(x, v)             uintx_xor64(x, 128U, v)

#define uintx128_print(x)                uintx_print(x, 128U)

void __test_inc (void) {
    uintx128_t xa;
    uintx128_t xb;
    size_t i;

    uintx128_init(xa);
    for (i = 1; i < 524288U; ++i) {
        uintx128_inc(xa);
        if (uintx128_compare64(xa, i) != 0) {
            uintx128_from_u64(xb, i);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" INC FAIL == COMPARE %lu\n", i);
        } else if (uintx128_compare64(xa, i + 1) != -1) {
            uintx128_from_u64(xb, i + 1);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" INC FAIL < COMPARE %lu\n", i);
        } else if (uintx128_compare64(xa, i - 1) != 1) {
            uintx128_from_u64(xb, i - 1);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" INC FAIL > COMPARE %lu\n", i);
        }
    }
}

void __test_add (void) {
    uintx128_t xa;
    uintx128_t xb;
    size_t i, y;

    for (y = 1; y < 300; y += 3) {
        uintx128_init(xa);
        for (i = y; i < 524288U; i += y) {
            uintx128_add64(xa, y);
            if (uintx128_compare64(xa, i) != 0) {
                uintx128_from_u64(xb, i);
                printf("\n A ");
                uintx128_print(xa);
                printf("\n B ");
                uintx128_print(xb);
                printf(" ADD FAIL == COMPARE %lu (%lu)\n", i, y);
            } else if (uintx128_compare64(xa, i + 1) != -1) {
                uintx128_from_u64(xb, i + 1);
                printf("\n A ");
                uintx128_print(xa);
                printf("\n B ");
                uintx128_print(xb);
                printf(" ADD FAIL < COMPARE %lu (%lu)\n", i, y);
            } else if (uintx128_compare64(xa, i - 1) != 1) {
                uintx128_from_u64(xb, i - 1);
                printf("\n A ");
                uintx128_print(xa);
                printf("\n B ");
                uintx128_print(xb);
                printf(" ADD FAIL > COMPARE %lu (%lu)\n", i, y);
            }
        }
    }
}

void __test_dec (void) {
    uintx128_t xa;
    uintx128_t xb;
    size_t i;

    uintx128_from_u64(xa, 524288U);
    for (i = 524287U; i > 0; --i) {
        uintx128_dec(xa);
        if (uintx128_compare64(xa, i) != 0) {
            uintx128_from_u64(xb, i);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" DEC FAIL == COMPARE %lu\n", i);
        } else if (uintx128_compare64(xa, i + 1) != -1) {
            uintx128_from_u64(xb, i + 1);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" DEC FAIL < COMPARE %lu\n", i);
        } else if (uintx128_compare64(xa, i - 1) != 1) {
            uintx128_from_u64(xb, i - 1);
            printf("\n A ");
            uintx128_print(xa);
            printf("\n B ");
            uintx128_print(xb);
            printf(" DEC FAIL > COMPARE %lu\n", i);
        }
    }
}

void __test_logic (void) {
    uintx128_t xr;
    uintx128_t x;

    uintx128_from_u64(xr, 1428245U);

    uintx128_from_u64(x, 2796221U);
    uintx128_and64(x, 1428245U);
    if (uintx128_compare64(x, 35349U) != 0) {
        printf("FAIL AND\n");
        uintx128_from_u64(x, 2796221U);
        uintx128_print(x);printf("\n");
        uintx128_print(xr);printf("\n");
        uintx128_xor64(x, 1428245U);
        uintx128_print(x);printf("\n");
    }

    uintx128_from_u64(x, 2796221U);
    uintx128_or64(x, 1428245U);
    if (uintx128_compare64(x, 4189117U) != 0) {
        printf("FAIL OR\n");
        uintx128_from_u64(x, 2796221U);
        uintx128_print(x);printf("\n");
        uintx128_print(xr);printf("\n");
        uintx128_xor64(x, 1428245U);
        uintx128_print(x);printf("\n");
    }

    uintx128_from_u64(x, 2796221U);
    uintx128_xor64(x, 1428245U);
    if (uintx128_compare64(x, 4153768U) != 0) {
        printf("FAIL XOR\n");
        uintx128_from_u64(x, 2796221U);
        uintx128_print(x);printf("\n");
        uintx128_print(xr);printf("\n");
        uintx128_xor64(x, 1428245U);
        uintx128_print(x);printf("\n");
    }
}

int main (int argc, char **argv) {
    __test_inc();
    __test_add();
    __test_dec();
    __test_logic();
    return(0);
}
