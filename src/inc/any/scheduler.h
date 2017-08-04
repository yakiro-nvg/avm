/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new scheduler.
\brief It will shadow current thread.
*/
ANY_API aerror_t ascheduler_init(
    ascheduler_t* self, avm_t* vm, aalloc_t alloc, void* alloc_ud);

/// Release all processes.
ANY_API void ascheduler_cleanup(ascheduler_t* self);

/// Run all processes, must be called on the creation thread.
ANY_API void ascheduler_run_once(ascheduler_t* self);

/** Create a new process, and store its pointer to `p`.
\note Must be started manually.
*/
ANY_API aerror_t ascheduler_new_process(ascheduler_t* self, aprocess_t** p);

#ifdef __cplusplus
} // extern "C"
#endif