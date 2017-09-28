/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/std_tuple.h>
#include <any/std_string.h>

static void push_tuple(aactor_t* a, aint_t num_elements)
{
    char buff[64];

    any_push_tuple(a, num_elements);

    aint_t t_idx = any_top(a);
    REQUIRE(any_tuple_size(a, t_idx) == num_elements);

    for (aint_t i = 0; i < num_elements; ++i) {
        any_import(a, "std-tuple", "set/3");
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
        any_push_integer(a, i);
        any_push_index(a, t_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 0; i < num_elements; ++i) {
        if (i % 2 == 0) {
            aint_t n = num_elements / 2;
            if (n == 0) {
                any_import(a, "std-tuple", "set/3");
                any_push_nil(a);
                any_push_integer(a, i);
                any_push_index(a, t_idx);
                any_call(a, 3);
                any_pop(a, 1);
            } else {
                any_import(a, "std-tuple", "set/3");
                push_tuple(a, n);
                any_push_integer(a, i);
                any_push_index(a, t_idx);
                any_call(a, 3);
                any_pop(a, 1);
            }
        }
    }
}

static void test_tuple(aactor_t* a, aint_t num_elements)
{
    char buff[64];

    aint_t t_idx = any_top(a);

    for (aint_t i = 0; i < num_elements; ++i) {
        any_import(a, "std-tuple", "get/2");
        any_push_integer(a, i);
        any_push_index(a, t_idx);
        any_call(a, 2);
        if (i % 2 == 0) {
            aint_t n = num_elements / 2;
            if (n == 0) {
                REQUIRE(any_type(a, any_top(a)).type == AVT_NIL);
            } else {
                test_tuple(a, n);
            }
        } else {
            snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
            CHECK_THAT(any_check_string(a, any_top(a)), Catch::Equals(buff));
        }
        any_pop(a, 1);
    }
}

static void nested_test(aactor_t* a)
{
    enum { NUM_ELEMENTS = 50 };

    push_tuple(a, NUM_ELEMENTS);
    test_tuple(a, NUM_ELEMENTS);

    any_push_integer(a, 0xFAEA);
}

TEST_CASE("std_tuple")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    astd_lib_add_tuple(&s.loader);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &nested_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xFAEA);

    ascheduler_cleanup(&s);
}

TEST_CASE("std_tuple_binding_new")
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

    astd_lib_add_tuple(&s.loader);

    SECTION("bad_sz")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");

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
            Catch::Equals("bad size -1"));
    }

    SECTION("empty")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");

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
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_TUPLE);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_tuple_binding_get")
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

    astd_lib_add_tuple(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lget = aasm_add_import(&as, "std-tuple", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_lsi(0), 5);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
    }

    SECTION("bad_no_elements")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lget = aasm_add_import(&as, "std-tuple", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_lsi(0), 5);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 0"));
    }

    SECTION("bad_index_underflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lget = aasm_add_import(&as, "std-tuple", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_lsi(-1), 5);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index -1"));
    }

    SECTION("bad_index_overflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lget = aasm_add_import(&as, "std-tuple", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_lsi(8), 5);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 8"));
    }

    SECTION("bad_index_not_a_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lget = aasm_add_import(&as, "std-tuple", "get/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lget), 4);
        aasm_emit(&as, ai_nil(), 5);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not integer"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_tuple_binding_set")
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

    astd_lib_add_tuple(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lset = aasm_add_import(&as, "std-tuple", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0), 5);
        aasm_emit(&as, ai_lsi(0), 6);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
    }

    SECTION("bad_no_elements")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lset = aasm_add_import(&as, "std-tuple", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0), 5);
        aasm_emit(&as, ai_lsi(0), 6);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 0"));
    }

    SECTION("bad_index_underflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lset = aasm_add_import(&as, "std-tuple", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0), 5);
        aasm_emit(&as, ai_lsi(-1), 6);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index -1"));
    }

    SECTION("bad_index_overflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lset = aasm_add_import(&as, "std-tuple", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0), 5);
        aasm_emit(&as, ai_lsi(8), 6);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 8"));
    }

    SECTION("bad_index_not_a_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-tuple", "new/1");
        aint_t lset = aasm_add_import(&as, "std-tuple", "set/3");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lset), 4);
        aasm_emit(&as, ai_lsi(0), 5);
        aasm_emit(&as, ai_nil(), 6);
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
        ascheduler_start(&s, a, 0);

        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("not integer"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}