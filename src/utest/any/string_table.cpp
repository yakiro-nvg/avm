/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/string_table.h>
#include <any/errno.h>

#define REQUIRE_STR_EQUALS(a, b) REQUIRE(strcmp(a, b) == 0)

static astring_table_t* grow(astring_table_t* st)
{
	st = (astring_table_t*)realloc(st, st->allocated_bytes * 2);
	any_st_grow(st, st->allocated_bytes * 2);
	return st;
}

TEST_CASE("string_table_basic")
{
	ahash_and_length_t hl = ahash_and_length("niklas frykholm");
	REQUIRE(hl.length == 15);

	char buffer[1024];
	astring_table_t* const st = (astring_table_t*)buffer;
	any_st_init(st, 1024, 10);

	REQUIRE(any_st_to_ref(st, "") == 0);
	REQUIRE_STR_EQUALS("", any_st_to_string(st, 0));

	astring_ref_t niklas = any_st_to_ref(st, "niklas");
	astring_ref_t frykholm = any_st_to_ref(st, "frykholm");

	REQUIRE(niklas == any_st_to_ref(st, "niklas"));
	REQUIRE(frykholm == any_st_to_ref(st, "frykholm"));
	REQUIRE(niklas != frykholm);

	REQUIRE(niklas == any_st_to_ref_const(st, "niklas"));
	REQUIRE(AERR_FULL == any_st_to_ref_const(st, "lax"));

	REQUIRE_STR_EQUALS("niklas", any_st_to_string(st, niklas));
	REQUIRE_STR_EQUALS("frykholm", any_st_to_string(st, frykholm));
	REQUIRE(ahash_and_length("niklas").hash == any_st_to_hash(st, niklas));
	REQUIRE(ahash_and_length("frykholm").hash == any_st_to_hash(st, frykholm));
}

TEST_CASE("string_table_grow")
{
	astring_table_t* st = (astring_table_t*)realloc(
		NULL, ANY_ST_MIN_SIZE);
	any_st_init(st, ANY_ST_MIN_SIZE, 4);

	REQUIRE(any_st_to_ref(st, "01234567890123456789") == AERR_FULL);

	for (int32_t i = 0; i < 10000; ++i) {
		char s[10];
		sprintf(s, "%i", i);
		astring_ref_t ref = any_st_to_ref(st, s);
		while (ref == AERR_FULL) {
			st = grow(st);
			ref = any_st_to_ref(st, s);
		}
		REQUIRE_STR_EQUALS(s, any_st_to_string(st, ref));
		REQUIRE(ahash_and_length(s).hash == any_st_to_hash(st, ref));
	}

	any_st_pack(st);
	st = (astring_table_t*)realloc(st, st->allocated_bytes);

	for (int32_t i = 0; i < 10000; ++i) {
		char s[10];
		sprintf(s, "%i", i);
		astring_ref_t ref = any_st_to_ref(st, s);
		REQUIRE(ref > 0);
		REQUIRE_STR_EQUALS(s, any_st_to_string(st, ref));
		REQUIRE(ahash_and_length(s).hash == any_st_to_hash(st, ref));
	}

	free(st);
}