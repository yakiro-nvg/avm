/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>

struct ctx_t
{
    int32_t val;
    atask_t self;
};

static void ASTDCALL fib_func(void* ud)
{
    ctx_t* ctx = (ctx_t*)ud;
    for (int32_t i = 0; true; ++i) {
        atask_yield(&ctx->self);
        ++ctx->val;
    }
}

TEST_CASE("task")
{
    atask_t m;
    atask_shadow(&m);

    enum { NUM_TASKS = 1000 };

    static ctx_t ctx[NUM_TASKS];

    for (int32_t i = 0; i < NUM_TASKS; ++i) {
        ctx[i].val = 0;
        REQUIRE(AERR_NONE == 
            atask_create(&ctx[i].self, &m, &fib_func, ctx + i, 512));
    }

    for (int32_t i = 0; i < 100; ++i) {
        atask_yield(&m);
        for (int32_t j = 0; j < NUM_TASKS; ++j) {
            REQUIRE(ctx[j].val == i);
        }
    }

    for (int32_t i = 0; i < NUM_TASKS; ++i) {
        atask_delete(&ctx[i].self);
    }
}
