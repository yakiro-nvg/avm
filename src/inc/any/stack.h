/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize as a new stack.
static AINLINE aerror_t
astack_init(
    astack_t* self, aint_t cap, aalloc_t alloc, void* alloc_ud)
{
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->v = (avalue_t*)alloc(alloc_ud, NULL, sizeof(avalue_t)*cap);
    self->sp = 0;
    self->cap = cap;
    return self->v != NULL ? AERR_NONE : AERR_FULL;
}

/// Release all allocated memory.
static AINLINE void
astack_cleanup(
    astack_t* self)
{
    self->alloc(self->alloc_ud, self->v, 0);
    self->v = NULL;
}

/// Ensures that there are `more` bytes in the stack.
ANY_API aerror_t
astack_reserve(
    astack_t* self, aint_t more);

#ifdef __cplusplus
} // extern "C"
#endif
