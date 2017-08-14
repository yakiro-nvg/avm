/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize as a new actor.
ANY_API aerror_t aactor_init(
    aactor_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud);

/// Release all internal allocated memory.
ANY_API void aactor_cleanup(aactor_t* self);

/// Ensures that there are `more` bytes in the stack.
ANY_API void aactor_reserve(aactor_t* self, int32_t more);

/// Throw an error, with description string pushed onto the stack.
ANY_API void any_error(aactor_t* a, aerror_t ec, const char* fmt, ...);

/** Allocate a new collectable object.
\brief Collect or grow if necessary.
\note Please refer \ref agc_alloc.
*/
ANY_API int32_t aactor_alloc(aactor_t* self, aabt_t abt, int32_t sz);

/// Push a value onto the stack, should be internal used.
static AINLINE void aactor_push(aactor_t* self, avalue_t* v)
{
    if (self->sp == self->stack_cap) aactor_reserve(self, 1);
    *(self->stack + self->sp) = *v;
    ++self->sp;
}

/// Get normalized index.
static AINLINE int32_t aactor_absidx(aactor_t* self, int32_t idx)
{
    if (idx < -self->frame->nargs) return 0;
    if (idx >= self->sp - self->frame->bp) {
        any_error(self, AERR_RUNTIME, "bad index %d", idx);
    }
    return self->frame->bp + idx;
}

/// Lookup for a module level symbol and push it onto the stack.
ANY_API void any_find(aactor_t* a, const char* module, const char* name);

/** Call a function.
\brief Please refer \ref aactor_t for the protocol.
\note Result will be placed on top of the stack.
*/
ANY_API void any_call(aactor_t* a, int32_t nargs);

/// \ref any_call in protected mode.
ANY_API void any_pcall(aactor_t* a, int32_t nargs);

/** Send a message.
\brief Please refer \ref AOC_SND.
*/
ANY_API void any_mbox_send(aactor_t* a);

/** Pickup next message.
\brief Please refer \ref AOC_RCV.
*/
ANY_API aerror_t any_mbox_recv(aactor_t* a, int32_t timeout);

/** Remove current message.
\brief Please refer \ref AOC_RMV.
*/
ANY_API void any_mbox_remove(aactor_t* a);

/// Suspends the execution flow.
ANY_API void any_yield(aactor_t* a);

/// Sleep for `nsecs`.
ANY_API void any_sleep(aactor_t* a, int32_t nsecs);

/// Execute in protected mode.
ANY_API aerror_t any_try(aactor_t* a, void(*f)(aactor_t*, void*), void* ud);

/// Throw an error.
ANY_API void any_throw(aactor_t* a, int32_t ec);

/// Get the value tag of the value at `idx`.
static AINLINE avalue_tag_t any_type(aactor_t* a, int32_t idx)
{
    return a->stack[aactor_absidx(a, idx)].tag;
}

// Stack manipulations.
static AINLINE void any_pop(aactor_t* a, int32_t n)
{
    a->sp -= n;
    if (a->sp < a->frame->bp) {
        any_error(a, AERR_RUNTIME, "no more elements");
    }
}

static AINLINE void any_push_nil(aactor_t* a)
{
    avalue_t v;
    v.tag.b = ABT_NIL;
    aactor_push(a, &v);
}

static AINLINE void any_push_bool(aactor_t* a, int32_t b)
{
    avalue_t v;
    v.tag.b = ABT_BOOL;
    v.v.boolean = b;
    aactor_push(a, &v);
}

static AINLINE void any_push_integer(aactor_t* a, aint_t i)
{
    avalue_t v;
    v.tag.b = ABT_NUMBER;
    v.tag.variant = AVTN_INTEGER;
    v.v.integer = i;
    aactor_push(a, &v);
}

static AINLINE void any_push_real(aactor_t* a, areal_t r)
{
    avalue_t v;
    v.tag.b = ABT_NUMBER;
    v.tag.variant = AVTN_REAL;
    v.v.real = r;
    aactor_push(a, &v);
}

static AINLINE void any_push_pid(aactor_t* a, apid_t pid)
{
    avalue_t v;
    v.tag.b = ABT_PID;
    v.v.pid = pid;
    aactor_push(a, &v);
}

static AINLINE void any_push_idx(aactor_t* a, int32_t idx)
{
    aactor_push(a, a->stack + aactor_absidx(a, idx));
}

static AINLINE int32_t any_to_bool(aactor_t* a, int32_t idx)
{
    avalue_t* v = a->stack + aactor_absidx(a, idx);
    return v->v.boolean;
}

static AINLINE aint_t any_to_integer(aactor_t* a, int32_t idx)
{
    avalue_t* v = a->stack + aactor_absidx(a, idx);
    return v->v.integer;
}

static AINLINE areal_t any_to_real(aactor_t* a, int32_t idx)
{
    avalue_t* v = a->stack + aactor_absidx(a, idx);
    return v->v.real;
}

static AINLINE apid_t any_to_pid(aactor_t* a, int32_t idx)
{
    avalue_t* v = a->stack + aactor_absidx(a, idx);
    return v->v.pid;
}

static AINLINE void any_remove(aactor_t* a, int32_t idx)
{
    int32_t num_tails;
    if (idx < 0) any_error(a, AERR_RUNTIME, "bad index %d", idx);
    idx = aactor_absidx(a, idx);
    num_tails = a->sp - idx - 1;
    --a->sp;
    if (num_tails == 0) return;
    memmove(
        a->stack + idx,
        a->stack + idx + 1,
        sizeof(avalue_t)*num_tails);
}

static AINLINE void any_insert(aactor_t* a, int32_t idx)
{
    any_pop(a, 1);
    a->stack[aactor_absidx(a, idx)] = a->stack[a->sp];
}

/// Returns the stack size.
static AINLINE int32_t any_count(aactor_t* a)
{
    return a->sp - a->frame->bp;
}

/** Spawn a new actor.
\brief This function follow the same protocol as \ref any_call.
*/
ANY_API aerror_t any_spawn(
    aactor_t* a, int32_t cstack_sz, int32_t nargs, apid_t* pid);

#ifdef __cplusplus
} // extern "C"
#endif