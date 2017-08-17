/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/actor.h>
#include <any/scheduler.h>
#include <any/gc_string.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, sz);
}

static void try_throw6(aactor_t*, void* ud)
{
    REQUIRE((size_t)ud == 0xF6);
}

static void try_throw5(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF5);
    REQUIRE(AERR_NONE == any_try(a, &try_throw6, (void*)0xF6));
}

static void try_throw41(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF41);
    REQUIRE(AERR_NONE == any_try(a, &try_throw5, (void*)0xF5));
}

static void try_throw42(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF42);
    any_throw(a, AERR_RUNTIME);
}

static void try_throw3(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF3);
    try_throw41(a, (void*)0xF41);
    try_throw42(a, (void*)0xF42);
}

static void try_throw2(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF2);
    REQUIRE(AERR_RUNTIME == any_try(a, &try_throw3, (void*)0xF3));
}

static void try_throw1(aactor_t* a, void* ud)
{
    REQUIRE((size_t)ud == 0xF1);
    try_throw2(a, (void*)0xF2);
}

static void try_throw(aactor_t* a)
{
    REQUIRE(AERR_NONE == any_try(a, &try_throw1, (void*)0xF1));
    any_push_nil(a);
}

static void stack_test(aactor_t* a)
{
    REQUIRE(any_type(a, -1).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, -1) == 0xA1);

    REQUIRE(any_type(a, -2).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, -2) == 0xA2);

    REQUIRE(any_type(a, -3).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, -3) == 0xA3);

    REQUIRE(any_type(a, -4).type == AVT_NIL);
    REQUIRE(any_type(a, -5).type == AVT_NIL);

    REQUIRE(any_count(a) == 0);

    enum { NUM_INTS = 10000 };

    for (aint_t i = 0; i < NUM_INTS; ++i) {
        any_push_integer(a, i);
    }

    any_push_bool(a, TRUE);
    any_push_bool(a, FALSE);
    any_push_integer(a, 1991);
    any_push_real(a, 18.12f);
    any_push_pid(a, 0xBEEF);
    any_push_native_func(a, (anative_func_t)0xF0);

    REQUIRE(any_count(a) == NUM_INTS + 6);

    REQUIRE(any_type(a, any_count(a) - 1).type == AVT_NATIVE_FUNC);
    REQUIRE(any_to_native_func(a, any_count(a) - 1) == (anative_func_t)0xF0);

    REQUIRE(any_type(a, any_count(a) - 2).type == AVT_PID);
    REQUIRE(any_to_pid(a, any_count(a) - 2) == 0xBEEF);


    REQUIRE(any_type(a, any_count(a) - 3).type == AVT_REAL);
    REQUIRE(any_to_real(a, any_count(a) - 3) == 18.12f);

    REQUIRE(any_type(a, any_count(a) - 4).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, any_count(a) - 4) == 1991);

    REQUIRE(any_type(a, any_count(a) - 5).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 5) == FALSE);

    REQUIRE(any_type(a, any_count(a) - 6).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 6) == TRUE);

    any_remove(a, any_count(a) - 3);

    REQUIRE(any_type(a, any_count(a) - 1).type == AVT_NATIVE_FUNC);
    REQUIRE(any_to_native_func(a, any_count(a) - 1) == (anative_func_t)0xF0);

    REQUIRE(any_type(a, any_count(a) - 2).type == AVT_PID);
    REQUIRE(any_to_pid(a, any_count(a) - 2) == 0xBEEF);

    REQUIRE(any_type(a, any_count(a) - 3).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, any_count(a) - 3) == 1991);

    REQUIRE(any_type(a, any_count(a) - 4).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 4) == FALSE);

    REQUIRE(any_type(a, any_count(a) - 5).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 5) == TRUE);

    any_remove(a, any_count(a) - 1);

    REQUIRE(any_type(a, any_count(a) - 1).type == AVT_PID);
    REQUIRE(any_to_pid(a, any_count(a) - 1) == 0xBEEF);

    REQUIRE(any_type(a, any_count(a) - 2).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, any_count(a) - 2) == 1991);

    REQUIRE(any_type(a, any_count(a) - 3).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 3) == FALSE);

    REQUIRE(any_type(a, any_count(a) - 4).type == AVT_BOOLEAN);
    REQUIRE(any_to_bool(a, any_count(a) - 4) == TRUE);

    any_pop(a, 4);

    any_remove(a, 0);

    for (aint_t i = NUM_INTS - 2; i >= 0; --i) {
        REQUIRE(any_type(a, i).type == AVT_INTEGER);
        REQUIRE(any_to_integer(a, i) == i + 1);
        any_pop(a, 1);
    }

    REQUIRE(any_count(a) == 0);

    any_push_integer(a, 1);
    any_push_integer(a, 2);
    any_push_integer(a, 3);
    any_push_integer(a, 0xFFAA);
    any_insert(a, 1);
    any_pop(a, 1);

    REQUIRE(any_count(a) == 2);
}

static aint_t num_spawn_tests;

static void spawn_test(aactor_t* a)
{
    REQUIRE(any_type(a, -1).type == AVT_INTEGER);
    REQUIRE(any_type(a, -2).type == AVT_NATIVE_FUNC);

    aint_t i = any_to_integer(a, -1);
    if (i < 10) {
        any_push_idx(a, -2);
        any_push_idx(a, -2);
        any_push_integer(a, i + 1);
        apid_t _;
        ++num_spawn_tests;
        any_spawn(a, CSTACK_SZ, 2, &_);
    }

    any_push_nil(a);
}

TEST_CASE("process_try_throw")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &try_throw);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    ascheduler_cleanup(&s);
}

TEST_CASE("process_stack")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &stack_test);
    any_push_integer(a, 0xA3);
    any_push_integer(a, 0xA2);
    any_push_integer(a, 0xA1);
    ascheduler_start(&s, a, 3);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, 1).type == AVT_NIL);
    REQUIRE(any_type(a, 0).type == AVT_INTEGER);
    REQUIRE(any_to_integer(a, 0) == 0xFFAA);

    ascheduler_cleanup(&s);
}

TEST_CASE("process_call_non_function")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));

    any_push_nil(a);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 1);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0),
        Catch::Equals("attempt to call a non-function"));

    ascheduler_cleanup(&s);
}

TEST_CASE("process_spawn")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    num_spawn_tests = 0;

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &spawn_test);
    any_push_native_func(a, &spawn_test);
    any_push_integer(a, 0);
    ascheduler_start(&s, a, 2);

    ascheduler_run_once(&s);

    REQUIRE(num_spawn_tests == 10);

    ascheduler_cleanup(&s);
}