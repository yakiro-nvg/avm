/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new scheduler.
\brief Scheduler will not allocate, grow or shrink its message queues.
\note `alloc` and `alloc_ud` is mandatory for non-borrowed processes.
\return AERR_NONE if successful.
*/
ANY_API int32_t ascheduler_init(
    ascheduler_t* self, avm_t* vm, adispatcher_t* bc_runner,
    ascheduler_mbox_t oqueues[2], ascheduler_mbox_t iqueues[2],
    aalloc_t alloc, void* alloc_ud);

/** Try to put a message into outgoing queue.
\return AERR_FULL if no more space, otherwise a non negative `load` factor of the
receiver will be returned to compute the reduction cost for this sending operation.
Because message sending to an overloaded process should cost more reduction than
usual case. This `load` factor is vary from 0 to 999, which will be computed by the
receiver, that have enough information to determine whether this process message
queue is keep growing over time.
*/
ANY_API int32_t ascheduler_outgoing(
    ascheduler_t* self, apid_t to, const avalue_t* msg);

/** Move all incoming messages to process queues.
\note Failed to grow processes will be crashed if necessary.
*/
ANY_API void ascheduler_empty_incoming(ascheduler_t* self);

#if 0
/** Upgrade code.
\note `chunks` and `natives` must be available until the next change.
*/
ANY_API void ascheduler_code_change(
    ascheduler_t* self, achunk_header_t** chunks, const anative_module_t* natives);
#endif

/** Spawn a borrowed process.
\return `AERR_NONE` if successful.
*/
ANY_API int32_t ascheduler_spawn_borrowed(
    ascheduler_t* self,
    const char* module, const char* name,
    const ambox_t* mbox,
    avalue_t* stack,
    int32_t stack_cap,
    aframe_t* frames,
    int32_t max_frames);

/** Spawn a process.
\return `AERR_NONE` if successful.
*/
ANY_API int32_t ascheduler_spawn(
    ascheduler_t* self, const char* module, const char* name);

#ifdef __cplusplus
} // extern "C"
#endif