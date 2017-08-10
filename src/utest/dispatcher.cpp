/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/asm.h>
#include <any/loader.h>
#include <any/vm.h>
#include <any/scheduler.h>
#include <any/process.h>
#include <any/dispatcher.h>
#include <any/gc_string.h>

enum { CSTACK_SZ = 8192 };

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static void add_test_module(aasm_t* a)
{
    aasm_prototype_t* p = aasm_prototype(a);
    p->symbol = aasm_string_to_ref(a, "mod_test");
}

TEST_CASE("dispatcher_loop")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    REQUIRE(aasm_load(&a, NULL) == AERR_NONE);
    add_test_module(&a);

    avm_t vm;
    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, &vm, &myalloc, NULL));

    SECTION("no instructions")
    {
        aasm_module_push(&a, "empty");
        aasm_pop(&a);
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "empty");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return missing"));
    }

    SECTION("return missing")
    {
        aasm_module_push(&a, "no_return");
        aasm_emit(&a, ai_nop());
        aasm_pop(&a);
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "no_return");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return missing"));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}