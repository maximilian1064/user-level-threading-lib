/* Priority-based scheduler */
#include <stdbool.h>
#include <stdint.h>
#include "threads.h"
#include "handler.h"
#include "scheduler.h"

// dram miss latency in cycles
#define DRAM_MISS_LATENCY 100000

extern af_thread_t thread[MAX_THR_NUM];
extern tid_t running_tid;

/* ready queue
 * holds tid of threads that voluntarily descheduled themselves
 */

static tid_t ready_queue[MAX_THR_NUM];
static int ready_queue_head, ready_queue_tail;
static bool ready_queue_full;

static inline bool ready_queue_is_empty()
{
    return ((ready_queue_head == ready_queue_tail) && (!ready_queue_full));
}

static inline int ready_queue_push(tid_t tid)
{
    if (!ready_queue_full) {
        ready_queue[ready_queue_tail] = tid;
        ready_queue_tail++;
        ready_queue_tail %= MAX_THR_NUM;
    } else {
        return 1;
    }
    if (ready_queue_tail == ready_queue_head)
        ready_queue_full = true;
    return 0;
}

static inline tid_t ready_queue_pop()
{
    if (ready_queue_is_empty())
        return -1;

    tid_t tid = ready_queue[ready_queue_head];
    ready_queue_head++;
    ready_queue_head %= MAX_THR_NUM;
    ready_queue_full = false;
    return tid;
}

/* miss queue
 * holds tid of threads that got descheduled due to miss in DRAM cache
 */

static tid_t miss_queue[MAX_THR_NUM];
static uint64_t miss_time[MAX_THR_NUM];
static int miss_queue_head, miss_queue_tail;
static bool miss_queue_full;

static inline bool miss_queue_is_empty()
{
    return ((miss_queue_head == miss_queue_tail) && (!miss_queue_full));
}

static inline int miss_queue_push(tid_t tid, uint64_t time)
{
    if (!miss_queue_full) {
        miss_queue[miss_queue_tail] = tid;
        miss_time[miss_queue_tail] = time;
        miss_queue_tail++;
        miss_queue_tail %= MAX_THR_NUM;
    } else {
        return 1;
    }
    if (miss_queue_tail == miss_queue_head)
        miss_queue_full = true;
    return 0;
}

static inline tid_t miss_queue_pop()
{
    if (miss_queue_is_empty())
        return -1;

    tid_t tid = miss_queue[miss_queue_head];
    miss_queue_head++;
    miss_queue_head %= MAX_THR_NUM;
    miss_queue_full = false;
    return tid;
}


// push the thread tid to the scheduler
// this means the thread is schedulable,
// HACK: this function is only called if a
// thread voluntarily deschedules itself
static int pr_scheduler_enqueue(tid_t tid)
{
    return ready_queue_push(tid);
}

// return the tid of the thread to be scheduled
// ret val of -1 means no thread is available for scheduling
static tid_t pr_scheduler_dequeue()
{
    tid_t ret;
    if (miss_queue_is_empty()) {
        ret = ready_queue_pop();
    } else {
        uint64_t time = get_cpu_cycle();
        // priority: pending thread in miss queue > thread in ready queue
        //           > non-pending thread in miss queue
        if (time > DRAM_MISS_LATENCY + miss_time[miss_queue_head])
            ret = miss_queue_pop();
        else if (!ready_queue_is_empty())
            ret = ready_queue_pop();
        else
            ret = miss_queue_pop();
    }
    return ret;
}

// NOTE!!!
// duplicated version of pr_scheduler_dequeue() for performance
// Do scheduling only if there is thread pending,
// i.e., a thread in miss queue that has waited enough time (50 us) for DRAM miss
static tid_t pr_scheduler_dequeue_pending()
{
    tid_t ret;
    if (miss_queue_is_empty()) {
        ret = -1;
    } else {
        uint64_t time = get_cpu_cycle();
        if (time > DRAM_MISS_LATENCY + miss_time[miss_queue_head])
            ret = miss_queue_pop();
        else
            ret = -1;
    }
    return ret;
}

// NOTE!!!
// duplicated version of pr_scheduler_dequeue() for performance
// This func is used inside DRAM miss handler
static inline tid_t pr_scheduler_dequeue_handler(uint64_t time)
{
    tid_t ret;
    if (miss_queue_is_empty()) {
        ret = ready_queue_pop();
    } else {
        // priority: pending thread in miss queue > thread in ready queue
        //           > non-pending thread in miss queue
        if (time > DRAM_MISS_LATENCY + miss_time[miss_queue_head])
            ret = miss_queue_pop();
        else if (!ready_queue_is_empty())
            ret = ready_queue_pop();
        else
            ret = miss_queue_pop();
    }
    return ret;
}

// init
static void pr_scheduler_init()
{
    // initialize ready queue
    ready_queue_head = ready_queue_tail = 0; 
    ready_queue_full = false;

    // initialize miss queue
    miss_queue_head = miss_queue_tail = 0; 
    miss_queue_full = false;
    for (int i = 0; i < MAX_THR_NUM; i++)
        miss_time[i] = 0;
}

// DRAM miss handler
void pr_dram_miss_handler();

// Scheduling function to be used in rr_dram_miss_handler
af_context_t pr_handler_scheduling(af_context_t o_ctx, uint64_t time)
{
    tid_t prev_tid = running_tid;
    tid_t temp = pr_scheduler_dequeue_handler(time);
    if (temp == -1)
        return o_ctx;
    else
        running_tid = temp;

    thread[prev_tid].state = ready;
    thread[prev_tid].ctx = o_ctx;
    miss_queue_push(prev_tid, time);

    thread[running_tid].state = running;
    return thread[running_tid].ctx;
}

// priority-based scheduler
af_scheduler_t pr_scheduler = {
    .enqueue = pr_scheduler_enqueue,
    .dequeue = pr_scheduler_dequeue,
    .dequeue_pending = pr_scheduler_dequeue_pending,
    .dram_miss_handler = pr_dram_miss_handler,
    .init = pr_scheduler_init
};
