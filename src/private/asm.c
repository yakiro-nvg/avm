/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/asm.h>

#include <any/string_table.h>

#define GROW_FACTOR 2
#define INIT_ST_BYTES 128
#define INIT_ST_SSIZE 16
#define INIT_SLOT_COUNT 4
#define INIT_MAX_INSTRUCTIONS 32
#define INIT_MAX_CONSTANTS 16
#define INIT_MAX_IMPORTS 16
#define INIT_MAX_NESTEDS 16

extern const achunk_header_t CHUNK_HEADER;

static const aasm_reserve_t DEFAULT_PROTO_SZ = {
    INIT_MAX_INSTRUCTIONS,
    INIT_MAX_CONSTANTS,
    INIT_MAX_IMPORTS,
    INIT_MAX_NESTEDS
};

typedef struct resolved_proto_s {
    ainstruction_t* instructions;
    aconstant_t* constants;
    aimport_t* imports;
    aint_t* source_lines;
} resolved_proto_t;

static inline void*
aalloc(
    aasm_t* self, void* old, const aint_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static inline aint_t
required_size(
    const aasm_reserve_t* sz)
{
    return sizeof(aasm_prototype_t) +
        sz->max_instructions * sizeof(ainstruction_t) +
        sz->max_constants * sizeof(aconstant_t) +
        sz->max_imports * sizeof(aimport_t) +
        sz->max_instructions * sizeof(aint_t) +
        sz->max_nesteds * sizeof(aint_t);
}

static inline aint_t
prototype_size(
    const aasm_prototype_t* p)
{
    aasm_reserve_t sz;
    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;
    return required_size(&sz);
}

static inline ainstruction_t*
instructions_of(
    aasm_prototype_t* p)
{
    return (ainstruction_t*)(p + 1);
}

static inline const ainstruction_t*
instructions_of_const(
    const aasm_prototype_t* p)
{
    return (const ainstruction_t*)(p + 1);
}

static inline aconstant_t*
constants_of(
    aasm_prototype_t* p)
{
    return (aconstant_t*)(instructions_of(p) + p->max_instructions);
}

static inline const aconstant_t*
constants_of_const(
    const aasm_prototype_t* p)
{
    return (const aconstant_t*)(
        instructions_of_const(p) + p->max_instructions);
}

static inline aimport_t*
imports_of(
    aasm_prototype_t* p)
{
    return (aimport_t*)(constants_of(p) + p->max_constants);
}

static inline const aimport_t*
imports_of_const(
    const aasm_prototype_t* p)
{
    return (const aimport_t*)(constants_of_const(p) + p->max_constants);
}

static inline aint_t*
source_lines_of(
    aasm_prototype_t* p)
{
    return (aint_t*)(imports_of(p) + p->max_imports);
}

static inline const aint_t*
source_lines_of_const(
    const aasm_prototype_t* p)
{
    return (const aint_t*)(imports_of_const(p) + p->max_imports);
}

static inline aint_t*
nesteds_of(
    aasm_prototype_t* p)
{
    return (aint_t*)(source_lines_of(p) + p->max_instructions);
}

static inline const aint_t*
nesteds_of_const(
    const aasm_prototype_t* p)
{
    return (const aint_t*)(source_lines_of_const(p) + p->max_instructions);
}

static inline aasm_current_t
resolve(
    aasm_prototype_t* p)
{
    aasm_current_t c;
    c.instructions = instructions_of(p);
    c.constants = constants_of(p);
    c.imports = imports_of(p);
    c.source_lines = source_lines_of(p);
    c.nesteds = nesteds_of(p);
    return c;
}

static void
gc(
    aasm_t* self, uint8_t* new_buff, aint_t* offset, const aint_t parent)
{
    aint_t i;

    // copy referenced prototypes
    aasm_prototype_t* p = aasm_prototype_at(self, parent);
    aint_t p_sz = prototype_size(p);
    memcpy(new_buff + *offset, p, (size_t)p_sz);

    // safe to update parent slot here
    // there are no more dereferences to that during gc
    self->_slots[parent] = *offset;
    *offset += p_sz;

    // recursive for children
    for (i = 0; i < p->num_nesteds; ++i) {
        aint_t n = nesteds_of(p)[i];
        gc(self, new_buff, offset, n);
    }
}

static void
grow_buff(
    aasm_t* self, aint_t expected)
{
    uint8_t* new_buff = NULL;

    expected += self->_buff_size;
    if (expected <= self->_buff_capacity) return;

    assert(self->_num_slots > 0 && "don't grow for root");
    while (self->_buff_capacity < expected) self->_buff_capacity *= GROW_FACTOR;
    new_buff = (uint8_t*)aalloc(self, NULL, self->_buff_capacity);

    self->_buff_size = 0;
    gc(self, new_buff, &self->_buff_size, 0);
    aalloc(self, self->_buff, 0);
    self->_buff = new_buff;
}

// returns a new prototype offset,
// and guarantee that at least 1 free slot is available.
static aint_t
new_prototype(
    aasm_t* self, const aasm_reserve_t* sz)
{
    aint_t poff;
    aasm_prototype_t* p = NULL;
    const aint_t rsz = required_size(sz);
    grow_buff(self, rsz);
    poff = self->_buff_size;

    if (self->_num_slots == self->_max_slots) {
        self->_max_slots *= GROW_FACTOR;
        self->_slots = aalloc(
            self, self->_slots, self->_max_slots * sizeof(aint_t));
    }

    p = (aasm_prototype_t*)(self->_buff + poff);
    memset(p, 0, (size_t)rsz);
    p->max_instructions = sz->max_instructions;
    p->max_constants = sz->max_constants;
    p->max_imports = sz->max_imports;
    p->max_nesteds = sz->max_nesteds;
    self->_buff_size += rsz;

    return poff;
}

static inline aint_t
new_prototype_default_size(
    aasm_t* self)
{
    return new_prototype(self, &DEFAULT_PROTO_SZ);
}

static inline aasm_ctx_t*
ctx(
    aasm_t* self)
{
    return self->_context + self->_nested_level;
}

// compute the required size for strings.
static aint_t
compute_strings_sz(
    const astring_table_t* st,
    const aasm_prototype_t* p,
    const aconstant_t* constants,
    const aimport_t* imports)
{
    aint_t i;
    aint_t sz;

    sz  = (aint_t)sizeof(uint32_t) + 1 +
        (aint_t)strlen(astring_table_to_string(st, p->source));
    sz += (aint_t)sizeof(uint32_t) + 1 +
        (aint_t)strlen(astring_table_to_string(st, p->symbol));

    for (i = 0; i < p->num_constants; ++i) {
        if (constants[i].type != ACT_STRING) continue;
        sz += (aint_t)sizeof(uint32_t) + 1 +
            (aint_t)strlen(astring_table_to_string(st, constants[i].string));
    }

    for (i = 0; i < p->num_imports; ++i) {
        sz += (aint_t)sizeof(uint32_t) + 1 +
            (aint_t)strlen(
                astring_table_to_string(st, imports[i].module));
        sz += (aint_t)sizeof(uint32_t) + 1 +
            (aint_t)strlen(astring_table_to_string(st, imports[i].name));
    }

    sz = AALIGN_FORWARD(sz, 8);

    return sz;
}

static void
compute_chunk_body_size(
    const aasm_t* self, aint_t parent, aint_t* psz)
{
    aint_t i;

    aasm_prototype_t* const p = aasm_prototype_at((aasm_t*)self, parent);
    const aasm_current_t c = resolve(p);

    *psz += sizeof(aprototype_header_t) +
        compute_strings_sz(self->st, p, c.constants, c.imports) +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_instructions * sizeof(aint_t);

    for (i = 0; i < p->num_nesteds; ++i) {
        compute_chunk_body_size(self, nesteds_of(p)[i], psz);
    }
}

static inline resolved_proto_t
rp_resolve(
    const aprototype_header_t* p, aint_t strings_sz)
{
    resolved_proto_t c;
    c.instructions = (ainstruction_t*)(((uint8_t*)(p + 1)) + strings_sz);
    c.constants = (aconstant_t*)(c.instructions + p->num_instructions);
    c.imports = (aimport_t*)(c.constants + p->num_constants);
    c.source_lines = (aint_t*)(c.imports + p->num_imports);
    return c;
}

static aint_t
chunk_add_str(
    const aasm_t* self, aprototype_header_t* header, aint_t string)
{
    const uint32_t hash = astring_table_to_hash(self->st, string);
    const char* const str = astring_table_to_string(self->st, string);
    const aint_t length = (aint_t)strlen(str);
    const aint_t off = header->strings_sz;
    uint8_t* const b = ((uint8_t*)header) + sizeof(aprototype_header_t) + off;
    memcpy(b, &hash, sizeof(uint32_t));
    memcpy(b + sizeof(uint32_t), str, (size_t)length + 1);
    header->strings_sz += sizeof(uint32_t) + length + 1;
    return off + sizeof(uint32_t);
}

// set header fields
static void
set_headers(
    const aasm_t* self, aprototype_header_t* header, const aasm_prototype_t* p)
{
    memset(header, 0, sizeof(*header));
    header->num_local_vars = p->num_local_vars;
    header->num_instructions = p->num_instructions;
    header->num_nesteds = p->num_nesteds;
    header->num_constants = p->num_constants;
    header->num_imports = p->num_imports;
    header->source = chunk_add_str(self, header, p->source);
    header->symbol = chunk_add_str(self, header, p->symbol);
}

static inline aconstant_t
to_chunk(
    aconstant_t c, aasm_t* self, aprototype_header_t* header)
{
    aconstant_t v = c;
    if (c.type == ACT_STRING) {
        v.string = chunk_add_str(self, header, c.string);
    }
    return v;
}

static void
save_chunk(
    aasm_t* self, aint_t parent);

static void
copy_prototype(
    aasm_t* self,
    const aasm_prototype_t* p,
    const aasm_current_t* c,
    aprototype_header_t* header,
    aint_t strings_sz)
{
    aint_t i;
    const resolved_proto_t rp = rp_resolve(header, strings_sz);

    // add instructions
    memcpy(
        rp.instructions,
        c->instructions,
        (size_t)p->num_instructions * sizeof(ainstruction_t));

    // add constants
    for (i = 0; i < p->num_constants; ++i) {
        rp.constants[i] = to_chunk(c->constants[i], self, header);
    }

    // add imports
    for (i = 0; i < p->num_imports; ++i) {
        rp.imports[i].module = chunk_add_str(
            self, header, c->imports[i].module);
        rp.imports[i].name = chunk_add_str(
            self, header, c->imports[i].name);
    }

    // add source lines
    memcpy(
        rp.source_lines,
        c->source_lines,
        (size_t)p->num_instructions * sizeof(aint_t));

    header->strings_sz = AALIGN_FORWARD(header->strings_sz, 8);
    assert(header->strings_sz == strings_sz);

    // add nesteds
    for (i = 0; i < p->num_nesteds; ++i) {
        save_chunk(self, c->nesteds[i]);
    }
}

static void
save_chunk(
    aasm_t* self, aint_t parent)
{
    const aasm_prototype_t* const p = aasm_prototype_at(self, parent);
    const aasm_current_t c = resolve((aasm_prototype_t*)p);
    aint_t str_sz = compute_strings_sz(self->st, p, c.constants, c.imports);
    aint_t psz = sizeof(aprototype_header_t) +
        str_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_instructions * sizeof(aint_t);
    aprototype_header_t* const header = (aprototype_header_t*)(
        ((uint8_t*)self->chunk) + self->chunk_size);
    set_headers(self, header, p);
    self->chunk_size += psz;
    copy_prototype(self, p, &c, header, str_sz);
}

static inline const char*
rp_string(
    const aprototype_header_t* p, aint_t string)
{
    return ((const char*)p) + sizeof(aprototype_header_t) + string;
}

static aconstant_t
from_chunk(
    aconstant_t v, aasm_t* self, const aprototype_header_t* p)
{
    aconstant_t c = v;
    if (v.type == ACT_STRING) {
        c.string = aasm_string_to_ref(self, rp_string(p, v.string));
    }
    return c;
}

static aerror_t
load_chunk(
    aasm_t* self, const achunk_header_t* input, aint_t* offset)
{
    aerror_t ec = AERR_NONE;
    aint_t i;
    aasm_prototype_t* ap;
    const aprototype_header_t* const p = (const aprototype_header_t*)(
        ((const uint8_t*)input) + *offset);
    const resolved_proto_t rp = rp_resolve(p, p->strings_sz);
    aasm_reserve_t sz;

    sz.max_instructions = p->num_instructions;
    sz.max_constants = p->num_constants;
    sz.max_imports = p->num_imports;
    sz.max_nesteds = p->num_nesteds;

    aasm_reserve(self, &sz);
    ap = aasm_prototype(self);

    ap->source = aasm_string_to_ref(self, rp_string(p, p->source));
    ap->symbol = aasm_string_to_ref(self, rp_string(p, p->symbol));

    ap->num_local_vars = p->num_local_vars;

    // load instructions
    memcpy(
        instructions_of(ap),
        rp.instructions,
        (size_t)p->num_instructions * sizeof(ainstruction_t));
    ap->num_instructions = p->num_instructions;

    // load constants
    for (i = 0; i < p->num_constants; ++i) {
        aasm_add_constant(self, from_chunk(rp.constants[i], self, p));
    }

    // load imports
    for (i = 0; i < p->num_imports; ++i) {
        aimport_t imp = rp.imports[i];
        aasm_add_import(
            self, rp_string(p, imp.module), rp_string(p, imp.name));
    }

    // load source lines
    memcpy(
        source_lines_of(ap),
        rp.source_lines,
        (size_t)p->num_instructions * sizeof(aint_t));

    *offset +=
        sizeof(aprototype_header_t) +
        p->strings_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_instructions * sizeof(aint_t);

    for (i = 0; i < p->num_nesteds; ++i) {
        aasm_push(self);
        ec = load_chunk(self, input, offset);
        if (ec != AERR_NONE) return ec;
        aasm_pop(self);
    }

    return ec;
}

static aint_t
push_unsafe(
    aasm_t* self)
{
    const aint_t np_off = new_prototype_default_size(self);
    const aint_t np = self->_num_slots++;
    aasm_prototype_t* const p = aasm_prototype(self);

    self->_slots[np] = np_off;
    assert(p->num_nesteds < p->max_nesteds);
    nesteds_of(p)[p->num_nesteds] = np;

    assert(self->_nested_level < ANY_ASM_MAX_NESTED_LEVEL);
    self->_nested_level++;
    self->_context[self->_nested_level].slot = np;
    self->_context[self->_nested_level].idx = p->num_nesteds;

    return p->num_nesteds++;
}

void
aasm_init(
    aasm_t* self, aalloc_t alloc, void* alloc_ud)
{
    memset(self, 0, sizeof(*self));
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
}

aerror_t
aasm_load(
    aasm_t* self, const achunk_header_t* input)
{
    aasm_cleanup(self);

    self->st = (astring_table_t*)aalloc(self, NULL, INIT_ST_BYTES);
    astring_table_init(self->st, INIT_ST_BYTES, INIT_ST_SSIZE);

    self->_max_slots = INIT_SLOT_COUNT;
    self->_slots = (aint_t*)aalloc(
        self, NULL, self->_max_slots * sizeof(aint_t));

    self->_buff_capacity = self->_max_slots * required_size(&DEFAULT_PROTO_SZ);
    self->_buff = (uint8_t*)aalloc(self, NULL, self->_buff_capacity);

    self->_slots[self->_num_slots] = new_prototype_default_size(self);
    self->_num_slots = 1;

    if (!input) return AERR_NONE;
    if (memcmp(&CHUNK_HEADER, input, sizeof(achunk_header_t)) == 0) {
        aint_t offset = sizeof(achunk_header_t);
        load_chunk(self, input, &offset);
        return AERR_NONE;
    } else {
        return AERR_MALFORMED;
    }
}

void
aasm_save(
    aasm_t* self)
{
    aint_t sz = 0;
    if (self->_num_slots == 0) return;
    compute_chunk_body_size(self, 0, &sz);
    sz += sizeof(achunk_header_t);
    self->chunk_size = 0;
    if (self->_chunk_capacity < sz) {
        aalloc(self, self->chunk, 0);
        self->chunk = (achunk_header_t*)aalloc(self, NULL, sz);
        self->_chunk_capacity = sz;
    }

    memcpy(self->chunk, &CHUNK_HEADER, sizeof(achunk_header_t));
    self->chunk_size += sizeof(achunk_header_t);

    save_chunk(self, 0);
    assert(self->chunk_size == sz);
}

void
aasm_cleanup(
    aasm_t* self)
{
    aalloc(self, self->st, 0);
    self->st = NULL;

    aalloc(self, self->_slots, 0);
    self->_slots = NULL;
    self->_num_slots = 0;
    self->_max_slots = 0;

    aalloc(self, self->_buff, 0);
    self->_buff = NULL;
    self->_buff_size = 0;
    self->_buff_capacity = 0;

    memset(self->_context, 0, sizeof(self->_context));
    self->_nested_level = 0;

    aalloc(self, self->chunk, 0);
    self->chunk_size = 0;
    self->_chunk_capacity = 0;
}

aint_t
aasm_emit(
    aasm_t* self, ainstruction_t instruction, aint_t source_line)
{
    aasm_prototype_t* p = aasm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions * GROW_FACTOR;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_instructions == p->max_instructions) {
        aasm_reserve(self, &sz);
        p = aasm_prototype(self);
    }

    assert(p->num_instructions < p->max_instructions);
    instructions_of(p)[p->num_instructions] = instruction;
    source_lines_of(p)[p->num_instructions] = source_line;
    return p->num_instructions++;
}

aint_t
aasm_add_constant(
    aasm_t* self, aconstant_t constant)
{
    aasm_prototype_t* p = aasm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants * GROW_FACTOR;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_constants == p->max_constants) {
        aasm_reserve(self, &sz);
        p = aasm_prototype(self);
    }

    assert(p->num_constants < p->max_constants);
    constants_of(p)[p->num_constants] = constant;
    return p->num_constants++;
}

aint_t
aasm_add_import(
    aasm_t* self, const char* module, const char* name)
{
    aimport_t import;
    aasm_prototype_t* p;
    aasm_reserve_t sz;

    import.module = aasm_string_to_ref(self, module);
    import.name = aasm_string_to_ref(self, name);
    p = aasm_prototype(self);

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports * GROW_FACTOR;
    sz.max_nesteds = p->max_nesteds;

    if (p->num_imports == p->max_imports) {
        aasm_reserve(self, &sz);
        p = aasm_prototype(self);
    }

    assert(p->num_imports < p->max_imports);
    imports_of(p)[p->num_imports] = import;
    return p->num_imports++;
}

aint_t
aasm_module_push(
    aasm_t* self, const char* name)
{
    aasm_prototype_t* p;
    aint_t idx;
    aint_t symbol = aasm_string_to_ref(self, name);
    assert(self->_nested_level == 0);
    idx = aasm_push(self);
    p = aasm_prototype(self);
    p->symbol = symbol;
    return idx;
}

aint_t
aasm_push(
    aasm_t* self)
{
    const aasm_prototype_t* p = aasm_prototype(self);
    aasm_reserve_t sz;

    sz.max_instructions = p->max_instructions;
    sz.max_constants = p->max_constants;
    sz.max_imports = p->max_imports;
    sz.max_nesteds = p->max_nesteds * GROW_FACTOR;

    if (p->num_nesteds == p->max_nesteds) {
        aasm_reserve(self, &sz);
        p = NULL;
    }

    return push_unsafe(self);
}

void
aasm_open(
    aasm_t* self, aint_t idx)
{
    aasm_prototype_t* const p = aasm_prototype(self);
    assert(idx < p->num_nesteds);
    assert(self->_nested_level < ANY_ASM_MAX_NESTED_LEVEL);
    self->_nested_level++;
    self->_context[self->_nested_level].slot = nesteds_of(p)[idx];
    self->_context[self->_nested_level].idx = idx;
}

aint_t
aasm_pop(
    aasm_t* self)
{
    aint_t idx;
    assert(self->_nested_level > 0);
    idx = self->_context[self->_nested_level].idx;
    self->_nested_level--;
    return idx;
}

aint_t
aasm_string_to_ref(
    aasm_t* self, const char* string)
{
    aint_t ref = AERR_FULL;
    do {
        ref = astring_table_to_ref(self->st, string);
        if (ref == AERR_FULL) {
            const aint_t new_cap = self->st->allocated_bytes * GROW_FACTOR;
            self->st = (astring_table_t*)aalloc(self, self->st, new_cap);
            astring_table_grow(self->st, new_cap);
        } else {
            break;
        }
    } while (TRUE);
    return ref;
}

void
aasm_reserve(
    aasm_t* self, const aasm_reserve_t* sz)
{
    {
        const aasm_prototype_t* const p = aasm_prototype(self);
        if (p->max_instructions >= sz->max_instructions &&
            p->max_constants >= sz->max_constants &&
            p->max_imports >= sz->max_imports &&
            p->max_nesteds >= sz->max_nesteds) return;
    }
    {
        const aint_t np_off = new_prototype(self, sz);
        aasm_prototype_t* const np = (aasm_prototype_t*)(self->_buff + np_off);
        const aasm_prototype_t* const cp = aasm_prototype(self);

        np->source = cp->source;
        np->symbol = cp->symbol;
        np->num_instructions = cp->num_instructions;
        np->num_constants = cp->num_constants;
        np->num_imports = cp->num_imports;
        np->num_nesteds = cp->num_nesteds;

        memcpy(
            instructions_of(np),
            instructions_of_const(cp),
            (size_t)cp->num_instructions * sizeof(ainstruction_t));
        memcpy(
            constants_of(np),
            constants_of_const(cp),
            (size_t)cp->num_constants * sizeof(aconstant_t));
        memcpy(
            imports_of(np),
            imports_of_const(cp),
            (size_t)cp->num_imports * sizeof(aimport_t));
        memcpy(
            source_lines_of(np),
            source_lines_of_const(cp),
            (size_t)cp->num_instructions * sizeof(aint_t));
        memcpy(
            nesteds_of(np),
            nesteds_of_const(cp),
            (size_t)cp->num_nesteds * sizeof(aint_t));

        self->_slots[ctx(self)->slot] = np_off;
    }
}

aasm_prototype_t*
aasm_prototype(
    aasm_t* self)
{
    return aasm_prototype_at(self, ctx(self)->slot);
}

aasm_current_t
aasm_resolve(
    aasm_t* self)
{
    return resolve(aasm_prototype(self));
}
