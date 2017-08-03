/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */

#ifdef ANY_TASK_GCCASM

.text

#ifdef __APPLE_CC__
#define ENTRYP _atask_ctx_entryp
#define SWITCH _atask_ctx_switch
#else
#define ENTRYP atask_ctx_entryp
#define SWITCH atask_ctx_switch
#endif

.globl ENTRYP
.globl SWITCH

#if defined(__i386__)
#error "TODO"
#elif defined(__x86_64__)
ENTRYP:
    movq    %r13, %rdi
    jmpq    *%r12
    ret

SWITCH:
    leaq    0x3D(%rip), %rax
    movq    %rax,       (%rdi)
    movq    %rsp,       8(%rdi)
    movq    %rbp,       16(%rdi)
    movq    %rbx,       24(%rdi)
    movq    %r12,       32(%rdi)
    movq    %r13,       40(%rdi)
    movq    %r14,       48(%rdi)
    movq    %r15,       56(%rdi)
    movq    56(%rsi),   %r15
    movq    48(%rsi),   %r14
    movq    40(%rsi),   %r13
    movq    32(%rsi),   %r12
    movq    24(%rsi),   %rbx
    movq    16(%rsi),   %rbp
    movq    8(%rsi),    %rsp
    jmpq    *(%rsi)
    ret
#elif defined(__arm__)
#error "TODO"
#endif

#endif // ANY_TASK_GCCASM
