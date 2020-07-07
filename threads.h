#ifndef THREADS_H
#define THREADS_H

#ifdef __cplusplus
extern "C"{
#endif 

#include <stdlib.h>
#include "afcontext/afcontext.h"
#include "common.h"
#include "scheduler.h"

#define MAX_THR_NUM 64

/* A thread can be in one of the following states. */
typedef enum {running, ready, waiting, terminated} state_t;

typedef struct af_thread af_thread_t;

typedef struct af_stack af_stack_t;

// Avilable schedulers
extern af_scheduler_t rr_scheduler;
extern af_scheduler_t pr_scheduler;

struct af_stack {
  void* sp;
  size_t stack_size;
};

/* Data to manage a single thread should be kept in this structure. Here are a few
   suggestions of data you may want in this structure but you may change this to
   your own liking.
*/
struct af_thread {
  tid_t tid;
  state_t state;
  af_context_t ctx;
  af_stack_t stack;
};

/* Initialization

   Initialization of global thread management data structures. A user program
   must call this function exactly once before calling any other functions in
   the Simple Threads API.

   Returns 1 on success and a negative value on failure.
*/
int af_thread_init(af_scheduler_t scheduler);

/* Creates a new thread executing the start function.

   start - func of the thread
   args - only argument of func

   Note!!! the memory pointed by args must outlive the created thread

   On success the positive thread ID of the new thread is returned. On failure a
   negative value is returned. 
*/
tid_t af_thread_create(void (*start)(void*), void* args);

/* Cooperative scheduling

   If there are other threads in the ready state, a thread calling yield() will
   trigger the thread scheduler to dispatch one of the threads in the ready
   state and change the state of the calling thread from running to ready.
*/
void af_thread_yield();

/* Duplicated version of af_thread_yield for performance

   Detect if any thread is pending state (waited more than 50us after a miss).
   If there is, schedule the oldest pending thread.
*/
void af_thread_yield_pending();

/* Join with a terminated thread

   A thread calling join() will be suspended and change state from running to
   waiting and trigger the thread scheduler to dispatch one of the ready
   threads. The suspended thread will change state from waiting to ready once another
   thread calls done() and the join() should then return the thread id of the
   terminated thread.
*/
void af_thread_join(tid_t tid);

#ifdef __cplusplus
}
#endif

#endif
