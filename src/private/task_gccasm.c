#include <any/task.h>

#ifdef ANY_TASK_GCCASM

#define ALIGNED_SP(p, s) \
    (((char*)0) + ((((char*)(p)-(char*)0)+(s)) & -16))

#define STACK_ADJUST(p, n) &((size_t*)p)[-n]

#ifdef ANY_USE_VALGRIND
#include <valgrind.h>
#define STACK_REG(t, p, sz) (t)->vgid = VALGRIND_STACK_REGISTER(p, p+sz)
#define STACK_DEREG(t)      VALGRIND_STACK_DEREGISTER((t)->vgid)
#else
#define STACK_REG(t, p, sz)
#define STACK_DEREG(id)
#endif

void atask_ctx_switch(atask_ctx_t*, atask_ctx_t*);
void atask_ctx_entryp();

aerror_t atask_shadow(struct atask_t* self)
{
    self->stack = NULL;
    self->node.prev = &self->node;
    self->node.next = &self->node;
    return AERR_NONE;
}

aerror_t atask_create(
    struct atask_t* self, struct atask_t* root,
    atask_entry_t entry, void* ud, int32_t stack_sz)
{
    self->stack = (uint8_t*)malloc(stack_sz);
    if (!self->stack) return AERR_FULL;
    alist_node_insert(&self->node, root->node.prev, &root->node);
    STACK_REG(self, self->stack, stack_sz);
#if defined(AARCH_I386)
#error "TODO"
#elif defined(AARCH_AMD64)
    self->ctx.rip = (void*)&atask_ctx_entryp;
    self->ctx.rsp = STACK_ADJUST(ALIGNED_SP(self->stack, stack_sz), 1);
    *(size_t*)self->ctx.rsp = 0xDEADFFFFDEADFFFF;
    self->ctx.rbp = 0;
    self->ctx.rbx = 0;
    self->ctx.r12 = (void*)entry;
    self->ctx.r13 = ud;
    self->ctx.r14 = 0;
    self->ctx.r15 = 0;
#elif defined(AARCH_ARM)
#error "TODO"
#endif
    return AERR_NONE;
}

void atask_delete(struct atask_t* self)
{
    alist_node_erase(&self->node);
    if (!self->stack) return;
    STACK_DEREG(self);
    free(self->stack);
    self->stack = NULL;
}

void atask_yield(struct atask_t* self)
{
    atask_t* next = ALIST_NODE_CAST(atask_t, self->node.next);
    if (next == self) return;
    atask_ctx_switch(&self->ctx, &next->ctx);
}

void atask_sleep(struct atask_t* self, int32_t nsecs)
{
    AUNUSED(self);
    AUNUSED(nsecs);
    assert(!"TODO");
}

#else // ANY_TASK_GCCASM
static char non_empty_unit;
#endif
