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
        aasm_module_push(&a, "test_f");
        aasm_pop(&a);
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return missing"));
    }

    SECTION("return missing")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_nop());
        aasm_pop(&a);
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return missing"));
    }

    SECTION("nop")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_nop());
        aasm_emit(&a, ai_ret());
        aasm_pop(&a);
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return value missing"));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_ldk")
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

    SECTION("no constant")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_ldk(0));
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad constant index 0"));
    }

    SECTION("overflow index")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_constant(&a, ac_integer(0xC0));
        aasm_emit(&a, ai_ldk(1));
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad constant index 1"));
    }

    SECTION("negative index")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_constant(&a, ac_integer(0xC0));
        aasm_emit(&a, ai_ldk(-1));
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad constant index -1"));
    }

    SECTION("return string")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_constant(&a, ac_integer(0xC0));
        aasm_add_constant(&a, ac_string(aasm_string_to_ref(&a, "0xC1")));
        aasm_add_constant(&a, ac_real(3.14f));
        aasm_emit(&a, ai_ldk(2));
        aasm_emit(&a, ai_ldk(0));
        aasm_emit(&a, ai_ldk(1));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("0xC1"));
    }

    SECTION("return integer")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_constant(&a, ac_integer(0xC0));
        aasm_add_constant(&a, ac_string(aasm_string_to_ref(&a, "0xC1")));
        aasm_add_constant(&a, ac_real(3.14f));
        aasm_emit(&a, ai_ldk(1));
        aasm_emit(&a, ai_ldk(2));
        aasm_emit(&a, ai_ldk(0));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 0xC0);
    }

    SECTION("return real")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_constant(&a, ac_integer(0xC0));
        aasm_add_constant(&a, ac_string(aasm_string_to_ref(&a, "0xC1")));
        aasm_add_constant(&a, ac_real(3.14f));
        aasm_emit(&a, ai_ldk(0));
        aasm_emit(&a, ai_ldk(1));
        aasm_emit(&a, ai_ldk(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_REAL);
        REQUIRE(any_to_real(p, 0) == Approx(3.14f));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_nil")
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

    aasm_module_push(&a, "test_f");
    aasm_emit(&a, ai_nil());
    aasm_emit(&a, ai_ret());
    aasm_save(&a);
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

    aprocess_t* p;
    REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
    any_find(p, "mod_test", "test_f");
    aprocess_start(p, CSTACK_SZ, 0);
    atask_yield(&s.task);
    REQUIRE(any_count(p) == 2);
    REQUIRE(any_type(p, 1).b == ABT_NIL);
    REQUIRE(any_type(p, 0).b == ABT_NIL);

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_ldb")
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

    SECTION("false")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_ldb(FALSE));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_BOOL);
        REQUIRE(any_to_bool(p, 0) == FALSE);
    }

    SECTION("true")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_ldb(TRUE));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_BOOL);
        REQUIRE(any_to_bool(p, 0) == TRUE);
    }

    SECTION("abnormal true")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_ldb(0xFA));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_BOOL);
        REQUIRE(any_to_bool(p, 0) == TRUE);
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_lsi")
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

    SECTION("zero")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 0);
    }

    SECTION("positive")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(2017));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 2017);
    }

    SECTION("negative")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(-2020));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == -2020);
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_pop")
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

    aasm_module_push(&a, "test_f");
    aasm_emit(&a, ai_lsi(1969));
    aasm_emit(&a, ai_lsi(1970));
    aasm_emit(&a, ai_lsi(1971));
    aasm_emit(&a, ai_lsi(1972));
    aasm_emit(&a, ai_pop(0));
    aasm_emit(&a, ai_pop(2));
    aasm_emit(&a, ai_pop(0));
    aasm_emit(&a, ai_ret());
    aasm_save(&a);
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

    aprocess_t* p;
    REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
    any_find(p, "mod_test", "test_f");
    aprocess_start(p, CSTACK_SZ, 0);
    atask_yield(&s.task);
    REQUIRE(any_count(p) == 2);
    REQUIRE(any_type(p, 1).b == ABT_NIL);
    REQUIRE(any_type(p, 0).b == ABT_NUMBER);
    REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
    REQUIRE(any_to_integer(p, 0) == 1970);

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_llv_slv")
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

    int pop_num;

    SECTION("pop 0") { pop_num = 0; }
    SECTION("pop 1") { pop_num = 1; }
    SECTION("pop 2") { pop_num = 2; }
    SECTION("pop 3") { pop_num = 3; }
    SECTION("pop 4") { pop_num = 4; }

    aasm_module_push(&a, "test_f");
    aasm_emit(&a, ai_lsi(1969));
    aasm_emit(&a, ai_lsi(1970));
    aasm_emit(&a, ai_lsi(1971));
    aasm_emit(&a, ai_lsi(1972));
    aasm_emit(&a, ai_pop(1));
    aasm_emit(&a, ai_llv(0));
    aasm_emit(&a, ai_llv(2));
    aasm_emit(&a, ai_slv(1));
    aasm_emit(&a, ai_lsi(1972));
    aasm_emit(&a, ai_slv(2));
    aasm_emit(&a, ai_lsi(1970));
    aasm_emit(&a, ai_slv(1));
    aasm_emit(&a, ai_lsi(0xBABE));
    aasm_emit(&a, ai_slv(0));
    aasm_emit(&a, ai_pop(pop_num));
    aasm_emit(&a, ai_ret());
    aasm_save(&a);
    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

    aprocess_t* p;
    REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
    any_find(p, "mod_test", "test_f");
    aprocess_start(p, CSTACK_SZ, 0);
    atask_yield(&s.task);
    if (pop_num < 4) {
        static const int32_t cmp_table[] = { 1969, 1972, 1970, 0xBABE };
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == cmp_table[pop_num]);
    } else {
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("return value missing"));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_imp")
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

    static alib_func_t nfuncs[] = {
        { "f0", (anative_func_t)0xF0 },
        { "f1", (anative_func_t)0xF1 },
        { "f2", (anative_func_t)0xF2 },
        { NULL, NULL }
    };
    static alib_t nmodule = { "mod_imp", nfuncs };

    aloader_add_lib(&vm.loader, &nmodule);

    SECTION("no import")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_imp(0));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad import index 0"));
    }

    SECTION("overflow index")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_import(&a, "mod_imp", "f0");
        aasm_add_import(&a, "mod_imp", "f1");
        aasm_add_import(&a, "mod_imp", "f2");
        aasm_emit(&a, ai_imp(3));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad import index 3"));
    }

    SECTION("negative index")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_import(&a, "mod_imp", "f0");
        aasm_add_import(&a, "mod_imp", "f1");
        aasm_add_import(&a, "mod_imp", "f2");
        aasm_emit(&a, ai_imp(-1));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad import index -1"));
    };

    SECTION("import 0")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_import(&a, "mod_imp", "f0");
        aasm_add_import(&a, "mod_imp", "f1");
        aasm_add_import(&a, "mod_imp", "f2");
        aasm_emit(&a, ai_imp(0));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_FUNCTION);
        REQUIRE(any_type(p, 0).variant == AVTF_NATIVE);
        REQUIRE(p->stack[aprocess_absidx(p, 0)].v.func == (anative_func_t)0xF0);
    };

    SECTION("import 1")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_import(&a, "mod_imp", "f0");
        aasm_add_import(&a, "mod_imp", "f1");
        aasm_add_import(&a, "mod_imp", "f2");
        aasm_emit(&a, ai_imp(1));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_FUNCTION);
        REQUIRE(any_type(p, 0).variant == AVTF_NATIVE);
        REQUIRE(p->stack[aprocess_absidx(p, 0)].v.func == (anative_func_t)0xF1);
    };

    SECTION("import 2")
    {
        aasm_module_push(&a, "test_f");
        aasm_add_import(&a, "mod_imp", "f0");
        aasm_add_import(&a, "mod_imp", "f1");
        aasm_add_import(&a, "mod_imp", "f2");
        aasm_emit(&a, ai_imp(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_FUNCTION);
        REQUIRE(any_type(p, 0).variant == AVTF_NATIVE);
        REQUIRE(p->stack[aprocess_absidx(p, 0)].v.func == (anative_func_t)0xF2);
    };

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_jmp")
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

    SECTION("normal")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_jmp(2));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_lsi(3));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 1);
    }

    SECTION("bad negative index")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_jmp(-4));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_lsi(3));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad jump"));
    }

    SECTION("bad positive index")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_jmp(3));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_lsi(3));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad jump"));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_jin")
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

    SECTION("true")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_ldb(TRUE));
        aasm_emit(&a, ai_jin(1));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 2);
    }

    SECTION("false")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_ldb(FALSE));
        aasm_emit(&a, ai_jin(1));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 1);
    }

    SECTION("bad negative index")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_ldb(FALSE));
        aasm_emit(&a, ai_jin(-5));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad jump"));
    }

    SECTION("bad positive index")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_ldb(FALSE));
        aasm_emit(&a, ai_jin(2));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0), Catch::Equals("bad jump"));
    }

    SECTION("bad condition")
    {
        aasm_module_push(&a, "test_f");
        aasm_emit(&a, ai_lsi(0));
        aasm_emit(&a, ai_lsi(1));
        aasm_emit(&a, ai_jin(2));
        aasm_emit(&a, ai_lsi(2));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 1);
        REQUIRE(any_type(p, 0).b == ABT_STRING);
        CHECK_THAT(any_to_string(p, 0),
            Catch::Equals("condition must be boolean"));
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}

TEST_CASE("dispatcher_mkc_ivk")
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

    SECTION("normal fefe")
    {
        aasm_module_push(&a, "test_f");
        int32_t npt = aasm_push(&a);
        {
            aasm_emit(&a, ai_llv(-1));
            aasm_emit(&a, ai_jin(2));
            aasm_emit(&a, ai_lsi(0xFEFE));
            aasm_emit(&a, ai_ret());
            aasm_emit(&a, ai_lsi(0xFEFA));
            aasm_emit(&a, ai_ret());
            aasm_pop(&a);
        }
        aasm_emit(&a, ai_mkc(npt));
        aasm_emit(&a, ai_ldb(TRUE));
        aasm_emit(&a, ai_ivk(1));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 0xFEFE);
    }

    SECTION("normal fefa")
    {
        aasm_module_push(&a, "test_f");
        int32_t npt = aasm_push(&a);
        {
            aasm_emit(&a, ai_llv(-1));
            aasm_emit(&a, ai_jin(2));
            aasm_emit(&a, ai_lsi(0xFEFE));
            aasm_emit(&a, ai_ret());
            aasm_emit(&a, ai_lsi(0xFEFA));
            aasm_emit(&a, ai_ret());
            aasm_pop(&a);
        }
        aasm_emit(&a, ai_mkc(npt));
        aasm_emit(&a, ai_ldb(FALSE));
        aasm_emit(&a, ai_ivk(1));
        aasm_emit(&a, ai_ret());
        aasm_save(&a);
        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&vm.loader, a.chunk, a.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&vm.loader, TRUE));

        aprocess_t* p;
        REQUIRE(AERR_NONE == ascheduler_new_process(&s, &p));
        any_find(p, "mod_test", "test_f");
        aprocess_start(p, CSTACK_SZ, 0);
        atask_yield(&s.task);
        REQUIRE(any_count(p) == 2);
        REQUIRE(any_type(p, 1).b == ABT_NIL);
        REQUIRE(any_type(p, 0).b == ABT_NUMBER);
        REQUIRE(any_type(p, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(p, 0) == 0xFEFA);
    }

    ascheduler_cleanup(&s);
    avm_shutdown(&vm);
    aasm_cleanup(&a);
}