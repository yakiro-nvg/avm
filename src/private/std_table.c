/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/std_table.h>

#include <any/gc.h>
#include <any/loader.h>
#include <any/std_string.h>
#include <any/std.h>

#define GROW_FACTOR 2
#define INIT_GROW 64

static agc_table_t*
set_capacity(
    aactor_t* a, avalue_t* t, aint_t cap)
{
    agc_table_t* o;
    aint_t bi;
    aint_t cap_bytes = cap * 2 * sizeof(avalue_t);
    aerror_t ec = aactor_heap_reserve(a, cap_bytes, 1);
    if (ec < 0) any_error(a, AERR_RUNTIME, "out of memory");
    bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap_bytes);
    o = AGC_CAST(agc_table_t, &a->gc, t->v.heap_idx);
    assert(cap >= o->sz);
    memcpy(
        AGC_CAST(void, &a->gc, bi),
        AGC_CAST(void, &a->gc, o->buff.v.heap_idx),
        (size_t)o->sz * 2 * sizeof(avalue_t));
    o->cap = cap;
    av_collectable(&o->buff, AVT_FIXED_BUFFER, bi);
    return o;
}

static const int32_t VALID_KEYS[] = {
    FALSE, // AVT_NIL
    TRUE,  // AVT_PID
    FALSE, // AVT_BOOLEAN
    TRUE,  // AVT_INTEGER
    TRUE,  // AVT_REAL
    FALSE, // AVT_NATIVE_FUNC
    FALSE, // AVT_BYTE_CODE_FUNC
    FALSE, // AVT_FIXED_BUFFER
    FALSE, // AVT_BUFFER
    TRUE,  // AVT_STRING
    FALSE, // AVT_TUPLE
    FALSE, // AVT_ARRAY
    FALSE  // AVT_TABLE
};

ASTATIC_ASSERT(__AVT_LAST__ == ASTATIC_ARRAY_COUNT(VALID_KEYS) - 1);

static AINLINE void
check_key(
    aactor_t* a, avalue_t* k)
{
    if (k->tag.type > __AVT_LAST__ ||
        k->tag.type < 0 ||
        VALID_KEYS[k->tag.type] == FALSE) {
        any_error(a, AERR_RUNTIME, "bad key type");
    }
}

static avalue_t*
search_for(
    aactor_t* a, agc_table_t* t, avalue_t* k)
{
    aint_t i;
    avalue_t* vals = AGC_CAST(avalue_t, &a->gc, t->buff.v.heap_idx);
    for (i = 0; i < t->sz; ++i) {
        avalue_t* p = vals + (i * 2);
        avalue_t* pk = p;
        avalue_t* pv = p + 1;
        if (pk->tag.type != k->tag.type) continue;
        switch (k->tag.type) {
        case AVT_PID:
            if (pk->v.pid == k->v.pid) {
                return pv;
            }
            break;
        case AVT_INTEGER:
            if (pk->v.integer == k->v.integer) {
                return pv;
            }
            break;
        case AVT_REAL:
            if (afuzzy_equals(pk->v.real, k->v.real)) {
                return pv;
            }
            break;
        case AVT_STRING:
            if (agc_string_compare(a, pk, k) == 0) {
                return pv;
            }
            break;
        }
    }
    return NULL;
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
    any_push_table(a, cap);
}

static void
lget(
    aactor_t* a)
{
    agc_table_t* o;
    avalue_t *t, *k, *v;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_key = any_check_index(a, -2);
    any_check_table(a, a_self);
    t = aactor_at(a, a_self);
    o = AGC_CAST(agc_table_t, &a->gc, t->v.heap_idx);
    k = aactor_at(a, a_key);
    check_key(a, k);
    v = search_for(a, o, k);
    if (v == NULL) {
        any_push_nil(a);
    } else {
        aactor_push(a, v);
    }
}

static void
lset(
    aactor_t* a)
{
    agc_table_t* o;
    avalue_t *t, *k, *v, *val;
    aint_t a_self = any_check_index(a, -1);
    aint_t a_key = any_check_index(a, -2);
    aint_t a_val = any_check_index(a, -3);
    any_check_table(a, a_self);
    t = aactor_at(a, a_self);
    o = AGC_CAST(agc_table_t, &a->gc, t->v.heap_idx);
    k = aactor_at(a, a_key);
    check_key(a, k);
    v = search_for(a, o, k);
    val = aactor_at(a, a_val);
    if (v != NULL) {
        *v = *val;
    } else {
        avalue_t* p;
        if (o->sz == o->cap) {
            o = set_capacity(
                a, t, o->cap == 0 ? INIT_GROW : o->cap * GROW_FACTOR);
        }
        assert(o->sz < o->cap);
        p = AGC_CAST(avalue_t, &a->gc, o->buff.v.heap_idx) + (2 * o->sz++);
        k = aactor_at(a, a_key);
        val = aactor_at(a, a_val);
        p[0] = *k;
        p[1] = *val;
    }
    any_push_nil(a);
}

static void
lsize(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_table(a, a_self);
    any_push_integer(a, any_table_size(a, a_self));
}

static void
lcapacity(
    aactor_t* a)
{
    aint_t a_self = any_check_index(a, -1);
    any_check_table(a, a_self);
    any_push_integer(a, any_table_capacity(a, a_self));
}

static alib_func_t funcs[] = {
    { "new/1",      &lnew },
    { "get/2",      &lget },
    { "set/3",      &lset },
    { "size/1",     &lsize },
    { "capacity/1", &lcapacity },
    { NULL, NULL }
};

static alib_t mod = { "std-table", funcs };

void
astd_lib_add_table(
    aloader_t* l)
{
    aloader_add_lib(l, &mod);
}

aint_t
agc_table_new(
    aactor_t* a, aint_t cap, avalue_t* v)
{
    aerror_t ec;
    aint_t cap_bytes = cap * 2 * sizeof(avalue_t);
    assert(cap >= 0);
    ec = aactor_heap_reserve(a, sizeof(agc_table_t) + cap_bytes, 2);
    if (ec < 0) {
        return ec;
    } else {
        aint_t oi = agc_alloc(&a->gc, AVT_TABLE, sizeof(agc_table_t));
        aint_t bi = agc_alloc(&a->gc, AVT_FIXED_BUFFER, cap_bytes);
        agc_table_t* o = AGC_CAST(agc_table_t, &a->gc, oi);
        o->cap = cap;
        o->sz = 0;
        av_collectable(&o->buff, AVT_FIXED_BUFFER, bi);
        av_collectable(v, AVT_TABLE, oi);
        return AERR_NONE;
    }
}