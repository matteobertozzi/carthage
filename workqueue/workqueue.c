/* [ workqueue.c ] - Simple Concurrent Work Queue based on pthread.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010, Matteo Bertozzi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL MATTEO BERTOZZI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "workqueue.h"

typedef enum _workqueue_state workqueue_state_t;
typedef struct _workunit_queue workunit_queue_t;
typedef struct _workunit workunit_t;

#define WORKQUEUE(queue)        ((workqueue_t *)(queue))

/*
 * Work-Unit Free-Pool Size.
 * Used to avoid malloc() call every additem().
 *      128 * sizeof(workunit_t) = 3KiB
 *
 * TODO: We can do better... Because now we've one malloc()
 *       every additem() with empty pool.
 *       Request 128 workunit each malloc() call?
 */
#define WORKUNIT_POOL_SIZE      (128)

/*
 * Macros to wrap malloc() and free() functions.
 */
#define __mmalloc(size)         malloc(size)
#define __mmfree(ptr)           free(ptr)

enum _workqueue_state {
    WORKQUEUE_STATE_ACTIVE,
    WORKQUEUE_STATE_CLOSED,
};

struct _workunit {
    workunit_t *next;                   /* Pointer to the next Work Unit */

    workunit_func_t func;               /* Work Unit Function */
    void *args;                         /* Work Unit Args */
};

struct _workunit_queue {
    workunit_t *first;                  /* First Unit of Work */
    workunit_t *last;                   /* Last Unit of Work */

    workunit_t *pool;                   /* Free Unit Blocks, reuse this */
    unsigned int free;                  /* Number of free units blocks */
};

struct _workqueue {
    workunit_queue_t  units;            /* Worker Units of Work Queue */

    unsigned int      ncore;            /* Number of Worker Thread */
    pthread_t         *core;            /* Worker Thread's Refs */
    pthread_mutex_t   lock;             /* Global Worker Lock */

    workqueue_state_t state;            /* Worker State */
};


/*
 * Allocate a new work unit. Try to avoid malloc() using a free unit pool
 * in the worker units queue.
 */
static workunit_t *__workunit_pool_alloc (workqueue_t *queue) {
    workunit_queue_t *unitq = &(queue->units);
    workunit_t *unit;

    if (unitq->pool != NULL) {
        unit = unitq->pool;
        unitq->pool = unit->next;
        unit->next = NULL;
    } else if ((unit = (workunit_t *) __mmalloc(sizeof(workunit_t))) != NULL) {
        unit->next = NULL;
    }

    return(unit);
}

/*
 * Release specified work unit. If work units queue free pool is "full"
 * really call free() to release memory.
 */
static void __workunit_pool_free (workqueue_t *queue, workunit_t *unit) {
    workunit_queue_t *unitq = &(queue->units);

    if (unitq->free == WORKUNIT_POOL_SIZE) {
        __mmfree(unit);
    } else {
        unit->next = unitq->pool;
        unitq->pool = unit;
    }
}

/*
 * Dequeue one item from the Work Queue.
 * Returns 0 if there's no item, one if item is fetched.
 */
static int __workqueue_fetch (workqueue_t *queue, workunit_t *unit) {
    workunit_t *mmunit;
    int fetched = 0;

    pthread_mutex_lock(&(queue->lock));

    if ((mmunit = queue->units.first) != NULL) {
        queue->units.first = mmunit->next;
        if (queue->units.last == mmunit)
            queue->units.last = NULL;

        /* Copy unit on worker unit and release from pool */
        unit->func = mmunit->func;
        unit->args = mmunit->args;
        __workunit_pool_free(queue, mmunit);

        fetched = 1;
    }

    pthread_mutex_unlock(&(queue->lock));

    return(fetched);
}

/*
 * Work Thread core loop:
 *      While work queue is active, try to dequeue an item and execute it.
 */
static void *__workqueue_loop (void *queue) {
    workunit_t unit;

    while (WORKQUEUE(queue)->state != WORKQUEUE_STATE_CLOSED) {
        if (!__workqueue_fetch(WORKQUEUE(queue), &unit)) {
            usleep(15);
            continue;
        }

        unit.func(unit.args);
    }

    return(NULL);
}

/**
 * Allocate a new Work Queue with n-threads where n is number of cpu core * 2.
 * Returns NULL if something fails else queue reference is returned.
 */
workqueue_t *workqueue_alloc (void) {
    workqueue_t *queue;
    pthread_t *tid;
    long ncore;
    void *mm;
    int err;

    /* How many processors we've? x*2 is a good thread number */
    ncore = sysconf(_SC_NPROCESSORS_ONLN) << 1;

    /* Allocate Memory for the Work Queue */
    if (!(mm = __mmalloc(sizeof(workqueue_t) + ncore * sizeof(pthread_t))))
        return(NULL);

    /* Assign Pointers to previous allocated memory */
    queue = (workqueue_t *)mm;
    queue->core = (pthread_t *)(mm + sizeof(workqueue_t));

    /* Init Work Unit Queue */
    queue->units.first = NULL;
    queue->units.last = NULL;
    queue->units.pool = NULL;
    queue->units.free = 0;

    /* Initialize the rest of the Work Queue */
    pthread_mutex_init(&(queue->lock), NULL);
    queue->state = WORKQUEUE_STATE_ACTIVE;
    queue->ncore = ncore;

    /* Create Worker Threads */
    while (ncore--) {
        tid = &(queue->core[ncore]);
        if (pthread_create(tid, NULL, __workqueue_loop, queue) != 0) {
            /* Huston we've a failure, rollback! */
            queue->state = WORKQUEUE_STATE_CLOSED;
            for (; ncore < queue->ncore; ++ncore)
                pthread_join(queue->core[ncore], NULL);
            free(queue);
            return(NULL);
        }
    }

    return(queue);
}

/**
 * Release work queue. And it's own resources.
 */
void workqueue_release (workqueue_t *queue) {
    workunit_queue_t *unitq = &(queue->units);
    workunit_t *unit;

    /* Set Queue State to Closed */
    queue->state = WORKQUEUE_STATE_CLOSED;

    /* Wait for threads end */
    while (queue->ncore--)
        pthread_join(queue->core[queue->ncore], NULL);

    /* Release Queue Resources (Not Processed Work Units) */
    while ((unit = unitq->first) != NULL) {
        unitq->first = unit->next;
        __mmfree(unit);
    }

    /* Release Queue Resources (Work Units Free Pool) */
    while ((unit = unitq->pool) != NULL) {
        unitq->pool = unit->next;
        __mmfree(unit);
    }

    __mmfree(queue);
}

/**
 * Wait until there's no work-unit in the queue.
 */
void workqueue_wait (workqueue_t *queue) {
    int wait = 1;
    while (wait) {
        usleep(50);
        pthread_mutex_lock(&(queue->lock));
        wait = (queue->units.first != NULL);
        pthread_mutex_unlock(&(queue->lock));
    }
}

/**
 * Add Item to the Work Queue.
 * Returns 0 if item is added to the queue. else an error occurred.
 *
 * void _do_task (void *args) {
 *     struct task_args *info = (struct task_args *)args;
 *     ...
 *     task_data.xxx += task_data.xyz;
 *     ...
 * }
 *
 * int main (int argc, char **argv) {
 *       struct task_args task_data;
 *       workqueue_t *queue;
 *
 *       task_data.xxx = 10;
 *       task_data.xyz = 20;
 *
 *       queue = workqueue_alloc();
 *       workqueue_additem(queue, _do_task, &task_data);
 *       ...
 */
int workqueue_additem (workqueue_t *queue,
                       workunit_func_t func,
                       void *args)
{
    workunit_t *unit;

    if (queue->state != WORKQUEUE_STATE_ACTIVE)
        return(2);

    pthread_mutex_lock(&(queue->lock));

    if ((unit = __workunit_pool_alloc(queue)) != NULL) {
        unit->func = func;
        unit->args = args;

        if (queue->units.first == NULL)
            queue->units.first = unit;
        else
            queue->units.last->next = unit;
        queue->units.last = unit;
    }

    pthread_mutex_unlock(&(queue->lock));

    return(unit == NULL);
}

