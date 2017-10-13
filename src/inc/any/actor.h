/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/stack.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANY_ASSERT_IDX(a, idx) \
    assert(idx == 0 || \
        ((idx >= a->frame->bp - a->frame->nargs) && (idx < a->stack.sp)))

/// Initialize as a new actor.
ANY_API aerror_t
aactor_init(
    aactor_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud);

/// Release all internal allocated memory.
ANY_API void
aactor_cleanup(
    aactor_t* self);

/// Throw an error, with description string pushed onto the stack.
ANY_API void
any_error(
    aactor_t* a, aerror_t ec, const char* fmt, ...);

/// Forces a garbage collection.
ANY_API void
aactor_gc(
    aactor_t* a);

/// Ensures that there are `more` bytes for `n` new objects in the heap.
ANY_API aerror_t
aactor_heap_reserve(
    aactor_t* self, aint_t more, aint_t n);

/// Push a value onto the stack, should be internal used.
static AINLINE void
aactor_push(
    aactor_t* self, avalue_t* v)
{
    if (astack_reserve(&self->stack, 1) != AERR_NONE) {
        any_error(self, AERR_RUNTIME, "out of memory");
    }
    self->stack.v[self->stack.sp] = *v;
    ++self->stack.sp;
}

/// Returns value at `idx` on the stack.
static AINLINE avalue_t*
aactor_at(
    aactor_t* self, aint_t idx)
{
    ANY_ASSERT_IDX(self, idx);
    return self->stack.v + idx;
}

/// Get the absolute index.
static AINLINE aint_t
any_check_index(
    aactor_t* self, aint_t idx)
{
    if (idx < -self->frame->nargs || idx >= self->stack.sp - self->frame->bp) {
        any_error(self, AERR_RUNTIME, "bad index %lld", (long long int)idx);
    }
    idx = self->frame->bp + idx;
    ANY_ASSERT_IDX(self, idx);
    return idx;
}

/// Lookup for a module level symbol and push it onto the stack.
ANY_API void
any_import(
    aactor_t* a, const char* module, const char* name);

/** Call a function.
\brief Please refer \ref aactor_t for the protocol.
\note Result will be placed on top of the stack.
*/
ANY_API void
any_call(
    aactor_t* a, aint_t nargs);

/// \ref any_call in protected mode.
ANY_API void
any_protected_call(
    aactor_t* a, aint_t nargs);

/** Send a message.
\brief Please refer \ref AOC_SND.
*/
ANY_API void
any_mbox_send(
    aactor_t* a);

/** Pickup next message.
\brief Please refer \ref AOC_RCV.
*/
ANY_API aerror_t
any_mbox_recv(
    aactor_t* a, aint_t timeout);

/** Remove current message.
\brief Please refer \ref AOC_RMV.
*/
ANY_API void
any_mbox_remove(
    aactor_t* a);

/** Rewind the peek pointer
\brief Please refer \ref AOC_RWD.
*/
ANY_API void
any_mbox_rewind(
    aactor_t* a);

/// Suspends the execution flow.
ANY_API void
any_yield(
    aactor_t* a);

/// Sleep for `nsecs`.
ANY_API void
any_sleep(
    aactor_t* a, aint_t nsecs);

/// Execute in protected mode.
ANY_API aerror_t
any_try(
    aactor_t* a, void(*f)(aactor_t*, void*), void* ud);

/// Throw an error.
ANY_API void
any_throw(
    aactor_t* a, aerror_t ec);

/// Get the value tag of the value at `idx`.
static AINLINE avalue_tag_t
any_type(
    aactor_t* a, aint_t idx)
{
    ANY_ASSERT_IDX(a, idx);
    return a->stack.v[idx].tag;
}

// Stack manipulations.
static AINLINE void
any_pop(
    aactor_t* a, aint_t n)
{
    a->stack.sp -= n;
    if (a->stack.sp <
        (a->frame->bp + (a->frame->pt
            ? a->frame->pt->header->num_local_vars : 0))) {
        any_error(a, AERR_RUNTIME, "pop underflow");
    }
}

static AINLINE void
any_push_nil(
    aactor_t* a)
{
    avalue_t v;
    av_nil(&v);
    aactor_push(a, &v);
}

static AINLINE void
any_push_bool(
    aactor_t* a, int32_t b)
{
    avalue_t v;
    av_boolean(&v, b);
    aactor_push(a, &v);
}

static AINLINE void
any_push_integer(
    aactor_t* a, aint_t i)
{
    avalue_t v;
    av_integer(&v, i);
    aactor_push(a, &v);
}

static AINLINE void
any_push_real(
    aactor_t* a, areal_t r)
{
    avalue_t v;
    av_real(&v, r);
    aactor_push(a, &v);
}

static AINLINE void
any_push_pid(
    aactor_t* a, apid_t pid)
{
    avalue_t v;
    av_pid(&v, pid);
    aactor_push(a, &v);
}

static AINLINE void
any_push_native_func(
    aactor_t* a, anative_func_t f)
{
    avalue_t v;
    av_native_func(&v, f);
    aactor_push(a, &v);
}

static AINLINE void
any_push_index(
    aactor_t* a, aint_t idx)
{
    ANY_ASSERT_IDX(a, idx);
    aactor_push(a, a->stack.v + idx);
}

static AINLINE int32_t
any_to_bool(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    switch (v->tag.type) {
    case AVT_NIL:     return FALSE;
    case AVT_BOOLEAN: return v->v.boolean;
    default:          return TRUE;
    }
}

static AINLINE int32_t
any_check_bool(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    ANY_ASSERT_IDX(a, idx);
    if (v->tag.type != AVT_BOOLEAN) {
        any_error(a, AERR_RUNTIME, "not boolean");
    }
    return v->v.boolean;
}

static AINLINE aint_t
any_to_integer(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    return v->v.integer;
}

static AINLINE aint_t
any_check_integer(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    ANY_ASSERT_IDX(a, idx);
    if (v->tag.type != AVT_INTEGER) {
        any_error(a, AERR_RUNTIME, "not integer");
    }
    return v->v.integer;
}

static AINLINE areal_t
any_to_real(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    return v->tag.type == AVT_REAL ? v->v.real : (areal_t)v->v.integer;
}

static AINLINE areal_t
any_check_real(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    ANY_ASSERT_IDX(a, idx);
    if (v->tag.type == AVT_REAL) {
        return v->v.real;
    } else if (v->tag.type == AVT_INTEGER) {
        return (areal_t)v->v.integer;
    } else {
        any_error(a, AERR_RUNTIME, "not number");
        return 0;
    }
}

static AINLINE apid_t
any_to_pid(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    return v->v.pid;
}

static AINLINE apid_t
any_check_pid(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    ANY_ASSERT_IDX(a, idx);
    if (v->tag.type != AVT_PID) {
        any_error(a, AERR_RUNTIME, "not pid");
    }
    return v->v.pid;
}

static AINLINE anative_func_t
any_to_native_func(
    aactor_t* a, aint_t idx)
{
    ANY_ASSERT_IDX(a, idx);
    return a->stack.v[idx].v.func;
}

static AINLINE void
any_remove(
    aactor_t* a, aint_t idx)
{
    aint_t num_tails;
    ANY_ASSERT_IDX(a, idx);
    num_tails = a->stack.sp - idx - 1;
    --a->stack.sp;
    if (num_tails == 0) return;
    memmove(
        a->stack.v + idx,
        a->stack.v + idx + 1,
        sizeof(avalue_t)*(size_t)num_tails);
}

static AINLINE void
any_insert(
    aactor_t* a, aint_t idx)
{
    ANY_ASSERT_IDX(a, idx);
    any_pop(a, 1);
    a->stack.v[idx] = a->stack.v[a->stack.sp];
}

/// Returns number of passed arguments.
static AINLINE aint_t
any_nargs(
    aactor_t* a)
{
    return a->frame->nargs;
}

/// Returns the stack size.
static AINLINE aint_t
any_count(
    aactor_t* a)
{
    return a->stack.sp - a->frame->bp;
}

/// Returns the stack top.
static AINLINE aint_t
any_top(
    aactor_t* a)
{
    return any_check_index(a, any_count(a) - 1);
}

/** Spawn a new actor.
\brief This function follow the same protocol as \ref any_call.
*/
ANY_API void
any_spawn(
    aactor_t* a, aint_t cstack_sz, aint_t nargs, apid_t* pid);

#ifdef __cplusplus
} // extern "C"
#endif
