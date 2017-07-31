/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <assert.h>
#include <any/errno.h>
#include <any/rt_types.h>
#include <any/process.h>
#include <any/gc.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Utility function to calculate the hash and length of string `s`.
ANY_API ahash_and_length_t ahash_and_length(const char* s);

/// Create a new string.
AINLINE int32_t agc_string_new(agc_t* gc, const char* s, avalue_t* v)
{
    ahash_and_length_t hal = ahash_and_length(s);
    int32_t oi = agc_alloc(
        gc, ABT_STRING, sizeof(agc_string_t) + hal.length + 1);
    if (oi < 0) return oi;
    else {
        agc_string_t* o = AGC_CAST(agc_string_t, gc, oi);
        o->hal = hal;
        memcpy(o + 1, s, hal.length + 1);
        v->tag.b = ABT_STRING;
        v->v.heap_idx = oi;
        return AERR_NONE;
    }
}

/// Push new string onto the stack.
AINLINE void any_push_string(aprocess_t* p, const char* s)
{
    avalue_t v;
    int32_t ec = agc_string_new(&p->gc, s, &v);
    if (ec != AERR_NONE) any_throw(p, ec);
    aprocess_push(p, &v);
}

/// Get NULL terminated string pointer, available until next gc.
AINLINE const char* any_to_string(aprocess_t* p, int32_t idx)
{
    avalue_t* v = p->stack + aprocess_absidx(p, idx);
    agc_string_t* s = AGC_CAST(agc_string_t, &p->gc, v->v.heap_idx);
    assert(v->tag.b == ABT_STRING);
    return (const char*)(s + 1);
}

#ifdef __cplusplus
} // extern "C"
#endif