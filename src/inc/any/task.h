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
ANY_API aerror_t atask_shadow(struct atask_t* self);

/// TODO
ANY_API aerror_t atask_create(
    struct atask_t* self, struct atask_t* root,
    atask_entry_t entry, void* ud, int32_t stack_sz);

/// TODO
ANY_API void atask_delete(struct atask_t* self);

/// TODO
ANY_API void atask_yield(struct atask_t* self);

/// TODO
ANY_API void atask_sleep(struct atask_t* self, int32_t nsecs);

#ifdef __cplusplus
} // extern "C"
#endif
