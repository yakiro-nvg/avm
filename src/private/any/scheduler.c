#include <any/scheduler.h>

#include <assert.h>
#include <any/errno.h>

int32_t any_sched_init(
    ascheduler_t* self, avm_t* vm, adispatcher_t* runner,
    asmbox_t oqueues[2], asmbox_t iqueues[2],
    arealloc_t realloc, void* realloc_ud)
{
    memset(self, 0, sizeof(*self));
    self->runner = runner;
    self->realloc = realloc;
    self->realloc_ud = realloc_ud;
    self->vm = vm;
#if ANY_SMP
    amutex_init(&self->omutex);
    amutex_init(&self->imutex);
#endif
    self->oback = oqueues[0];
    self->iback = iqueues[0];
    self->ofront = oqueues[1];
    self->ifront = iqueues[1];
    return AERR_NONE;
}