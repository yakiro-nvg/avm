// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_VALUE_STACK_H_
#define _AVM_VALUE_STACK_H_

#include <avm/prereq.h>
#include <avm/errno.h>
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic sized stack.
typedef struct avalue_stack_s {
    aalloc_t *a;
    avalue_t *items;
    u32 item_sz;
    u32 sp;
    u32 cap;
} avalue_stack_t;

/// Initialize as a new stack.
AINLINE aresult_t
avalue_stack_init(
    avalue_stack_t *s, aalloc_t *a, u32 capacity)
{
    s->a = a;
    s->sp = 0;
    s->cap = capacity;
    s->items = (avalue_t*)AREALLOC(a, NULL, sizeof(avalue_t)*capacity);
    return s->items ? AR_SUCCESS : AR_MEMORY;
}

/// Release all allocated memory.
AINLINE void
avalue_stack_cleanup(
    avalue_stack_t *s)
{
    AFREE(s->a, s->items);
    s->items = NULL;
}

/// Reallocate stack for `more` capacity.
aresult_t
avalue_stack_grow(
    avalue_stack_t *s, u32 more);

/// Ensures that there are `more` capacity.
AINLINE aresult_t
avalue_stack_reserve(
    avalue_stack_t *s, u32 more)
{
    return (s->sp + more <= s->cap) ?
        AR_SUCCESS : avalue_stack_grow(s, more);
}

/// Push `v` to the stack.
AINLINE aresult_t
avalue_stack_push(
    avalue_stack_t *s, const avalue_t *v)
{
    const aresult_t r = avalue_stack_reserve(s, 1);
    if (AFAILED(r)) return r;
    s->items[s->sp++] = *v;
    return AR_SUCCESS;
}

/// Push `v` to the stack `n` times.
AINLINE aresult_t
avalue_stack_fill(
    avalue_stack_t *s, const avalue_t *v, u32 n)
{
    const aresult_t r = avalue_stack_reserve(s, n);
    if (AFAILED(r)) return r;
    else {
        u32 i; for (i = 0; i < n; ++i) s->items[s->sp++] = *v;
    }
    return AR_SUCCESS;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_VALUE_STACK_H_
