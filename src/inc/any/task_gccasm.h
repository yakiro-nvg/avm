/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#define ATASK_GCCASM

#include <any/platform.h>
#include <any/list.h>

typedef struct atask_ctx_t {
#if defined(AARCH_I386)
#error "TODO"
#elif defined(AARCH_AMD64)
    void* rip;
    void* rsp;
    void* rbp;
    void* rbx;
    void* r12;
    void* r13;
    void* r14;
    void* r15;
#elif defined(AARCH_ARM)
#error "TODO"
#endif
} atask_ctx_t;

typedef struct atask_t {
    atask_ctx_t ctx;
    void* stack;
#ifdef ANY_USE_VALGRIND
    unsigned int vgid;
#endif
    alist_node_t node;
} atask_t;
