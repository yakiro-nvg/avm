#include <any/vm.h>

#include <assert.h>
#include <string.h>
#include <any/errno.h>
#include <any/scheduler.h>

#define MSG_QUEUE_GROW_FACTOR 2

static AINLINE void* aalloc(avm_t* self, void* old, const int32_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static void init_processes(avm_process_t* procs, int32_t num)
{
    int32_t i;
    for (i = 0; i < num; ++i) {
        procs[i].dead = TRUE;
        procs[i].gen = 0;
#ifdef ANY_SMP
        amutex_init(&procs[i].mutex);
#endif
    }
}

int32_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits,
    aalloc_t alloc, void* alloc_ud)
{
#ifdef ANY_SMP
    amutex_init(&self->mutex);
#endif
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->idx_bits = idx_bits;
    self->gen_bits = gen_bits;
    self->procs = aalloc(self, NULL, sizeof(avm_process_t)*(1 << idx_bits));
    self->next_idx = 0;
    init_processes(self->procs, 1 << idx_bits);
    return AERR_NONE;
}

void avm_shutdown(avm_t* self)
{
#ifdef ANY_SMP
    int32_t num = 1 << self->idx_bits, i;
    for (i = 0; i < num; ++i) {
        amutex_destroy(&self->procs[i].mutex);
    }
    amutex_destroy(&self->mutex);
#endif
    aalloc(self, self->procs, 0);
}

avm_process_t* avm_alloc(avm_t* self)
{
    avm_process_t* found = NULL;
    int32_t loop = 1 << self->idx_bits;
#ifdef ANY_SMP
    amutex_lock(&self->mutex);
#endif
    do {
        avm_process_t* vp = self->procs + self->next_idx;
        self->next_idx = (self->next_idx + 1) & ((1 << self->idx_bits) - 1);
        if (vp->dead) {
            int32_t idx = (int32_t)(vp - self->procs);
            vp->gen = (vp->gen + 1) & ((1 << self->gen_bits) - 1);
            vp->p.pid = apid_from(self->idx_bits, self->gen_bits, idx, vp->gen);
            vp->dead = FALSE;
            found = vp;
        }
    } while (--loop && !found);
#ifdef ANY_SMP
    amutex_unlock(&self->mutex);
#endif
    return found;
}