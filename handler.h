#ifndef HANDLER_H
#define HANDLER_H

#include <stdint.h>

// Inline assemblies used to manage DRAM miss handler
// NOTE: compiler could reorder these inline assemblies in the code,
// fortunately it didn't happen in the user-level threading library
// but we need to make sure it never happens when we publish the code
// TODO: Consider using some (compiler/language) barriers or fence in the future

static inline __attribute__ ((always_inline))
void register_handler(void (*handler)(void))
{
    __asm__ __volatile__ (
        "mov x14, %0\n"
        : /* output registers*/
        : "r"(handler)      /* input registers*/
    );
}

static inline __attribute__ ((always_inline))
void enable_switching()
{
    __asm__ __volatile__ (
        "and x14, x14, #0x7fffffffffffffff\n"
    );
}

static inline __attribute__ ((always_inline))
void disable_switching()
{
    __asm__ __volatile__ (
        "orr x14, x14, #0x8000000000000000\n"
    );
}

// Timer for astriflash priority-scheduling
static inline __attribute__ ((always_inline))
uint64_t get_cpu_cycle()
{
    uint64_t  ret_value;
    __asm__ __volatile__ (
        "mov x0, #0x2a\n"
        "mov x1, #0xb\n"
        "orr x30,x30,x30\n" /* <------ This is where the magic happens */
        "mov %0, x0\n"      /* Flexus can write x0 for the return value */
        : "=r"(ret_value)     /* output registers*/
        :
        : "x0", "x1", "x30"   /* clobbered registers*/
    );
    return ret_value;
}

#endif