#pragma once

#include <avm/prereq.h>
#include <avm/alloc.h>
#include <catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/state.h>

using namespace rc;

#define RC_SUCCESS(expr) RC_ASSERT(ASUCCESS(expr))
#define REQUIRE_SUCCESS(expr) REQUIRE(ASUCCESS(expr))

/* Malloc yields NULL if `full` */
struct StubAllocator : aalloc_t
{
    bool full;
    amalloc_t backing;
    StubAllocator();
    void release();
};