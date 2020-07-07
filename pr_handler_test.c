#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#define FLEXUS
#ifdef FLEXUS
#include "flexus.h"
#endif
#include "threads.h"
#include "handler.h"

static uint64_t sum = 0;
static uint64_t* allocated_mem = NULL;
static uint64_t job_queue_size = 2000000;
static volatile uint64_t job_queue_head = 0;

void request(uint64_t id)
{
    uint64_t ind = id % job_queue_size;
    uint64_t data = allocated_mem[ind * 512];
    call_magic_3_64(42, 12, data, data);
    sum += data;
    call_magic_3_64(42, 12, sum, sum);
}

// func of first two threads that keep missing in DRAM cache
void bench(void* args)
{
    // For simulation:
    // Make sure all threads are started before entering flexus
    for (int i = 0; i < 3; i++)
        af_thread_yield();

    uint64_t num_jobs = job_queue_size * 100;
    while (job_queue_head < num_jobs) {
        // poor man's lock()
        disable_switching();
        job_queue_head++;
        // poor man's unlock()
        enable_switching();
        // run another job
        request(job_queue_head);
        //af_thread_yield();
    }
}

// func of the third thread that never misses
static volatile int a = 0;
void fibonacci_fast(void* args) {
  // For simulation:
  // Make sure all threads are started before entering flexus
  for (int i = 0; i < 3; i++)
    af_thread_yield();

  int b = 1;
  int n = 0;
  int next = a + b;

  while(1) {
    next = a + b;
    a = b;
    b = next;
    n++;
    if (a < 0) {
      // Restart on overflow.
      a = 0;
      b = 1;
      n = 0;
    }
    af_thread_yield_pending();
  }
}

int main()
{
    uint64_t ind;
    posix_memalign(&allocated_mem, 4096, 4096ULL * 2000000ULL);
    printf("allocated_mem: %lx\n", allocated_mem);
    for (ind = 0; ind < 2000000; ind++)
        allocated_mem[ind * 512] = ind;
    
    tid_t threads[3];
    af_thread_init(pr_scheduler);

#ifdef FLEXUS
    BREAKPOINT();
#endif

    for (int i = 0; i < 2; i++)
        threads[i] = af_thread_create(bench, NULL);
    threads[2] = af_thread_create(fibonacci_fast, NULL);
    
    for (int i = 0; i < 3; i++)
        af_thread_join(threads[i]);

    printf("sum: %ld\n", sum);
}