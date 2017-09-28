/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/loader.h>
#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_string.h>

static void string_test(aactor_t* a)
{
    char buff[64];
    for (aint_t i = 0; i < 1000; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
			av_nil(aactor_at(a, any_check_index(a, i)));
        }
    }
    for (aint_t i = 1000; i < 5000; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 1000; i < 5000; ++i) {
        if (i % 2 == 0) {
            av_nil(aactor_at(a, any_check_index(a, i)));
        }
    }
    for (aint_t i = 5000; i < 10000; ++i) {
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        any_push_string(a, buff);
    }
    for (aint_t i = 5000; i < 10000; ++i) {
        if (i % 2 == 0) {
            av_nil(aactor_at(a, any_check_index(a, i)));
        }
    }
    for (aint_t i = 0; i < 10000; ++i) {
        if (i % 2 == 0) continue;
        snprintf(buff, sizeof(buff), "string %zd", (size_t)i);
        CHECK_THAT(any_check_string(a, any_check_index(a, i)),
            Catch::Equals(buff));
    }
    any_push_string(a, "ok");
}

TEST_CASE("std_string")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &string_test);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
    CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
        Catch::Equals("ok"));

    ascheduler_cleanup(&s);
}

TEST_CASE("std_string_binding_length")
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

    astd_lib_add_string(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t llength = aasm_add_import(&as, "std-string", "length/1");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "test")));

        aasm_emit(&as, ai_imp(llength), 1);
        aasm_emit(&as, ai_ldk(cstring), 2);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == 4);
    }

    SECTION("not_a_string")
    {
        aasm_module_push(&as, "test_f");
        aint_t llength = aasm_add_import(&as, "std-string", "length/1");

        aasm_emit(&as, ai_imp(llength), 1);
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
            Catch::Equals("not string"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_string_binding_hash")
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

    astd_lib_add_string(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lhash = aasm_add_import(&as, "std-string", "hash/1");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "test")));

        aasm_emit(&as, ai_imp(lhash), 1);
        aasm_emit(&as, ai_ldk(cstring), 2);
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
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) ==
            (aint_t)ahash_and_length("test").hash);
    }

    SECTION("not_a_string")
    {
        aasm_module_push(&as, "test_f");
        aint_t lhash = aasm_add_import(&as, "std-string", "hash/1");

        aasm_emit(&as, ai_imp(lhash), 1);
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
            Catch::Equals("not string"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_string_binding_get")
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

    astd_lib_add_string(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lget = aasm_add_import(&as, "std-string", "get/2");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "test")));

        aint_t idx;

        SECTION("0") { idx = 0; }
        SECTION("1") { idx = 1; }
        SECTION("2") { idx = 2; }
        SECTION("3") { idx = 3; }

        aasm_emit(&as, ai_imp(lget), 1);
        aasm_emit(&as, ai_lsi(idx), 2);
        aasm_emit(&as, ai_ldk(cstring), 3);
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

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_check_integer(a, any_check_index(a, 0)) == "test"[idx]);
    }

    SECTION("not_a_string")
    {
        aasm_module_push(&as, "test_f");
        aint_t lget = aasm_add_import(&as, "std-string", "get/2");

        aasm_emit(&as, ai_imp(lget), 1);
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
            Catch::Equals("not string"));
    }

    SECTION("no_elements")
    {
        aasm_module_push(&as, "test_f");
        aint_t lget = aasm_add_import(&as, "std-string", "get/2");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "")));

        aasm_emit(&as, ai_imp(lget), 1);
        aasm_emit(&as, ai_lsi(0), 2);
        aasm_emit(&as, ai_ldk(cstring), 3);
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
            Catch::Equals("bad index 0"));
    }

    SECTION("negative_index")
    {
        aasm_module_push(&as, "test_f");
        aint_t lget = aasm_add_import(&as, "std-string", "get/2");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "test")));

        aasm_emit(&as, ai_imp(lget), 1);
        aasm_emit(&as, ai_lsi(-1), 2);
        aasm_emit(&as, ai_ldk(cstring), 3);
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
            Catch::Equals("bad index -1"));
    }

    SECTION("overflow")
    {
        aasm_module_push(&as, "test_f");
        aint_t lget = aasm_add_import(&as, "std-string", "get/2");
        aint_t cstring = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "test")));

        aasm_emit(&as, ai_imp(lget), 1);
        aasm_emit(&as, ai_lsi(4), 2);
        aasm_emit(&as, ai_ldk(cstring), 3);
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
            Catch::Equals("bad index 4"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}

TEST_CASE("std_string_binding_concat")
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

    astd_lib_add_string(&s.loader);

    SECTION("normal")
    {
        aasm_module_push(&as, "test_f");
        aint_t lconcat = aasm_add_import(&as, "std-string", "concat/2");
        aint_t clhs = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "hello.")));
        aint_t crhs = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "KITTY")));

        aasm_emit(&as, ai_imp(lconcat), 1);
        aasm_emit(&as, ai_ldk(crhs), 2);
        aasm_emit(&as, ai_ldk(clhs), 3);
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

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        CHECK_THAT(any_check_string(a, any_check_index(a, 0)),
            Catch::Equals("hello.KITTY"));
        ahash_and_length_t hal = ahash_and_length("hello.KITTY");
        REQUIRE(any_string_length(a, any_check_index(a, 0)) == hal.length);
        REQUIRE(any_string_hash(a, any_check_index(a, 0)) == (aint_t)hal.hash);
    }

    SECTION("lhs_not_a_string")
    {
        aasm_module_push(&as, "test_f");
        aint_t lconcat = aasm_add_import(&as, "std-string", "concat/2");
        aint_t clhs = aasm_add_constant(&as, ac_integer(0));
        aint_t crhs = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "KITTY")));

        aasm_emit(&as, ai_imp(lconcat), 1);
        aasm_emit(&as, ai_ldk(crhs), 2);
        aasm_emit(&as, ai_ldk(clhs), 3);
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
            Catch::Equals("not string"));
    }

    SECTION("lhs_not_a_string")
    {
        aasm_module_push(&as, "test_f");
        aint_t lconcat = aasm_add_import(&as, "std-string", "concat/2");
        aint_t clhs = aasm_add_constant(&as,
            ac_string(aasm_string_to_ref(&as, "hello.")));
        aint_t crhs = aasm_add_constant(&as, ac_integer(0));

        aasm_emit(&as, ai_imp(lconcat), 1);
        aasm_emit(&as, ai_ldk(crhs), 2);
        aasm_emit(&as, ai_ldk(clhs), 3);
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
            Catch::Equals("not string"));
    }

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}