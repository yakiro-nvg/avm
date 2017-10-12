/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_string.h>

#include <any/gc.h>
#include <any/loader.h>

static AINLINE void
check_index(
    aactor_t* a, aint_t idx, aint_t len)
{
    if (idx >= len || idx < 0) {
        any_error(a, AERR_RUNTIME, "bad index %lld", (long long int)idx);
    }
}

static void
llength(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_string(a, a_self);
    any_push_integer(a, any_string_length(a, a_self));
}

static void
lhash(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_string(a, a_self);
    any_push_integer(a, any_string_hash(a, a_self));
}

static void
lget(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    const char* s = any_check_string(a, a_self);
    aint_t idx = any_check_integer(a, a_idx);
    aint_t len = any_string_length(a, a_self);
    check_index(a, idx, len);
    any_push_integer(a, s[idx]);
}

static void
lconcat(
    aactor_t* a)
{
    aint_t a_lhs = any_check_index(a, -1);
    aint_t a_rhs = any_check_index(a, -2);
    const char* lhs = any_check_string(a, a_lhs);
    const char* rhs = any_check_string(a, a_rhs);
    aint_t lhs_len = any_string_length(a, a_lhs);
    aint_t rhs_len = any_string_length(a, a_rhs);
    aerror_t ec = aactor_heap_reserve(
        a, sizeof(agc_string_t) + lhs_len + rhs_len + 1, 1);
    if (ec < 0) {
        any_error(a, AERR_RUNTIME, "out of memory");
    } else {
        avalue_t v;
        agc_string_t* o;
        aint_t sz = sizeof(agc_string_t) + lhs_len + rhs_len + 1;
        aint_t oi = agc_alloc(&a->gc, AVT_STRING, sz);
        o = AGC_CAST(agc_string_t, &a->gc, oi);
        lhs = any_to_string(a, a_lhs);
        rhs = any_to_string(a, a_rhs);
        memcpy(o + 1, lhs, (size_t)lhs_len);
        memcpy(((uint8_t*)(o + 1)) + lhs_len, rhs, (size_t)rhs_len + 1);
        o->hal = ahash_and_length((const char*)(o + 1));
        av_collectable(&v, AVT_STRING, oi);
        aactor_push(a, &v);
    }
}

static alib_func_t funcs[] = {
    { "length/1",   &llength },
    { "hash/1",     &lhash },
    { "get/2",      &lget },
    { "concat/2",   &lconcat },
    { NULL, NULL }
};

static alib_t mod = { "std-string", funcs };

void
astd_lib_add_string(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

ahash_and_length_t
ahash_and_length(
    const char* s)
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

aint_t
agc_string_new(
    aactor_t* a, const char* s, avalue_t* v)
{
    ahash_and_length_t hal = ahash_and_length(s);
    aint_t sz = sizeof(agc_string_t) + hal.length + 1;
    aerror_t ec = aactor_heap_reserve(a, sz, 1);
    if (ec < 0) {
        return ec;
    } else {
        aint_t oi = agc_alloc(&a->gc, AVT_STRING, sz);
        agc_string_t* o = AGC_CAST(agc_string_t, &a->gc, oi);
        o->hal = hal;
        memcpy(o + 1, s, (size_t)hal.length + 1);
        av_collectable(v, AVT_STRING, oi);
        return AERR_NONE;
    }
}