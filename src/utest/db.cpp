/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#define ADB_API static
#define WBY_STATIC
extern "C" {
#include <db.c>
}

#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>

static void wby_init(
    struct wby_server*, const struct wby_config* c, wby_size* needed_memory)
{
    *needed_memory = 128;
    CHECK_THAT(c->address, Catch::Equals("0.0.0.0"));
    REQUIRE(c->port == 1969);
}

static int wby_start(struct wby_server*, void*)
{
    // nop
    return 0;
}

static void wby_stop(struct wby_server*)
{
    // nop
}

static void wby_update(struct wby_server* s, int)
{
    // TODO
    s->con_count = 0;
}

TEST_CASE("db")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");
    aasm_module_push(&as, "test_f");
    aasm_emit(&as, ai_lsi(1969));
    aasm_emit(&as, ai_lsi(1970));
    aasm_emit(&as, ai_lsi(1971));
    aasm_emit(&as, ai_lsi(1972));
    aasm_emit(&as, ai_ret());
    aasm_pop(&as);
    aasm_save(&as);

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    adb_t db;
    REQUIRE(AERR_NONE ==
        adb_init(&db, &myalloc, NULL, &s, "0.0.0.0", 1969, 1));

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_find(a, "mod_test", "test_f");
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);
    adb_run_once(&db);

    adb_cleanup(&db);
    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}
