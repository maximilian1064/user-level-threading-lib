/* 
   Implementation of jump_afcontext()
   Adapted from https://raw.githubusercontent.com/boostorg/context/boost-1.60.0/src/asm/jump_arm64_aapcs_elf_gas.S
*/

/*
            Copyright Edward Nevill 2015
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/
/*******************************************************
 *                                                     *
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  | 0x0 | 0x4 | 0x8 | 0xc | 0x10| 0x14| 0x18| 0x1c|  *
 *  -------------------------------------------------  *
 *  |    d8     |    d9     |    d10    |    d11    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  | 0x20| 0x24| 0x28| 0x2c| 0x30| 0x34| 0x38| 0x3c|  *
 *  -------------------------------------------------  *
 *  |    d12    |    d13    |    d14    |    d15    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  *
 *  -------------------------------------------------  *
 *  | 0x40| 0x44| 0x48| 0x4c| 0x50| 0x54| 0x58| 0x5c|  *
 *  -------------------------------------------------  *
 *  |    x19    |    x20    |    x21    |    x22    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31 |  *
 *  -------------------------------------------------  *
 *  | 0x60| 0x64| 0x68| 0x6c| 0x70| 0x74| 0x78| 0x7c|  *
 *  -------------------------------------------------  *
 *  |    x23    |    x24    |    x25    |    x26    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  32 |  33 |  34 |  35 |  36 |  37 |  38 |  39 |  *
 *  -------------------------------------------------  *
 *  | 0x80| 0x84| 0x88| 0x8c| 0x90| 0x94| 0x98| 0x9c|  *
 *  -------------------------------------------------  *
 *  |    x27    |    x28    |    FP     |     LR    |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  40 |  41 |  42 | 43  |           |           |  *
 *  -------------------------------------------------  *
 *  | 0xa0| 0xa4| 0xa8| 0xac|           |           |  *
 *  -------------------------------------------------  *
 *  |     PC    |   align   |           |           |  *
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

.text
.align  2
.global jump_afcontext
.type   jump_afcontext, %function
jump_afcontext:
    sub  sp, sp, #0x70

    # save d8 - d15
    # stp  q8,  q9,  [sp, #0x00]
    # stp  q10, q11, [sp, #0x20]
    # stp  q12, q13, [sp, #0x40]
    # stp  q14, q15, [sp, #0x60]

    # save x19-x30
    stp  x19, x20, [sp, #0x00]
    stp  x21, x22, [sp, #0x10]
    stp  x23, x24, [sp, #0x20]
    stp  x25, x26, [sp, #0x30]
    stp  x27, x28, [sp, #0x40]
    stp  x29, x30, [sp, #0x50]

    # save PC & context identifier
    # context identifier is used to differentiate context saved due to yield() and DRAM miss
    mov  x19, #0x0
    stp  x30, x19, [sp, #0x60]

    # store RSP (pointing to context-data) in first argument (x0).
    # STR cannot have sp as a target register
    mov  x4, sp
    str  x4, [x0]

    # restore RSP (pointing to context-data) from A2 (x1)
    mov  sp, x1

    # load arg of context func from A3 (x2)
    mov  x0, x2

    # load d8 - d15
    # ldp  q8,  q9,  [sp, #0x00]
    # ldp  q10, q11, [sp, #0x20]
    # ldp  q12, q13, [sp, #0x40]
    # ldp  q14, q15, [sp, #0x60]

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

    cmp  x13, #0
    b.eq  1f

    # load x0-x18 & nzcv
    ldp  x0, x1, [sp, #0x00]
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
    
    add  sp, sp, #0x90

1:
    # set switching flag
    and x14, x14, #0x7fffffffffffffff

    br x15

.size   jump_afcontext,.-jump_afcontext
# Mark that we don't need executable stack.
.section .note.GNU-stack,"",%progbits
