// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "s32_stack.h"
#include "../utils.h"

aresult_t
as32_stack_realloc(
    as32_stack_t *s, u32 capacity)
{
    const u32 new_cap = apowof2_ceil(capacity);
    if (new_cap != 0) {
        void *const items = AMAKE_ARRAY(
            s->a, s32, new_cap);
        if (!items) return AR_MEMORY;
        s->count = AMIN(s->count, s->capacity);
        memcpy(items, s->items, sizeof(s32)*s->count);
        AFREE(s->a, s->items);
        s->items = (s32*)items;
        s->capacity = new_cap;
    }
    return AR_SUCCESS;
}
