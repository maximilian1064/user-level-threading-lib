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

void bench(void* args)
{
    // For simulation:
    // Make sure all threads are started before entering flexus
    for (int i = 0; i < 16; i++)
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

int main()
{
    uint64_t ind;
    posix_memalign(&allocated_mem, 4096, 4096ULL * 2000000ULL);
    printf("allocated_mem: %lx\n", allocated_mem);
    for (ind = 0; ind < 2000000; ind++)
        allocated_mem[ind * 512] = ind;
    
    tid_t threads[16];
    af_thread_init(rr_scheduler);

#ifdef FLEXUS
    BREAKPOINT();
#endif

    for (int i = 0; i < 16; i++)
        threads[i] = af_thread_create(bench, NULL);
    
    for (int i = 0; i < 16; i++)
        af_thread_join(threads[i]);

    printf("sum: %ld\n", sum);
}