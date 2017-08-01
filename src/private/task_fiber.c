#include <any/task.h>

#ifdef ATASK_FIBER

aerror_t atask_shadow(struct atask_t* self)
{
    self->fiber = GetCurrentFiber();
    self->fiber = (self->fiber && self->fiber != (void*)0x1E00)
        ? self->fiber
        : ConvertThreadToFiber(NULL);
    if (!self->fiber) return AERR_RUNTIME;
    self->node.prev = &self->node;
    self->node.next = &self->node;
    return AERR_NONE;
}

aerror_t atask_create(
    struct atask_t* self, struct atask_t* root,
    atask_entry_t entry, void* ud, int32_t stack_sz)
{
    self->fiber = CreateFiber((SIZE_T)stack_sz, entry, ud);
    if (!self->fiber) return AERR_RUNTIME;
    alist_node_insert(&self->node, root->node.prev, &root->node);
    return AERR_NONE;
}

void atask_delete(struct atask_t* self)
{
    alist_node_erase(&self->node);
    if (!self->fiber) return;
    DeleteFiber(self->fiber);
    self->fiber = NULL;
}

void atask_yield(struct atask_t* self)
{
    atask_t* next = ALIST_NODE_CAST(atask_t, self->node.next);
    if (next != self) SwitchToFiber(next->fiber);
}

void atask_sleep(struct atask_t* self, int32_t nsecs)
{
    AUNUSED(self);
    Sleep(nsecs * 1000000);
}

#else // ATASK_FIBER
static char non_empty_unit;
#endif
