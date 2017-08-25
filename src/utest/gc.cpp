/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <vector>

#include <any/scheduler.h>
#include <any/actor.h>
#include <any/gc.h>

static bool search_for(agc_t* gc, aint_t i)
{
    aint_t off = 0;
    while (off < gc->heap_sz) {
        agc_header_t* h = (agc_header_t*)(gc->cur_heap + off);
        if (h->type == AVT_INTEGER && *(aint_t*)(h + 1) == i) {
            return true;
        }
        off += h->sz;
    }
    return false;
}

TEST_CASE("gc")
{
    agc_t gc;
    agc_init(&gc, 1024, &myalloc, NULL);

    std::vector<avalue_t> stack;

    for (aint_t i = 0; i < 1000; ++i) {
        avalue_t v;
        REQUIRE(AERR_NONE == agc_reserve(&gc, sizeof(aint_t)));
        aint_t oi = agc_alloc(&gc, AVT_INTEGER, sizeof(aint_t));
        REQUIRE(oi >= 0);
        av_collectable(&v, AVT_INTEGER, oi);
        *AGC_CAST(aint_t, &gc, v.v.heap_idx) = i;
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
