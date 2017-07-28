/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(ASTDCALL*atask_entry_t)(void*);

/// Turn caller into a task.
ANY_API int32_t atask_shadow(struct atask_s* self);

/// TODO
ANY_API int32_t atask_create(
    struct atask_s* self, struct atask_s* root,
    atask_entry_t entry, void* ud, int32_t stack_sz);

/// TODO
ANY_API void atask_delete(struct atask_s* self);

/// TODO
ANY_API void atask_yield(struct atask_s* self);

/// TODO
ANY_API void atask_sleep(struct atask_s* self, int32_t nsecs);

#ifdef AWINDOWS
#include <any/task_fiber.h>
#elif defined(ACLANG) || defined(AGNUC)
#include <any/task_gccasm.h>
#endif

#ifdef __cplusplus
} // extern "C"
#endif