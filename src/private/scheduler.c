#include <any/scheduler.h>

#include <assert.h>
#include <any/errno.h>
#include <any/loader.h>
#include <any/vm.h>
#include <any/process.h>

#if 0

#define INIT_MBOX_CAP 16
#define INIT_STACK_CAP 64
#define INIT_MAX_FRAMES 16
#define INIT_MAX_CATCHES 8

AINLINE void* aalloc(ascheduler_t* self, void* old, const int32_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static void ASTDCALL dispatch(void* ud)
{
    aprocess_t* p = (aprocess_t*)ud;
    aprocess_protected_call(p);
}

int32_t ascheduler_init(
    ascheduler_t* self, avm_t* vm, adispatcher_t* runner,
    ascheduler_mbox_t oqueues[2], ascheduler_mbox_t iqueues[2],
    aalloc_t alloc, void* alloc_ud)
{
    self->runner = runner;
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->vm = vm;
#ifdef ANY_SMP
    amutex_init(&self->omutex);
    amutex_init(&self->imutex);
#endif
    self->oback = oqueues[0];
    self->iback = iqueues[0];
    self->ofront = oqueues[1];
    self->ifront = iqueues[1];
    afiber_get(&self->fiber);
    self->reduction = 0;
    self->chunks = NULL;
    self->natives = NULL;
    return AERR_NONE;
}

void ascheduler_code_change(
    ascheduler_t* self, achunk_header_t** chunks, const anative_module_t* natives)
{
    self->chunks = chunks;
    self->natives = natives;
}

int32_t ascheduler_spawn_borrowed(
    ascheduler_t* self,
    const char* module, const char* name,
    const ambox_t* mbox,
    avalue_t* stack,
    int32_t stack_cap,
    aframe_t* frames,
    int32_t max_frames)
{
    aprocess_t* p;
    int32_t err = AERR_NONE;
    p = avm_process_alloc(self->vm);
    if (!p) return AERR_FULL;
#ifdef ANY_SMP
    amutex_lock(&p->mutex);
#endif
    p->flags |= APF_BORROWED;
    p->owner = self;
    p->load = 0;
    p->mbox = *mbox;
    p->stack = stack;
    p->stack_cap = stack_cap;
    p->sp = p->stack;
    p->frames = frames;
    p->max_frames = max_frames;
    p->fp = NULL;
    p->error_jmp = NULL;
    afiber_create(&p->fiber, &dispatch, p);
    aprocess_find(p, module, name);
#ifdef ANY_SMP
    amutex_unlock(&p->mutex);
#endif
    return err;
}

int32_t ascheduler_spawn(ascheduler_t* self, const char* module, const char* name)
{
    aprocess_t* p;
    int32_t err = AERR_NONE;
    p = avm_process_alloc(self->vm);
    if (!p) return AERR_FULL;
#ifdef ANY_SMP
    amutex_lock(&p->mutex);
#endif
    p->owner = self;
    p->load = 0;
    p->mbox.cap = INIT_MBOX_CAP;
    p->mbox.msgs = (avalue_t*)
        aalloc(self, NULL, sizeof(avalue_t)*INIT_MBOX_CAP);
    p->mbox.sz = 0;
    p->stack_cap = INIT_STACK_CAP;
    p->stack = (avalue_t*)
        aalloc(self, NULL, sizeof(avalue_t)*INIT_STACK_CAP);
    p->sp = p->stack;
    p->max_frames = INIT_MAX_FRAMES;
    p->frames = (aframe_t*)
        aalloc(self, NULL, sizeof(aframe_t)*INIT_MAX_FRAMES);
    p->fp = NULL;
    p->error_jmp = NULL;
    afiber_create(&p->fiber, &dispatch, p);
    aprocess_find(p, module, name);
#ifdef ANY_SMP
    amutex_unlock(&p->mutex);
#endif
    return err;
}

#endif