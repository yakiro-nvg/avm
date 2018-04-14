// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_STACK_H_
#define _AVM_STACK_H_

#include <avm/prereq.h>
#include <avm/errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Dynamic sized stack.
typedef struct astack_s {
    aalloc_t *a;
    void *items;
    u32 item_sz;
    u32 sp;
    u32 cap;
    u32 grow;
} astack_t;

/// Initialize as a new stack.
AINLINE aresult_t
astack_init(
    astack_t *s, aalloc_t *a, u32 capacity, u32 item_sz, u32 grow_factor)
{
    s->a = a;
    s->sp = 0;
    s->cap = capacity;
    s->item_sz = item_sz;
    s->items = AREALLOC(a, NULL, item_sz*capacity);
    return s->items ? AR_SUCCESS : AR_MEMORY;
}

/// Release all allocated memory.
AINLINE void
astack_cleanup(
    astack_t *s)
{
    AFREE(s->a, s->items);
    s->items = NULL;
}

/// Ensures there are `more` items in the stack.
aresult_t
astack_reserve(
    astack_t *s, u32 more);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_STACK_H_