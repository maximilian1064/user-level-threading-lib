// TODO: Proper error handling

#include <stdlib.h>   /* exit(), EXIT_SUCCESS, EXIT_FAILURE, malloc(), free() */
#include <stdbool.h>  /* true, false */
#include <string.h>

#include "threads.h"
#include "scheduler.h"
#include "handler.h"
#include "afcontext/afcontext.h"

/* Stack size for each context. */
#define STACK_SIZE 65536 

af_thread_t thread[MAX_THR_NUM];
af_scheduler_t af_scheduler;
// TODO: keep a list of free tids
static tid_t free_tid;

tid_t running_tid;
// TODO: keep a hash map of waiting-waited tid pairs
static tid_t waiting_tid, waited_tid;

typedef struct thr_func {
    void (* func)(void*);
    void* args;
} thr_func_t;

/*waiting list*/

// TODO: recycle freed tids
static void thr_func_wrapper(thr_func_t* thr_func)
{
    // disable switching when loading func and args
    // memory pointed by thr_func could be invalidated if miss happens
    // THIS CODE (the ldp part) may not be PORTABLE, 
    // but anyway it works in our ubuntu image
    // with gcc 5.4.0
    void (* start)(void*); 
    void* args;
    __asm__ __volatile__ (
        "orr x14, x14, #0x8000000000000000\n"
        "ldp %0, %1, [%2]\n"
        "and x14, x14, #0x7fffffffffffffff\n"
        : "=r"(start), "=r"(args)     /* output registers*/
        : "r"(thr_func)
        : /* clobbered registers*/
    );

    start(args);
    
    disable_switching();

    // terminate current thread
    thread[running_tid].state = terminated;
    // mark the waiting thread as ready
    if (running_tid == waited_tid) {
        thread[waiting_tid].state = ready;
        af_scheduler.enqueue(waiting_tid);
        waiting_tid = -1;
        waited_tid = -1;
    }
    // switch to the next ready thread or exit
    tid_t prev_tid = running_tid;
    tid_t temp = af_scheduler.dequeue();
    if (temp == -1) {
        enable_switching();
        return;
    }
    else
        running_tid = temp;

    thread[running_tid].state = running;
    jump_afcontext(&(thread[prev_tid].ctx), thread[running_tid].ctx, NULL);
}

int af_thread_init(af_scheduler_t scheduler){
    // register scheduler
    af_scheduler = scheduler;
    af_scheduler.init();

    free_tid = 0;
    running_tid = 0;
    // no waiting thread in the beginning
    waiting_tid = -1;
    waited_tid = -1;
    // init main thread
    thread[free_tid].tid = free_tid;
    thread[free_tid].state = running;
    // context of main thread will be automatically initialized when spawning the first thread
    // thread[free_tid].ctx = NULL;
    // init stack for tid 0 in case the tid is used by other threads
    // not done here since we assume tid 0 will always be used by main thread
    // thread[free_tid].stack.sp = malloc(STACK_SIZE);
    // thread[free_tid].stack.stack_size = STACK_SIZE;

    free_tid++;

    // pre-allocate stack for all threads
    // Note: done for simulation purpose
    char* stack_mem = NULL;
    posix_memalign(&stack_mem, 4096, STACK_SIZE * MAX_THR_NUM);
    // TEMPORARY: for simulation purpose
    // avoid page faults in simulation
    // work around flexus's limitaions
    memset(stack_mem, 0, STACK_SIZE * MAX_THR_NUM);

    // debugging
    printf("stack mem: %p stack size: %d\n", stack_mem, STACK_SIZE);

    for (tid_t i = 0; i < MAX_THR_NUM; i++) {
        thread[i].stack.sp = (void*)(stack_mem + i * STACK_SIZE);
        thread[i].stack.stack_size = STACK_SIZE;
    }

    // init handler & switching flag
    register_handler(af_scheduler.dram_miss_handler);
    disable_switching();

    return 1;
}


// TODO: support args & return value
// Note: the memory pointed by args must outlive the created thread
tid_t af_thread_create(void (*start)(void*), void* args){
    disable_switching();

    tid_t tid = free_tid;
    free_tid++;
    
    thread[tid].tid = tid;
    thread[tid].state = running;

    tid_t prev_tid = running_tid;
    thread[prev_tid].state = ready;
    running_tid = tid;
    af_scheduler.enqueue(prev_tid);

    // thread[tid].stack.sp = malloc(STACK_SIZE);
    // thread[tid].stack.stack_size = STACK_SIZE;
    thread[tid].ctx = make_afcontext(thread[tid].stack.sp, thread[tid].stack.stack_size, (void (*)(void))thr_func_wrapper);

    thr_func_t thr_func = {
        .func = start,
        .args = args
    };

    jump_afcontext(&(thread[prev_tid].ctx), thread[tid].ctx, &thr_func);

    return tid;
}

void af_thread_yield(){
    disable_switching();

    tid_t prev_tid = running_tid;
    tid_t temp = af_scheduler.dequeue();
    if (temp == -1) {
        enable_switching();
        return;
    }
    else
        running_tid = temp;

    thread[prev_tid].state = ready;
    af_scheduler.enqueue(prev_tid);
    thread[running_tid].state = running;

    jump_afcontext(&(thread[prev_tid].ctx), thread[running_tid].ctx, NULL);
}

void af_thread_yield_pending(){
    disable_switching();

    tid_t prev_tid = running_tid;
    tid_t temp = af_scheduler.dequeue_pending();
    if (temp == -1) {
        enable_switching();
        return;
    }
    else
        running_tid = temp;

    thread[prev_tid].state = ready;
    af_scheduler.enqueue(prev_tid);
    thread[running_tid].state = running;

    jump_afcontext(&(thread[prev_tid].ctx), thread[running_tid].ctx, NULL);
}

void af_thread_join(tid_t tid) {
    // TODO: detect if tid is valid
    disable_switching();
    
    if (thread[tid].state == terminated) {
        enable_switching();
        return;
    }

    thread[running_tid].state = waiting;
    waiting_tid = running_tid;
    waited_tid = tid;

    // switch to the next ready thread
    tid_t prev_tid = running_tid;
    running_tid = af_scheduler.dequeue();
    thread[running_tid].state = running;

    jump_afcontext(&(thread[prev_tid].ctx), thread[running_tid].ctx, NULL);
}