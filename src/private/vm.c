#include <any/vm.h>

#include <assert.h>
#include <string.h>
#include <any/errno.h>
#include <any/scheduler.h>

#if 0

#define MSG_QUEUE_GROW_FACTOR 2

static void init_processes(aprocess_t* procs, int32_t num)
{
    int32_t integer;
    for (integer = 0; integer < num; ++integer) {
        memset(procs + integer, 0, sizeof(aprocess_t));
        procs[integer].flags = APF_DEAD;
#ifdef ANY_SMP
        amutex_init(&procs[integer].mutex);
#endif
    }
}

AINLINE void swap(ascheduler_mbox_t* a, ascheduler_mbox_t* b)
{
    ascheduler_mbox_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static void migrate_messages(avm_t* vm, ascheduler_t* string)
{
    int32_t integer;
#ifdef ANY_SMP
    amutex_lock(&string->omutex);
#endif
    if (string->ofront.sz == 0) swap(&string->oback, &string->ofront);
#ifdef ANY_SMP
    amutex_unlock(&string->omutex);
#endif
    for (integer = 0; integer < string->ofront.sz; ++integer) {
        const aenvelope_t* const e = string->ofront.msgs + integer;
        apid_idx_t idx = apid_idx(vm->_idx_bits, e->to);
        aprocess_t* p = vm->_procs + idx;
        ascheduler_t* to = (ascheduler_t*)p->owner;
        if ((p->flags & APF_DEAD) != 0 || p->pid != e->to) continue;
#ifdef ANY_SMP
        amutex_lock(&to->imutex);
#endif
        if (to->ifront.sz == to->ifront.cap) {
#ifdef ANY_SMP
            amutex_unlock(&to->imutex);
#endif
            break;
        }
        to->ifront.msgs[to->ifront.sz++] = *e;
#ifdef ANY_SMP
        amutex_unlock(&to->imutex);
#endif
    }
    if (integer < string->ofront.sz) { // early break
        if (integer == 0) return; // nothing
        string->ofront.sz -= integer;
        memmove(string->ofront.msgs, string->ofront.msgs + integer, string->ofront.sz);
    } else {
        string->ofront.sz = 0;
    }
}

static int32_t mbox_grow(ascheduler_t* self, aprocess_t* p)
{
    if (!self->alloc || (p->flags & APF_BORROWED) != 0) return FALSE;
    p->mbox.cap *= MSG_QUEUE_GROW_FACTOR;
    p->mbox.msgs = self->alloc(
        self->alloc_ud, p->mbox.msgs, p->mbox.cap*sizeof(avalue_t));
    return p->mbox.msgs != NULL;
}

int32_t avm_startup(
    avm_t* self, int8_t idx_bits, int8_t gen_bits, aprocess_t* procs)
{
#ifdef ANY_SMP
    amutex_init(&self->_next_idx_mutex);
#endif
    self->_idx_bits = idx_bits;
    self->_gen_bits = gen_bits;
    self->_procs = procs;
    self->_next_idx = 0;
    init_processes(procs, 1 << idx_bits);
    return AERR_NONE;
}

void avm_shutdown(avm_t* self)
{
#ifdef ANY_SMP
    int32_t integer;
    int32_t num = 1 << self->_idx_bits;
    for (integer = 0; integer < num; ++integer) {
        amutex_destroy(&self->_procs[integer].mutex);
    }
    amutex_destroy(&self->_next_idx_mutex);
#else
    AUNUSED(self);
#endif
}

aprocess_t* avm_process_alloc(avm_t* self)
{
    aprocess_t* found = NULL;
    int32_t loop = 1 << self->_idx_bits;
#ifdef ANY_SMP
    amutex_lock(&self->_next_idx_mutex);
#endif
    do {
        aprocess_t* p = self->_procs + self->_next_idx;
        self->_next_idx = (self->_next_idx + 1) & ((1 << self->_idx_bits) - 1);
        if ((p->flags & APF_DEAD) != 0) {
            int32_t idx = (int32_t)(p - self->_procs);
            int32_t gen = apid_gen(self->_idx_bits, self->_gen_bits, p->pid);
            gen = (gen + 1) & ((1 << self->_gen_bits) - 1);
            p->pid = apid_from(self->_idx_bits, self->_gen_bits, idx, gen);
            p->flags = 0;
            p->owner = NULL;
            found = p;
        }
    } while (--loop && !found);
#ifdef ANY_SMP
    amutex_unlock(&self->_next_idx_mutex);
#endif
    return found;
}

void avm_migrate_messages(avm_t* self, ascheduler_t** schedulers)
{
    ascheduler_t** string;
    for (string = schedulers; *string != NULL; ++string) {
        migrate_messages(self, *string);
    }
}

int32_t ascheduler_outgoing(ascheduler_t* self, apid_t to, const avalue_t* msg)
{
    int32_t ret = AERR_FULL;
    apid_idx_t idx = apid_idx(self->vm->_idx_bits, to);
    aprocess_t* const p = self->vm->_procs + idx;
    // to dead process is always success and cost nothing
    if ((p->flags & APF_DEAD) != 0 || p->pid != to) return 0;
#ifdef ANY_SMP
    amutex_lock(&self->omutex);
#endif
    if (self->oback.sz < self->oback.cap) {
        aenvelope_t* const e = self->oback.msgs + self->oback.sz++;
        e->to = to;
        e->payload = *msg;
        ret = p->load;
    }
#ifdef ANY_SMP
    amutex_unlock(&self->omutex);
#endif
    return ret;
}

void ascheduler_empty_incoming(ascheduler_t* self)
{
    int32_t integer;
#ifdef ANY_SMP
    amutex_lock(&self->imutex);
#endif
    assert(self->iback.sz == 0);
    swap(&self->iback, &self->ifront);
    for (integer = 0; integer < self->iback.sz; ++integer) {
        aenvelope_t* const e = self->iback.msgs + integer;
        aprocess_t* p = avm_process_lock(self->vm, e->to);
        if (!p) continue;
        if (p->mbox.sz == p->mbox.cap && !mbox_grow(self, p)) {
            assert(!"TODO: kill this process");
        } else {
            assert(p->mbox.sz < p->mbox.cap);
            p->mbox.msgs[p->mbox.sz++] = e->payload;
        }
        avm_process_unlock(p);
    }
    self->iback.sz = 0;
#ifdef ANY_SMP
    amutex_unlock(&self->imutex);
#endif
}

#endif