/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <assert.h>
#include <stdlib.h>
#include <any/platform.h>

enum { AFIBER_STACK_SZ = 4096 };

#ifdef AWINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef void* afiber_t;

AINLINE void afiber_get(afiber_t* fiber)
{
    PVOID f = GetCurrentFiber();
    *fiber = (f && f != (void*)0x1e00) ? f : ConvertThreadToFiber(NULL);
}

AINLINE void afiber_create(afiber_t* fiber, void(ASTDCALL*func)(void*), void* ud)
{
    *fiber = CreateFiber((SIZE_T)AFIBER_STACK_SZ, func, ud);
}

AINLINE void afiber_destroy(afiber_t* fiber)
{
    DeleteFiber(*fiber);
}

AINLINE void afiber_switch(afiber_t* self, const afiber_t* to)
{
    AUNUSED(self);
    assert(*self == GetCurrentFiber());
    SwitchToFiber(*to);
}
#elif defined(AAPPLE) || defined(ALINUX) 
#include <ucontext.h>
typedef ucontext_t afiber_t;

AINLINE void afiber_get(afiber_t* fiber)
{
    getcontext(fiber);
}

#ifdef A32_BITS
AINLINE void __afiber_do(int func, int ud) 
{
    ((void(ASTDCALL*)(void*))(func))((void*)ud);
}
#elif defined(A64_BITS)
AINLINE void __afiber_do(int lf, int hf, int lu, int hu)
{
    uint64_t hf64 = (uint32_t)hf, lf64 = (uint32_t)lf;
    uint64_t hu64 = (uint32_t)hu, lu64 = (uint32_t)lu;
    uint64_t f = (hf64 << 32) | lf64;
    uint64_t u = (hu64 << 32) | lu64;
    ((void(ASTDCALL*)(void*))(f))((void*)u);
}
#endif

AINLINE void afiber_create(afiber_t* fiber, void(ASTDCALL*func)(void*), void* ud)
{
    afiber_get(fiber);
    fiber->uc_stack.ss_sp = malloc(AFIBER_STACK_SZ);
    fiber->uc_stack.ss_size = AFIBER_STACK_SZ;
    fiber->uc_link = NULL;
#ifdef A32_BITS
    makecontext(fiber, (void(*)())&__afiber_do, 2, (int)func, (int)ud);
#else
    makecontext(fiber, (void(*)())&__afiber_do, 4, 
        (((uint64_t)func) << 32) >> 32, ((uint64_t)func) >> 32,
        (((uint64_t)ud) << 32) >> 32, ((uint64_t)ud) >> 32);
#endif
}

AINLINE void afiber_destroy(afiber_t* fiber)
{
    free(fiber->uc_stack.ss_sp);
}

AINLINE void afiber_switch(afiber_t* self, const afiber_t* to)
{
    swapcontext(self, to);
}
#endif