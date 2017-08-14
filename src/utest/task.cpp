/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>

enum { CSTACK_SZ = 8192 };

struct ctx_t
{
    int32_t val;
    atask_t task;
    alist_node_t node;
};

static void ASTDCALL fib_func(void* ud)
{
    ctx_t* ctx = (ctx_t*)ud;
    for (int32_t i = 0; true; ++i) {
        atask_yield(&ctx->task, &ALIST_NODE_CAST(ctx_t, ctx->node.next)->task);
        ++ctx->val;
    }
}

TEST_CASE("task")
{
    ctx_t m;
    atask_shadow(&m.task);
    m.node.next = &m.node;
    m.node.prev = &m.node;

    enum { NUM_TASKS = 1000 };

    static ctx_t ctx[NUM_TASKS];

    for (int32_t i = 0; i < NUM_TASKS; ++i) {
        ctx[i].val = 0;
        REQUIRE(AERR_NONE ==
            atask_create(&ctx[i].task, &fib_func, ctx + i, CSTACK_SZ));
        alist_node_insert(&ctx[i].node, m.node.prev, m.node.prev->next);
    }

    for (int32_t i = 0; i < 100; ++i) {
        atask_yield(&m.task, &ALIST_NODE_CAST(ctx_t, m.node.next)->task);
        for (int32_t j = 0; j < NUM_TASKS; ++j) {
            REQUIRE(ctx[j].val == i);
        }
    }

    for (int32_t i = 0; i < NUM_TASKS; ++i) {
        atask_delete(&ctx[i].task);
        alist_node_erase(&ctx[i].node);
    }
}
