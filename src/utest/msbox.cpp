/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <any/rt_types.h>
#include <any/actor.h>
#include <any/scheduler.h>
#include <any/gc_string.h>

enum { CSTACK_SZ = 16384 };

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}

static bool done;

static void producer_actor(aactor_t* a)
{
    for (int32_t i = 0; true; i += 2) {
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
    for (int32_t i = 0; i < 100; ++i) {
        REQUIRE(AERR_NONE == any_mbox_recv(a, AINFINITE));
        any_mbox_remove(a);
        REQUIRE(any_count(a) == 1);
        REQUIRE(any_type(a, 0).b == ABT_NUMBER);
        REQUIRE(any_type(a, 0).variant == AVTN_INTEGER);
        REQUIRE(any_to_integer(a, 0) == 0);
        any_pop(a, 1);
    }
    done = true;
}

static void timeout_consumer_actor(aactor_t* a)
{
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, 15000000));
    done = true;
}

static void remove_consumer_actor(aactor_t* a)
{
    avalue_t pid;
    pid.tag.b = ABT_PID;
    pid.v.pid = ascheduler_pid(a);

    for (int32_t i = 0; i < 5; ++i) {
        aactor_push(a, &pid);
        any_push_integer(a, i);
        any_mbox_send(a);
    }

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 3);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 4);
    any_pop(a, 5);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 3);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 4);
    any_pop(a, 5);
    any_mbox_remove(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 0);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 3);
    any_pop(a, 4);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 0);
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 2);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 3);
    any_pop(a, 4);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);

    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 2);
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 3);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_remove(a);
    REQUIRE(AERR_NONE == any_mbox_recv(a, ADONT_WAIT));
    REQUIRE(any_to_integer(a, any_count(a) - 1) == 1);
    any_pop(a, 5);
    any_mbox_remove(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));
    any_mbox_rewind(a);
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, ADONT_WAIT));

    REQUIRE(any_count(a) == 0);
    any_push_nil(a);
    done = true;
}

static void bad_remove_consumer_actor(aactor_t* a)
{
    REQUIRE(AERR_TIMEOUT == any_mbox_recv(a, 15000000));
    avalue_t f;
    f.tag.b = ABT_FUNCTION;
    f.tag.variant = AVTF_NATIVE;
    f.v.func = &any_mbox_remove;
    aactor_push(a, &f);
    any_pcall(a, 0);
    REQUIRE(any_count(a) == 1);
    REQUIRE(any_type(a, 0).b == ABT_STRING);
    CHECK_THAT(any_to_string(a, 0), Catch::Equals("no message to remove"));
    done = true;
}

static void string_producer_actor(aactor_t* a)
{
    char buff[32];
    for (int32_t i = 0; true; i += 2) {
        any_push_idx(a, -1);
        snprintf(buff, sizeof(buff), "string %d", i);
        any_push_string(a, buff);
        any_mbox_send(a);
        any_push_idx(a, -1);
        snprintf(buff, sizeof(buff), "string %d", i + 1);
        any_push_string(a, buff);
        any_mbox_send(a);
        any_yield(a);
    }
}

static void string_consumer_actor(aactor_t* a)
{
    char buff[32];
    for (int32_t i = 0; i < 100; ++i) {
        REQUIRE(AERR_NONE == any_mbox_recv(a, AINFINITE));
        any_mbox_remove(a);
        REQUIRE(any_count(a) == 1);
        REQUIRE(any_type(a, 0).b == ABT_STRING);
        snprintf(buff, sizeof(buff), "string %d", i);
        CHECK_THAT(any_to_string(a, 0), Catch::Equals(buff));
        any_pop(a, 1);
    }
    done = true;
}

TEST_CASE("msbox_normal")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t producer;
    producer.tag.b = ABT_FUNCTION;
    producer.tag.variant = AVTF_NATIVE;
    producer.v.func = &producer_actor;

    avalue_t consumer;
    consumer.tag.b = ABT_FUNCTION;
    consumer.tag.variant = AVTF_NATIVE;
    consumer.v.func = &consumer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    aactor_push(ca, &consumer);
    ascheduler_start(&s, ca, 0);

    aactor_t* pa;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &pa));
    aactor_push(pa, &producer);
    avalue_t ca_pid;
    ca_pid.tag.b = ABT_PID;
    ca_pid.v.pid = ascheduler_pid(ca);
    aactor_push(pa, &ca_pid);
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

    avalue_t producer;
    producer.tag.b = ABT_FUNCTION;
    producer.tag.variant = AVTF_NATIVE;
    producer.v.func = &producer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* pa;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &pa));
    aactor_push(pa, &producer);
    avalue_t ca_pid;
    ca_pid.tag.b = ABT_NIL;
    aactor_push(pa, &ca_pid);
    ascheduler_start(&s, pa, 1);

    ascheduler_run_once(&s);

    REQUIRE(any_count(pa) == 1);
    REQUIRE(any_type(pa, 0).b == ABT_STRING);
    CHECK_THAT(any_to_string(pa, 0), Catch::Equals("target must be a pid"));

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_timeout")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t consumer;
    consumer.tag.b = ABT_FUNCTION;
    consumer.tag.variant = AVTF_NATIVE;
    consumer.v.func = &timeout_consumer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    aactor_push(ca, &consumer);
    ascheduler_start(&s, ca, 0);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_remove")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t consumer;
    consumer.tag.b = ABT_FUNCTION;
    consumer.tag.variant = AVTF_NATIVE;
    consumer.v.func = &remove_consumer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    aactor_push(ca, &consumer);
    ascheduler_start(&s, ca, 0);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_bad_remove_consumer")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t consumer;
    consumer.tag.b = ABT_FUNCTION;
    consumer.tag.variant = AVTF_NATIVE;
    consumer.v.func = &timeout_consumer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    aactor_push(ca, &consumer);
    ascheduler_start(&s, ca, 0);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}

TEST_CASE("msbox_string")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    avalue_t producer;
    producer.tag.b = ABT_FUNCTION;
    producer.tag.variant = AVTF_NATIVE;
    producer.v.func = &string_producer_actor;

    avalue_t consumer;
    consumer.tag.b = ABT_FUNCTION;
    consumer.tag.variant = AVTF_NATIVE;
    consumer.v.func = &string_consumer_actor;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aactor_t* ca;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &ca));
    aactor_push(ca, &consumer);
    ascheduler_start(&s, ca, 0);

    aactor_t* pa;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &pa));
    aactor_push(pa, &producer);
    avalue_t ca_pid;
    ca_pid.tag.b = ABT_PID;
    ca_pid.v.pid = ascheduler_pid(ca);
    aactor_push(pa, &ca_pid);
    ascheduler_start(&s, pa, 1);

    done = false;
    while (!done) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}