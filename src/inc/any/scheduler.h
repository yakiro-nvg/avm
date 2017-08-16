/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new scheduler.
\brief
The `apid_t` consists of two parts are `index` and `generation`, its lengths can
be configured by `idx_bits` and `gen_bits`. The index will be used to directly
lookup for a process from array. In additional, the generation part also be used
to distinguish processes created at the same index slot. That caused by limited
size of process array so eventually the index will be reused.
\note This will shadow current thread.
*/
ANY_API aerror_t ascheduler_init(
    ascheduler_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud);

/// Release all processes.
ANY_API void ascheduler_cleanup(ascheduler_t* self);

/** Get alive actor by pid.
\return NULL if that is not found or died.
*/
static AINLINE aactor_t* ascheduler_actor(ascheduler_t* self, apid_t pid)
{
    apid_idx_t idx = apid_idx(self->idx_bits, pid);
    aprocess_t* p = self->procs + idx;
    if (idx >= (apid_idx_t)(1 << self->idx_bits)) return NULL;
    if (p->pid == pid && !p->dead) return &p->actor;
    else return NULL;
}

/** Take an unused slot from the pool.
\return NULL if no more space.
*/
ANY_API aprocess_t* ascheduler_alloc(ascheduler_t* self);

/// Return this process to the pool.
static AINLINE void ascheduler_free(aprocess_t* p)
{
    p->dead = TRUE;
}

/// Get pid of this actor.
static AINLINE apid_t ascheduler_pid(aactor_t* a)
{
    return ACAST_FROM_FIELD(aprocess_t, a, actor)->pid;
}

/// Run all processes, must be called on the creation thread.
ANY_API void ascheduler_run_once(ascheduler_t* self);

/** Suspends this actor, and switch to next.
\warning Suspends NOT running actor is undefined.
*/
ANY_API void ascheduler_yield(ascheduler_t* self, aactor_t* a);

/** Suspends this actor for `nsecs`.
\warning Suspends NOT running actor is undefined.
*/
ANY_API void ascheduler_sleep(ascheduler_t* self, aactor_t* a, aint_t nsecs);

/** Wait for incoming message in `nsecs`.
\warning Suspends NOT running actor is undefined.
\return Elapsed time.
*/
ANY_API aint_t ascheduler_wait(ascheduler_t* self, aactor_t* a, aint_t nsecs);

/// Wake-up this actor if its waiting for incoming message.
ANY_API void ascheduler_got_new_message(ascheduler_t* self, aactor_t* a);

/** Create a new actor, and store its pointer to `a`.
\note Must be started manually.
*/
ANY_API aerror_t ascheduler_new_actor(
    ascheduler_t* self, aint_t cstack_sz, aactor_t** a);

/// Start actor and invoke the entry point.
ANY_API void ascheduler_start(ascheduler_t* self, aactor_t* a, aint_t nargs);

#ifdef __cplusplus
} // extern "C"
#endif