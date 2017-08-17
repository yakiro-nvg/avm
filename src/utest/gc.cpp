/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/gc.h>
#include <any/gc_string.h>
#include <any/gc_buffer.h>
#include <any/scheduler.h>
#include <any/actor.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

static bool search_for(agc_t* gc, aint_t i)
{
    aint_t off = 0;
    while (off < gc->heap_sz) {
        agc_header_t* h = (agc_header_t*)(gc->cur_heap + off);
        if (h->type == AVT_FIXED_BUFFER && *(aint_t*)(h + 1) == i) {
            return true;
        }
        off += h->sz;
    }
    return false;
}

static void string_test(aactor_t* a)
{
    char buff[64];
    for (aint_t i = 0; i < 1000; ++i) {
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            av_nil(aactor_at(a, i));
        }
    }
    for (aint_t i = 1000; i < 5000; ++i) {
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 1000; i < 5000; ++i) {
        if (i % 2 == 0) {
            av_nil(aactor_at(a, i));
        }
    }
    for (aint_t i = 5000; i < 10000; ++i) {
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 5000; i < 10000; ++i) {
        if (i % 2 == 0) {
            av_nil(aactor_at(a, i));
        }
    }
    for (aint_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) continue;
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        CHECK_THAT(any_to_string(a, i), Catch::Equals(buff));
    }
    any_push_string(a, "ok");
}

TEST_CASE("gc")
{
    agc_t gc;
    agc_init(&gc, 1024, &myalloc, NULL);

    std::vector<avalue_t> stack;

    for (aint_t i = 0; i < 1000; ++i) {
        avalue_t v;
        while (AERR_NONE != agc_buffer_new(&gc, 100, &v)) {
            REQUIRE(AERR_NONE == agc_reserve(&gc, 50));
        }
        agc_buffer_t* b = AGC_CAST(agc_buffer_t, &gc, v.v.heap_idx);
        *AGC_CAST(aint_t, &gc, b->buff.v.heap_idx) = i;
        stack.push_back(v);
    }

    for (aint_t i = 0; i < 1000; ++i) {
        REQUIRE(search_for(&gc, i));
    }

    {
        avalue_t* roots[] = { stack.data(), NULL };
        aint_t num_roots[] = { (aint_t)stack.size() };
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
    }

    for (aint_t i = 0; i < 1000; ++i) {
        REQUIRE(search_for(&gc, i));
    }

    for (aint_t i = 0; i < (aint_t)stack.size(); ++i) {
        if (i % 2 != 0) av_nil(stack.data() + i);
    }

    {
        avalue_t* roots[] = { stack.data(), NULL };
        aint_t num_roots[] = { (aint_t)stack.size() };
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
    }

    for (aint_t i = 0; i < 1000; ++i) {
        REQUIRE((i % 2 == 0) == search_for(&gc, i));
    }

    for (aint_t i = 0; i < (aint_t)stack.size(); ++i) {
        if (i % 4 != 0) av_nil(stack.data() + i);
    }

    {
        avalue_t* roots[] = { stack.data(), NULL };
        aint_t num_roots[] = { (aint_t)stack.size() };
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
    }

    for (aint_t i = 0; i < 1000; ++i) {
        REQUIRE((i % 4 == 0) == search_for(&gc, i));
    }

    for (aint_t i = 0; i < (aint_t)stack.size(); ++i) {
        av_nil(stack.data() + i);
    }

    {
        avalue_t* roots[] = { stack.data(), NULL };
        aint_t num_roots[] = { (aint_t)stack.size() };
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
        agc_collect(&gc, roots, num_roots);
    }

    for (aint_t i = 0; i < 1000; ++i) {
        REQUIRE(!search_for(&gc, i));
    }

    REQUIRE(agc_heap_size(&gc) == 0);

    agc_cleanup(&gc);
}

TEST_CASE("gc_string")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &string_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    CHECK_THAT(any_to_string(a, 0), Catch::Equals("ok"));
    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, 1).type == AVT_NIL);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("ok"));

    ascheduler_cleanup(&s);
}
