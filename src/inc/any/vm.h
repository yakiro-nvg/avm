/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new Virtual Machine.
\brief
The `apid_t` consists of two parts are `index` and `generation`,
which lengths can be configured by `idx_bits` and `gen_bits`. The index will be
used to directly lookup for a process from `procs` array. The generation part is
used to distinguish processes created at the same index slot. That caused by the
natural limitation in the size of our `procs` array, which can't be too large so
eventually the index will be reused. AVM will not allocate memory for `procs`,
you must make sure that this much memory for this table is available.

\return AERR_NONE if successful.
*/
ANY_API int32_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits, aprocess_t* procs);

/// Graceful cleanup the VM.
ANY_API void avm_shutdown(avm_t* self);

/** Lock for alive process.
\return NULL if that is not found or died.
*/
AINLINE aprocess_t* avm_process_lock(avm_t* self, apid_t pid)
{
    apid_idx_t idx = apid_idx(self->_idx_bits, pid);
    aprocess_t* p = self->_procs + idx;
#ifdef ANY_SMP
    amutex_lock(&p->mutex);
#endif
    if (p->pid == pid && ((p->flags & APF_DEAD) == 0)) {
        return p;
    } else {
#ifdef ANY_SMP
        amutex_unlock(&p->mutex);
#endif
        return NULL;
    }
}

/** Lock for alive process that is belong to a scheduler.
\brief There is optimize to reduce contention for not `owned` processes.
\return NULL if that is not found or died.
*/
AINLINE aprocess_t* avm_process_lock_idx(
    avm_t* self, apid_idx_t idx, ascheduler_t* owner)
{
    aprocess_t* p = self->_procs + idx;
    if (p->owner != owner) return NULL;
#ifdef ANY_SMP
    amutex_lock(&p->mutex);
#endif
    if (p->owner == owner && ((p->flags & APF_DEAD) == 0)) {
        return p;
    } else {
#ifdef ANY_SMP
        amutex_unlock(&p->mutex);
#endif
        return NULL;
    }
}

/// Unlock a process.
AINLINE void avm_process_unlock(aprocess_t* p)
{
#ifdef ANY_SMP
    amutex_unlock(&p->mutex);
#else
    AUNUSED(p);
#endif
}

/** Take an unused slot from the pool.
\return NULL if no more space.
*/
ANY_API aprocess_t* avm_process_alloc(avm_t* self);

/// Return this process to the pool.
AINLINE void avm_process_free(aprocess_t* p)
{
    p->flags |= APF_DEAD;
}

/** Dispatch cross-scheduler messages.
\brief Depends on the number of pending messages, and also potential contention for
resources with schedulers, this function could be blocked for awhile. Therefore it
is fine to run this function on background thread, if you are facing timing problems.
The mechanisms is simple, it will in turns flush the outgoing queues for schedulers
and there are no transaction at all. That means we have no completeness guarantee.
This function should be called over and over again.
\note `schedulers` must be NULL terminated.
*/
ANY_API void avm_migrate_messages(avm_t* self, ascheduler_t** schedulers);

#ifdef __cplusplus
} // extern "C"
#endif