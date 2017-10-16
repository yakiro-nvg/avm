/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/loader.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_string.h>

TEST_CASE("logical_op_not")
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

    SECTION("nil")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nil(), 1);
        aasm_emit(&as, ai_not(), 2);
        aasm_emit(&as, ai_ret(), 3);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("false")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(FALSE), 1);
        aasm_emit(&as, ai_not(), 2);
        aasm_emit(&as, ai_ret(), 3);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(TRUE), 1);
        aasm_emit(&as, ai_not(), 2);
        aasm_emit(&as, ai_ret(), 3);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0), 1);
        aasm_emit(&as, ai_not(), 2);
        aasm_emit(&as, ai_ret(), 3);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("logical_op_eq")
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

    SECTION("true==true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(TRUE), 1);
        aasm_emit(&as, ai_ldb(TRUE), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("false==false")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(FALSE), 1);
        aasm_emit(&as, ai_ldb(FALSE), 1);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("false==true")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldb(TRUE), 1);
        aasm_emit(&as, ai_ldb(FALSE), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("4==3")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(3), 1);
        aasm_emit(&as, ai_lsi(4), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3==3")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(3), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3.13==3.14")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.13))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3.14==3.14")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3.0==3")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(3), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.0))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("'that'=='this'")
    {
        aasm_module_push(&as, "test_f");
        aint_t this_str = aasm_string_to_ref(&as, "this");
        aint_t that_str = aasm_string_to_ref(&as, "that");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(this_str))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(that_str))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("'this'=='this'")
    {
        aasm_module_push(&as, "test_f");
        aint_t this_str = aasm_string_to_ref(&as, "this");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(this_str))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(this_str))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("'314'==314")
    {
        aasm_module_push(&as, "test_f");
        aint_t str = aasm_string_to_ref(&as, "314");
        aasm_emit(&as, ai_lsi(314), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(str))), 2);
        aasm_emit(&as, ai_eq(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("logical_op_lt")
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

    SECTION("1<2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(1), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2<2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3<2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("1.0<2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2.0<2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3.0<2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.0))), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("1.0<2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2<2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2<'2'")
    {
        aasm_module_push(&as, "test_f");
        aint_t str = aasm_string_to_ref(&as, "2");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(str))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_lt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not number"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("logical_op_le")
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

    SECTION("1<=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(1), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2<=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3<=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("1.0<=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2.0<=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3.0<=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.0))), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("1.0<=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2<=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3<=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2<='2'")
    {
        aasm_module_push(&as, "test_f");
        aint_t str = aasm_string_to_ref(&as, "2");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(str))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_le(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not number"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("logical_op_gt")
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

    SECTION("1>2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(1), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2>2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3>2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("1.0>2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2.0>2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3.0>2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.0))), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2.0>2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("3>2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2>'2'")
    {
        aasm_module_push(&as, "test_f");
        aint_t str = aasm_string_to_ref(&as, "2");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(str))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_gt(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not number"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("logical_op_ge")
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

    SECTION("1>=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(1), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2>=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3>=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("1.0>=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2.0>=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3.0>=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.0))), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("1.0>=2")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(2), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(1.0))), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == FALSE);
    }

    SECTION("2>=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("3>=2.0")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(2.0))), 1);
        aasm_emit(&as, ai_lsi(3), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_bool(a, any_check_index(a, 0)) == TRUE);
    }

    SECTION("2>='2'")
    {
        aasm_module_push(&as, "test_f");
        aint_t str = aasm_string_to_ref(&as, "2");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_string(str))), 1);
        aasm_emit(&as, ai_lsi(2), 2);
        aasm_emit(&as, ai_ge(), 3);
        aasm_emit(&as, ai_ret(), 4);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not number"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}