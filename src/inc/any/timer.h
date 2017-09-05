/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AWINDOWS)

typedef LARGE_INTEGER atimer_t;

static AINLINE void
atimer_start(
    atimer_t* self)
{
    QueryPerformanceCounter(self);
}

static AINLINE aint_t
atimer_delta_nsecs(
    atimer_t* self)
{
    LARGE_INTEGER end, delta, frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&end);
    delta.QuadPart = end.QuadPart - self->QuadPart;
    delta.QuadPart *= 1000000000;
    delta.QuadPart /= frequency.QuadPart;
    *self = end;
    return (aint_t)delta.QuadPart;
}

#elif defined(ALINUX)

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <time.h>

typedef struct timespec atimer_t;

static AINLINE void
time_diff(
    const struct timespec* start, const struct timespec* end,
    struct timespec* r)
{
    if ((end->tv_nsec - start->tv_nsec) < 0) {
        r->tv_sec = end->tv_sec - start->tv_sec - 1;
        r->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        r->tv_sec = end->tv_sec - start->tv_sec;
        r->tv_nsec = end->tv_nsec - start->tv_nsec;
    }
}

static AINLINE void
atimer_start(
    atimer_t* self)
{
    clock_gettime(CLOCK_MONOTONIC, self);
}

static AINLINE aint_t
atimer_delta_nsecs(
    atimer_t* self)
{
    struct timespec end, delta;
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_diff(self, &end, &delta);
    *self = end;
    return (aint_t)(delta.tv_nsec + delta.tv_sec*1000000000);
}

#elif defined(AAPPLE)

#include <mach/mach_time.h>

typedef uint64_t atimer_t;

static AINLINE void
atimer_start(
    atimer_t* self)
{
    *self = mach_absolute_time();
}

static AINLINE aint_t
atimer_delta_nsecs(
    atimer_t* self)
{
    uint64_t end = mach_absolute_time(), delta;
    mach_timebase_info_data_t time_base_info;
    mach_timebase_info(&time_base_info);
    delta = end - *self;
    delta = (delta*time_base_info.numer) / time_base_info.denom;
    *self = end;
    return (aint_t)delta;
}

#else
#error "not supported"
#endif

#ifdef __cplusplus
} // extern "C"
#endif
