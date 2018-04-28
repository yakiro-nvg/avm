// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "instruction_stack.h"
#include "../utils.h"

aresult_t
ainstruction_stack_realloc(
    ainstruction_stack_t *s, u32 capacity)
{
    const u32 new_cap = apowof2_ceil(capacity);
    if (new_cap != 0) {
        void *const items = AMAKE_ARRAY(
            s->a, ainstruction_t, new_cap);
        if (!items) return AR_MEMORY;
        s->count = AMIN(s->count, s->capacity);
        memcpy(items, s->items, sizeof(ainstruction_t)*s->count);
        AFREE(s->a, s->items);
        s->items = (ainstruction_t*)items;
        s->capacity = new_cap;
    }
    return AR_SUCCESS;
}
