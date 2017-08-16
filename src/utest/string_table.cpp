/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#ifdef ANY_TOOL

#include <catch.hpp>

#include <string.h>
#include <stdio.h>
#include <any/string_table.h>
#include <any/gc_string.h>

#define REQUIRE_STR_EQUALS(a, b) REQUIRE(strcmp(a, b) == 0)

static astring_table_t* grow(astring_table_t* st)
{
    st = (astring_table_t*)realloc(st, st->allocated_bytes * 2);
    astring_table_grow(st, st->allocated_bytes * 2);
    return st;
}

TEST_CASE("string_table_basic")
{
    ahash_and_length_t hl = ahash_and_length("niklas frykholm");
    REQUIRE(hl.length == 15);

    char buffer[1024];
    astring_table_t* const st = (astring_table_t*)buffer;
    astring_table_init(st, 1024, 10);

    REQUIRE(astring_table_to_ref(st, "") == 0);
    REQUIRE_STR_EQUALS("", astring_table_to_string(st, 0));

    aint_t niklas = astring_table_to_ref(st, "niklas");
    aint_t frykholm = astring_table_to_ref(st, "frykholm");

    REQUIRE(niklas == astring_table_to_ref(st, "niklas"));
    REQUIRE(frykholm == astring_table_to_ref(st, "frykholm"));
    REQUIRE(niklas != frykholm);

    REQUIRE(niklas == astring_table_to_ref_const(st, "niklas"));
    REQUIRE(AERR_FULL == astring_table_to_ref_const(st, "lax"));

    REQUIRE_STR_EQUALS("niklas", astring_table_to_string(st, niklas));
    REQUIRE_STR_EQUALS("frykholm", astring_table_to_string(st, frykholm));
    REQUIRE(ahash_and_length("niklas").hash == astring_table_to_hash(st, niklas));
    REQUIRE(ahash_and_length("frykholm").hash == astring_table_to_hash(st, frykholm));
}

TEST_CASE("string_table_grow")
{
    astring_table_t* st = (astring_table_t*)realloc(
        NULL, ANY_ST_MIN_SIZE);
    astring_table_init(st, ANY_ST_MIN_SIZE, 4);

    REQUIRE(astring_table_to_ref(st, "01234567890123456789") == AERR_FULL);

    for (aint_t i = 0; i < 10000; ++i) {
        char string[10];
        snprintf(string, sizeof(string), "%i", i);
        aint_t ref = astring_table_to_ref(st, string);
        while (ref == AERR_FULL) {
            st = grow(st);
            ref = astring_table_to_ref(st, string);
        }
        REQUIRE_STR_EQUALS(string, astring_table_to_string(st, ref));
        REQUIRE(ahash_and_length(string).hash == astring_table_to_hash(st, ref));
    }

    astring_table_pack(st);
    st = (astring_table_t*)realloc(st, st->allocated_bytes);

    for (aint_t i = 0; i < 10000; ++i) {
        char string[10];
        snprintf(string, sizeof(string), "%i", i);
        aint_t ref = astring_table_to_ref(st, string);
        REQUIRE(ref > 0);
        REQUIRE_STR_EQUALS(string, astring_table_to_string(st, ref));
        REQUIRE(ahash_and_length(string).hash == astring_table_to_hash(st, ref));
    }

    free(st);
}

#endif // ANY_TOOL