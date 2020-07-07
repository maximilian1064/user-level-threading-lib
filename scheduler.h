/* Common header for schedulers */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

// poor man's polymorphism
typedef struct af_scheduler {
    int (*enqueue) (tid_t tid);
    tid_t (*dequeue) (void);
    tid_t (*dequeue_pending) (void);
    void (*dram_miss_handler) (void);
    void (*init) (void);
} af_scheduler_t;

#endif
