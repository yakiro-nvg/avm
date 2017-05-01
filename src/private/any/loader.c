#include <any/loader.h>

#include <string.h>
#include <any/errno.h>
#include <any/prototype.h>
#include <any/version.h>

const achunk_t CHUNK_HEADER = {
    { 0x41, 0x6E, 0x79, 0x00 },
    AVERSION_MAJOR,
    AVERSION_MINOR,
    ABIG_ENDIAN,
    sizeof(asize_t),
    sizeof(aint_t),
    sizeof(areal_t),
    sizeof(ainstruction_t),
    { 0, 0 }
};

AINLINE int32_t ptr_diff(void* a, void* b)
{
    return (int32_t)((uint8_t*)a - (uint8_t*)b);
}

static int32_t resolve_current(uint8_t* b, int32_t sz, int32_t* used)
{
    int32_t err = AERR_NONE;
    int32_t i;
    aprototype_t* const p = (aprototype_t*)(b + *used);
    acurrent_t* const c = &p->resolved;
    c->instructions = (ainstruction_t*)(((uint8_t*)(p + 1)) + p->strings_sz);
    c->constants = (avalue_t*)(c->instructions + p->num_instructions);
    c->imports = (aimport_t*)(c->constants + p->num_constants);
    c->nesteds = (aprototype_t**)(c->imports + p->num_imports);
    *used += ptr_diff(c->nesteds + p->num_nesteds, p);
    if (*used > sz) return AERR_MALFORMED;
    for (i = 0; i < p->num_nesteds; ++i) {
        c->nesteds[i] = (aprototype_t*)(b + *used);
        err = resolve_current(b, sz, used);
        if (err != AERR_NONE) return err;
    }
    return err;
}

static int32_t resolve_import(
    aprototype_t* p, achunk_t** chunks, const anative_module_t* natives)
{
    int32_t err = AERR_NONE;
    int32_t i, j;
    achunk_t** c;
    const anative_module_t* n;
    const anative_module_func_t* nf;
    const char* strs = (const char*)(p + 1);

    // for each import
    for (i = 0; i < p->num_imports; ++i) {
        aimport_t* imp = p->resolved.imports + i;
        const char* m_name = strs + imp->module;
        const char* s_name = strs + imp->name;

        // first, try each module
        for (c = chunks; *c != 0; ++c) {
            const aprototype_t* const m = (const aprototype_t*)(*c + 1);
            if (strcmp(any_pt_symbol(m), m_name) != 0) continue;
            // for each module function
            for (j = 0; j < m->num_nesteds; ++j) {
                if (strcmp(any_pt_symbol(m->resolved.nesteds[j]), s_name) == 0) {
                    imp->resolved.tag.tag = ATB_FUNCTION;
                    imp->resolved.tag.variant = AVTF_PURE;
                    imp->resolved.v.mf = m->resolved.nesteds[j];
                    goto found;
                }
            }
        }

        // still not found, try each native module
        if (natives) {
            for (n = natives; n->name != NULL; ++n) {
                if (strcmp(n->name, m_name) != 0) continue;
                // for each native module function
                for (nf = n->funcs; nf->name != NULL; ++nf) {
                    if (strcmp(nf->name, s_name) == 0) {
                        imp->resolved.tag.tag = ATB_FUNCTION;
                        imp->resolved.tag.variant = AVTF_NATIVE;
                        imp->resolved.v.f = nf->func;
                        goto found;
                    }
                }
            }
        }

        return AERR_UNRESOLVED;
    found:
        continue;
    }

    // recursive for children
    for (i = 0; i < p->num_nesteds; ++i) {
        err = resolve_import(p->resolved.nesteds[i], chunks, natives);
        if (err != AERR_NONE) return err;
    }

    return err;
}

int32_t any_link(
    achunk_t** chunks, int32_t* sizes, const anative_module_t* natives)
{
    int32_t err = AERR_NONE;
    achunk_t** c; 
    int32_t* sz;
    
    // resolve acurrent_t pointers
    for (c = chunks, sz = sizes; *c != NULL; ++c, ++sz) {
        int32_t off;
        if (*sz < (int32_t)sizeof(achunk_t) || 
            memcmp(*c, &CHUNK_HEADER, sizeof(achunk_t)) != 0)
            return AERR_MALFORMED;
        off = sizeof(achunk_t);
        err = resolve_current((uint8_t*)*c, *sz, &off);
        if (err != AERR_NONE) return err;
    }

    // resolve imports
    for (c = chunks; *c != NULL; ++c) {
        err = resolve_import((aprototype_t*)(*c + 1), chunks, natives);
        if (err != AERR_NONE) return err;
    }

    return AERR_NONE;
}