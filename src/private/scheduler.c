#include <any/scheduler.h>

#include <any/loader.h>
#include <any/vm.h>
#include <any/process.h>

static AINLINE void* aalloc(ascheduler_t* self, void* old, const int32_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static void cleanup(ascheduler_t* self, int32_t force)
{
    alist_node_t* i = self->task.node.next;
    while (i != &self->task.node) {
        alist_node_t* const next = i->next;
        atask_t* const t = ALIST_NODE_CAST(atask_t, i);
        aprocess_t* const p = ACAST_FROM_FIELD(aprocess_t, t, task);
        avm_process_t* const vp = ACAST_FROM_FIELD(avm_process_t, p, p);
        if (force || (p->flags & APF_EXIT) != 0) {
            aprocess_cleanup(p);
            avm_free(vp);
        }
        i = next;
    }
}

aerror_t ascheduler_init(
    ascheduler_t* self, avm_t* vm, aalloc_t alloc, void* alloc_ud)
{
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->vm = vm;
    atask_shadow(&self->task);
    return AERR_NONE;
}

void ascheduler_run_once(ascheduler_t* self)
{
    atask_yield(&self->task);
    cleanup(self, FALSE);
}

void ascheduler_cleanup(ascheduler_t* self)
{
    cleanup(self, TRUE);
}

aerror_t ascheduler_new_process(ascheduler_t* self, aprocess_t** p)
{
    aerror_t ec;
    avm_process_t* vp = avm_alloc(self->vm);
    if (!vp) return AERR_FULL;
    *p = &vp->p;
#ifdef ANY_SMP
    amutex_lock(&vp->mutex);
#endif
    ec = aprocess_init(*p, self, self->alloc, self->alloc_ud);
    if (ec != AERR_NONE) return ec;
#ifdef ANY_SMP
    amutex_unlock(&vp->mutex);
#endif
    return AERR_NONE;
}
