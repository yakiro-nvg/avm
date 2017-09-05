/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/config.h>

#ifdef ANY_TASK_GCCASM

#ifdef __APPLE_CC__
#define ENTRYP "_atask_ctx_entryp"
#define SWITCH "_atask_ctx_switch"
#else
#define ENTRYP "atask_ctx_entryp"
#define SWITCH "atask_ctx_switch"
#endif

__asm__(
    ".text\n"
    ".globl " ENTRYP "\n"
    ".globl " SWITCH "\n"
);

#if defined(__i386__)
#error "TODO"
#elif defined(__x86_64__)
__asm__(
    ENTRYP ":\n"
        "movq    %r13, %rdi\n"
        "jmpq    *%r12\n"
        "ret\n"

    SWITCH ":\n"
        "leaq    0x3D(%rip), %rax\n"
        "movq    %rax,       (%rdi)\n"
        "movq    %rsp,       8(%rdi)\n"
        "movq    %rbp,       16(%rdi)\n"
        "movq    %rbx,       24(%rdi)\n"
        "movq    %r12,       32(%rdi)\n"
        "movq    %r13,       40(%rdi)\n"
        "movq    %r14,       48(%rdi)\n"
        "movq    %r15,       56(%rdi)\n"
        "movq    56(%rsi),   %r15\n"
        "movq    48(%rsi),   %r14\n"
        "movq    40(%rsi),   %r13\n"
        "movq    32(%rsi),   %r12\n"
        "movq    24(%rsi),   %rbx\n"
        "movq    16(%rsi),   %rbp\n"
        "movq    8(%rsi),    %rsp\n"
        "jmpq    *(%rsi)\n"
        "ret\n"
);
#elif defined(__ARM_EABI__)
__asm__(
    ENTRYP ":\n"
        "mov     %r0, %r4\n"
        "mov     %ip, %r5\n"
        "mov     %lr, %r6\n"
        "bx      %ip\n"

    SWITCH ":\n"
#if !__SOFTFP__
        "vstmia  r0!, {d8-d15}\n"
#endif
        "str     sp,  [r0, #9*4]\n"
        "stmia   r0,  {r4-r11, lr}\n"
#if !__SOFTFP__
        "vldmia  r1!, {d8-d15}\n"
#endif
        "ldr     sp,  [r1, #9*4]\n"
        "ldmia   r1,  {r4-r11, pc}\n"
);
#endif

#else // ANY_TASK_GCCASM
static char non_empty_unit;
#endif
