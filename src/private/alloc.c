// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
#include <avm/alloc.h>

// borrowed from: https://bitbucket.org/bitsquid/foundation/

// header stored at the beginning of a memory allocation
// to indicate the size of the allocated data
typedef struct header_s { u32 size; } header_t;

// if we need to align the memory allocation we pad the
// header with this value after storing the size
const u32 HEADER_PAD_VALUE = 0xffffffffu;

AINLINE void*
data_pointer(
    header_t *header, int align)
{
    return align_forward(header + 1, align);
}

AINLINE header_t*
header(
    void *data)
{
    u32 *p = (u32*)data;
    while (p[-1] == HEADER_PAD_VALUE) --p;
    return ((header_t*)p) - 1;
}

AINLINE void
fill(
    header_t *header, void *data, u32 size)
{
    u32 *p = (u32*)(header + 1);
    header->size = size;
    while ((void*)p < data) *p++ = HEADER_PAD_VALUE;
}

// returns the size to allocate from malloc() for a given size and align
AINLINE u32
size_with_padding(
    u32 size, u32 align)
{
    return size + align + sizeof(header_t);
}

ASTATIC void*
malloc_malloc(
    aalloc_t *a, u32 sz, u32 align)
{
    amalloc_t *const m = AFROM_FIELD(amalloc_t, a, aif);
    const uint32_t ts = size_with_padding(sz, align);
    header_t *const h = (header_t*)malloc(ts);
    void *const p = data_pointer(h, align);
    fill(h, p, ts);
    m->allocated += ts;
    return p;
}

static void
malloc_dealloc(
    aalloc_t *a, void* p)
{
    if (!p) return;
    else {
        amalloc_t *const m = AFROM_FIELD(amalloc_t, a, aif);
        header_t *const h = header(p);
        m->allocated -= h->size;
        free(h);
    }
}

aresult_t
amalloc_init(
    amalloc_t* a)
{
    a->allocated = 0;
    a->aif.malloc = &malloc_malloc;
    a->aif.dealloc = &malloc_dealloc;
    return AR_SUCCESS;
}

void
amalloc_release(
    amalloc_t* a)
{
    AASSERT(a->allocated == 0, "memory leaked");
    memset(a, 0, sizeof(amalloc_t));
}