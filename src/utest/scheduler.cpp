/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <stdlib.h>
#include <any/scheduler.h>
#include <any/actor.h>

enum { CSTACK_SZ = 8192 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, sz);
}

static void nop(aactor_t* a)
{
    any_push_nil(a);
}

static void spawn_new(ascheduler_t* s, avalue_t* entry)
{
    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(s, CSTACK_SZ, &a));
    aactor_push(a, entry);
    ascheduler_start(s, a, 0);
}

TEST_CASE("scheduler_new_process")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t entry;
    entry.tag.b = ABT_FUNCTION;
    entry.tag.variant = AVTF_NATIVE;
    entry.v.func = &nop;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    for (aint_t i = 0; i < 10; ++i) {
        spawn_new(&s, &entry);
    }

    ascheduler_run_once(&s);
    ascheduler_run_once(&s); // cleanup is deferred
    REQUIRE(alist_head(&s.runnings) == &s.root.node);

    for (aint_t i = 0; i < 10; ++i) {
        spawn_new(&s, &entry);
    }

    ascheduler_cleanup(&s);
}
