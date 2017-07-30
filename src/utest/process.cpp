/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/errno.h>
#include <any/rt_types.h>
#include <any/process.h>

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void try_throw6(aprocess_t*, void* ud)
{
    REQUIRE((int32_t)ud == 0xF6);
}

static void try_throw5(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF5);
    REQUIRE(AERR_NONE == any_try(p, &try_throw6, (void*)0xF6));
}

static void try_throw41(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF41);
    REQUIRE(AERR_NONE == any_try(p, &try_throw5, (void*)0xF5));
}

static void try_throw42(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF42);
    any_throw(p, AERR_OVERFLOW);
}

static void try_throw3(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF3);
    try_throw41(p, (void*)0xF41);
    try_throw42(p, (void*)0xF42);
}

static void try_throw2(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF2);
    REQUIRE(AERR_OVERFLOW == any_try(p, &try_throw3, (void*)0xF3));
}

static void try_throw1(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF1);
    try_throw2(p, (void*)0xF2);
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

    REQUIRE(any_count(p) == NUM_INTS + 4);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 1).variant == AVTN_REAL);
    REQUIRE(any_to_real(p, any_count(p) - 1) == 18.12f);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 2).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, any_count(p) - 2) == 1991);

    REQUIRE(any_type(p, any_count(p) - 3).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 3) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 4).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 4) == TRUE);

    any_remove(p, any_count(p) - 2);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_NUMBER);
    REQUIRE(any_type(p, any_count(p) - 1).variant == AVTN_REAL);
    REQUIRE(any_to_real(p, any_count(p) - 1) == 18.12f);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 2) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 3).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 3) == TRUE);

    any_remove(p, any_count(p) - 1);

    REQUIRE(any_type(p, any_count(p) - 1).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 1) == FALSE);

    REQUIRE(any_type(p, any_count(p) - 2).b == ABT_BOOL);
    REQUIRE(any_to_bool(p, any_count(p) - 2) == TRUE);

    any_pop(p, 2);

    any_remove(p, 0);

    for (int32_t i = NUM_INTS - 2; i >= 0; --i) {
        REQUIRE(any_type(p, i).b == ABT_NUMBER);
        REQUIRE(any_type(p, i).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, i) == i + 1);
        any_pop(p, 1);
    }

    REQUIRE(any_count(p) == 0);

    any_push_integer(p, 0xFFAA);
}

TEST_CASE("process_try_throw")
{
    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));
    REQUIRE(AERR_NONE == any_try(&p, &try_throw1, (void*)0xF1));
}

TEST_CASE("process_stack")
{
    ascheduler_t s;
    atask_shadow(&s.task);

    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));

    aprocess_init(&p, &s, 0, &myalloc, NULL);

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &stack_test;
    aprocess_push(&p, &entry);
    any_push_integer(&p, 0xA3);
    any_push_integer(&p, 0xA2);
    any_push_integer(&p, 0xA1);
    aprocess_start(&p, 512, 3);
    atask_yield(&s.task);
    REQUIRE(any_count(&p) == 1);
    REQUIRE(any_type(&p, 0).b == ABT_NUMBER);
    REQUIRE(any_type(&p, 0).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(&p, 0) == 0xFFAA);

    aprocess_cleanup(&p);
}