/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/std_array.h>
#include <any/std_string.h>

static void push_array(aactor_t* a)
{
    char buff[64];

    any_push_array(a, 8);

    aint_t a_idx = any_check_index(a, 0);

    for (aint_t i = 0; i < 100; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            any_import(a, "std-array", "set/3");
            any_push_nil(a);
            any_push_integer(a, i);
            any_push_index(a, a_idx);
            any_call(a, 3);
            any_pop(a, 1);
        }
    }

    for (aint_t i = 100; i < 500; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 100; i < 500; ++i) {
        if (i % 2 == 0) {
            any_import(a, "std-array", "set/3");
            any_push_nil(a);
            any_push_integer(a, i);
            any_push_index(a, a_idx);
            any_call(a, 3);
            any_pop(a, 1);
        }
    }

    for (aint_t i = 500; i < 1000; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 500; i < 1000; ++i) {
        if (i % 2 == 0) {
            any_import(a, "std-array", "set/3");
            any_push_nil(a);
            any_push_integer(a, i);
            any_push_index(a, a_idx);
            any_call(a, 3);
            any_pop(a, 1);
        }
    }
}

static void test_array(aactor_t* a)
{
    char buff[64];

    aint_t arr_idx = any_check_index(a, -1);

    for (aint_t i = 0; i < 1000; ++i) {
        if (i % 2 == 0) continue;
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_import(a, "std-array", "get/2");
        any_push_integer(a, i);
        any_push_index(a, arr_idx);
        any_call(a, 2);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals(buff));
        any_pop(a, 1);
    }

    any_push_nil(a);
}

static void nested_test(aactor_t* a)
{
    any_push_array(a, 8);

    aint_t a_idx = any_check_index(a, 0);

    for (aint_t i = 0; i < 10; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        any_push_native_func(a, &push_array);
        any_call(a, 0);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 0; i < 10; ++i) {
        if (i % 2 == 0) continue;
        any_import(a, "std-array", "set/3");
        any_push_nil(a);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 10; i < 50; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        any_push_native_func(a, &push_array);
        any_call(a, 0);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 10; i < 50; ++i) {
        if (i % 2 == 0) continue;
        any_import(a, "std-array", "set/3");
        any_push_nil(a);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 50; i < 100; ++i) {
        aint_t sz = any_array_size(a, a_idx);
        REQUIRE(sz == i);
        any_array_resize(a, a_idx, sz + 1);
        any_import(a, "std-array", "set/3");
        any_push_native_func(a, &push_array);
        any_call(a, 0);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 50; i < 100; ++i) {
        if (i % 2 == 0) continue;
        any_import(a, "std-array", "set/3");
        any_push_nil(a);
        any_push_integer(a, i);
        any_push_index(a, a_idx);
        any_call(a, 3);
        any_pop(a, 1);
    }

    for (aint_t i = 0; i < 100; ++i) {
        if (i % 2 != 0) {
            any_import(a, "std-array", "get/2");
            any_push_integer(a, i);
            any_push_index(a, a_idx);
            any_call(a, 2);
            REQUIRE(AVT_NIL == any_type(a, any_top(a)).type);
            any_pop(a, 1);
        } else {
            any_push_native_func(a, &test_array);
            any_import(a, "std-array", "get/2");
            any_push_integer(a, i);
            any_push_index(a, a_idx);
            any_call(a, 2);
            any_call(a, 1);
            any_pop(a, 1);
        }
    }

    any_push_integer(a, 0xFFEE);
}

static void array_binding_test(aactor_t* a)
{
    any_import(a, "std-array", "new/1");
    any_push_integer(a, 8);
    any_call(a, 1);

    aint_t a_idx = any_check_index(a, 0);

    any_import(a, "std-array", "size/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) == 0);
    any_pop(a, 1);

    any_import(a, "std-array", "capacity/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) == 8);
    any_pop(a, 1);

    any_import(a, "std-array", "reserve/2");
    any_push_integer(a, 128);
    any_push_index(a, a_idx);
    any_call(a, 2);
    REQUIRE(any_type(a, a_idx + 1).type == AVT_NIL);
    any_pop(a, 1);

    any_import(a, "std-array", "size/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) == 0);
    any_pop(a, 1);

    any_import(a, "std-array", "capacity/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) >= 128);
    any_pop(a, 1);

    any_import(a, "std-array", "resize/2");
    any_push_integer(a, 64);
    any_push_index(a, a_idx);
    any_call(a, 2);
    REQUIRE(any_type(a, a_idx + 1).type == AVT_NIL);
    any_pop(a, 1);

    any_import(a, "std-array", "size/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) == 64);
    any_pop(a, 1);

    any_import(a, "std-array", "capacity/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) >= 128);
    any_pop(a, 1);

    any_import(a, "std-array", "resize/2");
    any_push_integer(a, 256);
    any_push_index(a, a_idx);
    any_call(a, 2);
    REQUIRE(any_type(a, a_idx + 1).type == AVT_NIL);
    any_pop(a, 1);

    any_import(a, "std-array", "size/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) == 256);
    any_pop(a, 1);

    any_import(a, "std-array", "capacity/1");
    any_push_index(a, a_idx);
    any_call(a, 1);
    REQUIRE(any_check_integer(a, a_idx + 1) >= 256);
    any_pop(a, 1);

    any_push_integer(a, 0xFFFB);
}

TEST_CASE("std_array")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    astd_lib_add_array(&s.loader);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &nested_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xFFEE);

    ascheduler_cleanup(&s);
}

TEST_CASE("std_array_binding")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    astd_lib_add_array(&s.loader);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &array_binding_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 0xFFFB);

    ascheduler_cleanup(&s);
}

TEST_CASE("std_array_binding_new")
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

    astd_lib_add_array(&s.loader);

    SECTION("bad_cap")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");

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
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");

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
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_ARRAY);
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_array_binding_reserve")
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

    astd_lib_add_array(&s.loader);

    SECTION("not_array")
    {
        aasm_module_push(&as, "test_f");
        aint_t lreserve = aasm_add_import(&as, "std-array", "reserve/2");

        aasm_emit(&as, ai_imp(lreserve), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_nil(), 3);
        aasm_emit(&as, ai_ivk(2), 4);

        aasm_emit(&as, ai_ret(), 5);
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
            Catch::Equals("not array"));
    }

    SECTION("cap_not_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lreserve = aasm_add_import(&as, "std-array", "reserve/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lreserve), 4);
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

    SECTION("negative_cap")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lreserve = aasm_add_import(&as, "std-array", "reserve/2");
        aint_t lcapacity = aasm_add_import(&as, "std-array", "capacity/1");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lreserve), 4);
        aasm_emit(&as, ai_lsi(-1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lcapacity), 9);
        aasm_emit(&as, ai_llv(0), 10);
        aasm_emit(&as, ai_ivk(1), 11);

        aasm_emit(&as, ai_ret(), 12);
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

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_array_binding_shrink")
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

    astd_lib_add_array(&s.loader);

    SECTION("not_array")
    {
        aasm_module_push(&as, "test_f");
        aint_t lshrink = aasm_add_import(&as, "std-array", "shrink_to_fit/1");

        aasm_emit(&as, ai_imp(lshrink), 1);
        aasm_emit(&as, ai_nil(), 2);
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
            Catch::Equals("not array"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_array_binding_resize")
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

    astd_lib_add_array(&s.loader);

    SECTION("not_array")
    {
        aasm_module_push(&as, "test_f");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lresize), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_nil(), 3);
        aasm_emit(&as, ai_ivk(2), 4);

        aasm_emit(&as, ai_ret(), 5);
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
            Catch::Equals("not array"));
    }

    SECTION("sz_not_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
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

    SECTION("negative_sz")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(-1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

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

        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad size -1"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_array_binding_get")
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

    astd_lib_add_array(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lget = aasm_add_import(&as, "std-array", "get/2");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lget), 9);
        aasm_emit(&as, ai_lsi(0), 10);
        aasm_emit(&as, ai_llv(0), 11);
        aasm_emit(&as, ai_ivk(2), 12);

        aasm_emit(&as, ai_ret(), 13);
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
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lget = aasm_add_import(&as, "std-array", "get/2");

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

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 0"));
    }

    SECTION("bad_index_underflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lget = aasm_add_import(&as, "std-array", "get/2");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lget), 9);
        aasm_emit(&as, ai_lsi(-1), 10);
        aasm_emit(&as, ai_llv(0), 11);
        aasm_emit(&as, ai_ivk(2), 12);

        aasm_emit(&as, ai_ret(), 13);
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
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lget = aasm_add_import(&as, "std-array", "get/2");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lget), 9);
        aasm_emit(&as, ai_lsi(1), 10);
        aasm_emit(&as, ai_llv(0), 11);
        aasm_emit(&as, ai_ivk(2), 12);

        aasm_emit(&as, ai_ret(), 13);
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
            Catch::Equals("bad index 1"));
    }

    SECTION("bad_index_not_a_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lget = aasm_add_import(&as, "std-array", "get/2");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lget), 9);
        aasm_emit(&as, ai_nil(), 10);
        aasm_emit(&as, ai_llv(0), 11);
        aasm_emit(&as, ai_ivk(2), 12);

        aasm_emit(&as, ai_ret(), 13);
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

TEST_CASE("std_array_binding_set")
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

    astd_lib_add_array(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lset = aasm_add_import(&as, "std-array", "set/3");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lset), 9);
        aasm_emit(&as, ai_lsi(6), 10);
        aasm_emit(&as, ai_lsi(0), 11);
        aasm_emit(&as, ai_llv(0), 12);
        aasm_emit(&as, ai_ivk(3), 13);

        aasm_emit(&as, ai_ret(), 14);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 6);
    }

    SECTION("bad_no_elements")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lset = aasm_add_import(&as, "std-array", "set/3");

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

        REQUIRE(any_count(a) == 1);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("bad index 0"));
    }

    SECTION("bad_index_underflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lset = aasm_add_import(&as, "std-array", "set/3");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lset), 9);
        aasm_emit(&as, ai_lsi(0), 10);
        aasm_emit(&as, ai_lsi(-1), 11);
        aasm_emit(&as, ai_llv(0), 12);
        aasm_emit(&as, ai_ivk(3), 13);

        aasm_emit(&as, ai_ret(), 14);
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
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lset = aasm_add_import(&as, "std-array", "set/3");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lset), 9);
        aasm_emit(&as, ai_lsi(0), 10);
        aasm_emit(&as, ai_lsi(1), 11);
        aasm_emit(&as, ai_llv(0), 12);
        aasm_emit(&as, ai_ivk(3), 13);

        aasm_emit(&as, ai_ret(), 14);
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
            Catch::Equals("bad index 1"));
    }

    SECTION("bad_index_not_a_integer")
    {
        aasm_module_push(&as, "test_f");
        aint_t lnew = aasm_add_import(&as, "std-array", "new/1");
        aint_t lset = aasm_add_import(&as, "std-array", "set/3");
        aint_t lresize = aasm_add_import(&as, "std-array", "resize/2");

        aasm_emit(&as, ai_imp(lnew), 1);
        aasm_emit(&as, ai_lsi(8), 2);
        aasm_emit(&as, ai_ivk(1), 3);

        aasm_emit(&as, ai_imp(lresize), 4);
        aasm_emit(&as, ai_lsi(1), 5);
        aasm_emit(&as, ai_llv(0), 6);
        aasm_emit(&as, ai_ivk(2), 7);
        aasm_emit(&as, ai_pop(1), 8);

        aasm_emit(&as, ai_imp(lset), 9);
        aasm_emit(&as, ai_lsi(0), 10);
        aasm_emit(&as, ai_nil(), 11);
        aasm_emit(&as, ai_llv(0), 12);
        aasm_emit(&as, ai_ivk(3), 13);

        aasm_emit(&as, ai_ret(), 14);
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