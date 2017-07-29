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
    REQUIRE(AERR_NONE == aprocess_try(p, &try_throw6, (void*)0xF6));
}

static void try_throw41(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF41);
    REQUIRE(AERR_NONE == aprocess_try(p, &try_throw5, (void*)0xF5));
}

static void try_throw42(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF42);
    aprocess_throw(p, AERR_OVERFLOW);
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
    REQUIRE(AERR_OVERFLOW == aprocess_try(p, &try_throw3, (void*)0xF3));
}

static void try_throw1(aprocess_t* p, void* ud)
{
    REQUIRE((int32_t)ud == 0xF1);
    try_throw2(p, (void*)0xF2);
}

static int32_t stack_test(aprocess_t* p)
{
    REQUIRE(aprocess_stack_top(p) == 0);

    int32_t total_ints = 0;
    for (int32_t j = 0; j < 10; ++j) {
        int32_t num_nils = p->stack_cap - p->stack_sz + 1;
        for (int32_t i = 0; i < num_nils; ++i) {
            aprocess_push_integer(p, total_ints);
            ++total_ints;
        }
    }

    aprocess_push_bool(p, TRUE);
    aprocess_push_bool(p, FALSE);
    aprocess_push_integer(p, 1991);
    aprocess_push_real(p, 18.12f);

    REQUIRE(aprocess_stack_top(p) == total_ints + 4);

    REQUIRE(aprocess_type(p, -1).b == ABT_NUMBER);
    REQUIRE(aprocess_type(p, -1).variant == AVTN_REAL);
    REQUIRE(aprocess_to_real(p, -1) == 18.12f);

    REQUIRE(aprocess_type(p, -2).b == ABT_NUMBER);
    REQUIRE(aprocess_type(p, -2).variant == AVTN_INTEGER);
    REQUIRE(aprocess_to_integer(p, -2) == 1991);

    REQUIRE(aprocess_type(p, -3).b == ABT_BOOL);
    REQUIRE(aprocess_to_bool(p, -3) == FALSE);

    REQUIRE(aprocess_type(p, -4).b == ABT_BOOL);
    REQUIRE(aprocess_to_bool(p, -4) == TRUE);

    aprocess_pop(p, 4);
    REQUIRE(aprocess_stack_top(p) == total_ints);

    for (int32_t i = total_ints - 1; i >= 0; --i) {
        REQUIRE(aprocess_type(p, i + 1).b == ABT_NUMBER);
        REQUIRE(aprocess_type(p, i + 1).variant == AVTN_INTEGER);
        REQUIRE(aprocess_to_integer(p, i + 1) == i);
        REQUIRE(aprocess_type(p, -1).b == ABT_NUMBER);
        REQUIRE(aprocess_type(p, -1).variant == AVTN_INTEGER);
        REQUIRE(aprocess_to_integer(p, -1) == i);
        aprocess_pop(p, 1);
    }

    aprocess_push_integer(p, 0xFFAA);

    return 0;
}

TEST_CASE("process_try_throw")
{
    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));
    REQUIRE(AERR_NONE == aprocess_try(&p, &try_throw1, (void*)0xF1));
}

TEST_CASE("process_stack")
{
    ascheduler_t s;
    s.alloc = &myalloc;
    s.alloc_ud = NULL;
    atask_shadow(&s.task);

    aprocess_t p;
    memset(&p, 0, sizeof(aprocess_t));

    aprocess_init(&p, &s, 0);

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.tag.collectable = FALSE;
    entry.v.func = &stack_test;
    aprocess_push(&p, &entry);
    aprocess_start(&p, 512);
    atask_yield(&s.task);
    REQUIRE(aprocess_stack_top(&p) == 1);
    REQUIRE(aprocess_type(&p, -1).b == ABT_NUMBER);
    REQUIRE(aprocess_type(&p, -1).variant == AVTN_INTEGER);
    REQUIRE(aprocess_to_integer(&p, -1) == 0xFFAA);

    aprocess_cleanup(&p);
}