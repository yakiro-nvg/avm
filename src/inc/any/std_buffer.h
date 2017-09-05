/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-buffer library.
ANY_API void
astd_lib_add_buffer(
    aloader_t* l);

/// Create a new buffer.
ANY_API aint_t
agc_buffer_new(
    aactor_t* a, aint_t cap, avalue_t* v);

/// Push new buffer onto the stack.
static AINLINE void
any_push_buffer(
    aactor_t* a, aint_t cap)
{
    avalue_t v;
    aint_t ec = agc_buffer_new(a, cap, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Get buffer pointer.
static AINLINE uint8_t*
any_to_buffer(
    aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, idx);
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return AGC_CAST(uint8_t, &a->gc, b->buff.v.heap_idx);
}

/// Check if that is buffer.
static AINLINE uint8_t*
any_check_buffer(
    aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, idx);
    if (any_type(a, idx).type != AVT_BUFFER) {
        any_error(a, AERR_RUNTIME, "not buffer");
    }
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return AGC_CAST(uint8_t, &a->gc, b->buff.v.heap_idx);
}

/// Returns size of buffer in bytes.
static AINLINE aint_t
any_buffer_size(
    aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, idx);
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return b->sz;
}

/// Returns capacity of buffer in bytes.
static AINLINE aint_t
any_buffer_capacity(
    aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, idx);
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return b->cap;
}

/// Increase the capacity of the buffer to a value that's >= `cap`.
ANY_API void
any_buffer_reserve(
    aactor_t* a, aint_t idx, aint_t cap);

/// Reduce the capacity to fit the size.
ANY_API void
any_buffer_shrink_to_fit(
    aactor_t* a, aint_t idx);

/// Resize the buffer to contain `sz` elements.
ANY_API void
any_buffer_resize(
    aactor_t* a, aint_t idx, aint_t sz);

#ifdef __cplusplus
} // extern "C"
#endif
