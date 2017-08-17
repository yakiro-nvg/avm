/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/actor.h>
#include <any/scheduler.h>
#include <any/gc_string.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

static bool done;

static void producer_actor(aactor_t* a)
{
    for (aint_t i = 0; true; i += 2) {
        any_push_idx(a, -1);
        any_push_integer(a, i);
        any_mbox_send(a);
        any_push_idx(a, -1);
        any_push_integer(a, i + 1);
        any_mbox_send(a);
        any_yield(a);
    }
}

static void consumer_actor(aactor_t* a)
{
    any_push_nil(a);
    for (aint_t i = 0; i < 100; ++i) {
        REQUIRE(AERR_NONE == any_mbox_recv(a, AINFINITE));
        any_mbox_remove(a);
        REQUIRE(any_count(a) == 1);
        REQUIRE(any_type(a, 0).type == AVT_INTEGER);
        REQUIRE(any_to_integer(a, 0) == i);
    }
    done = true;
}

static void timeout_actor(aactor_t* a)
{
    any_push_nil(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, amsec(15)));
    any_push_string(a, "timed out");
    done = true;
}

static void recv_to_empty_stack_actor(aactor_t* a)
{
    any_push_pid(a, ascheduler_pid(a));
    any_push_integer(a, 1);
    any_mbox_send(a);
    any_mbox_recv(a, ADONT_WAIT);
}

static void remove_actor(aactor_t* a)
{
    for (aint_t i = 0; i < 5; ++i) {
        any_push_pid(a, ascheduler_pid(a));
        any_push_integer(a, i);
        any_mbox_send(a);
    }

    any_push_nil(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 3);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 4);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 3);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 4);
    any_mbox_remove(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 3);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 0);
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 3);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 2);
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 3);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, 0) == 1);
    any_mbox_remove(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));

    REQUIRE(any_count(a) == 1);
    any_push_string(a, "removed");
}

static void bad_remove_actor(aactor_t* a)
{
    any_push_nil(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_remove(a);
}

static void string_producer_actor(aactor_t* a)
{
    char buff[32];
    for (aint_t i = 0; true; i += 2) {
        any_push_idx(a, -1);
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        any_push_string(a, buff);
        any_mbox_send(a);
        any_push_idx(a, -1);
        snprintf(buff, sizeof(buff), "string %d", (int)(i + 1));
        any_push_string(a, buff);
        any_mbox_send(a);
        any_yield(a);
    }
}

static void string_consumer_actor(aactor_t* a)
{
    char buff[32];
    any_push_nil(a);
    for (aint_t i = 0; i < 100; ++i) {
        REQUIRE(AERR_NONE == any_mbox_recv(a, AINFINITE));
        any_mbox_remove(a);
        REQUIRE(any_count(a) == 1);
        REQUIRE(any_type(a, 0).type == AVT_STRING);
        snprintf(buff, sizeof(buff), "string %d", (int)i);
        CHECK_THAT(any_to_string(a, 0), Catch::Equals(buff));
    }
    done = true;
}

TEST_CASE("msbox_normal")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    any_push_native_func(ca, &consumer_actor);
    ascheduler_start(&s, ca, 0);

    aactor_t* pa;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &pa));
    any_push_native_func(pa, &producer_actor);
    any_push_pid(pa, ascheduler_pid(ca));
    ascheduler_start(&s, pa, 1);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_not_a_pid")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &producer_actor);
    any_push_nil(a);
    ascheduler_start(&s, a, 1);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 1);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("target must be a pid"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_recv_to_empty_stack")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &recv_to_empty_stack_actor);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 1);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("receive to empty stack"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_timeout")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &timeout_actor);
    ascheduler_start(&s, a, 0);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, 1).type == AVT_NIL);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("timed out"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_remove")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &remove_actor);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 2);
    REQUIRE(any_type(a, 1).type == AVT_NIL);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("removed"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_bad_remove")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &bad_remove_actor);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(any_count(a) == 1);
    REQUIRE(any_type(a, 0).type == AVT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("no message to remove"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_string")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    any_push_native_func(ca, &string_consumer_actor);
    ascheduler_start(&s, ca, 0);

    aactor_t* pa;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &pa));
    any_push_native_func(pa, &string_producer_actor);
    any_push_pid(pa, ascheduler_pid(ca));
    ascheduler_start(&s, pa, 1);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}
