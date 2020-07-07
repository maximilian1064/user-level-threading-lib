#include "handler.h"
#include "flexus.h"

int main()
{
    BREAKPOINT();
    uint64_t a;
    while (1) {
        a = get_cpu_cycle();
        call_magic_3_64(42, 12, a, a);
    }
}