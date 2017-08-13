/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new Virtual Machine.
\brief
The `apid_t` consists of two parts are `index` and `generation`, its lengths can
be configured by `idx_bits` and `gen_bits`. The index will be used to directly
lookup for a process from array. In additional, the generation part also be used
to distinguish processes created at the same index slot. That caused by limited
size of process array, eventually the index will be reused.
*/
ANY_API aerror_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud);

/// Gracefully cleanup the VM.
ANY_API void avm_shutdown(avm_t* self);

/** Get alive process by pid.
\return NULL if that is not found or died.
*/
static AINLINE aprocess_t* avm_from_pid(avm_t* self, apid_t pid)
{
    apid_idx_t idx = apid_idx(self->idx_bits, pid);
    avm_process_t* vp = self->procs + idx;
    if (idx >= (apid_idx_t)(1 << self->idx_bits)) return NULL;
    if (vp->pid == pid && !vp->dead) return &vp->p;
    else return NULL;
}

/** Take an unused slot from the pool.
\return NULL if no more space.
*/
ANY_API aprocess_t* avm_alloc(avm_t* self);

/// Return this process to the pool.
static AINLINE void avm_free(aprocess_t* p)
{
    ACAST_FROM_FIELD(avm_process_t, p, p)->dead = TRUE;
}

/// Get pid of this process.
static AINLINE apid_t avm_pid(aprocess_t* p)
{
    return ACAST_FROM_FIELD(avm_process_t, p, p)->pid;
}

#ifdef __cplusplus
} // extern "C"
#endif
