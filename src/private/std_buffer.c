/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_buffer.h>

#include <any/gc.h>
#include <any/loader.h>

#define GROW_FACTOR 2
#define INIT_GROW 64

static void
set_capacity(
    aactor_t* a, aint_t idx, aint_t cap)
{
    avalue_t* v;
    agc_buffer_t* o;
    aint_t bi;
    aerror_t ec = aactor_heap_reserve(a, cap, 1);
    if (ec < 0) any_error(a, AERR_RUNTIME, "out of memory");
    bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap);
    v = aactor_at(a, idx);
    o = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    assert(cap >= o->sz);
    memcpy(
        AGC_CAST(void, &a->gc, bi),
        AGC_CAST(void, &a->gc, o->buff.v.heap_idx),
        (size_t)o->sz);
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
    any_push_buffer(a, cap);
}

static void
lreserve(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_cap = any_check_index(a, -2);
    aint_t cap = any_check_integer(a, a_cap);
    any_check_buffer(a, a_self);
    any_buffer_reserve(a, a_self, cap);
    any_push_nil(a);
}

static void
lshrink_to_fit(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_buffer(a, a_self);
    any_buffer_shrink_to_fit(a, a_self);
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
    any_check_buffer(a, a_self);
    any_buffer_resize(a, a_self, sz);
    any_push_nil(a);
}

static void
lget(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    uint8_t* b = any_check_buffer(a, a_self);
    aint_t idx = any_check_integer(a, a_idx);
    aint_t sz = any_buffer_size(a, a_self);
    check_index(a, idx, sz);
    any_push_integer(a, b[idx]);
}

static void
lset(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    aint_t a_idx = any_check_index(a, -2);
    aint_t a_val = any_check_index(a, -3);
    uint8_t* b = any_check_buffer(a, a_self);
    aint_t idx = any_check_integer(a, a_idx);
    aint_t val = any_check_integer(a, a_val);
    aint_t sz = any_buffer_size(a, a_self);
    check_index(a, idx, sz);
    b[idx] = (uint8_t)val;
    any_push_nil(a);
}

static void
lsize(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_buffer(a, a_self);
    any_push_integer(a, any_buffer_size(a, a_self));
}

static void
lcapacity(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_buffer(a, a_self);
    any_push_integer(a, any_buffer_capacity(a, a_self));
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
    { NULL, NULL }
};

static alib_t mod = { "std-buffer", funcs };

void
astd_lib_add_buffer(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

aint_t
agc_buffer_new(
    aactor_t* a, aint_t cap, avalue_t* v)
{
    aerror_t ec;
    assert(cap >= 0);
    ec = aactor_heap_reserve(a, sizeof(agc_buffer_t) + cap, 2);
    if (ec < 0) {
        return ec;
    } else {
        aint_t oi = agc_alloc(&a->gc, AVT_BUFFER, sizeof(agc_buffer_t));
        aint_t bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap);
        agc_buffer_t* o = AGC_CAST(agc_buffer_t, &a->gc, oi);
        o->cap = cap;
        o->sz = 0;
        av_collectable(&o->buff, AVT_FIXED_BUFFER, bi);
        av_collectable(v, AVT_BUFFER, oi);
        return AERR_NONE;
    }
}

void
any_buffer_reserve(
    aactor_t* a, aint_t idx, aint_t cap)
{
    avalue_t* v = aactor_at(a, idx);
    agc_buffer_t* o = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    if (o->cap < cap) {
        set_capacity(a, idx, cap);
    }
}

void
any_buffer_shrink_to_fit(
    aactor_t* a, aint_t idx)
{
    avalue_t* v = aactor_at(a, idx);
    agc_buffer_t* o = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    set_capacity(a, idx, o->sz);
}

void
any_buffer_resize(
    aactor_t* a, aint_t idx, aint_t sz)
{
    avalue_t* v = aactor_at(a, idx);
    agc_buffer_t* o = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    assert(sz >= 0);
    if (o->cap < sz) {
        aint_t new_cap = o->cap;
        if (new_cap == 0) new_cap = INIT_GROW;
        while (new_cap < sz) new_cap *= GROW_FACTOR;
        set_capacity(a, idx, new_cap);
        v = aactor_at(a, idx);
        o = AGC_CAST(agc_buffer_t, &a->gc, v->v.heap_idx);
    }
    assert(o->cap >= sz);
    o->sz = sz;
}