/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>

struct ctx_t
{
    int32_t i;
    afiber_t* m;
    afiber_t self;
};

static void ASTDCALL fib_func(void* ud)
{
    ctx_t* ctx = (ctx_t*)ud;
    for (int32_t i = 0; true; ++i) {
        REQUIRE(ctx->i == i);
        afiber_switch(&ctx->self, ctx->m);
    }
}

TEST_CASE("fiber")
{
    afiber_t m;
    afiber_get(&m);

    enum { NUM_FIBERS = 256 };

    static ctx_t ctx[NUM_FIBERS];

    for (int32_t i = 0; i < NUM_FIBERS; ++i) {
        ctx[i].i = 0;
        ctx[i].m = &m;
        afiber_create(&ctx[i].self, &fib_func, ctx + i);
    }
    for (int32_t i = 0; i < NUM_FIBERS; ++i) {
        afiber_switch(&m, &ctx[i].self);
        ++ctx[i].i;
    }
    for (int32_t i = 0; i < NUM_FIBERS; ++i) {
        afiber_destroy(&ctx[i].self);
    }
}