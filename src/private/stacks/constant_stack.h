// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_CONSTANT_STACK_H_
#define _AVM_CONSTANT_STACK_H_

#include "../chunk.h"
#include <avm/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic sized stack.
typedef struct aconstant_stack_s {
    aalloc_t *a;
    aconstant_t *items;
    u32 count;
    u32 capacity;
} aconstant_stack_t;

/// Reallocate stack for `capacity`, `count` maybe adjusted.
aresult_t
aconstant_stack_realloc(
    aconstant_stack_t *s, u32 capacity);

/// Initialize as a new stack.
AINLINE aresult_t
aconstant_stack_init(
    aconstant_stack_t *s, aalloc_t *a, u32 capacity)
{
    s->a = a;
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
    return aconstant_stack_realloc(s, capacity);
}

/// Release all allocated memory.
AINLINE void
aconstant_stack_cleanup(
    aconstant_stack_t *s)
{
    AFREE(s->a, s->items);
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
}

/// Shrink the stack to reclaim memory.
AINLINE void
aconstant_stack_shrink(
    aconstant_stack_t *s)
{
    AVERIFY(
        ASUCCESS(aconstant_stack_realloc(s, s->count)),
        "failed to reallocate to a smaller capacity?");
}

/// Ensures that there are `more` capacity.
AINLINE aresult_t
aconstant_stack_reserve(
    aconstant_stack_t *s, u32 more)
{
    const u32 required = s->count + more;
    return required <= s->capacity ?
        AR_SUCCESS : aconstant_stack_realloc(s, required);
}

/// Push `v` to the stack.
AINLINE aresult_t
aconstant_stack_push(
    aconstant_stack_t *s, const aconstant_t *v)
{
    const aresult_t r = aconstant_stack_reserve(s, 1);
    if (AFAILED(r)) return r;
    s->items[s->count++] = *v;
    return AR_SUCCESS;
}

/// Push `v` to the stack `n` times.
AINLINE aresult_t
aconstant_stack_fill(
    aconstant_stack_t *s, const aconstant_t *v, u32 n)
{
    u32 i;
    const aresult_t r = aconstant_stack_reserve(s, n);
    if (AFAILED(r)) return r;
    else for (i = 0; i < n; ++i) s->items[s->count++] = *v;
    return AR_SUCCESS;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_CONSTANT_STACK_H_
