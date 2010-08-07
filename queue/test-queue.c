#include <stdio.h>
#include "queue.h"

int main (int argc, char **argv) {
    queue_t queue;
    void *x;


    queue_alloc(&queue, NULL, NULL, NULL);

    queue_push(&queue, "E1");
    queue_push(&queue, "E2");
    queue_push(&queue, "E3");
    queue_push(&queue, "E4");
    queue_push(&queue, "E5");
    queue_push(&queue, "E6");
    queue_push(&queue, "E7");

    while ((x = queue_pop(&queue)) != NULL)
        printf("POP0: %s\n", x);

    queue_push(&queue, "E8");
    queue_push(&queue, "E9");
    queue_push(&queue, "E10");
    queue_push(&queue, "E11");
    queue_push(&queue, "E12");

    while ((x = queue_pop(&queue)) != NULL)
        printf("POP1: %s\n", x);

    queue_free(&queue);

    /* Circular Queue */
    queue_circular_alloc(&queue, 4U, NULL, NULL, NULL);

    queue_push(&queue, "E1");
    queue_push(&queue, "E2");
    queue_push(&queue, "E3");
    queue_push(&queue, "E4");
    queue_push(&queue, "E5");
    queue_push(&queue, "E6");
    queue_push(&queue, "E7");

    while ((x = queue_pop(&queue)) != NULL)
        printf("C-POP0: %s\n", x);

    queue_push(&queue, "E8");
    queue_push(&queue, "E9");
    queue_push(&queue, "E10");
    queue_push(&queue, "E11");
    queue_push(&queue, "E12");

    while ((x = queue_pop(&queue)) != NULL)
        printf("C-POP1: %s\n", x);

    queue_free(&queue);

    return(0);
}

