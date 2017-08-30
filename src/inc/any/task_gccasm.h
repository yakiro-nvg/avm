/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

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

#ifndef __ARM_EABI__
#error "TODO"
#endif

#if !__SOFTFP__
// d8-d15,
void* d[16];
#endif

// r4-r11
void* r[8];

void* lr;
void* sp;   

#endif
} atask_ctx_t;

typedef struct atask_t {
    atask_ctx_t ctx;
    void* stack;
#ifdef ANY_USE_VALGRIND
    unsigned int vgid;
#endif
} atask_t;
