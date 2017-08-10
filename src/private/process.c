/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/process.h>

#include <any/loader.h>
#include <any/scheduler.h>
#include <any/dispatcher.h>
#include <any/gc.h>
#include <any/gc_string.h>

#define GROW_FACTOR 2
#define INIT_STACK_SZ 64
#define INIT_HEAP_SZ 512

static AINLINE void* aalloc(aprocess_t* self, void* old, const int32_t sz)
{
    assert(self->alloc);
    return self->alloc(self->alloc_ud, old, sz);
}

static void call(aprocess_t* self, void* ud)
{
    int32_t* nargs = (int32_t*)ud;
    any_call(self, *nargs);
}

static AINLINE void save_ctx(aprocess_t* p, aframe_t* frame, int32_t nargs)
{
    frame->prev = p->frame;
    p->frame = frame;
    frame->bp = p->sp;
    frame->nargs = nargs;
}

static AINLINE void load_ctx(aprocess_t* p)
{
    int32_t nsp = p->frame->bp - p->frame->nargs;
    assert(p->sp > p->frame->bp && "return value missing");
    p->stack[nsp - 1] = p->stack[p->sp - 1];
    p->sp = nsp;
    p->frame = p->frame->prev;
}

static void ASTDCALL entry(void* ud)
{
    aframe_t frame;
    aprocess_t* p = (aprocess_t*)ud;
    int32_t nargs = (int32_t)p->stack[--p->sp].v.integer;
    acatch_t c;
    c.status = AERR_NONE;
    memset(&frame, 0, sizeof(aframe_t));
    frame.bp = 1; // start from stack[0] (nil)
    frame.nargs = 0;
    p->frame = &frame;
    p->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) any_pcall(p, nargs);
    p->flags |= APF_EXIT;
    while (TRUE) atask_yield(&p->task);
}

aerror_t aprocess_init(
    aprocess_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud)
{
    aerror_t ec;
    self->owner = owner;
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->flags = 0;
    self->stack = (avalue_t*)aalloc(
        self, NULL, sizeof(avalue_t)*INIT_STACK_SZ);
    if (!self->stack) return AERR_FULL;
    self->stack_cap = INIT_STACK_SZ;
    self->sp = 0;
    any_push_nil(self); // stack[0] is nil
    ec = adispatcher_init(&self->dispatcher, self);
    if (ec != AERR_NONE) aalloc(self, self->stack, 0);
    return agc_init(&self->gc, INIT_HEAP_SZ, alloc, alloc_ud);
}

void aprocess_start(
    aprocess_t* self, int32_t cstack_sz, int32_t nargs)
{
    any_push_integer(self, nargs);
    atask_create(&self->task, &self->owner->task, &entry, self, cstack_sz);
}

void aprocess_cleanup(aprocess_t* self)
{
    aalloc(self, self->stack, 0);
    self->stack = NULL;
    self->stack_cap = 0;
    self->sp = 0;
    atask_delete(&self->task);
    agc_cleanup(&self->gc);
}

void aprocess_reserve(aprocess_t* self, int32_t more)
{
    avalue_t* ns;
    int32_t new_cap;
    if (self->sp + more <= self->stack_cap) return;
    new_cap = self->stack_cap;
    while (new_cap < self->sp + more) new_cap *= GROW_FACTOR;
    ns = (avalue_t*)aalloc(self, self->stack, sizeof(avalue_t)*new_cap);
    if (!ns) any_throw(self, AERR_OVERFLOW);
    self->stack = ns;
    self->stack_cap = new_cap;
}

void any_find(aprocess_t* p, const char* module, const char* name)
{
    avalue_t v;
    aerror_t ec = aloader_find(&p->owner->vm->loader, module, name, &v);
    if (ec != AERR_NONE) any_push_nil(p);
    else aprocess_push(p, &v);
}

void any_call(aprocess_t* p, int32_t nargs)
{
    aframe_t frame;
    int32_t fp = p->sp - nargs - 1;
    avalue_t* f = p->stack + fp;

    if (fp < p->frame->bp || f->tag.b != ABT_FUNCTION) {
        any_error(p, AERR_RUNTIME, "attempt to call a non-function");
    }

    memset(&frame, 0, sizeof(aframe_t));
    save_ctx(p, &frame, nargs);

    switch (f->tag.variant) {
    case AVTF_NATIVE:
        f->v.func(p);
        break;
    case AVTF_AVM:
        frame.pt = f->v.avm_func;
        adispatcher_call(&p->dispatcher);
        break;
    default: assert(FALSE);
    }

    load_ctx(p);
}

void any_pcall(aprocess_t* p, int32_t nargs)
{
    if (any_try(p, &call, &nargs) != AERR_NONE) {
        avalue_t ev = p->stack[p->sp - 1];
        any_pop(p, 1 + nargs + 1); //error value, args and function
        aprocess_push(p, &ev);
    }
}

void any_yield(aprocess_t* p)
{
    atask_yield(&p->task);
}

aerror_t any_try(aprocess_t* p, void(*f)(aprocess_t*, void*), void* ud)
{
    int32_t sp = p->sp;
    aframe_t* frame = p->frame;
    avalue_t ev;
    acatch_t c;
    c.status = AERR_NONE;
    c.prev = p->error_jmp;
    p->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(p, ud);
    if (c.status != AERR_NONE) {
        ev = p->stack[p->sp - 1];
        p->sp = sp;
        p->frame = frame;
    } else {
        ev.tag.b = ABT_NIL;
    }
    aprocess_push(p, &ev);
    p->error_jmp = c.prev;
    return c.status;
}

void any_throw(aprocess_t* p, int32_t ec)
{
    assert(p->error_jmp);
    p->error_jmp->status = ec;
    longjmp(p->error_jmp->jbuff, 1);
}

void any_error(aprocess_t* p, aerror_t ec, const char* fmt, ...)
{
    va_list args;
    char buf[128];
    va_start(args, fmt);
    snprintf(buf, sizeof(buf), fmt, args);
    any_push_string(p, buf);
    va_end(args);
    any_throw(p, ec);
}

aerror_t any_spawn(aprocess_t* p, int32_t cstack_sz, int32_t nargs, apid_t* pid)
{
    aprocess_t* np;
    int32_t i;
    aerror_t ec = ascheduler_new_process(p->owner, &np);
    if (ec != AERR_NONE) return ec;
    for (i = 0; i < nargs + 1; ++i) {
        aprocess_push(np, p->stack + p->sp - nargs - 1 + i);
    }
    any_pop(p, nargs + 1);
    aprocess_start(np, cstack_sz, nargs);
    *pid = np->pid;
    return AERR_NONE;
}
