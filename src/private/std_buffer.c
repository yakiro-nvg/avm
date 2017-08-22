#include <any/std_buffer.h>

aint_t agc_fixed_buffer_new(agc_t* gc, aint_t sz, avalue_t* v)
{
    aint_t oi = agc_alloc(gc, AVT_FIXED_BUFFER, sz);
    if (oi < 0) return oi;
    av_collectable(v, AVT_FIXED_BUFFER, oi);
    return AERR_NONE;
}

/// Create a new buffer.
aint_t agc_buffer_new(agc_t* gc, aint_t cap, avalue_t* v)
{
    aint_t oi = agc_alloc(gc, AVT_BUFFER, sizeof(agc_buffer_t));
    if (oi < 0) return oi;
    else {
        agc_buffer_t* o = AGC_CAST(agc_buffer_t, gc, oi);
        aint_t bi = agc_fixed_buffer_new(gc, cap, &o->buff);
        if (bi < 0) return bi;
        o->cap = cap;
        o->sz = 0;
        av_collectable(v, AVT_BUFFER, oi);
        return AERR_NONE;
    }
}