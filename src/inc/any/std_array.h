/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-array library.
ANY_API void
astd_lib_add_array(
    aloader_t* l);

/// Create a new array.
ANY_API aint_t
agc_array_new(
    aactor_t* a, aint_t cap, avalue_t* v);

/// Push new array onto the stack.
static AINLINE void
any_push_array(
    aactor_t* a, aint_t cap)
{
    avalue_t v;
    aint_t ec = agc_array_new(a, cap, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Check if that is array.
static AINLINE void
any_check_array(
    aactor_t* a, aint_t idx)
{
    if (any_type(a, idx).type != AVT_ARRAY) {
        any_error(a, AERR_RUNTIME, "not array");
    }
}

/// Returns the number of elements.
static AINLINE aint_t
any_array_size(
    aactor_t* a, aint_t idx)
{
    agc_array_t* o;
    avalue_t* v = aactor_at(a, idx);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    return o->sz;
}

/// Returns the capacity.
static AINLINE aint_t
any_array_capacity(
    aactor_t* a, aint_t idx)
{
    agc_array_t* o;
    avalue_t* v = aactor_at(a, idx);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    return o->cap;
}

/// Increase the capacity of the array to a value that's >= `cap`.
ANY_API void
any_array_reserve(
    aactor_t* a, aint_t idx, aint_t cap);

/// Reduce the capacity to fit the size.
ANY_API void
any_array_shrink_to_fit(
    aactor_t* a, aint_t idx);

/// Resize the array to contain `sz` elements.
ANY_API void
any_array_resize(
    aactor_t* a, aint_t idx, aint_t sz);

#ifdef __cplusplus
} // extern "C"
#endif
