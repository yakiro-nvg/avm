#include <any/process.h>

#include <assert.h>
#include <any/errno.h>
#include <any/loader.h>

#if 0

#define GROW_FACTOR 2

AINLINE void* aalloc(aprocess_t* p, void* old, const int32_t sz)
{
    assert(p->owner->alloc);
    return p->owner->alloc(p->owner->alloc_ud, old, sz);
}

static void reserve_stack(aprocess_t* p, void* ud)
{
    int32_t more = *(int32_t*)ud;
    if (p->sp < p->stack + p->stack_cap) return;
    if ((p->flags & APF_BORROWED) == 0) {
        avalue_t* ns;
        int32_t new_cap = p->stack_cap;
        while (new_cap < p->stack_cap + more) new_cap *= GROW_FACTOR;
        ns = (avalue_t*)aalloc(p, p->stack, sizeof(avalue_t)*new_cap);
        if (ns) {
            p->sp = ns + (p->stack - p->sp);
            p->stack = ns;
            return;
        }
    }
    aprocess_throw(p, AERR_OVERFLOW);
}

AINLINE void push(aprocess_t* p, void* ud)
{
    avalue_t* v = (avalue_t*)ud;
    int32_t more = 1;
    reserve_stack(p, &more);
    *p->sp = *v;
    ++p->sp;
}

static void call(aprocess_t* p, void* ud)
{
    AUNUSED(ud);
    aprocess_call(p);
}

void aprocess_find(aprocess_t* p, const char* module, const char* name)
{
    avalue_t v;
    int32_t err = aloader_find(
        p->owner->chunks, p->owner->natives, module, name, &v);
    if (err != AERR_NONE) v.tag.b = ABT_NIL;
    push(p, &v);
}

void aprocess_push_nil(aprocess_t* p)
{
    avalue_t v;
    v.tag.b = ABT_NIL;
    push(p, &v);
}

void aprocess_protected_call(aprocess_t* p)
{
    aprocess_try(p, &call, NULL);
}

int32_t aprocess_try(aprocess_t* p, void(*f)(aprocess_t*, void*), void* ud)
{
    acatch_t c;
    c.status = AERR_NONE;
    c.previous = p->error_jmp;
    p->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(p, ud);
    p->error_jmp = c.previous;
    return c.status;
}

void aprocess_throw(aprocess_t* p, int32_t ec)
{
    assert(p->error_jmp);
    p->error_jmp->status = ec;
    longjmp(p->error_jmp->jbuff, 1);
}

void aprocess_error(aprocess_t* p, const char* fmt, ...)
{
    AUNUSED(fmt);
    aprocess_push_nil(p); // TODO: push error message
    aprocess_throw(p, AERR_RUNTIME);
}

static void naive_call(aprocess_t* p)
{
    avalue_t* f = p->sp;
    if (f->tag.b != ABT_FUNCTION) {
        aprocess_error(p, "attempt to call a non-function");
    }
    switch (f->tag.variant) {
    case AVTF_NATIVE:
        f->v.func(p);
        break;
    case AVTF_AVM:
        assert(!"TODO");
        break;
    }
}

void any_naive(adispatcher_t* self)
{
    self->call = &naive_call;
}

#endif