.text
.align  2
.global rr_dram_miss_handler
.global rr_handler_scheduling 
#.global running_tid
.type   rr_dram_miss_handler, %function
rr_dram_miss_handler:
    # unset switching flag
    orr x14, x14, #0x8000000000000000

    # preserve thread execution state (x{0-30}, NZCV, PC, SP)
    # TODO: preserving q{0-31} as an option to the user of the threads
    # TODO: preserving other Aarch64 execution states (FPSR, FPCR...)

    # load x0-x18 & nzcv
    sub  sp, sp, #0x90

    stp  x0, x1, [sp, #0x00]

    # measuring latency of DRAM miss handling
    mov x0, #0x2a
    mov x1, #0xd
    orr x30,x30,x30 /* <------ This is where the magic happens */

    stp  x2, x3, [sp, #0x10]
    stp  x4, x5, [sp, #0x20]
    stp  x6, x7, [sp, #0x30]
    stp  x8, x9, [sp, #0x40]
    stp  x10, x11, [sp, #0x50]
    stp  x12, x13, [sp, #0x60]
    stp  x16, x17, [sp, #0x70]
    str  x18, [sp, #0x80]
    mrs  x18, nzcv
    str  x18, [sp, #0x88]
    
    # save x19-x30
    sub  sp, sp, #0x70

    stp  x19, x20, [sp, #0x00]
    stp  x21, x22, [sp, #0x10]
    stp  x23, x24, [sp, #0x20]
    stp  x25, x26, [sp, #0x30]
    stp  x27, x28, [sp, #0x40]
    stp  x29, x30, [sp, #0x50]

    # save PC & context identifier
    # context identifier is used to differentiate between contexts saved due to yield() and DRAM miss
    mov  x19, #0x1
    stp  x15, x19, [sp, #0x60]

    # debuging
    # mov x0, #0x2a
    # mov x1, #0xc
    # mov x2, x15
    # ldr x13, =running_tid
    # ldr x3, [x13]
    # orr x30,x30,x30 /* <------ This is where the magic happens */

    # scheduling
    # save the sp of the current thread
    # load the sp of next thread
    mov x0, sp
    bl rr_handler_scheduling
    mov sp, x0

    # restore execution state of next thread 
    # load x19-x30
    ldp  x19, x20, [sp, #0x00]
    ldp  x21, x22, [sp, #0x10]
    ldp  x23, x24, [sp, #0x20]
    ldp  x25, x26, [sp, #0x30]
    ldp  x27, x28, [sp, #0x40]
    ldp  x29, x30, [sp, #0x50]

    # load PC & context identifier
    ldp  x15, x13, [sp, #0x60]
    add  sp, sp, #0x70

    # debuging
    # mov x0, #0x2a
    # mov x1, #0xc
    # mov x2, x15
    # ldr x12, =running_tid
    # ldr x3, [x12]
    # orr x30,x30,x30 /* <------ This is where the magic happens */

    cmp  x13, #0
    b.eq  1f

    # load x0-x18 & nzcv
    # ldp  x0, x1, [sp, #0x00]
    ldp  x2, x3, [sp, #0x10]
    ldp  x4, x5, [sp, #0x20]
    ldp  x6, x7, [sp, #0x30]
    ldp  x8, x9, [sp, #0x40]
    ldp  x10, x11, [sp, #0x50]
    ldp  x12, x13, [sp, #0x60]
    ldp  x16, x17, [sp, #0x70]
    ldr  x18, [sp, #0x88]
    msr  nzcv, x18
    ldr  x18, [sp, #0x80]

    # measuring latency of DRAM miss handling
    # only logged if returning to a thread re-scheduled from DRAM miss
    mov x0, #0x2a
    mov x1, #0xe
    orr x30,x30,x30 /* <------ This is where the magic happens */

    ldp  x0, x1, [sp, #0x00]
    
    add  sp, sp, #0x90

1:
    # set switching flag
    and x14, x14, #0x7fffffffffffffff

    # jump back to the inst. that misses
    br x15

.size   rr_dram_miss_handler,.-rr_dram_miss_handler
# Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits