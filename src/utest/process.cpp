/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/errno.h>
#include <any/rt_types.h>
#include <any/process.h>

#if 0

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

TEST_CASE("proc_try_throw")
{
    aprocess_t p;
    p.error_jmp = NULL;
    REQUIRE(AERR_NONE == aprocess_try(&p, &try_throw1, (void*)0xF1));
}

#endif