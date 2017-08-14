/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>
#include <any/gc.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Create a new fixed size buffer.
static AINLINE int32_t agc_fixed_buffer_new(agc_t* gc, int32_t sz, avalue_t* v)
{
    int32_t oi = agc_alloc(gc, ABT_FIXED_BUFFER, sz);
    if (oi < 0) return oi;
    v->tag.b = ABT_FIXED_BUFFER;
    v->v.heap_idx = oi;
    return AERR_NONE;
}

/// Create a new buffer.
static AINLINE int32_t agc_buffer_new(agc_t* gc, int32_t cap, avalue_t* v)
{
    int32_t oi = agc_alloc(gc, ABT_BUFFER, sizeof(agc_buffer_t));
    if (oi < 0) return oi;
    else {
        agc_buffer_t* o = AGC_CAST(agc_buffer_t, gc, oi);
        int32_t bi = agc_fixed_buffer_new(gc, cap, &o->buff);
        if (bi < 0) return bi;
        o->cap = cap;
        o->sz = 0;
        v->tag.b = ABT_BUFFER;
        v->v.heap_idx = oi;
        return AERR_NONE;
    }
}

/// Push new buffer onto the stack.
static AINLINE void any_push_buffer(aactor_t* a, int32_t cap)
{
    avalue_t v;
    int32_t ec = agc_buffer_new(&a->gc, cap, &v);
    if (ec != AERR_NONE) any_error(a, (aerror_t)ec, "not enough memory");
    aactor_push(a, &v);
}

/// Get buffer pointer, available until next gc.
static AINLINE uint8_t* any_to_buffer(aactor_t* a, int32_t idx)
{
    agc_buffer_t* b;
    avalue_t* v = a->stack + aactor_absidx(a, idx);
    assert(v->tag.b == ABT_BUFFER);
    b = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    return AGC_CAST(uint8_t, &a->gc, b->buff.v.heap_idx);
}

#ifdef __cplusplus
} // extern "C"
#endif
