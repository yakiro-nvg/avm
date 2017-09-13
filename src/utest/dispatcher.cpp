/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/loader.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_string.h>

TEST_CASE("dispatcher_loop")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("no_instructions")
    {
        aasm_module_push(&as, "test_f");
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("return missing"));
    }

    SECTION("return_missing")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nop());
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("return missing"));
    }

    SECTION("nop_brk")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nop());
        aasm_emit(&as, ai_brk());
        aasm_emit(&as, ai_ret());
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("return value missing"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_ldk")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("no_constant")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(0));
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad constant index 0"));
    }

    SECTION("overflow_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_constant(&as, ac_integer(0xC0));
        aasm_emit(&as, ai_ldk(1));
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad constant index 1"));
    }

    SECTION("negative_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_constant(&as, ac_integer(0xC0));
        aasm_emit(&as, ai_ldk(-1));
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad constant index -1"));
    }

    SECTION("return_string")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_constant(&as, ac_integer(0xC0));
        aasm_add_constant(&as, ac_string(aasm_string_to_ref(&as, "0xC1")));
        aasm_add_constant(&as, ac_real(3.14f));
        aasm_emit(&as, ai_ldk(2));
        aasm_emit(&as, ai_ldk(0));
        aasm_emit(&as, ai_ldk(1));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("0xC1"));
    }

    SECTION("return_integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_constant(&as, ac_integer(0xC0));
        aasm_add_constant(&as, ac_string(aasm_string_to_ref(&as, "0xC1")));
        aasm_add_constant(&as, ac_real(3.14f));
        aasm_emit(&as, ai_ldk(1));
        aasm_emit(&as, ai_ldk(2));
        aasm_emit(&as, ai_ldk(0));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xC0);
    }

    SECTION("return_real")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_constant(&as, ac_integer(0xC0));
        aasm_add_constant(&as, ac_string(aasm_string_to_ref(&as, "0xC1")));
        aasm_add_constant(&as, ac_real(3.14f));
        aasm_emit(&as, ai_ldk(0));
        aasm_emit(&as, ai_ldk(1));
        aasm_emit(&as, ai_ldk(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_real(a, any_check_index(a, 0)) == Approx(3.14));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_nil")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    aasm_module_push(&as, "test_f");
    aasm_emit(&as, ai_nil());
    aasm_emit(&as, ai_ret());
    aasm_save(&as);

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_find(a, "mod_test", "test_f");
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_ldb")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("false")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(FALSE));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(TRUE));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("abnormal_true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(0xFA));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_lsi")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("zero")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0);
    }

    SECTION("positive")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2017));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 2017);
    }

    SECTION("negative")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(-2020));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == -2020);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_pop")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    aasm_module_push(&as, "test_f");
    aasm_emit(&as, ai_lsi(1969));
    aasm_emit(&as, ai_lsi(1970));
    aasm_emit(&as, ai_lsi(1971));
    aasm_emit(&as, ai_lsi(1972));
    aasm_emit(&as, ai_pop(0));
    aasm_emit(&as, ai_pop(2));
    aasm_emit(&as, ai_pop(0));
    aasm_emit(&as, ai_ret());
    aasm_save(&as);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_find(a, "mod_test", "test_f");
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 1970);

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_llv_slv")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    int pop_num;

    SECTION("pop 0") { pop_num = 0; }
    SECTION("pop 1") { pop_num = 1; }
    SECTION("pop 2") { pop_num = 2; }
    SECTION("pop 3") { pop_num = 3; }
    SECTION("pop 4") { pop_num = 4; }

    aasm_module_push(&as, "test_f");
    aasm_emit(&as, ai_lsi(1969));
    aasm_emit(&as, ai_lsi(1970));
    aasm_emit(&as, ai_lsi(1971));
    aasm_emit(&as, ai_lsi(1972));
    aasm_emit(&as, ai_pop(1));
    aasm_emit(&as, ai_llv(0));
    aasm_emit(&as, ai_llv(2));
    aasm_emit(&as, ai_slv(1));
    aasm_emit(&as, ai_lsi(1972));
    aasm_emit(&as, ai_slv(2));
    aasm_emit(&as, ai_lsi(1970));
    aasm_emit(&as, ai_slv(1));
    aasm_emit(&as, ai_lsi(0xBABE));
    aasm_emit(&as, ai_slv(0));
    aasm_emit(&as, ai_pop(pop_num));
    aasm_emit(&as, ai_ret());
    aasm_save(&as);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_find(a, "mod_test", "test_f");
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    if (pop_num < 4) {
        static const aint_t cmp_table[] = { 1969, 1972, 1970, 0xBABE };
        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(cmp_table[pop_num] ==
            any_check_integer(a, any_check_index(a, 0)));
    } else {
        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("return value missing"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_imp")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    alib_func_t nfuncs[] = {
        { "f0", (anative_func_t)0xF0 },
        { "f1", (anative_func_t)0xF1 },
        { "f2", (anative_func_t)0xF2 },
        { NULL, NULL }
    };
    alib_t nmodule;
    memset(&nmodule, 0, sizeof(alib_t));
    nmodule.name = "mod_imp";
    nmodule.funcs = nfuncs;

    aloader_add_lib(&s.loader, &nmodule);

    SECTION("no_import")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_imp(0));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad import index 0"));
    }

    SECTION("overflow_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_import(&as, "mod_imp", "f0");
        aasm_add_import(&as, "mod_imp", "f1");
        aasm_add_import(&as, "mod_imp", "f2");
        aasm_emit(&as, ai_imp(3));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad import index 3"));
    }

    SECTION("negative_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_import(&as, "mod_imp", "f0");
        aasm_add_import(&as, "mod_imp", "f1");
        aasm_add_import(&as, "mod_imp", "f2");
        aasm_emit(&as, ai_imp(-1));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad import index -1"));
    };

    SECTION("import_0")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_import(&as, "mod_imp", "f0");
        aasm_add_import(&as, "mod_imp", "f1");
        aasm_add_import(&as, "mod_imp", "f2");
        aasm_emit(&as, ai_imp(0));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NATIVE_FUNC);
        REQUIRE((anative_func_t)0xF0 ==
            aactor_at(a, any_check_index(a, 0))->v.func);
    };

    SECTION("import_1")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_import(&as, "mod_imp", "f0");
        aasm_add_import(&as, "mod_imp", "f1");
        aasm_add_import(&as, "mod_imp", "f2");
        aasm_emit(&as, ai_imp(1));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NATIVE_FUNC);
        REQUIRE((anative_func_t)0xF1 ==
            aactor_at(a, any_check_index(a, 0))->v.func);
    };

    SECTION("import_2")
    {
        aasm_module_push(&as, "test_f");
        aasm_add_import(&as, "mod_imp", "f0");
        aasm_add_import(&as, "mod_imp", "f1");
        aasm_add_import(&as, "mod_imp", "f2");
        aasm_emit(&as, ai_imp(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NATIVE_FUNC);
        REQUIRE((anative_func_t)0xF2 ==
            aactor_at(a, any_check_index(a, 0))->v.func);
    };

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_jmp")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_jmp(2));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_lsi(3));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 1);
    }

    SECTION("bad_negative_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_jmp(-4));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_lsi(3));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad jump"));
    }

    SECTION("bad_positive_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_jmp(3));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_lsi(3));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_to_string(a, any_check_index(a, 0)),
            Catch::Equals("bad jump"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_jin")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_ldb(TRUE));
        aasm_emit(&as, ai_jin(1));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 2);
    }

    SECTION("false")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_ldb(FALSE));
        aasm_emit(&as, ai_jin(1));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 1);
    }

    SECTION("nil")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_nil());
        aasm_emit(&as, ai_jin(1));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 1);
    }

    SECTION("bad_negative_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_ldb(FALSE));
        aasm_emit(&as, ai_jin(-5));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad jump"));
    }

    SECTION("bad_positive_index")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_ldb(FALSE));
        aasm_emit(&as, ai_jin(2));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad jump"));
    }

    SECTION("bad_condition")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_lsi(1));
        aasm_emit(&as, ai_jin(2));
        aasm_emit(&as, ai_lsi(2));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("condition must be boolean or nil"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_mkc_ivk")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    SECTION("normal_fefe")
    {
        aasm_module_push(&as, "test_f");
        aint_t npt = aasm_push(&as);
        {
            aasm_emit(&as, ai_llv(-1));
            aasm_emit(&as, ai_jin(2));
            aasm_emit(&as, ai_lsi(0xFEFE));
            aasm_emit(&as, ai_ret());
            aasm_emit(&as, ai_lsi(0xFEFA));
            aasm_emit(&as, ai_ret());
            aasm_pop(&as);
        }
        aasm_emit(&as, ai_cls(npt));
        aasm_emit(&as, ai_ldb(TRUE));
        aasm_emit(&as, ai_ivk(1));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xFEFE);
    }

    SECTION("normal_fefa")
    {
        aasm_module_push(&as, "test_f");
        aint_t npt = aasm_push(&as);
        {
            aasm_emit(&as, ai_llv(-1));
            aasm_emit(&as, ai_jin(2));
            aasm_emit(&as, ai_lsi(0xFEFE));
            aasm_emit(&as, ai_ret());
            aasm_emit(&as, ai_lsi(0xFEFA));
            aasm_emit(&as, ai_ret());
            aasm_pop(&as);
        }
        aasm_emit(&as, ai_cls(npt));
        aasm_emit(&as, ai_ldb(FALSE));
        aasm_emit(&as, ai_ivk(1));
        aasm_emit(&as, ai_ret());
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_find(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xFEFA);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("dispatcher_msbox")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");

    ascheduler_t s;
    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    aasm_module_push(&as, "test_f");

    aasm_emit(&as, ai_llv(-1));
    aasm_emit(&as, ai_lsi(1));
    aasm_emit(&as, ai_snd());
    aasm_emit(&as, ai_llv(-1));
    aasm_emit(&as, ai_lsi(2));
    aasm_emit(&as, ai_snd());
    aasm_emit(&as, ai_llv(-1));
    aasm_emit(&as, ai_lsi(3));
    aasm_emit(&as, ai_snd());

    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));
    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));
    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));

    aasm_emit(&as, ai_rwd());

    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));
    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));
    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));

    aasm_emit(&as, ai_rmv());

    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));
    aasm_emit(&as, ai_rmv());

    aasm_emit(&as, ai_lsi(0));
    aasm_emit(&as, ai_rcv(1));

    bool timeout = false;

    SECTION("timeout")
    {
        timeout = true;
        aasm_emit(&as, ai_lsi(0));
        aasm_emit(&as, ai_rcv(1));
        aasm_emit(&as, ai_ret());
        aasm_emit(&as, ai_lsi(5));
        aasm_emit(&as, ai_ret());
    }

    SECTION("normal")
    {
        aasm_emit(&as, ai_ret());
    }

    aasm_save(&as);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_find(a, "mod_test", "test_f");
    any_push_pid(a, ascheduler_pid(&s, a));
    ascheduler_start(&s, a, 1);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE((timeout ? 5 : 2) ==
        any_check_integer(a, any_check_index(a, 0)));

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}