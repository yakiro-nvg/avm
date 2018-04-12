// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "stack.h"

aresult_t
astack_reserve(
    astack_t *s, u32 more)
{
    u32 new_cap = s->cap;
    if (s->sp + more <= new_cap) return AR_SUCCESS;
    while (new_cap < s->sp + more) new_cap *= s->grow;
    void *nitems = AREALLOC(s->a, s->items, s->item_sz*new_cap);
    if (!nitems) return AR_MEMORY;
    s->items = nitems;
    s->cap = new_cap;
    return AR_SUCCESS;
}