/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/stack.h>

#define GROW_FACTOR 2

aerror_t astack_reserve(astack_t* self, int32_t more)
{
    avalue_t* nv;
    int32_t new_cap;
    if (self->sp + more <= self->cap) {
        return AERR_NONE;
    } else {
        new_cap = self->cap;
        while (new_cap < self->sp + more) new_cap *= GROW_FACTOR;
        nv = (avalue_t*)self->alloc(
            self->alloc_ud, self->v, sizeof(avalue_t)*new_cap);
        if (nv == NULL) {
            return AERR_FULL;
        } else {
            self->v = nv;
            self->cap = new_cap;
            return AERR_NONE;
        }
    }
}