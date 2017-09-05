/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Add std-buffer library.
ANY_API void
astd_lib_add_string(
    aloader_t* l);

/// Utility function to calculate the hash and length of string `s`.
ANY_API ahash_and_length_t
ahash_and_length(
    const char* s);

/// Get NULL terminated string pointer.
static AINLINE const char*
agc_string_to_cstr(
    aactor_t* a, avalue_t* v)
{
    agc_string_t* s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return (const char*)(s + 1);
}

/// Create a new string.
ANY_API aint_t
agc_string_new(
    aactor_t* a, const char* s, avalue_t* v);

/// Compare two strings.
static AINLINE aint_t
agc_string_compare(
    aactor_t* a, avalue_t* lhs, avalue_t* rhs)
{
    agc_string_t* ls = AGC_CAST(agc_string_t, &a->gc, lhs->v.heap_idx);
    agc_string_t* rs = AGC_CAST(agc_string_t, &a->gc, rhs->v.heap_idx);
    if (ls->hal.hash != rs->hal.hash) {
        return ls->hal.hash < rs->hal.hash ? -1 : 1;
    } else {
        return strcmp((const char*)(ls + 1), (const char*)(rs + 1));
    }
}

/// Push new string onto the stack.
static AINLINE void
any_push_string(
    aactor_t* a, const char* s)
{
    avalue_t v;
    aint_t ec = agc_string_new(a, s, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Get NULL terminated string pointer.
static AINLINE const char*
any_to_string(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    agc_string_t* s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return (const char*)(s + 1);
}

/// Get NULL terminated string pointer.
static AINLINE const char*
any_check_string(
    aactor_t* a, aint_t idx)
{
    agc_string_t* s;
    avalue_t* v = a->stack.v + idx;
    if (v->tag.type != AVT_STRING) {
        any_error(a, AERR_RUNTIME, "not string");
    }
    s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return (const char*)(s + 1);
}

/// Returns number of characters.
static AINLINE aint_t
any_string_length(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    agc_string_t* s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return s->hal.length;
}

/// Returns the hashed value.
static AINLINE aint_t
any_string_hash(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    agc_string_t* s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return (aint_t)s->hal.hash;
}

#ifdef __cplusplus
} // extern "C"
#endif
