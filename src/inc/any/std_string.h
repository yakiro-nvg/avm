/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>
#include <any/actor.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Utility function to calculate the hash and length of string `s`.
ANY_API ahash_and_length_t ahash_and_length(const char* s);

/// Create a new string.
ANY_API aint_t agc_string_new(aactor_t* a, const char* s, avalue_t* v);

/// Push new string onto the stack.
static AINLINE void any_push_string(aactor_t* a, const char* s)
{
    avalue_t v;
    aint_t ec = agc_string_new(a, s, &v);
    if (ec != AERR_NONE) any_error(a, AERR_RUNTIME, "out of memory");
    aactor_push(a, &v);
}

/// Get NULL terminated string pointer.
static AINLINE const char* agc_string_to_cstr(aactor_t* a, avalue_t* v)
{
    agc_string_t* s = AGC_CAST(agc_string_t, &a->gc, v->v.heap_idx);
    return (const char*)(s + 1);
}

/// Get NULL terminated string pointer.
static AINLINE const char* any_to_string(aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    return agc_string_to_cstr(a, v);
}

/// Get NULL terminated string pointer.
static AINLINE const char* any_check_string(aactor_t* a, aint_t idx)
{
    avalue_t* v = a->stack.v + idx;
    if (v->tag.type != AVT_STRING) {
        any_error(a, AERR_RUNTIME, "not a string");
    }
    return agc_string_to_cstr(a, v);
}

#ifdef __cplusplus
} // extern "C"
#endif
