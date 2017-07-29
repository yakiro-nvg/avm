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
    aprocess_call(self);
}

static ASTDCALL entry(void* ud)
{
    aprocess_t* self = (aprocess_t*)ud;
    aprocess_pcall(self);
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

void aprocess_find(aprocess_t* self, const char* module, const char* name)
{
    avalue_t v;
    int32_t err = aloader_find(&self->vm->_loader, module, name, &v);
    if (err != AERR_NONE) aprocess_push_nil(self);
    aprocess_push(self, &v);
}

void aprocess_call(aprocess_t* self)
{
    avalue_t* f;
    aprocess_pop(self, 1);
    f = self->stack + self->stack_sz;
    if (f->tag.b != ABT_FUNCTION) {
        aprocess_error(self, "attempt to call a non-function");
    }
    switch (f->tag.variant) {
    case AVTF_NATIVE:
        f->v.func(self);
        break;
    case AVTF_AVM:
        aprocess_error(self, "not implemented");
        break;
    default: assert(FALSE);
    }
}

void aprocess_pcall(aprocess_t* self)
{
    aprocess_try(self, &call, NULL);
}

void aprocess_reserve(aprocess_t* self, int32_t more)
{
    avalue_t* ns;
    int32_t new_cap;
    if (self->stack_sz + more <= self->stack_cap) return;
    new_cap = self->stack_cap;
    while (new_cap < self->stack_sz + more) new_cap *= GROW_FACTOR;
    ns = (avalue_t*)aalloc(self, self->stack, sizeof(avalue_t)*new_cap);
    if (!ns) aprocess_throw(self, AERR_OVERFLOW);
    self->stack = ns;
    self->stack_cap = new_cap;
}

void aprocess_yield(aprocess_t* self)
{
    ascheduler_t* owner = (ascheduler_t*)self->owner;
    atask_yield(&self->task);
}

int32_t aprocess_try(aprocess_t* self, void(*f)(aprocess_t*, void*), void* ud)
{
    int32_t stack_sz = self->stack_sz;
    acatch_t c;
    c.status = AERR_NONE;
    c.prev = self->error_jmp;
    self->error_jmp = &c;
    if (setjmp(c.jbuff) == 0) f(self, ud);
    if (c.status != AERR_NONE) self->stack_sz = stack_sz;
    self->error_jmp = c.prev;
    return c.status;
}

void aprocess_throw(aprocess_t* self, int32_t ec)
{
    assert(self->error_jmp);
    self->error_jmp->status = ec;
    longjmp(self->error_jmp->jbuff, 1);
}

void aprocess_error(aprocess_t* self, const char* fmt, ...)
{
    AUNUSED(fmt);
    aprocess_push_nil(self); // TODO: push error message
    aprocess_throw(self, AERR_RUNTIME);
}