/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/gc.h>

#include <any/errno.h>
#include <any/process.h>

#define GROW_FACTOR 2
#define NOT_FORWARED -1

static AINLINE void* aalloc(agc_t* self, void* old, const int32_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static AINLINE uint8_t* low_heap(agc_t* self)
{
    return self->cur_heap < self->new_heap ? self->cur_heap : self->new_heap;
}

static AINLINE void swap(agc_t* self)
{
    uint8_t* tmp = self->cur_heap;
    self->cur_heap = self->new_heap;
    self->new_heap = tmp;
}

static AINLINE void copy(agc_t* self, avalue_t* v)
{
    agc_header_t* ogch = (agc_header_t*)(self->cur_heap + v->v.heap_idx);
    agc_header_t* ngch = (agc_header_t*)(self->new_heap + self->heap_sz);
    if (!avalue_collectable(v)) return;
    if (ogch->forwared == NOT_FORWARED) {
        memcpy(ngch, ogch, ogch->sz);
        ogch->forwared = self->heap_sz;
        self->heap_sz += ogch->sz;
    }
    v->v.heap_idx = ogch->forwared;
}

static AINLINE void scan(agc_t* self, agc_header_t* gch)
{
    switch (gch->abt) {
    case ABT_NIL:
    case ABT_PID:
    case ABT_BOOL:
    case ABT_POINTER:
    case ABT_NUMBER:
    case ABT_FUNCTION:
    case ABT_FIXED_BUFFER:
    case ABT_STRING:
        // nop
        break;
    case ABT_BUFFER:
        copy(self, &((agc_buffer_t*)(gch + 1))->buff);
        break;
    case ABT_TUPLE:
    case ABT_ARRAY:
    case ABT_MAP:
        assert(!"TODO");
        break;
    default: assert(!"bad value type");
    }
}

int32_t agc_init(agc_t* self, int32_t heap_cap, aalloc_t alloc, void* alloc_ud)
{
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->cur_heap = (uint8_t*)aalloc(self, NULL, heap_cap*2);
    if (!self->cur_heap) return AERR_FULL;
    self->new_heap = self->cur_heap + heap_cap;
    self->heap_cap = heap_cap;
    self->heap_sz = 0;
    return AERR_NONE;
}

void agc_cleanup(agc_t* self)
{
    aalloc(self, low_heap(self), 0);
    self->new_heap = NULL;
    self->cur_heap = NULL;
    self->heap_cap = 0;
    self->heap_sz = 0;
}

int32_t agc_alloc(agc_t* self, int32_t abt, int32_t sz)
{
    agc_header_t* gch;
    int32_t more = sz + sizeof(agc_header_t);
    int32_t new_heap_sz = self->heap_sz + more;
    int32_t heap_idx = self->heap_sz;
    if (new_heap_sz > self->heap_cap) return AERR_FULL;
    self->heap_sz = new_heap_sz;
    gch = ((agc_header_t*)(self->cur_heap + heap_idx));
    gch->abt = abt;
    gch->forwared = NOT_FORWARED;
    gch->sz = more;
    return heap_idx;
}

int32_t agc_reserve(agc_t* self, int32_t more)
{
    uint8_t* nh;
    int32_t new_cap = self->heap_cap;
    while (new_cap < self->heap_sz + more) new_cap *= GROW_FACTOR;
    nh = (uint8_t*)aalloc(self, NULL, new_cap * 2);
    if (!nh) return AERR_FULL;
    memcpy(nh, self->cur_heap, self->heap_sz);
    aalloc(self, low_heap(self), 0);
    self->cur_heap = nh;
    self->new_heap = nh + new_cap;
    self->heap_cap = new_cap;
    return AERR_NONE;
}

void agc_collect(agc_t* self, avalue_t* root, int32_t num_roots)
{
    int32_t i;
    self->heap_sz = 0;
    self->scan = 0;
    for (i = 0; i < num_roots; ++i) {
        copy(self, root + i);
    }
    while (self->scan != self->heap_sz) {
        agc_header_t* header = (agc_header_t*)(self->new_heap + self->scan);
        scan(self, header);
        self->scan += header->sz;
    }
    swap(self);
}