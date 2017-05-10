/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <assert.h>
#include <any/platform.h>

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
    *fiber = CreateFiber(0, func, ud);
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

AINLINE void afiber_create(afiber_t* fiber, void(ASTDCALL*func)(void*), void* ud)
{
    makecontext(fiber, func, 1, ud);
}

AINLINE void afiber_destroy(afiber_t* fiber)
{
    // nop
    AUNUSED(fiber);
}

AINLINE void afiber_switch(afiber_t* self, const afiber_t* to)
{
    swapcontext(self, to);
}
#endif