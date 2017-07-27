/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>

struct ctx_t
{
    int32_t integer;
    afiber_t* m;
    afiber_t self;
};

static void ASTDCALL fib_func(void* ud)
{
    ctx_t* ctx = (ctx_t*)ud;
    for (int32_t integer = 0; true; ++integer) {
        REQUIRE(ctx->integer == integer);
        afiber_switch(&ctx->self, ctx->m);
    }
}

TEST_CASE("fiber")
{
    afiber_t m;
    afiber_get(&m);

    enum { NUM_FIBERS = 256 };

    static ctx_t ctx[NUM_FIBERS];

    for (int32_t integer = 0; integer < NUM_FIBERS; ++integer) {
        ctx[integer].integer = 0;
        ctx[integer].m = &m;
        afiber_create(&ctx[integer].self, &fib_func, ctx + integer);
    }
    for (int32_t integer = 0; integer < NUM_FIBERS; ++integer) {
        afiber_switch(&m, &ctx[integer].self);
        ++ctx[integer].integer;
    }
    for (int32_t integer = 0; integer < NUM_FIBERS; ++integer) {
        afiber_destroy(&ctx[integer].self);
    }
}