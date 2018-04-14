// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "value_stack.h"
#include "utils.h"

aresult_t
avalue_stack_grow(
    avalue_stack_t *s, u32 more)
{
    const u32 new_cap = apowof2_ceil(s->cap + more);
    void *const items = AREALLOC(s->a, s->items, sizeof(avalue_t)*new_cap);
    if (!items) return AR_MEMORY;
    s->items = (avalue_t*)items;
    s->cap = new_cap;
    return AR_SUCCESS;
}
