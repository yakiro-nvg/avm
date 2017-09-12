/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/scheduler.h>

#include <any/loader.h>
#include <any/actor.h>

void ASTDCALL
actor_entry(
    void* ud);

static AINLINE void*
aalloc(
    ascheduler_t* self, void* old, const aint_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static void
init_processes(
    aprocess_t* procs, aint_t num)
{
    aint_t i;
    for (i = 0; i < num; ++i) {
        procs[i].dead = TRUE;
        procs[i].pid = 0;
    }
}

static void
cleanup(
    ascheduler_t* self, int32_t shutdown)
{
    alist_node_t* i = alist_head(&self->runnings);
    while (i != &self->root.node) {
        alist_node_t* const next = i->next;
        aprocess_task_t* const t = ALIST_NODE_CAST(aprocess_task_t, i);
        aprocess_t* const p = ACAST_FROM_FIELD(aprocess_t, t, ptask);
        if (shutdown || (p->actor.flags & APF_EXIT) != 0) {
            aactor_cleanup(&p->actor);
            alist_node_erase(&t->node);
            ascheduler_free(p->actor.owner, p);
        }
        i = next;
    }
    if (shutdown == FALSE) return;
    i = alist_head(&self->pendings);
    while (!alist_is_end(&self->pendings, i)) {
        alist_node_t* const next = i->next;
        aprocess_task_t* const t = ALIST_NODE_CAST(aprocess_task_t, i);
        aprocess_t* const p = ACAST_FROM_FIELD(aprocess_t, t, ptask);
        aactor_cleanup(&p->actor);
        alist_node_erase(&t->node);
        ascheduler_free(p->actor.owner, p);
        i = next;
    }
    i = alist_head(&self->waitings);
    while (!alist_is_end(&self->waitings, i)) {
        alist_node_t* const next = i->next;
        aprocess_task_t* const t = ALIST_NODE_CAST(aprocess_task_t, i);
        aprocess_t* const p = ACAST_FROM_FIELD(aprocess_t, t, ptask);
        aactor_cleanup(&p->actor);
        alist_node_erase(&t->node);
        ascheduler_free(p->actor.owner, p);
        i = next;
    }
}

static void
wait_for(
    ascheduler_t* self, aactor_t* a, aint_t nsecs, int32_t msg_wake)
{
    aprocess_t* p = ACAST_FROM_FIELD(aprocess_t, a, actor);
    alist_node_t* next_node = p->ptask.node.next;
    aprocess_task_t* next = ALIST_NODE_CAST(aprocess_task_t, next_node);
    alist_node_t* wback = alist_back(&self->waitings);
    assert(p->wait_for == 0);
    alist_node_erase(&p->ptask.node);
    alist_node_insert(&p->ptask.node, wback, wback->next);
    p->wait_for = nsecs;
    p->msg_wake = msg_wake;
    atask_yield(&p->ptask.task, &next->task);
}

static AINLINE void
add_to_runnings(
    ascheduler_t* self, aprocess_t* p)
{
    alist_node_t* n = &p->ptask.node;
    alist_node_t* r = &self->root.node;
    alist_node_erase(n);
    alist_node_insert(n, r->prev, r);
}

static void
check_waitings(
    ascheduler_t* self, aint_t delta)
{
    alist_node_t* i = alist_head(&self->waitings);
    while (!alist_is_end(&self->waitings, i)) {
        alist_node_t* const next = i->next;
        aprocess_task_t* const t = ALIST_NODE_CAST(aprocess_task_t, i);
        aprocess_t* const p = ACAST_FROM_FIELD(aprocess_t, t, ptask);
        if (p->wait_for >= 0) {
            p->wait_for -= delta;
            if (p->wait_for <= 0) {
                p->wait_for = 0;
                p->msg_wake = FALSE;
                add_to_runnings(self, p);
            }
        }
        i = next;
    }
}

static AINLINE void
run_once(
    ascheduler_t* self)
{
    alist_node_t* head = alist_head(&self->runnings);
    if (head != &self->root.node) {
        aprocess_task_t* next = ALIST_NODE_CAST(aprocess_task_t, head);
        atask_yield(&self->root.task, &next->task);
    }
}

aerror_t
ascheduler_init(
    ascheduler_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud)
{
    aerror_t ec;
    memset(self, 0, sizeof(ascheduler_t));
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->idx_bits = idx_bits;
    self->gen_bits = gen_bits;
    self->procs = aalloc(self, NULL,
        ((aint_t)sizeof(aprocess_t)) * (aint_t)(1 << idx_bits));
    self->next_idx = 0;
    aloader_init(&self->loader, alloc, alloc_ud);
    init_processes(self->procs, (aint_t)(1 << idx_bits));
    alist_init(&self->pendings);
    alist_init(&self->runnings);
    alist_init(&self->waitings);
    alist_push_back(&self->runnings, &self->root.node);
    ec = atask_shadow(&self->root.task);
    if (ec != AERR_NONE) goto failed;
    self->first_run = TRUE;
    return ec;
failed:
    if (self->procs) aalloc(self, self->procs, 0);
    return ec;
}

void
ascheduler_run_once(
    ascheduler_t* self)
{
    cleanup(self, FALSE);
    if (self->first_run) {
        self->first_run = FALSE;
        atimer_start(&self->timer);
    } else {
        check_waitings(self, atimer_delta_nsecs(&self->timer));
    }
    run_once(self);
}

void
ascheduler_yield(
    ascheduler_t* self, aactor_t* a)
{
    aprocess_t* p = ACAST_FROM_FIELD(aprocess_t, a, actor);
    alist_node_t* next_node = p->ptask.node.next;
    aprocess_task_t* next = ALIST_NODE_CAST(aprocess_task_t, next_node);
    atask_yield(&p->ptask.task, &next->task);
}

void
ascheduler_sleep(
    ascheduler_t* self, aactor_t* a, aint_t nsecs)
{
    wait_for(self, a, nsecs, FALSE);
}

void
ascheduler_wait(
    ascheduler_t* self, aactor_t* a, aint_t nsecs)
{
    wait_for(self, a, nsecs, TRUE);
}

void
ascheduler_got_new_message(
    ascheduler_t* self, aactor_t* a)
{
    aprocess_t* p = ACAST_FROM_FIELD(aprocess_t, a, actor);
    if (p->msg_wake) {
        p->wait_for = 0;
        p->msg_wake = FALSE;
        add_to_runnings(self, p);
    }
}

void
ascheduler_cleanup(
    ascheduler_t* self)
{
    cleanup(self, TRUE);
    aalloc(self, self->procs, 0);
    aloader_cleanup(&self->loader);
}

aprocess_t*
ascheduler_alloc(
    ascheduler_t* self)
{
    aint_t loop = (aint_t)(1 << self->idx_bits);
    do {
        aprocess_t* p = self->procs + self->next_idx;
        self->next_idx = (self->next_idx + 1) & ((1 << self->idx_bits) - 1);
        if (p->dead) {
            apid_idx_t idx = (apid_idx_t)(p - self->procs);
            apid_gen_t gen = apid_gen(self->idx_bits, self->gen_bits, p->pid);
            gen = (gen + 1) & ((1 << self->gen_bits) - 1);
            p->pid = apid_from(self->idx_bits, self->gen_bits, idx, gen);
            p->dead = FALSE;
            p->wait_for = 0;
            p->msg_wake = FALSE;
            ++self->num_procs;
            return p;
        }
    } while (--loop);
    return NULL;
}

aerror_t
ascheduler_new_actor(
    ascheduler_t* self, aint_t cstack_sz, aactor_t** a)
{
    aerror_t ec;
    aprocess_t* p = ascheduler_alloc(self);
    if (p == NULL) return AERR_FULL;
    *a = &p->actor;
    ec = atask_create(&p->ptask.task, &actor_entry, *a, cstack_sz);
    if (ec != AERR_NONE) return ec;
    ec = aactor_init(*a, self, self->alloc, self->alloc_ud);
    if (ec != AERR_NONE) atask_delete(&p->ptask.task);
    if (self->on_spawn) {
        self->on_spawn(*a, self->on_spawn_ud);
    }
    alist_push_back(&self->pendings, &p->ptask.node);
    return ec;
}

void
ascheduler_start(
    ascheduler_t* self, aactor_t* a, aint_t nargs)
{
    aprocess_t* p = ACAST_FROM_FIELD(aprocess_t, a, actor);
    any_push_integer(a, nargs);
    add_to_runnings(self, p);
}
