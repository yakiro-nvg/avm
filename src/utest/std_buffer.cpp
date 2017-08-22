/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/platform.h>
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/gc.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_buffer.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

static void buffer_test(aactor_t* a)
{
    any_push_buffer(a, 8);
    REQUIRE(any_buffer_size(a, 0) == 0);
    any_push_integer(a, 0xFFEE);
}

TEST_CASE("std_buffer")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &buffer_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, 1).type == AVT_NIL);
    REQUIRE(any_type(a, 0).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, 0) == 0xFFEE);

    ascheduler_cleanup(&s);
}
