/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>
#include <any/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(ASTDCALL*atask_entry_t)(void*);

#if defined(ANY_TASK_FIBER)
#include <any/task_fiber.h>
#elif defined(ANY_TASK_GCCASM)
#include <any/task_gccasm.h>
#endif

/// Turn caller into a task.
ANY_API aerror_t
atask_shadow(
    struct atask_t* self);

/// Create a new cooperative task.
ANY_API aerror_t
atask_create(
    struct atask_t* self, atask_entry_t entry, void* ud, aint_t stack_sz);

/// Delete a task.
ANY_API void
atask_delete(
    struct atask_t* self);

/// Switch to `next` task.
ANY_API void
atask_yield(
    struct atask_t* self, struct atask_t* next);

#ifdef __cplusplus
} // extern "C"
#endif
