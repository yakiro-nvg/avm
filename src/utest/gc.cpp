/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/gc.h>
#include <any/gc_string.h>
#include <any/gc_buffer.h>

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static bool search_for(agc_t* gc, int32_t i)
{
    int32_t off = 0;
    while (off < gc->heap_sz) {
        agc_header_t* h = (agc_header_t*)(gc->cur_heap + off);
        if (h->abt == ABT_FIXED_BUFFER && *(int32_t*)(h + 1) == i) return true;
        off += h->sz;
    }
    return false;
}

TEST_CASE("gc")
{
    agc_t gc;
    agc_init(&gc, 1024, &myalloc, NULL);

    std::vector<avalue_t> stack;

    for (int32_t i = 0; i < 1000; ++i) {
        avalue_t v;
        while (AERR_NONE != agc_buffer_new(&gc, 100, &v)) {
            REQUIRE(AERR_NONE == agc_reserve(&gc, 50));
        }
        agc_buffer_t* b = AGC_CAST(agc_buffer_t, &gc, v.v.heap_idx);
        *AGC_CAST(int32_t, &gc, b->buff.v.heap_idx) = i;
        stack.push_back(v);
    }

    for (int32_t i = 0; i < (int32_t)stack.size(); ++i) {
        agc_buffer_t* b = AGC_CAST(agc_buffer_t, &gc, stack[i].v.heap_idx);
        REQUIRE(*AGC_CAST(int32_t, &gc, b->buff.v.heap_idx) == i);
    }

    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());

    for (int32_t i = 0; i < (int32_t)stack.size(); ++i) {
        agc_buffer_t* b = AGC_CAST(agc_buffer_t, &gc, stack[i].v.heap_idx);
        REQUIRE(*AGC_CAST(int32_t, &gc, b->buff.v.heap_idx) == i);
    }

    for (int32_t i = 0; i < (int32_t)stack.size(); ++i) {
        if (i % 2 != 0) stack[i].tag.b = ABT_NIL;
    }

    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());

    for (int32_t i = 0; i < 1000; ++i) {
        REQUIRE((i % 2 == 0) == search_for(&gc, i));
    }

    for (int32_t i = 0; i < (int32_t)stack.size(); ++i) {
        if (i % 4 != 0) stack[i].tag.b = ABT_NIL;
    }

    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());

    for (int32_t i = 0; i < 1000; ++i) {
        REQUIRE((i % 4 == 0) == search_for(&gc, i));
    }

    for (int32_t i = 0; i < (int32_t)stack.size(); ++i) {
        stack[i].tag.b = ABT_NIL;
    }

    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());
    agc_collect(&gc, stack.data(), (int32_t)stack.size());

    for (int32_t i = 0; i < 1000; ++i) {
        REQUIRE(!search_for(&gc, i));
    }

    REQUIRE(agc_heap_size(&gc) == 0);

    agc_cleanup(&gc);
}