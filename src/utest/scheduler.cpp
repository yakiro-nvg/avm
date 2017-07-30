/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <stdlib.h>
#include <any/vm.h>
#include <any/scheduler.h>
#include <any/process.h>
#include <any/errno.h>

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void nop(aprocess_t* p)
{
    any_push_nil(p);
}

static void spawn_new(ascheduler_t* s, avalue_t* entry)
{
    aprocess_t* p;
    REQUIRE(AERR_NONE == ascheduler_new_process(s, &p));
    aprocess_push(p, entry);
    aprocess_start(p, 512, 0);
}

TEST_CASE("scheduler_new_process")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    avm_t vm;
    ascheduler_t s;

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &nop;

    REQUIRE(AERR_NONE ==
        avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    REQUIRE(AERR_NONE == ascheduler_init(&s, &vm, &myalloc, NULL));

    for (int32_t i = 0; i < 10; ++i) {
        spawn_new(&s, &entry);
    }

    ascheduler_run_once(&s);
    REQUIRE(s.task.node.next == &s.task.node);

    for (int32_t i = 0; i < 10; ++i) {
        spawn_new(&s, &entry);
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
}