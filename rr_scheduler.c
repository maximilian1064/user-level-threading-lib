/* Round-robin scheduler */
#include <stdbool.h>
#include <stdint.h>
#include "threads.h"
#include "scheduler.h"

extern af_thread_t thread[MAX_THR_NUM];
extern tid_t running_tid;

/*ready queue*/
static tid_t ready_queue[MAX_THR_NUM];
static int ready_queue_head, ready_queue_tail;
static bool queue_full;

static inline bool is_empty()
{
    return ((ready_queue_head == ready_queue_tail) && (!queue_full));
}

static inline int push(tid_t tid)
{
    if (!queue_full) {
        ready_queue[ready_queue_tail] = tid;
        ready_queue_tail++;
        ready_queue_tail %= MAX_THR_NUM;
    } else {
        return 1;
    }
    if (ready_queue_tail == ready_queue_head)
        queue_full = true;
    return 0;
}

static inline tid_t pop()
{
    if (is_empty())
        return -1;

    tid_t tid = ready_queue[ready_queue_head];
    ready_queue_head++;
    ready_queue_head %= MAX_THR_NUM;
    queue_full = false;
    return tid;
}

// push the thread tid to the scheduler
// this means the thread is schedulable,
static int rr_scheduler_enqueue(tid_t tid)
{
    return push(tid);
}

// return the tid of the thread to be scheduled
// ret val of -1 means no thread is available for scheduling
static tid_t rr_scheduler_dequeue()
{
    return pop();
}

// init
static void rr_scheduler_init()
{
    // initialize ready queue
    ready_queue_head = ready_queue_tail = 0; 
    queue_full = false;
}

// DRAM miss handler
void rr_dram_miss_handler();

// Scheduling function to be used in rr_dram_miss_handler
af_context_t rr_handler_scheduling(af_context_t o_ctx)
{
    tid_t prev_tid = running_tid;
    tid_t temp = rr_scheduler_dequeue();
    if (temp == -1)
        return o_ctx;
    else
        running_tid = temp;

    thread[prev_tid].state = ready;
    thread[prev_tid].ctx = o_ctx;
    rr_scheduler_enqueue(prev_tid);
    thread[running_tid].state = running;
    return thread[running_tid].ctx;
}

// round-robin scheduler
af_scheduler_t rr_scheduler = {
    .enqueue = rr_scheduler_enqueue,
    .dequeue = rr_scheduler_dequeue,
    .dequeue_pending = NULL,
    .dram_miss_handler = rr_dram_miss_handler,
    .init = rr_scheduler_init
};
