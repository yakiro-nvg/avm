/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/platform.h>
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/gc.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_string.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
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
        CHECK_THAT(any_check_string(a, any_check_index(a, i)),
            Catch::Equals(buff));
    }
    any_push_string(a, "ok");
}

TEST_CASE("std_string")
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

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
        Catch::Equals("ok"));

    ascheduler_cleanup(&s);
}
