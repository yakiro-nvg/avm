// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_STACK_H_
#define _AVM_STACK_H_

#include <avm/prereq.h>
#include <avm/errno.h>
#include "utils.h"

#define ASTACK_DCL(T, name) \
/* Dynamic sized stack. */ \
typedef struct a##name##_stack_s { \
    aalloc_t *a; \
    T *items; \
    u32 count; \
    u32 capacity; \
} a##name##_stack_t; \
/* Reallocate stack for `capacity`, `count` may be adjusted afterthat. */ \
aresult_t \
AAPI a##name##_stack_realloc( \
    a##name##_stack_t *s, u32 capacity); \
/* Initialize as a new stack. */ \
AINLINE aresult_t \
a##name##_stack_init( \
    a##name##_stack_t *s, aalloc_t *a, u32 capacity) \
{ \
    s->a = a; \
    s->items = NULL; \
    s->count = 0; \
    s->capacity = 0; \
    return a##name##_stack_realloc(s, capacity); \
} \
/* Release all allocated memory. */ \
AINLINE void \
a##name##_stack_cleanup( \
    a##name##_stack_t *s) \
{ \
    AFREE(s->a, s->items); \
    s->items = NULL; \
    s->count = 0; \
    s->capacity = 0; \
} \
/* Shrink the stack to reclaim memory. */ \
AINLINE void \
a##name##_stack_shrink( \
    a##name##_stack_t *s) \
{ \
    AVERIFY( \
        ASUCCESS(a##name##_stack_realloc(s, s->count)), \
        "failed to reallocate to a smaller capacity?"); \
} \
/* Ensures that there are `more` capacity. */ \
AINLINE aresult_t \
a##name##_stack_reserve( \
    a##name##_stack_t *s, u32 more) \
{ \
    const u32 required = s->count + more; \
    return required <= s->capacity ? \
        AR_SUCCESS : a##name##_stack_realloc(s, required); \
} \
/* Push `v` to the stack. */ \
AINLINE aresult_t \
a##name##_stack_push( \
    a##name##_stack_t *s, const T *v) \
{ \
    const aresult_t r = a##name##_stack_reserve(s, 1); \
    if (AFAILED(r)) return r; \
    s->items[s->count++] = *v; \
    return AR_SUCCESS; \
} \
/* Push `v` to the stack `n` times. */ \
AINLINE aresult_t \
a##name##_stack_fill( \
    a##name##_stack_t *s, const T *v, u32 n) \
{ \
    u32 i; \
    const aresult_t r = a##name##_stack_reserve(s, n); \
    if (AFAILED(r)) return r; \
    for (i = 0; i < n; ++i) s->items[s->count++] = *v; \
    return AR_SUCCESS; \
}

#define ASTACK_DEF(T, name) \
aresult_t \
a##name##_stack_realloc( \
    a##name##_stack_t *s, u32 capacity) \
{ \
    const u32 new_cap = apowof2_ceil(capacity*sizeof(T)); \
    void *const items = AMAKE_ARRAY(s->a, T, new_cap); \
    if (!items) return AR_MEMORY; \
    s->count = AMIN(s->count, capacity); \
    memcpy(items, s->items, sizeof(T)*s->count); \
    AFREE(s->a, s->items); \
    s->items = (T*)items; \
    s->capacity = new_cap; \
    return AR_SUCCESS; \
}

#endif // !_AVM_STACK_H_