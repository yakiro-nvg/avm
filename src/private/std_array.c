/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_array.h>

#include <any/gc.h>
#include <any/loader.h>

#define GROW_FACTOR 2
#define INIT_GROW 64

static void
set_capacity(
    aactor_t* a, aint_t idx, aint_t cap)
{
    avalue_t* v;
    agc_array_t* o;
    aint_t bi;
    aint_t cap_bytes = cap * sizeof(avalue_t);
    aerror_t ec = aactor_heap_reserve(a, cap_bytes, 1);
    if (ec < 0) any_error(a, AERR_RUNTIME, "out of memory");
    bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap_bytes);
    v = aactor_at(a, idx);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    assert(cap >= o->sz);
    memcpy(
        AGC_CAST(void, &a->gc, bi),
        AGC_CAST(void, &a->gc, o->buff.v.heap_idx),
        (size_t)o->sz * sizeof(avalue_t));
    o->cap = cap;
    av_collectable(&o->buff, AVT_FIXED_BUFFER, bi);
}

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
    aint_t a_cap = any_check_index(a, -1);
    aint_t cap = any_check_integer(a, a_cap);
    if (cap < 0) {
        any_error(a, AERR_RUNTIME, "bad capacity %lld",
            (long long int)cap);
    }
    any_push_array(a, cap);
}

static void
lreserve(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_cap = any_check_index(a, -2);
    aint_t cap = any_check_integer(a, a_cap);
    any_check_array(a, a_self);
    any_array_reserve(a, a_self, cap);
    any_push_nil(a);
}

static void
lshrink_to_fit(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_array(a, a_self);
    any_array_shrink_to_fit(a, a_self);
    any_push_nil(a);
}

static void
lresize(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_sz = any_check_index(a, -2);
    aint_t sz = any_check_integer(a, a_sz);
    if (sz < 0) {
        any_error(a, AERR_RUNTIME, "bad size %lld", (long long int)sz);
    }
    any_check_array(a, a_self);
    any_array_resize(a, a_self, sz);
    any_push_nil(a);
}

static void
lget(
    aactor_t* a)
{
    aint_t sz;
    agc_array_t* o;
    avalue_t* v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    aint_t idx = any_check_integer(a, a_idx);
    any_check_array(a, a_self);
    sz = any_array_size(a, a_self);
    check_index(a, idx, sz);
    v = aactor_at(a, a_self);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    aactor_push(a, AGC_CAST(avalue_t, &a->gc, o->buff.v.heap_idx) + idx);
}

static void
lset(
    aactor_t* a)
{
    aint_t sz;
    agc_array_t* o;
    avalue_t* v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    aint_t a_val = any_check_index(a, -3);
    aint_t idx = any_check_integer(a, a_idx);
    any_check_array(a, a_self);
    sz = any_array_size(a, a_self);
    check_index(a, idx, sz);
    v = aactor_at(a, a_self);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    AGC_CAST(avalue_t, &a->gc, o->buff.v.heap_idx)[idx] = *aactor_at(a, a_val);
    aactor_push(a, aactor_at(a, a_val));
}

static void
lsize(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_array(a, a_self);
    any_push_integer(a, any_array_size(a, a_self));
}

static void
lcapacity(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_array(a, a_self);
    any_push_integer(a, any_array_capacity(a, a_self));
}

static void
lpush(
    aactor_t* a)
{
    agc_array_t* o;
    avalue_t* v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_val = any_check_index(a, -2);
    aint_t sz = any_array_size(a, a_self);
    aint_t cap = any_array_capacity(a, a_self);
    if (sz + 1 > cap) {
        any_array_resize(a, a_self, sz + 1);
    }
    v = aactor_at(a, a_self);
    o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    AGC_CAST(avalue_t, &a->gc, o->buff.v.heap_idx)[sz] =
        *aactor_at(a, a_val);
    any_push_integer(a, sz + 1);
}

static alib_func_t funcs[] = {
    { "new/1",           &lnew },
    { "reserve/2",       &lreserve },
    { "shrink_to_fit/1", &lshrink_to_fit },
    { "resize/2",        &lresize },
    { "get/2",           &lget },
    { "set/3",           &lset },
    { "size/1",          &lsize },
    { "capacity/1",      &lcapacity },
    { "push/2",          &lpush },
    { NULL, NULL }
};

static alib_t mod = { "std-array", funcs };

void
astd_lib_add_array(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

aint_t
agc_array_new(
    aactor_t* a, aint_t cap, avalue_t* v)
{
    aerror_t ec;
    aint_t cap_bytes = cap * sizeof(avalue_t);
    assert(cap >= 0);
    ec = aactor_heap_reserve(a, sizeof(agc_array_t) + cap_bytes, 2);
    if (ec < 0) {
        return ec;
    } else {
        aint_t oi = agc_alloc(&a->gc, AVT_ARRAY, sizeof(agc_array_t));
        aint_t bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap_bytes);
        agc_array_t* o = AGC_CAST(agc_array_t, &a->gc, oi);
        o->cap = cap;
        o->sz = 0;
        av_collectable(&o->buff, AVT_FIXED_BUFFER, bi);
        av_collectable(v, AVT_ARRAY, oi);
        return AERR_NONE;
    }
}

void
any_array_reserve(
    aactor_t* a, aint_t idx, aint_t cap)
{
    avalue_t* v = aactor_at(a, idx);
    agc_array_t* o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    if (o->cap < cap) {
        set_capacity(a, idx, cap);
    }
}

void
any_array_shrink_to_fit(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = aactor_at(a, idx);
    agc_array_t* o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    set_capacity(a, idx, o->sz);
}

void
any_array_resize(
    aactor_t* a, aint_t idx, aint_t sz)
{
    avalue_t* v = aactor_at(a, idx);
    agc_array_t* o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    assert(sz >= 0);
    if (o->sz == sz) {
        return;
    }
    if (o->cap < sz) {
        aint_t new_cap = o->cap;
        if (new_cap == 0) new_cap = INIT_GROW;
        while (new_cap < sz) new_cap *= GROW_FACTOR;
        set_capacity(a, idx, new_cap);
        v = aactor_at(a, idx);
        o = AGC_CAST(agc_array_t, &a->gc, v->v.heap_idx);
    }
    assert(o->cap >= sz);
    if (sz > o->sz) {
        aint_t i;
        avalue_t* data = AGC_CAST(avalue_t, &a->gc, o->buff.v.heap_idx);
        for (i = o->sz; i < sz; ++i) {
            av_nil(data + i);
        }
    }
    o->sz = sz;
}