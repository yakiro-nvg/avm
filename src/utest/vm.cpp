/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <catch.hpp>

#include <stdlib.h>
#include <any/vm.h>
#include <any/scheduler.h>
#include <any/errno.h>

#if 0

enum { MSG_QUEUE_CAP = 16 };

static void* myalloc(void*, void* old, int32_t sz)
{
    return realloc(old, sz);
}


static void init(avm_t* vm, ascheduler_t* string)
{
    ascheduler_mbox_t s1_oqueues[2];
    s1_oqueues[0].msgs = new aenvelope_t[MSG_QUEUE_CAP];
    s1_oqueues[0].cap = MSG_QUEUE_CAP;
    s1_oqueues[0].sz = 0;
    s1_oqueues[1].msgs = new aenvelope_t[MSG_QUEUE_CAP];
    s1_oqueues[1].cap = MSG_QUEUE_CAP;
    s1_oqueues[1].sz = 0;

    ascheduler_mbox_t s1_iqueues[2];
    s1_iqueues[0].msgs = new aenvelope_t[MSG_QUEUE_CAP];
    s1_iqueues[0].cap = MSG_QUEUE_CAP;
    s1_iqueues[0].sz = 0;
    s1_iqueues[1].msgs = new aenvelope_t[MSG_QUEUE_CAP];
    s1_iqueues[1].cap = MSG_QUEUE_CAP;
    s1_iqueues[1].sz = 0;

    REQUIRE(AERR_NONE == ascheduler_init(
        string, vm, NULL, s1_oqueues, s1_iqueues, &myalloc, NULL));
}

static void free(ascheduler_t* string)
{
    delete string->oback.msgs;
    delete string->iback.msgs;
    delete string->ofront.msgs;
    delete string->ifront.msgs;
}

TEST_CASE("vm_proc_allocation")
{
    avm_t vm;
    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };
    aprocess_t* procs = (aprocess_t*)malloc((1 << NUM_IDX_BITS)*sizeof(aprocess_t));
    REQUIRE(avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, procs) == AERR_NONE);

    // empty
    for (int32_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            REQUIRE(!avm_process_lock(
                &vm, apid_from(NUM_IDX_BITS, NUM_GEN_BITS, i, g)));
            REQUIRE(!avm_process_lock_idx(
                &vm, i, NULL));
        }
    }

    // basic
    for (int32_t g = 0; g < 1 << NUM_GEN_BITS; ++g) {
        for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
            auto p = avm_process_alloc(&vm);
            REQUIRE(p != NULL);
            REQUIRE(p == avm_process_lock(&vm, p->pid));
            avm_process_unlock(p);
            REQUIRE(p == avm_process_lock_idx(&vm, i, NULL));
            avm_process_unlock(p);
            avm_process_free(p);
        }
    }

    // over-allocate
    for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        REQUIRE(avm_process_alloc(&vm));
    }
    REQUIRE(avm_process_alloc(&vm) == NULL);
    for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        auto p = avm_process_lock_idx(&vm, i, NULL);
        avm_process_unlock(p);
        avm_process_free(p);
    }
    for (int32_t i = 0; i < 1 << NUM_IDX_BITS; ++i) {
        REQUIRE(avm_process_alloc(&vm));
    }

    avm_shutdown(&vm);
    free(procs);
}

TEST_CASE("vm_migrate_messages")
{
    avm_t vm;
    ascheduler_t s0;
    ascheduler_t s1;
    ascheduler_t s2;
    ascheduler_t* scheds[] = { &s0, &s1, &s2, NULL };

    enum { NUM_IDX_BITS = 4 };
    enum { NUM_GEN_BITS = 4 };
    aprocess_t* procs = (aprocess_t*)malloc((1 << NUM_IDX_BITS) * sizeof(aprocess_t));
    REQUIRE(avm_startup(&vm, NUM_IDX_BITS, NUM_GEN_BITS, procs) == AERR_NONE);
    init(&vm, &s0);
    init(&vm, &s1);
    init(&vm, &s2);

    auto p0 = avm_process_alloc(&vm);
    p0->owner = &s0;
    p0->flags |= APF_BORROWED;
    avalue_t p0msgs[MSG_QUEUE_CAP];
    p0->mbox.msgs = p0msgs;
    p0->mbox.cap = MSG_QUEUE_CAP;
    p0->mbox.sz = 0;
    auto p1 = avm_process_alloc(&vm);
    p1->owner = &s1;
    p1->flags |= APF_BORROWED;
    avalue_t p1msgs[MSG_QUEUE_CAP];
    p1->mbox.msgs = p1msgs;
    p1->mbox.cap = MSG_QUEUE_CAP;
    p1->mbox.sz = 0;
    auto p2 = avm_process_alloc(&vm);
    p2->owner = &s2;
    p2->mbox.msgs = (avalue_t*)myalloc(NULL, NULL, MSG_QUEUE_CAP*sizeof(avalue_t));
    p2->mbox.cap = MSG_QUEUE_CAP;
    p2->mbox.sz = 0;

    // basic outgoing enqueue
    for (int32_t i = 0; i < MSG_QUEUE_CAP; ++i) {
        avalue_t v;
        v.v.i = i;
        REQUIRE(ascheduler_outgoing(&s0, p1->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s1, p2->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s2, p0->pid, &v) >= 0);
    }

    // outgoing queue now full
    {
        REQUIRE(s0.oback.sz == MSG_QUEUE_CAP);
        REQUIRE(s1.oback.sz == MSG_QUEUE_CAP);
        REQUIRE(s2.oback.sz == MSG_QUEUE_CAP);
        avalue_t v;
        REQUIRE(AERR_FULL == ascheduler_outgoing(&s0, p1->pid, &v));
        REQUIRE(AERR_FULL == ascheduler_outgoing(&s1, p2->pid, &v));
        REQUIRE(AERR_FULL == ascheduler_outgoing(&s2, p0->pid, &v));
    }

    // flush and empty incoming queues
    avm_migrate_messages(&vm, scheds);
    REQUIRE(s0.ifront.sz == MSG_QUEUE_CAP);
    REQUIRE(s1.ifront.sz == MSG_QUEUE_CAP);
    REQUIRE(s2.ifront.sz == MSG_QUEUE_CAP);
    ascheduler_empty_incoming(&s0);
    ascheduler_empty_incoming(&s1);
    ascheduler_empty_incoming(&s2);
    for (int32_t i = 0; i < MSG_QUEUE_CAP; ++i) {
        REQUIRE(p0->mbox.msgs[i].v.integer == i);
        REQUIRE(p1->mbox.msgs[i].v.integer == i);
        REQUIRE(p2->mbox.msgs[i].v.integer == i);
    }

    // enqueue more messages
    for (int32_t i = 0; i < MSG_QUEUE_CAP; ++i) {
        avalue_t v;
        v.v.i = i;
        REQUIRE(ascheduler_outgoing(&s0, p1->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s1, p2->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s2, p0->pid, &v) >= 0);
    }
    avm_migrate_messages(&vm, scheds);
    for (int32_t i = 0; i < MSG_QUEUE_CAP; ++i) {
        avalue_t v;
        v.v.integer = i;
        REQUIRE(ascheduler_outgoing(&s0, p1->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s1, p2->pid, &v) >= 0);
        REQUIRE(ascheduler_outgoing(&s2, p0->pid, &v) >= 0);
    }

    // can't flush when incoming queues is full
    avm_migrate_messages(&vm, scheds);
    REQUIRE(s0.ofront.sz == MSG_QUEUE_CAP);
    REQUIRE(s1.ofront.sz == MSG_QUEUE_CAP);
    REQUIRE(s2.ofront.sz == MSG_QUEUE_CAP);

    // consume and flush
    s0.ifront.sz /= 2;
    s1.ifront.sz /= 2;
    s2.ifront.sz /= 2;
    avm_migrate_messages(&vm, scheds);
    REQUIRE(s0.ofront.sz == MSG_QUEUE_CAP / 2);
    REQUIRE(s1.ofront.sz == MSG_QUEUE_CAP / 2);
    REQUIRE(s2.ofront.sz == MSG_QUEUE_CAP / 2);

    // this should be ok
    ascheduler_empty_incoming(&s2);
    REQUIRE(p2->mbox.sz == MSG_QUEUE_CAP * 2);
    {
        for (int32_t i = 0; i < MSG_QUEUE_CAP; ++i) {
            REQUIRE(p2->mbox.msgs[i].v.integer == i);
        }
        for (int32_t i = 0; i < MSG_QUEUE_CAP / 2; ++i) {
            REQUIRE(p2->mbox.msgs[MSG_QUEUE_CAP + i].v.integer == i);
        }
        for (int32_t i = 0; i < MSG_QUEUE_CAP / 2; ++i) {
            REQUIRE(p2->mbox.msgs[
                MSG_QUEUE_CAP + (MSG_QUEUE_CAP / 2) + i].v.integer == i);
        }
    }

    avm_shutdown(&vm);
    myalloc(NULL, p2->mbox.msgs, 0);
    free(&s0);
    free(&s1);
    free(&s2);
    free(procs);
}

#endif