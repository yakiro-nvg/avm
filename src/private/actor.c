/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/actor.h>

#include <any/loader.h>
#include <any/scheduler.h>
#include <any/gc.h>
#include <any/std_string.h>

#define INIT_STACK_SZ 64
#define INIT_MSBOX_SZ 32
#define INIT_HEAP_SZ 512

void
actor_dispatch(
    aactor_t* a);

static AINLINE void*
aalloc(
    aactor_t* self, void* old, const aint_t sz)
{
    assert(self->alloc);
    return self->alloc(self->alloc_ud, old, sz);
}

static void
call(
    aactor_t* self, void* ud)
{
    any_call(self, *(aint_t*)ud);
}

static AINLINE void
save_ctx(
    aactor_t* a, aframe_t* frame, aint_t nargs)
{
    frame->prev = a->frame;
    a->frame = frame;
    frame->bp = a->stack.sp;
    frame->nargs = nargs;
}

static AINLINE void
load_ctx(
    aactor_t* a)
{
    aint_t nsp = a->frame->bp - a->frame->nargs;
    if (a->stack.sp <= a->frame->bp) {
        any_error(a, AERR_RUNTIME, "return value missing");
    }
    a->stack.v[nsp - 1] = a->stack.v[a->stack.sp - 1];
    a->stack.sp = nsp;
    a->frame = a->frame->prev;
}

void ASTDCALL
actor_entry(
    void* ud)
{
	aframe_t frame;
    aactor_t* a = (aactor_t*)ud;
    aint_t nargs = a->stack.v[--a->stack.sp].v.integer;
	memset(&frame, 0, sizeof(aframe_t));
	a->frame = &frame;
	any_protected_call(a, nargs);
    a->flags |= APF_EXIT;
    if (a->owner->on_exit) {
        a->owner->on_exit(a, a->owner->on_exit_ud);
    }
    for (;;) {
        ascheduler_yield(a->owner, a);
    }
}

aerror_t
aactor_init(
    aactor_t* self, ascheduler_t* owner, aalloc_t alloc, void* alloc_ud)
{
    aerror_t ec;
    memset(self, 0, sizeof(aactor_t));
    self->owner = owner;
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    ec = astack_init(&self->stack, INIT_STACK_SZ, alloc, alloc_ud);
    if (ec != AERR_NONE) goto failed;
    ec = astack_init(&self->msbox, INIT_MSBOX_SZ, alloc, alloc_ud);
    if (ec != AERR_NONE) goto failed;
    ec = agc_init(&self->gc, INIT_HEAP_SZ, alloc, alloc_ud);
    if (ec != AERR_NONE) goto failed;
    return ec;
failed:
    astack_cleanup(&self->stack);
    astack_cleanup(&self->msbox);
    return ec;
}

void
aactor_cleanup(
    aactor_t* self)
{
    astack_cleanup(&self->stack);
    astack_cleanup(&self->msbox);
    agc_cleanup(&self->gc);
}

void
any_import(
    aactor_t* a, const char* module, const char* name)
{
    avalue_t v;
    aerror_t ec = aloader_find(&a->owner->loader, module, name, &v);
    if (ec != AERR_NONE) any_push_nil(a);
    else aactor_push(a, &v);
}

void
any_call(
    aactor_t* a, aint_t nargs)
{
    aframe_t frame;
    aint_t fp = a->stack.sp - nargs - 1;
    avalue_t* f = a->stack.v + fp;

    if (fp < a->frame->bp) {
        any_error(a, AERR_RUNTIME, "no function to call");
    }

    if (f->tag.type != AVT_NATIVE_FUNC && f->tag.type != AVT_BYTE_CODE_FUNC) {
        any_error(a, AERR_RUNTIME, "attempt to call a non-function");
    }

    memset(&frame, 0, sizeof(aframe_t));
    save_ctx(a, &frame, nargs);

    switch (f->tag.type) {
    case AVT_NATIVE_FUNC:
        f->v.func(a);
        break;
    case AVT_BYTE_CODE_FUNC:
        frame.pt = f->v.avm_func;
        actor_dispatch(a);
        break;
    }

    load_ctx(a);
}

void
any_protected_call(
    aactor_t* a, aint_t nargs)
{
    avalue_t ev;
    aint_t num_pops, count;
    if (any_try(a, &call, &nargs) == AERR_NONE) return;
    ev = a->stack.v[a->stack.sp - 1];
    num_pops = 1 + nargs + 1; // error value, args and function
    count = any_count(a);
    if (num_pops > count) {
        num_pops = count;
    }
    any_pop(a, count);
    aactor_push(a, &ev);
}

void
any_mbox_send(
    aactor_t* a)
{
    avalue_t* pid;
    avalue_t* msg;
    aactor_t* ta;
    any_pop(a, 2);
    pid = a->stack.v + a->stack.sp;
    msg = a->stack.v + a->stack.sp + 1;
    if (pid->tag.type != AVT_PID) {
        any_error(a, AERR_RUNTIME, "target must be a pid");
    }
    ta = ascheduler_actor(a->owner, pid->v.pid);
    if (!ta) return;
    if (astack_reserve(&ta->msbox, 1) != AERR_NONE) {
        any_error(a, AERR_RUNTIME, "out of memory");
    }
    switch (msg->tag.type) {
    case AVT_NIL:
    case AVT_PID:
    case AVT_BOOLEAN:
    case AVT_INTEGER:
    case AVT_REAL:
        ta->msbox.v[ta->msbox.sp] = *msg;
        break;
    case AVT_STRING:
        if (AERR_NONE != agc_string_new(
            ta,
            agc_string_to_cstr(a, msg),
            ta->msbox.v + ta->msbox.sp)) {
            return; // TODO: review it
        }
        break;
    case AVT_NATIVE_FUNC:
    case AVT_BYTE_CODE_FUNC:
    case AVT_FIXED_BUFFER:
    case AVT_BUFFER:
    case AVT_TUPLE:
    case AVT_ARRAY:
    case AVT_TABLE:
        any_error(a, AERR_RUNTIME, "not supported type");
        break;
    }
    ++ta->msbox.sp;
    ascheduler_got_new_message(a->owner, ta);
}

aerror_t
any_mbox_recv(
    aactor_t* a, aint_t timeout)
{
    for (;;) {
        if (a->msg_pp < a->msbox.sp) {
            if (a->stack.sp <= a->frame->bp) {
                any_error(a, AERR_RUNTIME, "receive to empty stack");
            }
            a->stack.v[a->stack.sp - 1] = a->msbox.v[a->msg_pp++];
            return AERR_NONE;
        } else {
            if (timeout == ADONT_WAIT) {
                return AERR_TIMEOUT;
            } else {
                ascheduler_wait(a->owner, a, timeout);
                timeout = ADONT_WAIT;
            }
        }
    }
}

void
any_mbox_remove(
    aactor_t* a)
{
    if (a->msg_pp <= 0) {
        any_error(a, AERR_RUNTIME, "no message to remove");
    } else {
        aint_t num_tails = a->msbox.sp - a->msg_pp;
        if (num_tails != 0) {
            memmove(
                a->msbox.v + a->msg_pp - 1,
                a->msbox.v + a->msg_pp,
                sizeof(avalue_t) * (size_t)num_tails);
        }
        a->msg_pp = 0;
        --a->msbox.sp;
    }
}

void
any_mbox_rewind(
    aactor_t* a)
{
    a->msg_pp = 0;
}

void
any_yield(
    aactor_t* a)
{
    ascheduler_yield(a->owner, a);
}

void
any_sleep(
    aactor_t* a, aint_t nsecs)
{
    ascheduler_sleep(a->owner, a, nsecs);
}

aerror_t
any_try(
    aactor_t* a, void(*f)(aactor_t*, void*), void* ud)
{
    aint_t sp = a->stack.sp;
    aframe_t* frame = a->frame;
    avalue_t ev;
    acatch_t c;
    c.status = AERR_NONE;
    c.prev = a->error_jmp;
    a->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(a, ud);
    if (c.status != AERR_NONE) {
        ev = a->stack.v[a->stack.sp - 1];
        a->stack.sp = sp;
        a->frame = frame;
    } else {
        ev.tag.type = AVT_NIL;
    }
    aactor_push(a, &ev);
    a->error_jmp = c.prev;
    return c.status;
}

void
any_throw(
    aactor_t* a, aerror_t ec)
{
    assert(a->error_jmp);
    a->error_jmp->status = ec;
    if (a->error_jmp->prev == NULL) {
        if (a->owner->on_panic) {
            a->owner->on_panic(a, a->owner->on_panic_ud);
        }
    } else {
        if (a->owner->on_throw) {
            a->owner->on_throw(a, a->owner->on_throw_ud);
        }
    }
    longjmp(a->error_jmp->jbuff, 1);
}

void
any_error(
    aactor_t* a, aerror_t ec, const char* fmt, ...)
{
    va_list args;
    char buf[128];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    any_push_string(a, buf);
    va_end(args);
    any_throw(a, ec);
}

void
aactor_gc(
    aactor_t* a)
{
    avalue_t* roots[] = {
        a->stack.v,
        a->msbox.v,
        NULL
    };
    aint_t num_roots[] = {
        a->stack.sp,
        a->msbox.sp
    };
    agc_collect(&a->gc, roots, num_roots);
}

aerror_t
aactor_heap_reserve(
    aactor_t* self, aint_t more, aint_t n)
{
    if (agc_check(&self->gc, more, n) == FALSE) {
        aactor_gc(self);
        if (agc_check(&self->gc, more, n) == FALSE) {
            return agc_reserve(&self->gc, more, n);
        }
    }
    return AERR_NONE;
}

void
any_spawn(
    aactor_t* a, aint_t cstack_sz, aint_t nargs, apid_t* pid)
{
    aactor_t* na;
    aint_t i;
    aerror_t ec = ascheduler_new_actor(a->owner, cstack_sz, &na);
    if (ec != AERR_NONE) {
        any_error(a, ec, "failed to create actor");
    }
    for (i = 0; i < nargs + 1; ++i) {
        avalue_t* v = a->stack.v + a->stack.sp - nargs - 1 + i;
        switch (v->tag.type) {
        case AVT_NIL:
        case AVT_PID:
        case AVT_BOOLEAN:
        case AVT_INTEGER:
        case AVT_REAL:
        case AVT_NATIVE_FUNC:
        case AVT_BYTE_CODE_FUNC:
            aactor_push(na, v);
            break;
        default:
            any_error(a, AERR_RUNTIME, "not supported type");
            break;
        }
    }
    any_pop(a, nargs + 1);
    ascheduler_start(a->owner, na, nargs);
    *pid = ascheduler_pid(na->owner, na);
}
