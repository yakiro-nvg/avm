/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-table library.
ANY_API void
astd_lib_add_table(
    aloader_t* l);

/// Create a new table.
ANY_API aint_t
agc_table_new(
    aactor_t* a, aint_t cap, avalue_t* v);

/// Push new table onto the stack.
static AINLINE void
any_push_table(
    aactor_t* a, aint_t cap)
{
    avalue_t v;
    aint_t ec = agc_table_new(a, cap, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Check if that is table.
static AINLINE void
any_check_table(
    aactor_t* a, aint_t idx)
{
    if (any_type(a, idx).type != AVT_TABLE) {
        any_error(a, AERR_RUNTIME, "not table");
    }
}

/// Returns the number of elements.
static AINLINE aint_t
any_table_size(
    aactor_t* a, aint_t idx)
{
    agc_table_t* o;
    avalue_t* v = aactor_at(a, idx);
    o = AGC_CAST(agc_table_t, &a->gc, v->v.heap_idx);
    return o->sz;
}

/// Returns the capacity.
static AINLINE aint_t
any_table_capacity(
    aactor_t* a, aint_t idx)
{
    agc_table_t* o;
    avalue_t* v = aactor_at(a, idx);
    o = AGC_CAST(agc_table_t, &a->gc, v->v.heap_idx);
    return o->cap;
}

#ifdef __cplusplus
} // extern "C"
#endif
