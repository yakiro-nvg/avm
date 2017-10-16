/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/loader.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_string.h>

TEST_CASE("math_op_add")
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

    SECTION("integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(1991), 1);
        aasm_emit(&as, ai_lsi(9119), 2);
        aasm_emit(&as, ai_add(), 3);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 9119 + 1991);
    }

    SECTION("real")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_add(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 + 3.14));
    }

    SECTION("mixed")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(314), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_add(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 + 314));
    }

    SECTION("not_number")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nil(), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_add(), 3);
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

TEST_CASE("math_op_sub")
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

    SECTION("integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(1991), 1);
        aasm_emit(&as, ai_lsi(9119), 2);
        aasm_emit(&as, ai_sub(), 3);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 9119 - 1991);
    }

    SECTION("real")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_sub(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 - 3.14));
    }

    SECTION("mixed")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(314), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_sub(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 - 314));
    }

    SECTION("not_number")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nil(), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_sub(), 3);
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

TEST_CASE("math_op_mul")
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

    SECTION("integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(1991), 1);
        aasm_emit(&as, ai_lsi(9119), 2);
        aasm_emit(&as, ai_mul(), 3);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 9119 * 1991);
    }

    SECTION("real")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_mul(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 * 3.14));
    }

    SECTION("mixed")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(314), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_mul(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 * 314));
    }

    SECTION("not_number")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nil(), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_mul(), 3);
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

TEST_CASE("math_op_div")
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

    SECTION("integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(1991), 1);
        aasm_emit(&as, ai_lsi(9119), 2);
        aasm_emit(&as, ai_div(), 3);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 9119 / 1991);
    }

    SECTION("real")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(3.14))), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_div(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 / 3.14));
    }

    SECTION("mixed")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(314), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_div(), 3);
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
        REQUIRE(any_check_real(
            a, any_check_index(a, 0)) == Approx(4.13 / 314));
    }

    SECTION("divide_by_zero_integer")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_lsi(0), 1);
        aasm_emit(&as, ai_lsi(314), 2);
        aasm_emit(&as, ai_div(), 3);
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
            Catch::Equals("divide by zero"));
    }

    SECTION("divide_by_zero_real")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(0))), 1);
        aasm_emit(&as, ai_lsi(314), 2);
        aasm_emit(&as, ai_div(), 3);
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
            Catch::Equals("divide by zero"));
    }

    SECTION("not_number")
    {
        aasm_module_push(&as, "test_f");
        aasm_emit(&as, ai_nil(), 1);
        aasm_emit(&as, ai_ldk(aasm_add_constant(&as, ac_real(4.13))), 2);
        aasm_emit(&as, ai_div(), 3);
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