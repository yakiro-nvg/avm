/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AWINDOWS)

typedef LARGE_INTEGER atimer_t;

static AINLINE void atimer_start(atimer_t* self)
{
    QueryPerformanceCounter(self);
}

static AINLINE aint_t atimer_elapsed_usecs(atimer_t* self)
{
    LARGE_INTEGER end, frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&end);
    end.QuadPart = end.QuadPart - self->QuadPart;
    end.QuadPart *= 1000000;
    end.QuadPart /= frequency.QuadPart;
    return (aint_t)end.QuadPart;
}

#elif defined(ALINUX)

#include <sys/time.h>

typedef timespec atimer_t;

static AINLINE void atimer_start(atimer_t* self)
{
    clock_gettime(CLOCK_MONOTONIC, self);
}

static AINLINE aint_t atimer_elapsed_usecs(atimer_t* self)
{
    timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (aint_t)((end.tv_nsec - self->tv_nsec) / 1000);
}

#elif defined(AAPPLE)

#include <mach/mach_time.h>

typedef uint64_t atimer_t;

static AINLINE void atimer_start(atimer_t* self)
{
    *self = mach_absolute_time();
}

static AINLINE aint_t atimer_elapsed_usecs(atimer_t* self)
{
    uint64_t end = mach_absolute_time();
    mach_timebase_info_data_t time_base_info;
    mach_timebase_info(&time_base_info);
    end = end - *self;
    end = (end * time_base_info.number) / time_base_info.denom;
    return (aint_t)(end / 1000);
}

#else
#error "not supported"
#endif

#ifdef __cplusplus
} // extern "C"
#endif
