/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/vm.h>

#include <any/loader.h>
#include <any/process.h>

static AINLINE void* aalloc(avm_t* self, void* old, const int32_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static void init_processes(avm_process_t* procs, int32_t num)
{
    int32_t i;
    for (i = 0; i < num; ++i) {
        procs[i].dead = TRUE;
        procs[i].pid = 0;
    }
}

aerror_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud)
{
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->idx_bits = idx_bits;
    self->gen_bits = gen_bits;
    self->procs = aalloc(self, NULL,
        ((int32_t)sizeof(avm_process_t)) * (1 << idx_bits));
    self->next_idx = 0;
    aloader_init(&self->loader, alloc, alloc_ud);
    init_processes(self->procs, 1 << idx_bits);
    return AERR_NONE;
}

void avm_shutdown(avm_t* self)
{
    int32_t num = 1 << self->idx_bits, i;
#ifdef ANY_DEBUG
    for (i = 0; i < num; ++i) {
        assert(self->procs[i].dead && "allocated process must be freed first");
    }
#endif
    aalloc(self, self->procs, 0);
    aloader_cleanup(&self->loader);
}

aprocess_t* avm_alloc(avm_t* self)
{
    int32_t loop = 1 << self->idx_bits;
    do {
        avm_process_t* vp = self->procs + self->next_idx;
        self->next_idx = (self->next_idx + 1) & ((1 << self->idx_bits) - 1);
        if (vp->dead) {
            int32_t idx = (int32_t)(vp - self->procs);
            int32_t gen = apid_gen(self->idx_bits, self->gen_bits, vp->pid);
            gen = (gen + 1) & ((1 << self->gen_bits) - 1);
            vp->pid = apid_from(self->idx_bits, self->gen_bits, idx, gen);
            vp->dead = FALSE;
            return &vp->p;
        }
    } while (--loop);
    return NULL;
}