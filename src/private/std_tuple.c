/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_tuple.h>

#include <any/gc.h>
#include <any/loader.h>

static AINLINE void
check_index(
    aactor_t* a, aint_t idx, aint_t sz)
{
    if (idx >= sz || idx < 0) {
        any_error(a, AERR_RUNTIME, "bad index %lld", (long long int)idx);
    }
}

static void
lnew(
    aactor_t* a)
{
    aint_t a_sz = any_check_index(a, -1);
    aint_t sz = any_check_integer(a, a_sz);
    if (sz < 0) {
        any_error(a, AERR_RUNTIME, "bad size %lld", (long long int)sz);
    }
    any_push_tuple(a, sz);
}

static void
lget(
    aactor_t* a)
{
    aint_t sz;
    agc_tuple_t* o;
    avalue_t* v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    aint_t idx = any_check_integer(a, a_idx);
    any_check_tuple(a, a_self);
    sz = any_tuple_size(a, a_self);
    check_index(a, idx, sz);
    v = aactor_at(a, a_self);
    o = AGC_CAST(agc_tuple_t, &a->gc, v->v.heap_idx);
    aactor_push(a, ((avalue_t*)(o + 1)) + idx);
}

static void
lset(
    aactor_t* a)
{
    aint_t sz;
    agc_tuple_t* o;
    avalue_t* v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    aint_t a_val = any_check_index(a, -3);
    aint_t idx = any_check_integer(a, a_idx);
    any_check_tuple(a, a_self);
    sz = any_tuple_size(a, a_self);
    check_index(a, idx, sz);
    v = aactor_at(a, a_self);
    o = AGC_CAST(agc_tuple_t, &a->gc, v->v.heap_idx);
    ((avalue_t*)(o + 1))[idx] = *aactor_at(a, a_val);
    any_push_nil(a);
}

static void
lsize(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_tuple(a, a_self);
    any_push_integer(a, any_tuple_size(a, a_self));
}

static alib_func_t funcs[] = {
    { "new/1",  &lnew },
    { "get/2",  &lget },
    { "set/3",  &lset },
    { "size/1", &lsize },
    { NULL, NULL }
};

static alib_t mod = { "std-tuple", funcs };

void
astd_lib_add_tuple(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

aint_t
agc_tuple_new(
    aactor_t* a, aint_t sz, avalue_t* v)
{
    aerror_t ec;
    aint_t bytes = sizeof(agc_tuple_t) +  (sz * sizeof(avalue_t));
    assert(sz >= 0);
    ec = aactor_heap_reserve(a, bytes, 1);
    if (ec < 0) {
        return ec;
    } else {
        aint_t i;
        aint_t oi = agc_alloc(&a->gc, AVT_TUPLE, bytes);
        agc_tuple_t* o = AGC_CAST(agc_tuple_t, &a->gc, oi);
        avalue_t* vals = (avalue_t*)(o + 1);
        for (i = 0; i < sz; ++i) {
            av_nil(vals + i);
        }
        o->sz = sz;
        av_collectable(v, AVT_TUPLE, oi);
        return AERR_NONE;
    }
}