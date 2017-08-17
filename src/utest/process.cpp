/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/scheduler.h>

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

TEST_CASE("process_allocation")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    // empty
    for (apid_gen_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (apid_idx_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            REQUIRE(NULL == ascheduler_actor(
                &s, apid_from(NUM_IDX_BITS, NUM_GEN_BITS, i, g)));
        }
    }

    // basic
    for (apid_gen_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (apid_idx_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            aprocess_t* p = ascheduler_alloc(&s);
            REQUIRE(p != NULL);
            REQUIRE(&p->actor == ascheduler_actor(&s, p->pid));
            ascheduler_free(p);
        }
    }

    // over-allocate
    aprocess_t* procs[1 << NUM_IDX_BITS];
    for (apid_idx_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        procs[i] = ascheduler_alloc(&s);
        REQUIRE(procs[i]);
    }
    REQUIRE(ascheduler_alloc(&s) == NULL);
    for (apid_idx_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        ascheduler_free(procs[i]);
    }

    ascheduler_cleanup(&s);
}
