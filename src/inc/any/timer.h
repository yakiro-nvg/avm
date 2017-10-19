/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AWINDOWS)

static AINLINE aint_t
atimer_usecs()
{
    LARGE_INTEGER c, f;
    QueryPerformanceCounter(&c);
    QueryPerformanceFrequency(&f);
    return (aint_t)((c.QuadPart / (areal_t)f.QuadPart) * 1000000);
}

#elif defined(ALINUX)

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <time.h>

static AINLINE aint_t
atimer_usecs()
{
    struct timespec c;
    clock_gettime(CLOCK_MONOTONIC, &c);
    return (aint_t)(c.tv_nsec / 1000 + c.tv_sec * 1000000);
}

#elif defined(AAPPLE)

#include <mach/mach_time.h>

typedef uint64_t atimer_t;

static AINLINE aint_t
atimer_usecs()
{
    uint64_t c = mach_absolute_time();
    mach_timebase_info_data_t time_base_info;
    mach_timebase_info(&time_base_info);
    return ((c * time_base_info.numer) / time_base_info.denom) / 1000;
}

#else
#error "not supported"
#endif

#ifdef __cplusplus
} // extern "C"
#endif
