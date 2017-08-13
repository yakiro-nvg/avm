/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <stdlib.h>
#include <any/vm.h>
#include <any/scheduler.h>

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

TEST_CASE("vm_allocation")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    avm_t vm;

    REQUIRE(AERR_NONE ==
        avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    // empty
    for (int32_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            REQUIRE(!avm_from_pid(
                &vm, apid_from(NUM_IDX_BITS, NUM_GEN_BITS, i, g)));
        }
    }

    // basic
    for (int32_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            aprocess_t* p = avm_alloc(&vm);
            REQUIRE(p != NULL);
            REQUIRE(p == avm_from_pid(&vm, avm_pid(p)));
            avm_free(p);
        }
    }

    // over-allocate
    aprocess_t* procs[1 << NUM_IDX_BITS];
    for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        procs[i] = avm_alloc(&vm);
        REQUIRE(procs[i]);
    }
    REQUIRE(avm_alloc(&vm) == NULL);
    for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        avm_free(procs[i]);
    }

    avm_shutdown(&vm);
}
