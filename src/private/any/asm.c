/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/asm.h>

#ifdef ANY_TOOL

#include <string.h>
#include <assert.h>
#include <any/string_table.h>
#include <any/version.h>
#include <any/errno.h>

#define GROW_FACTOR 2
#define INIT_ST_BYTES 128
#define INIT_ST_SSIZE 16
#define INIT_SLOT_COUNT 4
#define INIT_MAX_INSTRUCTIONS 32
#define INIT_MAX_CONSTANTS 16
#define INIT_MAX_IMPORTS 16
#define INIT_MAX_NESTEDS 16

static const uint8_t CHUNK_HEADER[] = {
    0x41, 0x6E, 0x79, 0x00, // signature
    AVERSION_MAJOR*0x10 + AVERSION_MINOR, // version
    ABIG_ENDIAN,
    sizeof(asize_t),
    sizeof(aint_t),
    sizeof(areal_t),
    sizeof(ainstruction_t),
    0, 0 // reserved
};
ASTATIC_ASSERT(sizeof(CHUNK_HEADER) == 12);

static const aasm_reserve_t DEFAULT_PROTO_SZ = {
    INIT_MAX_INSTRUCTIONS,
    INIT_MAX_CONSTANTS,
    INIT_MAX_IMPORTS,
    INIT_MAX_NESTEDS
};

AINLINE void* arealloc(aasm_t* self, void* old, const int32_t sz)
{
    return self->realloc(self->realloc_ud, old, sz);
}

AINLINE int32_t required_size(const aasm_reserve_t* sz)
{
    return sizeof(aasm_prototype_t) +
        sz->max_instructions * sizeof(ainstruction_t) +
        sz->max_constants * sizeof(aconstant_t) +
        sz->max_imports * sizeof(aimport_t) +
        sz->max_nesteds * sizeof(int32_t);
}

AINLINE int32_t prototype_size(const aasm_prototype_t* p)
{
    aasm_reserve_t sz;
    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;
    return required_size(&sz);
}

AINLINE ainstruction_t* instructions_of(aasm_prototype_t* p)
{
    return (ainstruction_t*)(p + 1);
}

AINLINE const ainstruction_t* instructions_of_const(const aasm_prototype_t* p)
{
    return (const ainstruction_t*)(p + 1);
}

AINLINE aconstant_t* constants_of(aasm_prototype_t* p)
{
    return (aconstant_t*)(instructions_of(p) + p->max_instructions);
}

AINLINE const aconstant_t* constants_of_const(const aasm_prototype_t* p)
{
    return (const aconstant_t*)(instructions_of_const(p) + p->max_instructions);
}

AINLINE aimport_t* imports_of(aasm_prototype_t* p)
{
    return (aimport_t*)(constants_of(p) + p->max_constants);
}

AINLINE const aimport_t* imports_of_const(const aasm_prototype_t* p)
{
    return (const aimport_t*)(constants_of_const(p) + p->max_constants);
}

AINLINE int32_t* nesteds_of(aasm_prototype_t* p)
{
    return (int32_t*)(imports_of(p) + p->max_imports);
}

AINLINE const int32_t* nesteds_of_const(const aasm_prototype_t* p)
{
    return (const int32_t*)(imports_of_const(p) + p->max_imports);
}

AINLINE acurrent_t resolve(aasm_prototype_t* p)
{
    acurrent_t c;
    c.instructions = instructions_of(p);
    c.constants = constants_of(p);
    c.imports = imports_of(p);
    c.nesteds = nesteds_of(p);
    return c;
}

static void gc(
    aasm_t* self, uint8_t* new_buff, int32_t* offset, const int32_t parent)
{
    int32_t i;

    // copy referenced prototypes
    aasm_prototype_t* p = any_asm_prototype_at(self, parent);
    int32_t p_sz = prototype_size(p);
    memcpy(new_buff + *offset, p, p_sz);

    // safe to update parent slot here
    // there are no more dereferences to that during gc
    self->_slots[parent] = *offset;
    *offset += p_sz;

    // recursive for children
    for (i = 0; i < p->num_nesteds; ++i) {
        int32_t n = nesteds_of(p)[i];
        gc(self, new_buff, offset, n);
    }
}

static void grow_buff(aasm_t* self, int32_t expected)
{
    uint8_t* new_buff = NULL;

    expected += self->_buff_size;
    if (expected <= self->_buff_capacity) return; 
    
    assert(self->_num_slots > 0 && "don't grow for root");
    while (self->_buff_capacity < expected) self->_buff_capacity *= GROW_FACTOR;
    new_buff = (uint8_t*)arealloc(self, NULL, self->_buff_capacity);

    self->_buff_size = 0;
    gc(self, new_buff, &self->_buff_size, 0);
    arealloc(self, self->_buff, 0);
    self->_buff = new_buff;
}

// Returns a new prototype offset, 
// and guarantee that at least 1 free slot available.
static int32_t new_prototype(aasm_t* self, const aasm_reserve_t* sz)
{
    int32_t poff;
    aasm_prototype_t* p = NULL;
    const int32_t rsz = required_size(sz);
    grow_buff(self, rsz);
    poff = self->_buff_size;

    if (self->_num_slots == self->_max_slots) {
        self->_max_slots *= GROW_FACTOR;
        self->_slots = arealloc(
            self, self->_slots, self->_max_slots * sizeof(int32_t));
    }

    p = (aasm_prototype_t*)(self->_buff + poff);
    memset(p, 0, rsz);
    p->max_instructions = sz->max_instructions;
    p->max_constants = sz->max_constants;
    p->max_imports = sz->max_imports;
    p->max_nesteds = sz->max_nesteds;
    self->_buff_size += rsz;

    return poff;
}

AINLINE int32_t new_prototype_default_size(aasm_t* self)
{
    return new_prototype(self, &DEFAULT_PROTO_SZ);
}

AINLINE aasm_ctx_t* ctx(aasm_t* self)
{
    return self->_context + self->_nested_level;
}

// Compute required size for strings.
AINLINE int32_t compute_strings_sz(
    const astring_table_t* st,
    const aasm_prototype_t* p,
    const aconstant_t* constants,
    const aimport_t* imports)
{
    int32_t i;
    int32_t sz;

    sz = sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(st, p->source_name));
    sz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(st, p->module_name));
    sz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(st, p->symbol));

    for (i = 0; i < p->num_constants; ++i) {
        if (constants[i].b.type != ACT_STRING) continue;
        sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(st, constants[i].s.ref));
    }

    for (i = 0; i < p->num_imports; ++i) {
        sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(st, imports[i].module));
        sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(st, imports[i].name));
    }

    return sz;
}

static int32_t compute_chunk_body_size(const aasm_t* self, int32_t parent)
{
    int32_t i;

    aasm_prototype_t* const p = any_asm_prototype_at((aasm_t*)self, parent);
    const acurrent_t c = resolve(p);

    int32_t psz = sizeof(aprototype_t) +
        compute_strings_sz(self->st, p, c.constants, c.imports) +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_nesteds * sizeof(int32_t);

    for (i = 0; i < p->num_nesteds; ++i) {
        psz += compute_chunk_body_size(self, nesteds_of(p)[i]);
    }

    return psz;
}

AINLINE acurrent_t cp_resolve(const aprototype_t* p, uint32_t strings_sz)
{
    acurrent_t c;
    c.instructions = (ainstruction_t*)(((uint8_t*)(p + 1)) + strings_sz);
    c.constants = (aconstant_t*)(c.instructions + p->num_instructions);
    c.imports = (aimport_t*)(c.constants + p->num_constants);
    c.nesteds = (int32_t*)(c.imports + p->num_imports);
    return c;
}

AINLINE int32_t chunk_add_str(
    const aasm_t* self, aprototype_t* header, astring_ref_t s)
{
    const uint32_t hash = any_st_to_hash(self->st, s);
    const char* const str = any_st_to_string(self->st, s);
    const int32_t length = strlen(str);
    const int32_t off = header->strings_sz;
    uint8_t* const b = ((uint8_t*)header) + sizeof(aprototype_t) + off;
    memcpy(b, &hash, sizeof(uint32_t));
    memcpy(b + sizeof(uint32_t), str, length + 1);
    header->strings_sz += (uint16_t)(sizeof(uint32_t) + length + 1);
    return off + sizeof(uint32_t);
}

// set header fields
AINLINE void set_headers(
    const aasm_t* self, aprototype_t* header, const aasm_prototype_t* p)
{
    memset(header, 0, sizeof(*header));
    header->num_instructions = p->num_instructions;
    header->num_nesteds = p->num_nesteds;
    header->num_upvalues = p->num_upvalues;
    header->num_arguments = p->num_arguments;
    header->num_constants = p->num_constants;
    header->num_local_vars = p->num_local_vars;
    header->num_imports = p->num_imports;
    header->source_name = chunk_add_str(self, header, p->source_name);
    header->module_name = chunk_add_str(self, header, p->module_name);
    header->symbol = chunk_add_str(self, header, p->symbol);
}

static void save_chunk(aasm_t* self, int32_t parent);

static void copy_prototype(
    aasm_t* self, 
    const aasm_prototype_t* p, 
    const acurrent_t* c, 
    aprototype_t* const header, 
    int32_t strings_sz)
{
    int32_t i;
    const acurrent_t cp = cp_resolve(header, strings_sz);

    // add instructions
    memcpy(
        cp.instructions,
        c->instructions,
        p->num_instructions * sizeof(ainstruction_t));

    // add constants
    for (i = 0; i < p->num_constants; ++i) {
        cp.constants[i] = c->constants[i];
        if (c->constants[i].b.type != ACT_STRING) continue;
        cp.constants[i].s.ref = chunk_add_str(
            self, header, c->constants[i].s.ref);
    }

    // add imports
    for (i = 0; i < p->num_imports; ++i) {
        cp.imports[i].module = chunk_add_str(self, header, c->imports[i].module);
        cp.imports[i].name = chunk_add_str(self, header, c->imports[i].name);
    }

    assert(header->strings_sz == strings_sz);

    // add nesteds
    for (i = 0; i < p->num_nesteds; ++i) {
        cp.nesteds[i] = self->chunk_size;
        save_chunk(self, c->nesteds[i]);
    }
}

static void save_chunk(aasm_t* self, int32_t parent)
{
    const aasm_prototype_t* const p = any_asm_prototype_at(self, parent);
    const acurrent_t c = resolve((aasm_prototype_t*)p);
    int32_t str_sz = compute_strings_sz(self->st, p, c.constants, c.imports);
    int32_t psz = sizeof(aprototype_t) +
        str_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_nesteds * sizeof(int32_t);
    aprototype_t* const header = (aprototype_t*)(
        ((uint8_t*)self->chunk) + self->chunk_size);
    set_headers(self, header, p);
    self->chunk_size += psz;
    copy_prototype(self, p, &c, header, str_sz);
}

AINLINE const char* cp_string(const aprototype_t* p, astring_ref_t s)
{
    return ((const char*)p) + sizeof(aprototype_t) + s;
}

static void load_chunk(
    aasm_t* self, const achunk_t* input, int32_t* offset)
{
    int32_t i;
    aasm_prototype_t* ap = NULL;
    const aprototype_t* const p = (const aprototype_t*)(
        ((const uint8_t*)input) + *offset);
    const acurrent_t cp = cp_resolve(p, p->strings_sz);
    aasm_reserve_t sz;

    sz.max_instructions = p->num_instructions;
    sz.max_constants = p->num_constants;
    sz.max_imports = p->num_imports;
    sz.max_nesteds = p->num_nesteds;

    any_asm_reserve(self, &sz);
    ap = any_asm_prototype(self);

    ap->source_name = any_asm_string_to_ref(self, cp_string(p, p->source_name));
    ap->module_name = any_asm_string_to_ref(self, cp_string(p, p->module_name));
    ap->symbol = any_asm_string_to_ref(self, cp_string(p, p->symbol));
    
    ap->num_upvalues = p->num_upvalues;
    ap->num_arguments = p->num_arguments;
    ap->num_local_vars = p->num_local_vars;

    memcpy(
        instructions_of(ap),
        cp.instructions,
        p->num_instructions * sizeof(ainstruction_t));
    ap->num_instructions = p->num_instructions;

    for (i = 0; i < p->num_constants; ++i) {
        aconstant_t constant = cp.constants[i];
        if (constant.b.type == ACT_STRING) {
            constant.s.ref = any_asm_string_to_ref(
                self, cp_string(p, constant.s.ref));
        }
        any_asm_add_constant(self, constant);
    }

    for (i = 0; i < p->num_imports; ++i) {
        aimport_t import = cp.imports[i];
        import.module = any_asm_string_to_ref(self, cp_string(p, import.module));
        import.name = any_asm_string_to_ref(self, cp_string(p, import.name));
        any_asm_add_import(self, import);
    }

    *offset += 
        sizeof(aprototype_t) +
        p->strings_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_nesteds * sizeof(int32_t);

    for (i = 0; i < p->num_nesteds; ++i) {
        any_asm_push(self);
        load_chunk(self, input, offset);
        any_asm_pop(self);
    }
}

AINLINE int32_t push_unsafe(aasm_t* self)
{
    const int32_t np_off = new_prototype_default_size(self);
    const int32_t np = self->_num_slots++;
    aasm_prototype_t* const p = any_asm_prototype(self);

    self->_slots[np] = np_off;
    assert(p->num_nesteds < p->max_nesteds);
    nesteds_of(p)[p->num_nesteds] = np;

    assert(self->_nested_level < ANY_ASM_MAX_NESTED_LEVEL);
    self->_nested_level++;
    self->_context[self->_nested_level].slot = np;
    self->_context[self->_nested_level].idx = p->num_nesteds;

    return p->num_nesteds++;
}

void any_asm_init(aasm_t* self, arealloc_t realloc, void* realloc_ud)
{
    memset(self, 0, sizeof(*self));
    self->realloc = realloc;
    self->realloc_ud = realloc_ud;
}

int32_t any_asm_load(aasm_t* self, const achunk_t* input)
{
    any_asm_cleanup(self);

    self->st = (astring_table_t*)arealloc(self, NULL, INIT_ST_BYTES);
    any_st_init(self->st, INIT_ST_BYTES, INIT_ST_SSIZE);

    self->_max_slots = INIT_SLOT_COUNT;
    self->_slots = (int32_t*)arealloc(
        self, NULL, self->_max_slots * sizeof(int32_t));

    self->_buff_capacity = self->_max_slots * required_size(&DEFAULT_PROTO_SZ);
    self->_buff = (uint8_t*)arealloc(self, NULL, self->_buff_capacity);

    self->_slots[self->_num_slots] = new_prototype_default_size(self);
    self->_num_slots = 1;

    if (!input) return AERR_NONE;
    if (memcmp(CHUNK_HEADER, input, sizeof(CHUNK_HEADER)) == 0) {
        int32_t offset = sizeof(CHUNK_HEADER);
        load_chunk(self, input, &offset);
        return AERR_NONE;
    } else {
        return AERR_MALFORMED;
    }
}

void any_asm_save(aasm_t* self)
{
    const int32_t sz = compute_chunk_body_size(self, 0) + sizeof(CHUNK_HEADER);
    self->chunk_size = 0;
    if (self->_chunk_capacity < sz) {
        arealloc(self, self->chunk, 0);
        self->chunk = (achunk_t*)arealloc(self, NULL, sz);
        self->_chunk_capacity = sz;
    }

    memcpy(self->chunk->header, CHUNK_HEADER, sizeof(CHUNK_HEADER));
    self->chunk_size += sizeof(CHUNK_HEADER);

    save_chunk(self, 0);
    assert(self->chunk_size == sz);
}

void any_asm_cleanup(aasm_t* self)
{
    arealloc(self, self->st, 0);
    self->st = NULL;

    arealloc(self, self->_slots, 0);
    self->_slots = NULL;
    self->_num_slots = 0;
    self->_max_slots = 0;

    arealloc(self, self->_buff, 0);
    self->_buff = NULL;
    self->_buff_size = 0;
    self->_buff_capacity = 0;

    memset(self->_context, 0, sizeof(self->_context));
    self->_nested_level = 0;

    arealloc(self, self->chunk, 0);
    self->chunk_size = 0;
    self->_chunk_capacity = 0;
}

int32_t any_asm_emit(aasm_t* self, ainstruction_t instruction)
{
    aasm_prototype_t* p = any_asm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions * GROW_FACTOR;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_instructions == p->max_instructions) {
        any_asm_reserve(self, &sz);
        p = any_asm_prototype(self);
    }

    assert(p->num_instructions < p->max_instructions);
    instructions_of(p)[p->num_instructions] = instruction;
    return p->num_instructions++;
}

int32_t any_asm_add_constant(aasm_t* self, aconstant_t constant)
{
    aasm_prototype_t* p = any_asm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants * GROW_FACTOR;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_constants == p->max_constants) {
        any_asm_reserve(self, &sz);
        p = any_asm_prototype(self);
    }

    assert(p->num_constants < p->max_constants);
    constants_of(p)[p->num_constants] = constant;
    return p->num_constants++;
}

int32_t any_asm_add_import(aasm_t* self, aimport_t import)
{
    aasm_prototype_t* p = any_asm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports * GROW_FACTOR;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_imports == p->max_imports) {
        any_asm_reserve(self, &sz);
        p = any_asm_prototype(self);
    }

    assert(p->num_imports < p->max_imports);
    imports_of(p)[p->num_imports] = import;
    return p->num_imports++;
}

int32_t any_asm_module_push(aasm_t* self, const char* module, const char* name)
{
    aasm_prototype_t* p = NULL;
    int32_t idx;
    astring_ref_t module_name = any_asm_string_to_ref(self, module);
    astring_ref_t symbol_name = any_asm_string_to_ref(self, name);
    assert(self->_nested_level == 0);
    idx = any_asm_push(self);
    p = any_asm_prototype(self);
    p->module_name = module_name;
    p->symbol = symbol_name;
    return idx;
}

int32_t any_asm_push(aasm_t* self)
{
    const aasm_prototype_t* p = any_asm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds * GROW_FACTOR;

    if (p->num_nesteds == p->max_nesteds) {
        any_asm_reserve(self, &sz);
        p = NULL;
    }

    return push_unsafe(self);
}

void any_asm_open(aasm_t* self, int32_t idx)
{
    aasm_prototype_t* const p = any_asm_prototype(self);
    assert(idx < p->num_nesteds);
    assert(self->_nested_level < ANY_ASM_MAX_NESTED_LEVEL);
    self->_nested_level++;
    self->_context[self->_nested_level].slot = nesteds_of(p)[idx];
    self->_context[self->_nested_level].idx = idx;
}

int32_t any_asm_pop(aasm_t* self)
{
    int32_t idx;
    assert(self->_nested_level > 0);
    idx = self->_context[self->_nested_level].idx;
    self->_nested_level--;
    return idx;
}

astring_ref_t any_asm_string_to_ref(aasm_t* self, const char* s)
{
    astring_ref_t ref = AERR_FULL;
    do {
        ref = any_st_to_ref(self->st, s);
        if (ref == AERR_FULL) {
            const int32_t new_cap = self->st->allocated_bytes * GROW_FACTOR;
            self->st = (astring_table_t*)arealloc(self, self->st, new_cap);
            any_st_grow(self->st, new_cap);
        } else {
            break;
        }
    } while (TRUE);
    return ref;
}

void any_asm_reserve(aasm_t* self, const aasm_reserve_t* sz)
{
    {
        const aasm_prototype_t* const p = any_asm_prototype(self);
        if (p->max_instructions >= sz->max_instructions &&
            p->max_constants >= sz->max_constants &&
            p->max_imports >= sz->max_imports &&
            p->max_nesteds >= sz->max_nesteds) return;
    }
    {
        const int32_t np_off = new_prototype(self, sz);
        aasm_prototype_t* const np = (aasm_prototype_t*)(self->_buff + np_off);
        const aasm_prototype_t* const cp = any_asm_prototype(self);

        np->num_instructions = cp->num_instructions;
        np->num_constants = cp->num_constants;
        np->num_imports = cp->num_imports;
        np->num_nesteds = cp->num_nesteds;

        memcpy(
            instructions_of(np),
            instructions_of_const(cp),
            cp->num_instructions * sizeof(ainstruction_t));
        memcpy(
            constants_of(np),
            constants_of_const(cp),
            cp->num_constants * sizeof(aconstant_t));
        memcpy(
            imports_of(np),
            imports_of_const(cp),
            cp->num_imports * sizeof(aimport_t));
        memcpy(
            nesteds_of(np),
            nesteds_of_const(cp),
            cp->num_nesteds * sizeof(int32_t));

        self->_slots[ctx(self)->slot] = np_off;
    }
}

aasm_prototype_t* any_asm_prototype(aasm_t* self)
{
    return any_asm_prototype_at(self, ctx(self)->slot);
}

acurrent_t any_asm_resolve(aasm_t* self)
{
    return resolve(any_asm_prototype(self));
}

#else // ANY_TOOL
static char non_empty_unit;
#endif