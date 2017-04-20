/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/asm.h>

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

enum {
    MAX_NESTED_LEVEL = (sizeof(((aasm_t*)0)->context) /
    sizeof(((aasm_t*)0)->context[0]))
};

static const uint8_t CHUNK_HEADER[] = {
    0x41, 0x6E, 0x79, 0x00, // signature
    ((AVERSION_MAJOR << 8) | AVERSION_MINOR), // version
    ABIG_ENDIAN,
    sizeof(asize_t),
    sizeof(aint_t),
    sizeof(afloat_t),
    sizeof(ainstruction_t),
    0, 0 // reserved
};
ASTATIC_ASSERT(sizeof(CHUNK_HEADER) == 12);

static inline void* realloc(aasm_t* self, void* old, int32_t sz)
{
    return self->realloc(self->realloc_ud, old, sz);
}

static inline int32_t required_size(
    uint32_t max_instructions,
    uint8_t max_constants,
    uint8_t max_imports,
    uint8_t max_nesteds)
{
    return sizeof(aasm_prototype_t) +
        max_instructions * sizeof(ainstruction_t) +
        max_constants * sizeof(aconstant_t) +
        max_imports * sizeof(aimport_t) +
        max_nesteds * sizeof(int32_t);
}

static inline int32_t prototype_size(aasm_prototype_t* p)
{
    return required_size(
        p->max_instructions, p->max_constants, p->max_imports, p->max_nesteds);
}

static inline ainstruction_t* instructions(aasm_prototype_t* p)
{
    uint8_t* pc = (uint8_t*)p;
    return (ainstruction_t*)(pc + sizeof(*p));
}

static inline aconstant_t* constants(aasm_prototype_t* p)
{
    uint8_t* pc = (uint8_t*)p;
    return (aconstant_t*)(pc + sizeof(*p) +
        p->max_instructions * sizeof(ainstruction_t));
}

static inline aimport_t* imports(aasm_prototype_t* p)
{
    uint8_t* pc = (uint8_t*)p;
    return (aimport_t*)(pc + sizeof(*p) +
        p->max_instructions * sizeof(ainstruction_t) +
        p->max_constants * sizeof(aconstant_t));
}

static inline int32_t* nesteds(aasm_prototype_t* p)
{
    uint8_t* pc = (uint8_t*)p;
    return (int32_t*)(pc + sizeof(*p) +
        p->max_instructions * sizeof(ainstruction_t) +
        p->max_constants * sizeof(aconstant_t) +
        p->max_imports * sizeof(aimport_t));
}

static int32_t gc(
    aasm_t* self, uint8_t* new_buff, int32_t offset, int32_t parent)
{
    // copy referenced prototypes
    aasm_prototype_t* p = any_asm_prototype_at(self, parent);
    int32_t p_sz = prototype_size(p);
    memcpy(new_buff + offset, p, p_sz);

    // safe to update parent slot here
    // there're no more dereferences to that during gc
    self->slots[parent] = offset;
    offset += p_sz;
    for (int32_t i = 0; i < p->num_nesteds; ++i) {
        int32_t n = nesteds(p)[i];
        offset = gc(self, new_buff, offset, n);
    }

    return offset;
}

static void grow_buff(aasm_t* self, int32_t expected)
{
    expected += self->buff_size;
    if (expected <= self->buff_capacity) return; 
    
    assert(self->num_slots > 0 && "don't grow for root");
    int32_t new_capacity = self->buff_capacity;
    while (new_capacity < expected) new_capacity *= GROW_FACTOR;
    uint8_t* new_buff = (uint8_t*)realloc(self, NULL, new_capacity);

    self->buff_size = gc(self, new_buff, 0, 0);
    realloc(self, self->buff, 0);
    self->buff = new_buff;
    self->buff_capacity = new_capacity;
}

// returns a new prototype offset, 
// and guarantee that at least 1 free slot available.
static int32_t new_prototype(
    aasm_t* self,
    uint32_t max_instructions,
    uint8_t max_constants,
    uint8_t max_imports,
    uint8_t max_nesteds)
{
    int32_t sz = required_size(
        max_instructions, max_constants, max_imports, max_nesteds);
    grow_buff(self, sz);

    if (self->num_slots == self->max_slots) {
        self->max_slots *= GROW_FACTOR;
        self->slots = realloc(
            self, self->slots, self->max_slots * sizeof(int32_t));
    }

    int32_t poff = self->buff_size;
    aasm_prototype_t* p = (aasm_prototype_t*)(self->buff + poff);
    memset(p, 0, sz);
    p->max_instructions = max_instructions;
    p->max_constants = max_constants;
    p->max_imports = max_imports;
    p->max_nesteds = max_nesteds;
    self->buff_size += sz;
    return poff;
}

static inline int32_t new_prototype_default_size(aasm_t* self)
{
    return new_prototype(
        self,
        INIT_MAX_INSTRUCTIONS,
        INIT_MAX_CONSTANTS,
        INIT_MAX_IMPORTS,
        INIT_MAX_NESTEDS);
}

static inline aasm_ctx_t* ctx(aasm_t* self)
{
    return self->context + self->nested_level;
}

static void grow_chunk(aasm_t* self, int32_t expected)
{
    expected += self->chunk_size;
    if (expected <= self->chunk_capacity) return;
    while (self->chunk_capacity < expected) self->chunk_capacity *= GROW_FACTOR;
    self->chunk = (achunk_t*)realloc(self, self->chunk, self->chunk_capacity);
}

static int32_t required_chunk_body_size(aasm_t* self, int32_t parent)
{
    aasm_prototype_t* p = any_asm_prototype_at(self, parent);
    aasm_current_t c = any_asm_resolve(self);

    int32_t psz = sizeof(aprototype_t);

    psz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->source_name));
    psz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->module_name));
    psz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->exported));
    for (int32_t i = 0; i < p->num_constants; ++i) {
        if (c.constants[i].b.type != ACT_STRING) continue;
        psz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.constants[i].s.ref));
    }
    for (int32_t i = 0; i < p->num_imports; ++i) {
        psz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.imports[i].module));
        psz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.imports[i].name));
    }

    psz += p->num_instructions * sizeof(ainstruction_t);
    psz += p->num_constants * sizeof(aconstant_t);
    psz += p->num_imports * sizeof(aimport_t);
    psz += p->num_nesteds * sizeof(int32_t);

    for (int32_t i = 0; i < p->num_nesteds; ++i) {
        psz += required_chunk_body_size(self, nesteds(p)[i]);
    }

    return psz;
}

static inline aasm_current_t cp_resolve(aprototype_t* p, uint32_t strings_sz)
{
    aasm_current_t cur;
    cur.instructions = (ainstruction_t*)(((uint8_t*)p) + 
        sizeof(aprototype_t) + strings_sz);
    cur.constants = (aconstant_t*)(((uint8_t*)cur.instructions) + 
        p->num_instructions * sizeof(ainstruction_t));
    cur.imports = (aimport_t*)(((uint8_t*)cur.constants) + 
        p->num_constants * sizeof(aconstant_t));
    cur.nesteds = (int32_t*)(((uint8_t*)cur.imports) +
        p->num_imports * sizeof(aimport_t));
    return cur;
}

static inline int32_t chunk_add_str(
    aasm_t* self, aprototype_t* header, astring_ref_t s)
{
    uint32_t hash = any_st_to_hash(self->st, s);
    const char* str = any_st_to_string(self->st, s);
    int32_t length = strlen(str);
    int32_t off = header->strings_sz;
    uint8_t* b = ((uint8_t*)header) + sizeof(aprototype_t) + off;
    memcpy(b, &hash, sizeof(uint32_t));
    memcpy(b + sizeof(uint32_t), str, length + 1);
    header->strings_sz += (uint16_t)(sizeof(uint32_t) + length + 1);
    return off + sizeof(uint32_t);
}

static void save_chunk(aasm_t* self, int32_t parent)
{
    aasm_prototype_t* p = any_asm_prototype_at(self, parent);
    aasm_current_t c = any_asm_resolve(self);

    // compute required size for strings
    int32_t str_sz = 0;
    str_sz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->source_name));
    str_sz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->module_name));
    str_sz += sizeof(uint32_t) + 1 +
        strlen(any_st_to_string(self->st, p->exported));
    for (int32_t i = 0; i < p->num_constants; ++i) {
        if (c.constants[i].b.type != ACT_STRING) continue;
        str_sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.constants[i].s.ref));
    }
    for (int32_t i = 0; i < p->num_imports; ++i) {
        str_sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.imports[i].module));
        str_sz += sizeof(uint32_t) + 1 +
            strlen(any_st_to_string(self->st, c.imports[i].name));
    }
    
    // allocate new prototype
    int32_t psz = sizeof(aprototype_t) +
        str_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_nesteds * sizeof(int32_t);

    // set headers
    aprototype_t* header = (aprototype_t*)(
        ((uint8_t*)self->chunk) + self->chunk_size);
    self->chunk_size += psz;
    memset(header, 0, sizeof(aprototype_t));
    header->num_instructions = p->num_instructions;
    header->num_upvalues = p->num_upvalues;
    header->num_arguments = p->num_arguments;
    header->num_constants = p->num_constants;
    header->num_local_vars = p->num_local_vars;
    header->num_imports = p->num_imports;
    header->num_nesteds = p->num_nesteds;
    header->source_name = chunk_add_str(self, header, p->source_name);
    header->module_name = chunk_add_str(self, header, p->module_name);
    header->exported = chunk_add_str(self, header, p->exported);

    aasm_current_t cp = cp_resolve(header, str_sz);

    // add instructions
    memcpy(
        cp.instructions,
        c.instructions, 
        p->num_instructions * sizeof(ainstruction_t));

    // add constants
    for (int32_t i = 0; i < p->num_constants; ++i) {
        cp.constants[i] = c.constants[i];
        if (c.constants[i].b.type != ACT_STRING) continue;
        cp.constants[i].s.ref = chunk_add_str(self, header, c.constants[i].s.ref);
    }

    // add imports
    for (int32_t i = 0; i < p->num_imports; ++i) {
        cp.imports[i].module = chunk_add_str(self, header, c.imports[i].module);
        cp.imports[i].name = chunk_add_str(self, header, c.imports[i].name);
    }

    assert(header->strings_sz == str_sz);

    // add nesteds
    for (int32_t i = 0; i < p->num_nesteds; ++i) {
        cp.nesteds[i] = self->chunk_size;
        save_chunk(self, c.nesteds[i]);
    }
}

static const char* cp_string(aprototype_t* p, astring_ref_t s)
{
    return ((char*)p) + sizeof(aprototype_t) + s;
}

static int32_t load_chunk(aasm_t* self, achunk_t* input, int32_t offset)
{
    aprototype_t* p = (aprototype_t*)(((uint8_t*)input) + offset);
    aasm_current_t cp = cp_resolve(p, p->strings_sz);

    any_asm_grow(
        self, 
        p->num_instructions, 
        p->num_constants, 
        p->num_imports, 
        p->num_nesteds);
    aasm_prototype_t* ap = any_asm_prototype(self);

    ap->source_name = any_st_to_ref(self->st, cp_string(p, p->source_name));
    ap->module_name = any_st_to_ref(self->st, cp_string(p, p->module_name));
    ap->exported = any_st_to_ref(self->st, cp_string(p, p->exported));
    
    ap->num_upvalues = p->num_upvalues;
    ap->num_arguments = p->num_arguments;
    ap->num_local_vars = p->num_local_vars;

    memcpy(
        instructions(ap),
        cp.instructions,
        p->num_instructions * sizeof(ainstruction_t));
    ap->num_instructions = p->num_instructions;

    for (int32_t i = 0; i < p->num_constants; ++i) {
        aconstant_t constant = cp.constants[i];
        if (constant.b.type == ACT_STRING) {
            constant.s.ref = any_st_to_ref(
                self->st, cp_string(p, constant.s.ref));
        }
        any_asm_add_constant(self, constant);
    }

    for (int32_t i = 0; i < p->num_imports; ++i) {
        aimport_t import = cp.imports[i];
        import.module = any_st_to_ref(self->st, cp_string(p, import.module));
        import.name = any_st_to_ref(self->st, cp_string(p, import.name));
        any_asm_add_import(self, import);
    }

    int32_t psz = offset + 
        sizeof(aprototype_t) +
        p->strings_sz +
        p->num_instructions * sizeof(ainstruction_t) +
        p->num_constants * sizeof(aconstant_t) +
        p->num_imports * sizeof(aimport_t) +
        p->num_nesteds * sizeof(int32_t);

    for (int32_t i = 0; i < p->num_nesteds; ++i) {
        any_asm_push(self);
        psz += load_chunk(self, input, psz);
        any_asm_pop(self);
    }

    return psz;
}

void any_asm_init(aasm_t* self, arealloc_t realloc, void* realloc_ud)
{
    memset(self, 0, sizeof(*self));
    self->realloc = realloc;
    self->realloc_ud = realloc_ud;
}

int32_t any_asm_load(aasm_t* self, achunk_t* input)
{
    any_asm_cleanup(self);

    self->st = (astring_table_t*)realloc(self, NULL, INIT_ST_BYTES);
    any_st_init(self->st, INIT_ST_BYTES, INIT_ST_SSIZE);

    self->max_slots = INIT_SLOT_COUNT;
    self->slots = (int32_t*)realloc(
        self, NULL, self->max_slots * sizeof(int32_t));

    self->buff_capacity = self->max_slots * required_size(
        INIT_MAX_INSTRUCTIONS,
        INIT_MAX_CONSTANTS,
        INIT_MAX_IMPORTS,
        INIT_MAX_NESTEDS);
    self->buff = (uint8_t*)realloc(self, NULL, self->buff_capacity);

    self->slots[self->num_slots] = new_prototype_default_size(self);
    self->num_slots = 1;

    if (!input) return AERR_NONE;
    if (memcmp(CHUNK_HEADER, input, sizeof(CHUNK_HEADER)) != 0) return AERR_BAD;
    load_chunk(self, input, sizeof(aprototype_t));
    return AERR_NONE;
}

void any_asm_save(aasm_t* self)
{
    int32_t sz = required_chunk_body_size(self, 0);
    sz += sizeof(CHUNK_HEADER);
    self->chunk_size = 0;
    if (self->chunk_capacity < sz) grow_chunk(self, sz);

    memcpy(self->chunk->header, CHUNK_HEADER, sizeof(CHUNK_HEADER));
    self->chunk_size += sizeof(CHUNK_HEADER);

    save_chunk(self, 0);
}

void any_asm_cleanup(aasm_t* self)
{
    realloc(self, self->st, 0);
    self->st = NULL;

    realloc(self, self->slots, 0);
    self->slots = NULL;
    self->num_slots = 0;
    self->max_slots = 0;

    realloc(self, self->buff, 0);
    self->buff = NULL;
    self->buff_size = 0;
    self->buff_capacity = 0;

    memset(self->context, 0, sizeof(self->context));
    self->nested_level = 0;

    self->realloc(self, self->chunk, 0);
    self->chunk_size = 0;
    self->chunk_capacity = 0;
}

int32_t any_asm_emit(aasm_t* self, ainstruction_t instruction)
{
    aasm_prototype_t* p = any_asm_prototype(self);

    if (p->num_instructions == p->max_instructions) {
        any_asm_grow(
            self,
            p->max_instructions * GROW_FACTOR,
            p->max_constants,
            p->max_imports,
            p->max_nesteds);
        p = any_asm_prototype(self);
    }

    assert(p->num_instructions < p->max_instructions);
    instructions(p)[p->num_instructions] = instruction;
    return p->num_instructions++;
}

int32_t any_asm_add_constant(aasm_t* self, aconstant_t constant)
{
    aasm_prototype_t* p = any_asm_prototype(self);

    if (p->num_constants == p->max_constants) {
        any_asm_grow(
            self,
            p->max_instructions,
            p->max_constants * GROW_FACTOR,
            p->max_imports,
            p->max_nesteds);
        p = any_asm_prototype(self);
    }

    assert(p->num_constants < p->max_constants);
    constants(p)[p->num_constants] = constant;
    return p->num_constants++;
}

int32_t any_asm_add_import(aasm_t* self, aimport_t import)
{
    aasm_prototype_t* p = any_asm_prototype(self);

    if (p->num_imports == p->max_imports) {
        any_asm_grow(
            self,
            p->max_instructions,
            p->max_constants,
            p->max_imports * GROW_FACTOR,
            p->max_nesteds);
        p = any_asm_prototype(self);
    }

    assert(p->num_imports < p->max_imports);
    imports(p)[p->num_imports] = import;
    return p->num_imports++;
}

int32_t any_asm_push(aasm_t* self)
{
    assert(self->nested_level < MAX_NESTED_LEVEL);

    aasm_prototype_t* p = any_asm_prototype(self);
    if (p->num_nesteds == p->max_nesteds) {
        any_asm_grow(
            self,
            p->max_instructions,
            p->max_constants,
            p->max_imports,
            p->max_nesteds * GROW_FACTOR);
    }
    p = NULL; // maybe relocated

    int32_t np_off = new_prototype_default_size(self);
    int32_t np = self->num_slots++;
    self->slots[np] = np_off;
    p = any_asm_prototype(self);
    assert(p->num_nesteds < p->max_nesteds);
    nesteds(p)[p->num_nesteds] = np;

    self->nested_level++;
    self->context[self->nested_level].slot = np;
    self->context[self->nested_level].idx = p->num_nesteds;

    return p->num_nesteds++;
}

void any_asm_open(aasm_t* self, int32_t idx)
{
    assert(self->nested_level < MAX_NESTED_LEVEL);

    aasm_prototype_t* p = any_asm_prototype(self);
    assert(idx < p->num_nesteds);
    self->context[++self->nested_level].slot = nesteds(p)[idx];
}

int32_t any_asm_pop(aasm_t* self)
{
    assert(self->nested_level > 0);
    int32_t idx = self->context[self->nested_level].idx;
    self->nested_level--;
    return idx;
}

astring_ref_t any_asm_string_to_ref(aasm_t* self, const char* s)
{
    astring_ref_t ref = AERR_FULL;
    do {
        ref = any_st_to_ref(self->st, s);
        if (ref != AERR_FULL) break;
        self->st = (astring_table_t*)realloc(
            self, self->st, self->st->allocated_bytes * GROW_FACTOR);
        any_st_grow(self->st, self->st->allocated_bytes);
    } while (TRUE);
    return ref;
}

void any_asm_grow(
    aasm_t* self,
    uint32_t max_instructions,
    uint8_t max_constants,
    uint8_t max_imports,
    uint8_t max_nesteds)
{
    assert(max_instructions >= any_asm_prototype(self)->max_instructions);
    assert(max_constants    >= any_asm_prototype(self)->max_constants);
    assert(max_imports      >= any_asm_prototype(self)->max_imports);
    assert(max_nesteds      >= any_asm_prototype(self)->max_nesteds);

    int32_t np_off = new_prototype(
        self,
        max_instructions,
        max_constants,
        max_imports,
        max_nesteds);
    aasm_prototype_t* np = (aasm_prototype_t*)(self->buff + np_off);
    aasm_prototype_t* cp = any_asm_prototype(self);
    
    np->num_instructions = cp->num_instructions;
    np->num_constants = cp->num_constants;
    np->num_imports = cp->num_imports;
    np->num_nesteds = cp->num_nesteds;

    memcpy(
        instructions(np), 
        instructions(cp), 
        cp->num_instructions * sizeof(ainstruction_t));
    memcpy(
        constants(np),
        constants(cp),
        cp->num_constants * sizeof(aconstant_t));
    memcpy(
        imports(np),
        imports(cp),
        cp->num_imports * sizeof(aimport_t));
    memcpy(
        nesteds(np),
        nesteds(cp),
        cp->num_nesteds * sizeof(int32_t));

    self->slots[ctx(self)->slot] = np_off;
}

aasm_prototype_t* any_asm_prototype(aasm_t* self)
{
    return any_asm_prototype_at(self, ctx(self)->slot);
}

aasm_current_t any_asm_resolve(aasm_t* self)
{
    aasm_current_t cur;
    aasm_prototype_t* p = any_asm_prototype(self);
    cur.instructions = instructions(p);
    cur.constants = constants(p);
    cur.imports = imports(p);
    cur.nesteds = nesteds(p);
    return cur;
}