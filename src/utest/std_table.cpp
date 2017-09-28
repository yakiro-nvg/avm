/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/std_table.h>
#include <any/std_buffer.h>
#include <any/std_string.h>
#include <any/std_tuple.h>
#include <any/std_array.h>
#include <any/std_table.h>

static void push_table(aactor_t* a, aint_t num_elements)
{
    char buff[64];

    any_push_table(a, 8);

    aint_t t_idx = any_top(a);

    for (aint_t i = 0; i < num_elements; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_import(a, "std-table", "set/3");
        any_push_string(a, buff);
        any_push_string(a, buff);
        any_push_index(a, t_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 0; i < num_elements; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        if (i % 2 == 0) {
            aint_t n = num_elements / 2;
            if (n == 0) {
                any_import(a, "std-table", "set/3");
                any_push_nil(a);
                any_push_string(a, buff);
                any_push_index(a, t_idx);
                any_call(a, 3);
                any_pop(a, 1);
            } else {
                any_import(a, "std-table", "set/3");
                push_table(a, n);
                any_push_string(a, buff);
                any_push_index(a, t_idx);
                any_call(a, 3);
                any_pop(a, 1);
            }
        }
    }
}

static void test_table(aactor_t* a, aint_t num_elements)
{
    char buff[64];

    aint_t t_idx = any_top(a);

    for (aint_t i = 0; i < num_elements; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_import(a, "std-table", "get/2");
        any_push_string(a, buff);
        any_push_index(a, t_idx);
        any_call(a, 2);
        if (i % 2 == 0) {
            aint_t n = num_elements / 2;
            if (n == 0) {
                REQUIRE(any_type(a, any_top(a)).type == AVT_NIL);
            } else {
                test_table(a, n);
            }
        } else {
            CHECK_THAT(any_check_string(a, any_top(a)), Catch::Equals(buff));
        }
        any_pop(a, 1);
    }
}

static void nested_test(aactor_t* a)
{
    enum { NUM_ELEMENTS = 50 };

    push_table(a, NUM_ELEMENTS);
    test_table(a, NUM_ELEMENTS);

    any_push_integer(a, 0xEAEA);
}

TEST_CASE("std_table")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    astd_lib_add_table(&s.loader);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &nested_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xEAEA);

    ascheduler_cleanup(&s);
}

TEST_CASE("std_table_binding")
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

    astd_lib_add_table(&s.loader);

    SECTION("capacity")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lcapacity = aasm_add_import(&as, "std-table", "capacity/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lcapacity), 4);
        aasm_emit(&as, ai_llv(0), 5);
        aasm_emit(&as, ai_ivk(1), 6);

        aasm_emit(&as, ai_ret(), 7);
        aasm_pop(&as);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 8);
    }

    SECTION("size")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lsize = aasm_add_import(&as, "std-table", "size/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lsize), 4);
        aasm_emit(&as, ai_llv(0), 5);
        aasm_emit(&as, ai_ivk(1), 6);

        aasm_emit(&as, ai_ret(), 7);
        aasm_pop(&as);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0);
    }

    SECTION("zero_capacity")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_ret(), 4);
        aasm_pop(&as);
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
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_TABLE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_table_binding_new")
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

    astd_lib_add_table(&s.loader);

    SECTION("bad_cap")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(-1), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_ret(), 4);
        aasm_pop(&as);
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
            Catch::Equals("bad capacity -1"));
    }

    SECTION("zero_capacity")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_ret(), 4);
        aasm_pop(&as);
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
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_TABLE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_table_binding_get_set")
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

    astd_lib_add_table(&s.loader);

    SECTION("pid")
    {
        aint_t arg_idx;

        SECTION("arg_1") { arg_idx = 1; };
        SECTION("arg_2") { arg_idx = 2; };
        SECTION("arg_3") { arg_idx = 3; };
        SECTION("arg_4") { arg_idx = 4; };

        aint_t vals[] = { 0, 0xA1, 0xB2, 0xC3 };

        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lget = aasm_add_import(&as, "std-table", "get/2");
        aint_t lset = aasm_add_import(&as, "std-table", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(vals[1]), 5);
        aasm_emit(&as, ai_llv(-1), 6);
        aasm_emit(&as, ai_llv(0), 7);
        aasm_emit(&as, ai_ivk(3), 8);
        aasm_emit(&as, ai_pop(1), 9);

        aasm_emit(&as, ai_imp(lset), 10);
        aasm_emit(&as, ai_lsi(vals[2]), 11);
        aasm_emit(&as, ai_llv(-2), 12);
        aasm_emit(&as, ai_llv(0), 13);
        aasm_emit(&as, ai_ivk(3), 14);
        aasm_emit(&as, ai_pop(1), 15);

        aasm_emit(&as, ai_imp(lset), 16);
        aasm_emit(&as, ai_lsi(vals[3]), 17);
        aasm_emit(&as, ai_llv(-3), 18);
        aasm_emit(&as, ai_llv(0), 19);
        aasm_emit(&as, ai_ivk(3), 20);
        aasm_emit(&as, ai_pop(1), 21);

        aasm_emit(&as, ai_imp(lget), 22);
        aasm_emit(&as, ai_llv(-arg_idx), 23);
        aasm_emit(&as, ai_llv(0), 24);
        aasm_emit(&as, ai_ivk(2), 25);

        aasm_emit(&as, ai_ret(), 26);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        any_push_pid(a, 4);
        any_push_pid(a, 3);
        any_push_pid(a, 2);
        any_push_pid(a, 1);
        ascheduler_start(&s, a, 4);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        if (arg_idx < 4) {
            REQUIRE(vals[arg_idx] ==
                any_check_integer(a, any_check_index(a, 0)));
        } else {
            REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        }
    }

    SECTION("integer")
    {
        aint_t arg_idx;

        SECTION("arg_1") { arg_idx = 1; };
        SECTION("arg_2") { arg_idx = 2; };
        SECTION("arg_3") { arg_idx = 3; };
        SECTION("arg_4") { arg_idx = 4; };

        aint_t vals[] = { 0, 0xA1, 0xB2, 0xC3 };

        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lget = aasm_add_import(&as, "std-table", "get/2");
        aint_t lset = aasm_add_import(&as, "std-table", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(vals[1]), 5);
        aasm_emit(&as, ai_llv(-1), 6);
        aasm_emit(&as, ai_llv(0), 7);
        aasm_emit(&as, ai_ivk(3), 8);
        aasm_emit(&as, ai_pop(1), 9);

        aasm_emit(&as, ai_imp(lset), 10);
        aasm_emit(&as, ai_lsi(vals[2]), 11);
        aasm_emit(&as, ai_llv(-2), 12);
        aasm_emit(&as, ai_llv(0), 13);
        aasm_emit(&as, ai_ivk(3), 14);
        aasm_emit(&as, ai_pop(1), 15);

        aasm_emit(&as, ai_imp(lset), 16);
        aasm_emit(&as, ai_lsi(vals[3]), 17);
        aasm_emit(&as, ai_llv(-3), 18);
        aasm_emit(&as, ai_llv(0), 19);
        aasm_emit(&as, ai_ivk(3), 20);
        aasm_emit(&as, ai_pop(1), 21);

        aasm_emit(&as, ai_imp(lget), 22);
        aasm_emit(&as, ai_llv(-arg_idx), 23);
        aasm_emit(&as, ai_llv(0), 24);
        aasm_emit(&as, ai_ivk(2), 25);

        aasm_emit(&as, ai_ret(), 26);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        any_push_integer(a, 4);
        any_push_integer(a, 3);
        any_push_integer(a, 2);
        any_push_integer(a, 1);
        ascheduler_start(&s, a, 4);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        if (arg_idx < 4) {
            REQUIRE(vals[arg_idx] ==
                any_check_integer(a, any_check_index(a, 0)));
        } else {
            REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        }
    }

    SECTION("real")
    {
        aint_t arg_idx;

        SECTION("arg_1") { arg_idx = 1; };
        SECTION("arg_2") { arg_idx = 2; };
        SECTION("arg_3") { arg_idx = 3; };
        SECTION("arg_4") { arg_idx = 4; };

        aint_t vals[] = { 0, 0xA1, 0xB2, 0xC3 };

        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lget = aasm_add_import(&as, "std-table", "get/2");
        aint_t lset = aasm_add_import(&as, "std-table", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(vals[1]), 5);
        aasm_emit(&as, ai_llv(-1), 6);
        aasm_emit(&as, ai_llv(0), 7);
        aasm_emit(&as, ai_ivk(3), 8);
        aasm_emit(&as, ai_pop(1), 9);

        aasm_emit(&as, ai_imp(lset), 10);
        aasm_emit(&as, ai_lsi(vals[2]), 11);
        aasm_emit(&as, ai_llv(-2), 12);
        aasm_emit(&as, ai_llv(0), 13);
        aasm_emit(&as, ai_ivk(3), 14);
        aasm_emit(&as, ai_pop(1), 15);

        aasm_emit(&as, ai_imp(lset), 16);
        aasm_emit(&as, ai_lsi(vals[3]), 17);
        aasm_emit(&as, ai_llv(-3), 18);
        aasm_emit(&as, ai_llv(0), 19);
        aasm_emit(&as, ai_ivk(3), 20);
        aasm_emit(&as, ai_pop(1), 21);

        aasm_emit(&as, ai_imp(lget), 22);
        aasm_emit(&as, ai_llv(-arg_idx), 23);
        aasm_emit(&as, ai_llv(0), 24);
        aasm_emit(&as, ai_ivk(2), 25);

        aasm_emit(&as, ai_ret(), 26);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        any_push_real(a, 4.13);
        any_push_real(a, 3.14);
        any_push_real(a, 2.11);
        any_push_real(a, 1.12);
        ascheduler_start(&s, a, 4);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        if (arg_idx < 4) {
            REQUIRE(vals[arg_idx] ==
                any_check_integer(a, any_check_index(a, 0)));
        } else {
            REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        }
    }

    SECTION("string")
    {
        aint_t arg_idx;

        SECTION("arg_1") { arg_idx = 1; };
        SECTION("arg_2") { arg_idx = 2; };
        SECTION("arg_3") { arg_idx = 3; };
        SECTION("arg_4") { arg_idx = 4; };

        aint_t vals[] = { 0, 0xA1, 0xB2, 0xC3 };

        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lget = aasm_add_import(&as, "std-table", "get/2");
        aint_t lset = aasm_add_import(&as, "std-table", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(vals[1]), 5);
        aasm_emit(&as, ai_llv(-1), 6);
        aasm_emit(&as, ai_llv(0), 7);
        aasm_emit(&as, ai_ivk(3), 8);
        aasm_emit(&as, ai_pop(1), 9);

        aasm_emit(&as, ai_imp(lset), 10);
        aasm_emit(&as, ai_lsi(vals[2]), 11);
        aasm_emit(&as, ai_llv(-2), 12);
        aasm_emit(&as, ai_llv(0), 13);
        aasm_emit(&as, ai_ivk(3), 14);
        aasm_emit(&as, ai_pop(1), 15);

        aasm_emit(&as, ai_imp(lset), 16);
        aasm_emit(&as, ai_lsi(vals[3]), 17);
        aasm_emit(&as, ai_llv(-3), 18);
        aasm_emit(&as, ai_llv(0), 19);
        aasm_emit(&as, ai_ivk(3), 20);
        aasm_emit(&as, ai_pop(1), 21);

        aasm_emit(&as, ai_imp(lget), 22);
        aasm_emit(&as, ai_llv(-arg_idx), 23);
        aasm_emit(&as, ai_llv(0), 24);
        aasm_emit(&as, ai_ivk(2), 25);

        aasm_emit(&as, ai_ret(), 26);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        any_push_string(a, "4");
        any_push_string(a, "3");
        any_push_string(a, "2");
        any_push_string(a, "1");
        ascheduler_start(&s, a, 4);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        if (arg_idx < 4) {
            REQUIRE(vals[arg_idx] ==
                any_check_integer(a, any_check_index(a, 0)));
        } else {
            REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        }
    }

    SECTION("set_bad_key")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lset = aasm_add_import(&as, "std-table", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0xFE), 5);
        aasm_emit(&as, ai_llv(-1), 6);
        aasm_emit(&as, ai_llv(0), 7);
        aasm_emit(&as, ai_ivk(3), 8);

        aasm_emit(&as, ai_ret(), 9);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        SECTION("nil") { any_push_nil(a); }
        SECTION("boolean") { any_push_bool(a, FALSE); }
        SECTION("native function") { any_push_native_func(a, NULL); }
        SECTION("byte code function") { any_import(a, "std-table", "new/1"); }
        SECTION("buffer") { any_push_buffer(a, 0); }
        SECTION("tuple") { any_push_tuple(a, 0); }
        SECTION("array") { any_push_array(a, 0); }
        SECTION("table") { any_push_table(a, 0); }
        ascheduler_start(&s, a, 1);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad key type"));
    }

    SECTION("get_bad_key")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-table", "new/1");
        aint_t lget = aasm_add_import(&as, "std-table", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_llv(-1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);

        aasm_emit(&as, ai_ret(), 8);
        aasm_pop(&as);
        aasm_save(&as);

        REQUIRE(AERR_NONE ==
            aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
        REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

        aactor_t* a;
        REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
        any_import(a, "mod_test", "test_f");
        SECTION("nil") { any_push_nil(a); }
        SECTION("boolean") { any_push_bool(a, FALSE); }
        SECTION("native function") { any_push_native_func(a, NULL); }
        SECTION("byte code function") { any_import(a, "std-table", "new/1"); }
        SECTION("buffer") { any_push_buffer(a, 0); }
        SECTION("tuple") { any_push_tuple(a, 0); }
        SECTION("array") { any_push_array(a, 0); }
        SECTION("table") { any_push_table(a, 0); }
        ascheduler_start(&s, a, 1);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad key type"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}