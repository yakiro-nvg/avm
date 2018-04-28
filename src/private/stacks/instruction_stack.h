// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_INSTRUCTION_STACK_H_
#define _AVM_INSTRUCTION_STACK_H_

#include "../opcode.h"
#include <avm/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic sized stack.
typedef struct ainstruction_stack_s {
    aalloc_t *a;
    ainstruction_t *items;
    u32 count;
    u32 capacity;
} ainstruction_stack_t;

/// Reallocate stack for `capacity`, `count` maybe adjusted.
aresult_t
ainstruction_stack_realloc(
    ainstruction_stack_t *s, u32 capacity);

/// Initialize as a new stack.
AINLINE aresult_t
ainstruction_stack_init(
    ainstruction_stack_t *s, aalloc_t *a, u32 capacity)
{
    s->a = a;
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
    return ainstruction_stack_realloc(s, capacity);
}

/// Release all allocated memory.
AINLINE void
ainstruction_stack_cleanup(
    ainstruction_stack_t *s)
{
    AFREE(s->a, s->items);
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
}

/// Shrink the stack to reclaim memory.
AINLINE void
ainstruction_stack_shrink(
    ainstruction_stack_t *s)
{
    AVERIFY(
        ASUCCESS(ainstruction_stack_realloc(s, s->count)),
        "failed to reallocate to a smaller capacity?");
}

/// Ensures that there are `more` capacity.
AINLINE aresult_t
ainstruction_stack_reserve(
    ainstruction_stack_t *s, u32 more)
{
    const u32 required = s->count + more;
    return required <= s->capacity ?
        AR_SUCCESS : ainstruction_stack_realloc(s, required);
}

/// Push `v` to the stack.
AINLINE aresult_t
ainstruction_stack_push(
    ainstruction_stack_t *s, const ainstruction_t *v)
{
    const aresult_t r = ainstruction_stack_reserve(s, 1);
    if (AFAILED(r)) return r;
    s->items[s->count++] = *v;
    return AR_SUCCESS;
}

/// Push `v` to the stack `n` times.
AINLINE aresult_t
ainstruction_stack_fill(
    ainstruction_stack_t *s, const ainstruction_t *v, u32 n)
{
    u32 i;
    const aresult_t r = ainstruction_stack_reserve(s, n);
    if (AFAILED(r)) return r;
    else for (i = 0; i < n; ++i) s->items[s->count++] = *v;
    return AR_SUCCESS;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_INSTRUCTION_STACK_H_
