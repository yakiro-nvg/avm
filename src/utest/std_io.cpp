/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/scheduler.h>
#include <any/actor.h>
#include <any/std_io.h>
#include <any/std_string.h>

#include <sstream>

static void out(void* ud, const char* s)
{
    std::stringstream* output = (std::stringstream*)ud;
    *output << s;
}

TEST_CASE("std_io")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    std::stringstream output;

    astd_lib_add_io(&s.loader, &out, &output);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_import(a, "std-io", "print/*");

    SECTION("no_argument")
    {
        ascheduler_start(&s, a, 0);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals(""));
    }

    SECTION("nil")
    {
        any_push_nil(a);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("nil"));
    }

    SECTION("pid")
    {
        char pid_buf[64];
        apid_t pid = ascheduler_pid(&s, a);
        snprintf(pid_buf, sizeof(pid_buf), "<%d.%d>",
            apid_idx(s.idx_bits, pid), apid_gen(s.idx_bits, s.gen_bits, pid));
        any_push_pid(a, pid);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals(pid_buf));
    }

    SECTION("boolean")
    {
        any_push_bool(a, TRUE);
        any_push_bool(a, FALSE);
        ascheduler_start(&s, a, 2);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("falsetrue"));
    }

    SECTION("integer")
    {
        any_push_integer(a, 1024768);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("1024768"));
    }

    SECTION("real")
    {
        any_push_real(a, (areal_t)3.14);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        REQUIRE(atof(output.str().c_str()) == Approx(3.14));
    }

    SECTION("native_function")
    {
        any_push_native_func(a, (anative_func_t)NULL);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<native function>"));
    }

    SECTION("byte_code_function")
    {
        avalue_t v;
        v.tag.type = AVT_BYTE_CODE_FUNC;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<function>"));
    }

    SECTION("fixed_buffer")
    {
        avalue_t v;
        v.tag.type = AVT_FIXED_BUFFER;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<fixed buffer>"));
    }

    SECTION("buffer")
    {
        avalue_t v;
        v.tag.type = AVT_BUFFER;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<buffer>"));
    }

    SECTION("string")
    {
        any_push_string(a, "that's string");
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("that's string"));
    }

    SECTION("tuple")
    {
        avalue_t v;
        v.tag.type = AVT_TUPLE;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<tuple>"));
    }

    SECTION("array")
    {
        avalue_t v;
        v.tag.type = AVT_ARRAY;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<array>"));
    }

    SECTION("table")
    {
        avalue_t v;
        v.tag.type = AVT_TABLE;
        aactor_push(a, &v);
        ascheduler_start(&s, a, 1);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("<table>"));
    }

    SECTION("greeting")
    {
        any_push_integer(a, 123);
        any_push_string(a, " KITTY ");
        any_push_string(a, "hello");
        ascheduler_start(&s, a, 3);
        ascheduler_run_once(&s);

        REQUIRE(any_count(a) == 2);
        REQUIRE(any_type(a, any_check_index(a, 1)).type == AVT_NIL);
        REQUIRE(any_type(a, any_check_index(a, 0)).type == AVT_NIL);
        CHECK_THAT(output.str(), Catch::Equals("hello KITTY 123"));
    }

    ascheduler_cleanup(&s);
}