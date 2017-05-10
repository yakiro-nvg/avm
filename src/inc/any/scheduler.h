/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize as a new scheduler.
\brief Scheduler will not allocate, grow or shrink its message queues.
\return AERR_NONE if successful.
*/
ANY_API int32_t any_sched_init(
    ascheduler_t* self, avm_t* vm,
    asmbox_t oqueues[2], asmbox_t iqueues[2],
    arealloc_t realloc, void* realloc_ud);

/** Try to put a message into outgoing queue.
\return -1 if no more space, otherwise a non negative `load` factor of the receiver 
will be returned to compute the reduction cost for this sending operation. Because
message sending to an overloaded process should cost more reduction than usual case. 
This `load` factor is vary from 0 to 1000, which be computed by the receiver owners, 
that have enough information to determine whether this process message queue is keep
growing over time, and there are not enough resources to deal with that.
*/
ANY_API int32_t any_sched_outgoing(ascheduler_t* self, apid_t to, avalue_t* msg);

/** Move all incoming messages to process queues.
\note Failed to grow processes will be crashed if necessary.
*/
ANY_API void any_sched_empty_incoming(ascheduler_t* self);

#ifdef __cplusplus
} // extern "C"
#endif