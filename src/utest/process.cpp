/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/process.h>
#include <any/vm.h>
#include <any/scheduler.h>
#include <any/gc_string.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void try_throw6(aprocess_t*, void* ud)
{
    REQUIRE((size_t)ud == 0xF6);
}

static void try_throw5(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF5);
    REQUIRE(AERR_NONE == any_try(p, &try_throw6, (void*)0xF6));
}

static void try_throw41(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF41);
    REQUIRE(AERR_NONE == any_try(p, &try_throw5, (void*)0xF5));
}

static void try_throw42(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF42);
    any_throw(p, AERR_RUNTIME);
}

static void try_throw3(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF3);
    try_throw41(p, (void*)0xF41);
    try_throw42(p, (void*)0xF42);
}

static void try_throw2(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF2);
    REQUIRE(AERR_RUNTIME == any_try(p, &try_throw3, (void*)0xF3));
}

static void try_throw1(aprocess_t* p, void* ud)
{
    REQUIRE((size_t)ud == 0xF1);
    try_throw2(p, (void*)0xF2);
}

static void try_throw(aprocess_t* p)
{
    REQUIRE(AERR_NONE == any_try(p, &try_throw1, (void*)0xF1));
    any_push_nil(p);
}

static void stack_test(aprocess_t* p)
{
    REQUIRE(any_type(p, -1).b == ABT_NUMBER);
    REQUIRE(any_type(p, -1).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, -1) == 0xA1);

    REQUIRE(any_type(p, -2).b == ABT_NUMBER);
    REQUIRE(any_type(p, -2).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, -2) == 0xA2);

    REQUIRE(any_type(p, -3).b == ABT_NUMBER);
    REQUIRE(any_type(p, -3).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, -3) == 0xA3);

    REQUIRE(any_type(p, -4).b == ABT_NIL);
    REQUIRE(any_type(p, -5).b == ABT_NIL);

    REQUIRE(any_count(p) == 0);

    enum { NUM_INTS = 10000 };

    for (int32_t i = 0; i < NUM_INTS; ++i) {
        any_push_integer(p, i);
    }

    any_push_bool(p, TRUE);
    any_push_bool(p, FALSE);
    any_push_integer(p, 1991);
    any_push_real(p, 18.12f);
    any_push_pid(p, 0xBEEF);

    REQUIRE(any_count(p) == NUM_INTS + 5);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_PID);
    REQUIRE(any_to_pid(p, any_count(p) - 1) == 0xBEEF);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 2).variant == AVTN_REAL);
    REQUIRE(any_to_real(p, any_count(p) - 2) == 18.12f);

    REQUIRE(any_type(p, any_count(p) - 3).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 3).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, any_count(p) - 3) == 1991);

    REQUIRE(any_type(p, any_count(p) - 4).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 4) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 5).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 5) == TRUE);

    any_remove(p, any_count(p) - 3);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_PID);
    REQUIRE(any_to_pid(p, any_count(p) - 1) == 0xBEEF);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 2).variant == AVTN_REAL);
    REQUIRE(any_to_real(p, any_count(p) - 2) == 18.12f);

    REQUIRE(any_type(p, any_count(p) - 3).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 3) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 4).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 4) == TRUE);

    any_remove(p, any_count(p) - 1);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 1).variant == AVTN_REAL);
    REQUIRE(any_to_real(p, any_count(p) - 1) == 18.12f);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 2) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 3).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 3) == TRUE);

    any_pop(p, 3);

    any_remove(p, 0);

    for (int32_t i = NUM_INTS - 2; i >= 0; --i) {
        REQUIRE(any_type(p, i).b == ABT_NUMBER);
        REQUIRE(any_type(p, i).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, i) == i + 1);
        any_pop(p, 1);
    }

    REQUIRE(any_count(p) == 0);

    any_push_integer(p, 1);
    any_push_integer(p, 2);
    any_push_integer(p, 3);
    any_push_integer(p, 0xFFAA);
    any_insert(p, 1);
    any_pop(p, 1);

    REQUIRE(any_count(p) == 2);
}

static int32_t num_spawn_tests;

static void spawn_test(aprocess_t* p)
{
    REQUIRE(any_type(p, -1).b == ABT_NUMBER);
    REQUIRE(any_type(p, -1).variant == AVTN_INTEGER);

    REQUIRE(any_type(p, -2).b == ABT_FUNCTION);
    REQUIRE(any_type(p, -2).variant == AVTF_NATIVE);

    int32_t i = (int32_t)any_to_integer(p, -1);
    if (i < 10) {
        any_push_idx(p, -2);
        any_push_idx(p, -2);
        any_push_integer(p, i + 1);
        apid_t _;
        ++num_spawn_tests;
        any_spawn(p, CSTACK_SZ, 2, &_);
    }

    any_push_nil(p);
}

TEST_CASE("process_try_throw")
{
    ascheduler_t s;
    atask_shadow(&s.task);

    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));
    aprocess_init(&p, &s, &myalloc, NULL);

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &try_throw;
    aprocess_start(&p, CSTACK_SZ, 0);
    atask_yield(&s.task);

    aprocess_cleanup(&p);
}

TEST_CASE("process_stack")
{
    ascheduler_t s;
    atask_shadow(&s.task);

    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));

    aprocess_init(&p, &s, &myalloc, NULL);

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &stack_test;
    aprocess_push(&p, &entry);
    any_push_integer(&p, 0xA3);
    any_push_integer(&p, 0xA2);
    any_push_integer(&p, 0xA1);
    aprocess_start(&p, CSTACK_SZ, 3);
    atask_yield(&s.task);
    REQUIRE(any_count(&p) == 2);
    REQUIRE(any_type(&p, 1).b == ABT_NIL);
    REQUIRE(any_type(&p, 0).b == ABT_NUMBER);
    REQUIRE(any_type(&p, 0).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(&p, 0) == 0xFFAA);

    aprocess_cleanup(&p);
}

TEST_CASE("process_call_non_function")
{
    ascheduler_t s;
    atask_shadow(&s.task);

    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));

    aprocess_init(&p, &s, &myalloc, NULL);

    avalue_t entry;
    entry.tag.b = ABT_NIL;
    aprocess_push(&p, &entry);
    aprocess_start(&p, CSTACK_SZ, 0);
    atask_yield(&s.task);
    REQUIRE(any_count(&p) == 1);
    REQUIRE(any_type(&p, 0).b == ABT_STRING);
    CHECK_THAT(any_to_string(&p, 0),
        Catch::Equals("attempt to call a non-function"));

    aprocess_cleanup(&p);
}

TEST_CASE("process_spawn")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    avm_t vm;
    ascheduler_t s;

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &spawn_test;

    REQUIRE(AERR_NONE ==
        avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    REQUIRE(AERR_NONE == ascheduler_init(&s, &vm, &myalloc, NULL));

    num_spawn_tests = 0;

    aprocess_t* p;
    REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
    aprocess_push(p, &entry);
    aprocess_push(p, &entry);
    any_push_integer(p, 0);
    aprocess_start(p, CSTACK_SZ, 2);
    ascheduler_run_once(&s);

    REQUIRE(num_spawn_tests == 10);

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
}