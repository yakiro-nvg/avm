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
\return AERR_NONE if successful.
*/
ANY_API int32_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud);

/// Gracefully cleanup the VM.
ANY_API void avm_shutdown(avm_t* self);

/** Lock for alive process by pid.
\return NULL if that is not found or died.
*/
AINLINE avm_process_t* avm_lock_pid(avm_t* self, apid_t pid)
{
    apid_idx_t idx = apid_idx(self->idx_bits, pid);
    avm_process_t* vp = self->procs + idx;
    if (idx >= (apid_idx_t)(1 << self->idx_bits)) return NULL;
#ifdef ANY_SMP
    amutex_lock(&vp->mutex);
#endif
    if (vp->p.pid == pid && !vp->dead) return vp;
    else {
#ifdef ANY_SMP
        amutex_unlock(&vp->mutex);
#endif
        return NULL;
    }
}

/// Unlock a process.
AINLINE void avm_unlock(avm_process_t* vp)
{
#ifdef ANY_SMP
    amutex_unlock(&vp->mutex);
#else
    AUNUSED(vp);
#endif
}

/** Take an unused slot from the pool.
\return NULL if no more space.
*/
ANY_API avm_process_t* avm_alloc(avm_t* self);

/// Return this process to the pool.
AINLINE void avm_free(avm_process_t* vp)
{
    vp->dead = TRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif