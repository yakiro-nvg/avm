// Auto-generated, don't edit.
// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "s32.h"
#include "../utils.h"

aresult_t
as32_stack_realloc(
    as32_stack_t *s, u32 capacity)
{
    const u32 new_cap = apowof2_ceil(capacity);
    if (new_cap != 0) {
        void *const items = AREALLOC(
            s->a, s->items, sizeof(s32)*new_cap);
        if (!items) return AR_MEMORY;
        s->items = (s32*)items;
        s->capacity = new_cap;
        s->count = AMIN(s->count, s->capacity);
    }
    return AR_SUCCESS;
}
