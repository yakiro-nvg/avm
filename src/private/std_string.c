/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_string.h>

#include <any/gc.h>

ahash_and_length_t ahash_and_length(const char* s)
{
    // The hash function is borrowed from Lua.
    // Since we need to walk the entire string anyway for finding the
    // length, this is a decent hash function.
    ahash_and_length_t result;
    uint32_t h = 0;
    const char* i = s;
    for (; *i; ++i) h = h ^ ((h << 5) + (h >> 2) + (unsigned char)*i);
    result.hash = h;
    result.length = i - s;
    return result;
}

aint_t agc_string_new(aactor_t* a, const char* s, avalue_t* v)
{
    ahash_and_length_t hal = ahash_and_length(s);
    aint_t sz = sizeof(agc_string_t) + hal.length + 1;
    aerror_t ec = aactor_heap_reserve(a, sz);
    if (ec < 0) {
        return ec;
    } else {
        aint_t oi = agc_alloc(&a->gc, AVT_STRING, sz);
        agc_string_t* o = AGC_CAST(agc_string_t, &a->gc, oi);
        assert(oi >= 0);
        o->hal = hal;
        memcpy(o + 1, s, (size_t)hal.length + 1);
        av_collectable(v, AVT_STRING, oi);
        return AERR_NONE;
    }
}