/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-tuple library.
ANY_API void
astd_lib_add_tuple(
    aloader_t* l);

/// Create a new tuple.
ANY_API aint_t
agc_tuple_new(
    aactor_t* a, aint_t sz, avalue_t* v);

/// Push new tuple onto the stack.
static AINLINE void
any_push_tuple(
    aactor_t* a, aint_t sz)
{
    avalue_t v;
    aint_t ec = agc_tuple_new(a, sz, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Check if that is tuple.
static AINLINE void
any_check_tuple(
    aactor_t* a, aint_t idx)
{
    if (any_type(a, idx).type != AVT_TUPLE) {
        any_error(a, AERR_RUNTIME, "not tuple");
    }
}

/// Returns the number of elements.
static AINLINE aint_t
any_tuple_size(
    aactor_t* a, aint_t idx)
{
    agc_tuple_t* o;
    avalue_t* v = aactor_at(a, idx);
    o = AGC_CAST(agc_tuple_t, &a->gc, v->v.heap_idx);
    return o->sz;
}

#ifdef __cplusplus
} // extern "C"
#endif
