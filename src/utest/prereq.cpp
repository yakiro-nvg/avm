// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#include "prereq.h"

static void* stub_malloc(aalloc_t *a, u32 sz, u32 align)
{
    StubAllocator &self = *(StubAllocator*)a;
    aalloc_t *baif = &self.backing.aif;
    return self.full ? NULL : baif->malloc(baif, sz, align);
}

static void stub_dealloc(aalloc_t *a, void *p)
{
    StubAllocator &self = *(StubAllocator*)a;
    aalloc_t *baif = &self.backing.aif;
    baif->dealloc(baif, p);
}

StubAllocator::StubAllocator() : full(false)
{
    amalloc_init(&backing);
    malloc = &stub_malloc;
    dealloc = &stub_dealloc;
}

void StubAllocator::release()
{
    RC_ASSERT(backing.allocated == 0);
}