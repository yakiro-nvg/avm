#include <any/process.h>

#include <assert.h>
#include <any/errno.h>
#include <any/loader.h>

#define GROW_FACTOR 2
#define INIT_STACK_SZ 64

static AINLINE void* aalloc(aprocess_t* self, void* old, const int32_t sz)
{
    assert(self->owner->alloc);
    return self->owner->alloc(self->owner->alloc_ud, old, sz);
}

static void call(aprocess_t* self, void* ud)
{
    AUNUSED(ud);
    any_call(self);
}

static ASTDCALL entry(void* ud)
{
    aprocess_t* self = (aprocess_t*)ud;
    any_pcall(self);
    self->flags |= APF_EXIT;
    atask_yield(&self->task);
}

void aprocess_init(aprocess_t* self, ascheduler_t* owner, apid_t pid)
{
    self->pid = pid;
    self->flags = 0;
    self->owner = owner;
    self->stack = (avalue_t*)aalloc(
        self, NULL, sizeof(avalue_t)*INIT_STACK_SZ);
    self->stack_cap = INIT_STACK_SZ;
}

void aprocess_start(aprocess_t* self, int32_t cstack_sz)
{
    atask_create(
        &self->task,
        &((ascheduler_t*)self->owner)->task,
        &entry,
        self,
        cstack_sz);
}

void aprocess_cleanup(aprocess_t* self)
{
    aalloc(self, self->stack, 0);
    self->stack = NULL;
    self->stack_cap = 0;
    self->stack_sz = 0;
    atask_delete(&self->task);
}

void aprocess_reserve(aprocess_t* self, int32_t more)
{
    avalue_t* ns;
    int32_t new_cap;
    if (self->stack_sz + more <= self->stack_cap) return;
    new_cap = self->stack_cap;
    while (new_cap < self->stack_sz + more) new_cap *= GROW_FACTOR;
    ns = (avalue_t*)aalloc(self, self->stack, sizeof(avalue_t)*new_cap);
    if (!ns) any_throw(self, AERR_OVERFLOW);
    self->stack = ns;
    self->stack_cap = new_cap;
}

void any_find(aprocess_t* p, const char* module, const char* name)
{
    avalue_t v;
    int32_t err = aloader_find(&p->vm->_loader, module, name, &v);
    if (err != AERR_NONE) any_push_nil(p);
    aprocess_push(p, &v);
}

void any_call(aprocess_t* p)
{
    avalue_t* f;
    any_pop(p, 1);
    f = p->stack + p->stack_sz;
    if (f->tag.b != ABT_FUNCTION) {
        any_error(p, "attempt to call a non-function");
    }
    switch (f->tag.variant) {
    case AVTF_NATIVE:
        f->v.func(p);
        break;
    case AVTF_AVM:
        any_error(p, "not implemented");
        break;
    default: assert(FALSE);
    }
}

void any_pcall(aprocess_t* p)
{
    any_try(p, &call, NULL);
}

void any_yield(aprocess_t* p)
{
    ascheduler_t* owner = (ascheduler_t*)p->owner;
    atask_yield(&p->task);
}

int32_t any_try(aprocess_t* p, void(*f)(aprocess_t*, void*), void* ud)
{
    int32_t stack_sz = p->stack_sz;
    acatch_t c;
    c.status = AERR_NONE;
    c.prev = p->error_jmp;
    p->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(p, ud);
    if (c.status != AERR_NONE) p->stack_sz = stack_sz;
    p->error_jmp = c.prev;
    return c.status;
}

void any_throw(aprocess_t* p, int32_t ec)
{
    assert(p->error_jmp);
    p->error_jmp->status = ec;
    longjmp(p->error_jmp->jbuff, 1);
}

void any_error(aprocess_t* p, const char* fmt, ...)
{
    AUNUSED(fmt);
    any_push_nil(p); // TODO: push error message
    any_throw(p, AERR_RUNTIME);
}