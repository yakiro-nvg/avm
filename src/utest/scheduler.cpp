/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "prereq.h"

#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>

static void spawn_new(ascheduler_t* s)
{
    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(s, CSTACK_SZ, &a));
    any_push_native_func(a, &any_push_nil);
    ascheduler_start(s, a, 0);
}

static void expected(aactor_t* a, void* ud)
{
    *(aerror_t*)ud = a->error_jmp->status;
}

static void unexpect(aactor_t* a, void* ud)
{
    REQUIRE(false);
}

static void throw_runtime(aactor_t* a, void*)
{
    any_throw(a, AERR_RUNTIME);
}

static void root_throw(aactor_t* a)
{
    throw_runtime(a, NULL);
}

static void safe_throw(aactor_t* a)
{
    any_try(a, &throw_runtime, NULL);
}

static void get_pid(aactor_t* a, void* ud)
{
    *(apid_t*)ud = ascheduler_pid(a->owner, a);
}

struct step_ud_t
{
    bool brk;
    aint_t idx;
};

static int32_t step(aactor_t* a, void* ud)
{
    step_ud_t& sud = *(step_ud_t*)ud;
    if (sud.brk) return FALSE;
    REQUIRE(a->frame->ip == sud.idx++);
    return TRUE;
}

TEST_CASE("scheduler_new_process")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    for (aint_t i = 0; i < 10; ++i) {
        spawn_new(&s);
    }

    ascheduler_run_once(&s);
    ascheduler_run_once(&s); // cleanup is deferred
    REQUIRE(alist_head(&s.runnings) == &s.root.node);

    for (aint_t i = 0; i < 10; ++i) {
        spawn_new(&s);
    }

    ascheduler_cleanup(&s);
}

TEST_CASE("scheduler_on_panic")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aerror_t ec = AERR_NONE;

    ascheduler_on_panic(&s, &expected, &ec);
    ascheduler_on_throw(&s, &unexpect, NULL);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &root_throw);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(ec == AERR_RUNTIME);

    ascheduler_cleanup(&s);
}

TEST_CASE("scheduler_on_throw")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));

    aerror_t ec = AERR_NONE;

    ascheduler_on_panic(&s, &unexpect, NULL);
    ascheduler_on_throw(&s, &expected, &ec);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &safe_throw);
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);

    REQUIRE(ec == AERR_RUNTIME);

    ascheduler_cleanup(&s);
}

TEST_CASE("scheduler_on_spawn_exit")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    apid_t spid = (apid_t)-1;
    apid_t epid = (apid_t)-1;

    ascheduler_on_spawn(&s, &get_pid, &spid);
    ascheduler_on_exit(&s,  &get_pid, &epid);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_push_native_func(a, &any_push_nil);
    ascheduler_start(&s, a, 0);

    apid_t expected_pid = ascheduler_pid(&s, a);

    ascheduler_run_once(&s);

    REQUIRE(spid == expected_pid);

    ascheduler_run_once(&s); // cleanup is deferred

    REQUIRE(epid == expected_pid);

    ascheduler_cleanup(&s);
}

TEST_CASE("scheduler_on_step")
{
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };

    aasm_t as;
    aasm_init(&as, &myalloc, NULL);
    REQUIRE(aasm_load(&as, NULL) == AERR_NONE);
    add_module(&as, "mod_test");
    aasm_module_push(&as, "test_f");
    aasm_emit(&as, ai_lsi(1969), 1);
    aasm_emit(&as, ai_lsi(1970), 2);
    aasm_emit(&as, ai_lsi(1971), 3);
    aasm_emit(&as, ai_lsi(1972), 4);
    aasm_emit(&as, ai_ret(), 5);
    aint_t num_instructions = aasm_prototype(&as)->num_instructions;
    aasm_pop(&as);
    aasm_save(&as);

    ascheduler_t s;

    REQUIRE(AERR_NONE ==
        ascheduler_init(&s, NUM_IDX_BITS, NUM_GEN_BITS, &myalloc, NULL));
    ascheduler_on_panic(&s, &on_panic, NULL);

    REQUIRE(AERR_NONE ==
        aloader_add_chunk(&s.loader, as.chunk, as.chunk_size, NULL, NULL));
    REQUIRE(AERR_NONE == aloader_link(&s.loader, TRUE));

    step_ud_t sud;
    sud.brk = TRUE;
    sud.idx = 0;

    ascheduler_on_step(&s, &step, &sud);

    aactor_t* a;
    REQUIRE(AERR_NONE == ascheduler_new_actor(&s, CSTACK_SZ, &a));
    any_import(a, "mod_test", "test_f");
    ascheduler_start(&s, a, 0);

    ascheduler_run_once(&s);
    ascheduler_run_once(&s);

    REQUIRE(sud.idx == 0);

    sud.brk = FALSE;
    ascheduler_run_once(&s);

    REQUIRE(sud.idx == num_instructions);

    ascheduler_cleanup(&s);
    aasm_cleanup(&as);
}