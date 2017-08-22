/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>
#include <any/gc.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Create a new fixed size buffer.
ANY_API aint_t agc_fixed_buffer_new(agc_t* gc, aint_t sz, avalue_t* v);

/// Create a new buffer.
ANY_API aint_t agc_buffer_new(agc_t* gc, aint_t cap, avalue_t* v);

/// Push new buffer onto the stack.
static AINLINE void any_push_buffer(aactor_t* a, aint_t cap)
{
    avalue_t v;
    aint_t ec = agc_buffer_new(&a->gc, cap, &v);
    if (ec != AERR_NONE) any_error(a, (aerror_t)ec, "not enough memory");
    aactor_push(a, &v);
}

/// Get buffer pointer, available until next gc.
static AINLINE uint8_t* any_to_buffer(aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, aactor_absidx(a, idx));
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return AGC_CAST(uint8_t, &a->gc, b->buff.v.heap_idx);
}

/// Returns size of buffer in bytes.
static AINLINE aint_t any_buffer_size(aactor_t* a, aint_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = aactor_at(a, aactor_absidx(a, idx));
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return b->sz;
}

#if 0
/// Resize the buffer to contain count elements.
static AINLINE void any_buffer_resize(aactor_t* a)
{

}
#endif

#ifdef __cplusplus
} // extern "C"
#endif
