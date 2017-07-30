/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <assert.h>
#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize as a new process.
ANY_API void aprocess_init(
    aprocess_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud);

/// Start process with entry point on top of the stack.
ANY_API void aprocess_start(aprocess_t* self, int32_t cstack_sz, int32_t nargs);

/// Release all internal allocated memory.
ANY_API void aprocess_cleanup(aprocess_t* self);

/// Ensures that there are `more` bytes in the stack.
ANY_API void aprocess_reserve(aprocess_t* self, int32_t more);

/// Throw an error, with description string pushed onto the stack.
ANY_API void any_error(aprocess_t* p, const char* fmt, ...);

/// Push a value onto the stack, should be internal used.
AINLINE void aprocess_push(aprocess_t* self, avalue_t* v)
{
    if (self->sp == self->stack_cap) aprocess_reserve(self, 1);
    *(self->stack + self->sp) = *v;
    ++self->sp;
}

/// Get normalized index.
AINLINE int32_t aprocess_absidx(aprocess_t* self, int32_t idx)
{
    if (idx < -self->frame->nargs) return 0;
    if (idx >= self->sp - self->frame->bp) {
        any_error(self, "bad index %d", idx);
    }
    return self->frame->bp + idx;
}

/// Lookup for a module level symbol and push it onto the stack.
ANY_API void any_find(aprocess_t* p, const char* module, const char* name);

/** Call a function on top of the stack.
\brief Please refer \ref aprocess_t for the protocol.
\note Result will be placed on top of the stack.
*/
ANY_API void any_call(aprocess_t* p, int32_t nargs);

/// \ref any_call in protected mode.
ANY_API void any_pcall(aprocess_t* p, int32_t nargs);

/// Suspends the execution flow.
ANY_API void any_yield(aprocess_t* p);

/// Execute in protected mode.
ANY_API int32_t any_try(aprocess_t* p, void(*f)(aprocess_t*, void*), void* ud);

/// Throw an error.
ANY_API void any_throw(aprocess_t* p, int32_t ec);

/// Get the value tag of the value at `idx`.
AINLINE avalue_tag_t any_type(aprocess_t* p, int32_t idx)
{
    return p->stack[aprocess_absidx(p, idx)].tag;
}

// Stack manipulations.
AINLINE void any_push_nil(aprocess_t* p)
{
    avalue_t v;
    v.tag.b = ABT_NIL;
    aprocess_push(p, &v);
}

AINLINE void any_push_bool(aprocess_t* p, int32_t b)
{
    avalue_t v;
    v.tag.b = ABT_BOOL;
    v.v.boolean = b;
    aprocess_push(p, &v);
}

AINLINE void any_push_integer(aprocess_t* p, aint_t i)
{
    avalue_t v;
    v.tag.b = ABT_NUMBER;
    v.tag.variant = AVTN_INTEGER;
    v.v.integer = i;
    aprocess_push(p, &v);
}

AINLINE void any_push_real(aprocess_t* p, areal_t r)
{
    avalue_t v;
    v.tag.b = ABT_NUMBER;
    v.tag.variant = AVTN_REAL;
    v.v.real = r;
    aprocess_push(p, &v);
}

AINLINE void any_push_pid(aprocess_t* p, apid_t pid)
{
    avalue_t v;
    v.tag.b = ABT_PID;
    v.v.pid = pid;
    aprocess_push(p, &v);
}

AINLINE void any_push_idx(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    aprocess_push(p, v);
}

AINLINE int32_t any_to_bool(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    assert(v->tag.b == ABT_BOOL);
    return v->v.boolean;
}

AINLINE aint_t any_to_integer(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    assert(v->tag.b == ABT_NUMBER);
    assert(v->tag.variant == AVTN_INTEGER);
    return v->v.integer;
}

AINLINE areal_t any_to_real(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    assert(v->tag.b == ABT_NUMBER);
    assert(v->tag.variant == AVTN_REAL);
    return v->v.real;
}

AINLINE apid_t any_to_pid(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    assert(v->tag.b == ABT_PID);
    return v->v.pid;
}

AINLINE void any_pop(aprocess_t* p, int32_t n)
{
    p->sp -= n;
    if (p->sp < p->frame->bp) {
        any_error(p, "stack underflow");
    }
}

AINLINE void any_remove(aprocess_t* p, int32_t idx)
{
    int32_t num_tails;
    if (idx < 0) any_error(p, "bad index %d", idx);
    idx = aprocess_absidx(p, idx);
    num_tails = p->sp - idx - 1;
    --p->sp;
    if (num_tails == 0) return;
    memmove(
        p->stack + idx,
        p->stack + idx + 1,
        sizeof(avalue_t)*num_tails);
}

/// Returns the stack size.
AINLINE int32_t any_count(aprocess_t* p)
{
    return p->sp - p->frame->bp;
}

/** Spawn a new process.
\brief
This function follow the same protocol as \ref any_call.
*/
ANY_API int32_t any_spawn(
    aprocess_t* p, int32_t cstack_sz, int32_t nargs, apid_t* pid);

#ifdef __cplusplus
} // extern "C"
#endif