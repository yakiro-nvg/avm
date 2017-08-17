/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/task.h>

#ifdef ANY_TASK_FIBER

aerror_t atask_shadow(struct atask_t* self)
{
    self->fiber = GetCurrentFiber();
    self->fiber = (self->fiber && self->fiber != (void*)0x1E00)
        ? self->fiber
        : ConvertThreadToFiber(NULL);
    if (!self->fiber) return AERR_RUNTIME;
    return AERR_NONE;
}

aerror_t atask_create(
    struct atask_t* self, atask_entry_t entry, void* ud, aint_t stack_sz)
{
    self->fiber = CreateFiber((SIZE_T)stack_sz, entry, ud);
    if (!self->fiber) return AERR_FULL;
    return AERR_NONE;
}

void atask_delete(struct atask_t* self)
{
    DeleteFiber(self->fiber);
}

void atask_yield(struct atask_t* self, struct atask_t* next)
{
    assert(self != next);
    SwitchToFiber(next->fiber);
}

#else // ANY_TASK_FIBER
static char non_empty_unit;
#endif
