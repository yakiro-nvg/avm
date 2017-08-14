/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/actor.h>

#include <any/loader.h>
#include <any/scheduler.h>
#include <any/gc.h>
#include <any/gc_string.h>

#define GROW_FACTOR 2
#define INIT_STACK_SZ 64
#define INIT_HEAP_SZ 512
#define INIT_MBOX_SZ 32

void actor_dispatch(aactor_t* a);

static AINLINE void* aalloc(aactor_t* self, void* old, const int32_t sz)
{
    assert(self->alloc);
    return self->alloc(self->alloc_ud, old, sz);
}

static void call(aactor_t* self, void* ud)
{
    any_call(self, *(int32_t*)ud);
}

static AINLINE void save_ctx(aactor_t* a, aframe_t* frame, int32_t nargs)
{
    frame->prev = a->frame;
    a->frame = frame;
    frame->bp = a->sp;
    frame->nargs = nargs;
}

static AINLINE void load_ctx(aactor_t* a)
{
    int32_t nsp = a->frame->bp - a->frame->nargs;
    if (a->sp <= a->frame->bp) {
        any_error(a, AERR_RUNTIME, "return value missing");
    }
    a->stack[nsp - 1] = a->stack[a->sp - 1];
    a->sp = nsp;
    a->frame = a->frame->prev;
}

void ASTDCALL actor_entry(void* ud)
{
    aframe_t frame;
    aactor_t* a = (aactor_t*)ud;
    int32_t nargs = (int32_t)a->stack[--a->sp].v.integer;
    acatch_t c;
    c.status = AERR_NONE;
    memset(&frame, 0, sizeof(aframe_t));
    frame.bp = 1; // start from stack[0] (nil)
    frame.nargs = 0;
    a->frame = &frame;
    a->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) any_pcall(a, nargs);
    a->flags |= APF_EXIT;
    while (TRUE) ascheduler_yield(a->owner, a);
}

static void mbox_reserve(aactor_t* self, int32_t more)
{
    avalue_t* nmbx;
    int32_t new_cap;
    if (self->mbox.sz + more <= self->mbox.cap) return;
    new_cap = self->mbox.cap;
    while (new_cap < self->mbox.sz + more) new_cap *= GROW_FACTOR;
    nmbx = (avalue_t*)aalloc(self, self->mbox.msgs, sizeof(avalue_t)*new_cap);
    if (!nmbx) any_error(self, AERR_RUNTIME, "out of memory");
    self->mbox.msgs = nmbx;
    self->mbox.cap = new_cap;
}

aerror_t aactor_init(
    aactor_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud)
{
    aerror_t ec;
    memset(self, 0, sizeof(aactor_t));
    self->owner = owner;
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->stack = (avalue_t*)aalloc(
        self, NULL, sizeof(avalue_t)*INIT_STACK_SZ);
    if (!self->stack) {
        ec = AERR_FULL;
        goto failed;
    }
    self->stack_cap = INIT_STACK_SZ;
    any_push_nil(self); // stack[0] is nil
    self->mbox.msgs = (avalue_t*)aalloc(
        self, NULL, sizeof(avalue_t)*INIT_MBOX_SZ);
    if (!self->mbox.msgs) {
        ec = AERR_FULL;
        goto failed;
    }
    self->mbox.cap = INIT_MBOX_SZ;
    ec = agc_init(&self->gc, INIT_HEAP_SZ, alloc, alloc_ud);
    if (ec != AERR_NONE) goto failed;
    return ec;
failed:
    if (self->stack) aalloc(self, self->stack, 0);
    if (self->mbox.msgs) aalloc(self, self->mbox.msgs, 0);
    return ec;
}

void aactor_cleanup(aactor_t* self)
{
    aalloc(self, self->stack, 0);
    aalloc(self, self->mbox.msgs, 0);
    agc_cleanup(&self->gc);
}

void aactor_reserve(aactor_t* self, int32_t more)
{
    avalue_t* ns;
    int32_t new_cap;
    if (self->sp + more <= self->stack_cap) return;
    new_cap = self->stack_cap;
    while (new_cap < self->sp + more) new_cap *= GROW_FACTOR;
    ns = (avalue_t*)aalloc(self, self->stack, sizeof(avalue_t)*new_cap);
    if (!ns) any_error(self, AERR_RUNTIME, "out of memory");
    self->stack = ns;
    self->stack_cap = new_cap;
}

void any_find(aactor_t* a, const char* module, const char* name)
{
    avalue_t v;
    aerror_t ec = aloader_find(&a->owner->loader, module, name, &v);
    if (ec != AERR_NONE) any_push_nil(a);
    else aactor_push(a, &v);
}

void any_call(aactor_t* a, int32_t nargs)
{
    aframe_t frame;
    int32_t fp = a->sp - nargs - 1;
    avalue_t* f = a->stack + fp;

    if (fp < a->frame->bp || f->tag.b != ABT_FUNCTION) {
        any_error(a, AERR_RUNTIME, "attempt to call a non-function");
    }

    memset(&frame, 0, sizeof(aframe_t));
    save_ctx(a, &frame, nargs);

    switch (f->tag.variant) {
    case AVTF_NATIVE:
        f->v.func(a);
        break;
    case AVTF_AVM:
        frame.pt = f->v.avm_func;
        actor_dispatch(a);
        break;
    default: assert(FALSE);
    }

    load_ctx(a);
}

void any_pcall(aactor_t* a, int32_t nargs)
{
    avalue_t ev;
    if (any_try(a, &call, &nargs) == AERR_NONE) return;
    ev = a->stack[a->sp - 1];
    any_pop(a, 1 + nargs + 1); //error value, args and function
    aactor_push(a, &ev);
}

void any_mbox_send(aactor_t* a)
{
    avalue_t* pid;
    avalue_t* msg;
    aactor_t* ta;
    any_pop(a, 2);
    pid = a->stack + a->sp;
    msg = a->stack + a->sp + 1;
    if (pid->tag.b != ABT_PID) {
        any_error(a, AERR_RUNTIME, "target must be a pid");
    }
    ta = ascheduler_actor(a->owner, pid->v.pid);
    if (!ta) return;
    if (ta->mbox.sz == ta->mbox.cap) mbox_reserve(ta, 1);
    switch (msg->tag.b) {
    case ABT_NIL:
    case ABT_PID:
    case ABT_BOOL:
    case ABT_NUMBER:
        *(ta->mbox.msgs + ta->mbox.sz) = *msg;
        break;
    case ABT_STRING:
        if (AERR_NONE != agc_string_new(
            ta,
            agc_string_to_cstr(a, msg),
            ta->mbox.msgs + ta->mbox.sz)) {
            return; // TODO: review it
        }
        break;
    case ABT_POINTER:
    case ABT_FUNCTION:
    case ABT_FIXED_BUFFER:
    case ABT_BUFFER:
    case ABT_TUPLE:
    case ABT_ARRAY:
    case ABT_MAP:
        any_error(a, AERR_RUNTIME, "not supported type");
        break;
    }
    ++ta->mbox.sz;
}

aerror_t any_mbox_recv(aactor_t* a, int32_t timeout)
{
    AUNUSED(timeout);
    any_error(a, AERR_RUNTIME, "TODO");
    return AERR_TIMEOUT;
}

void any_mbox_remove(aactor_t* a)
{
    any_error(a, AERR_RUNTIME, "TODO");
}

void any_yield(aactor_t* a)
{
    ascheduler_yield(a->owner, a);
}

void any_sleep(aactor_t* a, int32_t nsecs)
{
    ascheduler_sleep(a->owner, a, nsecs);
}

aerror_t any_try(aactor_t* a, void(*f)(aactor_t*, void*), void* ud)
{
    int32_t sp = a->sp;
    aframe_t* frame = a->frame;
    avalue_t ev;
    acatch_t c;
    c.status = AERR_NONE;
    c.prev = a->error_jmp;
    a->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(a, ud);
    if (c.status != AERR_NONE) {
        ev = a->stack[a->sp - 1];
        a->sp = sp;
        a->frame = frame;
    } else {
        ev.tag.b = ABT_NIL;
    }
    aactor_push(a, &ev);
    a->error_jmp = c.prev;
    return c.status;
}

void any_throw(aactor_t* a, int32_t ec)
{
    assert(a->error_jmp);
    a->error_jmp->status = ec;
    longjmp(a->error_jmp->jbuff, 1);
}

void any_error(aactor_t* a, aerror_t ec, const char* fmt, ...)
{
    va_list args;
    char buf[128];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    any_push_string(a, buf);
    va_end(args);
    any_throw(a, ec);
}

int32_t aactor_alloc(aactor_t* self, aabt_t abt, int32_t sz)
{
    agc_t* gc = &self->gc;
    int32_t i = agc_alloc(gc, abt, sz);
    if (i >= 0) return i;
    else {
        avalue_t* roots[] = { self->stack, self->mbox.msgs, NULL };
        int32_t num_roots[] = { self->sp, self->mbox.sz };
        agc_collect(gc, roots, num_roots);
        i = agc_alloc(gc, abt, sz);
        if (i >= 0) return i;
        agc_reserve(gc, sz);
        return agc_alloc(gc, abt, sz);
    }
}

aerror_t any_spawn(aactor_t* a, int32_t cstack_sz, int32_t nargs, apid_t* pid)
{
    aactor_t* na;
    int32_t i;
    aerror_t ec = ascheduler_new_actor(a->owner, cstack_sz, &na);
    if (ec != AERR_NONE) return ec;
    for (i = 0; i < nargs + 1; ++i) {
        avalue_t* v = a->stack + a->sp - nargs - 1 + i;
        switch (v->tag.b) {
        case ABT_NIL:
        case ABT_PID:
        case ABT_BOOL:
        case ABT_NUMBER:
        case ABT_FUNCTION:
            aactor_push(na, v);
            break;
        default:
            any_error(a, AERR_RUNTIME, "not supported type");
            break;
        }
    }
    any_pop(a, nargs + 1);
    ascheduler_start(a->owner, na, nargs);
    *pid = ascheduler_pid(na);
    return AERR_NONE;
}
