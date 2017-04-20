/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/string_table.h>

#include <assert.h>
#include <string.h>
#include <any/errno.h>

#define HASH_FACTOR (2.0f)

#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct {
    uint32_t offset;
} hash_slot_t;

static inline hash_slot_t* hashtable(astring_table_t* self)
{
    return (hash_slot_t*)(self + 1);
}

static inline char* strings(astring_table_t* self)
{
    return (char*)(hashtable(self) + self->num_hash_slots);
}

static inline int32_t available_string_bytes(astring_table_t* self)
{
    return self->allocated_bytes - sizeof(*self) - 
        self->num_hash_slots * sizeof(hash_slot_t);
}

static void rebuild_hash_table(astring_table_t* self)
{
    const char* strs = strings(self);
    const char* s = strs + sizeof(uint32_t) + 1;

    hash_slot_t* const ht = hashtable(self);
    memset(ht, 0, self->num_hash_slots * sizeof(hash_slot_t));
    while (s < strs + self->string_bytes) {
        const ahash_and_length_t hl = ahash_and_length(s + sizeof(uint32_t));
        int32_t i = hl.hash % self->num_hash_slots;
        while (ht[i].offset) i = (i + 1) % self->num_hash_slots;
        ht[i].offset = s - strs;
        s = s + sizeof(uint32_t) + hl.length + 1;
    }
}

void any_st_init(astring_table_t* self, int32_t bytes, int32_t average_strlen)
{
    assert(bytes >= ANY_ST_MIN_SIZE);

    self->allocated_bytes = bytes;
    self->count = 0;

    float bytes_per_string = average_strlen + sizeof(uint32_t) + 1 + 
        sizeof(hash_slot_t)*HASH_FACTOR;
    float num_strings = (bytes - sizeof(*self)) / bytes_per_string;
    self->num_hash_slots = (int32_t)MAX(num_strings * HASH_FACTOR, 1);

    memset(hashtable(self), 0, self->num_hash_slots * sizeof(hash_slot_t));

    // Empty string is stored at index 0. 
    // This way, we can use 0 as a marker for empty hash slots.
    const ahash_and_length_t hl = ahash_and_length("");
    char* const strs = strings(self);
    *(uint32_t*)strs = hl.hash;
    strs[sizeof(uint32_t)] = 0;
    self->string_bytes = sizeof(uint32_t) + 1;
}

void any_st_grow(astring_table_t* self, int32_t bytes)
{
    assert(bytes >= self->allocated_bytes);

    const char* const old_strings = strings(self);

    self->allocated_bytes = bytes;

    float average_strlen = self->count > 0
        ? (float)self->string_bytes / (float)self->count
        : 15.0f;
    float bytes_per_string = average_strlen + sizeof(uint32_t) + 1 + 
        sizeof(hash_slot_t)*HASH_FACTOR;
    float num_strings = (bytes - sizeof(*self)) / bytes_per_string;
    self->num_hash_slots = (int32_t)MAX(
        num_strings*HASH_FACTOR, self->num_hash_slots);

    char* const new_strings = strings(self);
    memmove(new_strings, old_strings, self->string_bytes);
    rebuild_hash_table(self);
}

int32_t any_st_pack(astring_table_t* self)
{
    const char* old_strings = strings(self);

    self->num_hash_slots = (aint_t)(self->count*HASH_FACTOR);
    if (self->num_hash_slots < 1)
        self->num_hash_slots = 1;
    if (self->num_hash_slots < self->count + 1)
        self->num_hash_slots = self->count + 1;

    char* const new_strings = strings(self);
    memmove(new_strings, old_strings, self->string_bytes);
    rebuild_hash_table(self);

    self->allocated_bytes = (new_strings + self->string_bytes) - (char*)self;
    return self->allocated_bytes;
}

astring_ref_t any_st_to_ref(astring_table_t* self, const char* s)
{
    // "" maps to 0
    if (!*s) return 0;

    const ahash_and_length_t hl = ahash_and_length(s);
    char* const strs = strings(self);

    hash_slot_t* const ht = hashtable(self);
    int32_t i = hl.hash % self->num_hash_slots;
    while (ht[i].offset) {
        if (strcmp(s, strs + ht[i].offset + sizeof(uint32_t)) == 0) 
            return ht[i].offset;
        i = (i + 1) % self->num_hash_slots;
    }

    if (self->count + 1 >= self->num_hash_slots)
        return AERR_FULL;

    if ((float)self->num_hash_slots / (float)(self->count + 1) < HASH_FACTOR)
        return AERR_FULL;

    char* const dest = strs + self->string_bytes;
    if (self->string_bytes + (int)sizeof(uint32_t) + hl.length + 1 > 
        available_string_bytes(self))
        return AERR_FULL;

    const astring_ref_t ref = self->string_bytes;
    ht[i].offset = ref;
    self->count++;
    *(uint32_t*)dest = hl.hash;
    memcpy(dest + sizeof(uint32_t), s, hl.length + 1);
    self->string_bytes += sizeof(uint32_t) + hl.length + 1;
    return ref;
}

astring_ref_t any_st_to_ref_const(const astring_table_t* self, const char* s)
{
    astring_table_t* st = (astring_table_t*)self;

    // "" maps to 0
    if (!*s) return 0;

    const ahash_and_length_t hl = ahash_and_length(s);
    const char* const strs = strings(st);

    int32_t i = 0;
    const hash_slot_t* const ht = hashtable(st);
    i = hl.hash % st->num_hash_slots;
    while (ht[i].offset) {
        if (strcmp(s, strs + ht[i].offset + sizeof(uint32_t)) == 0) 
            return ht[i].offset;
        i = (i + 1) % st->num_hash_slots;
    }

    return AERR_FULL;
}

const char* any_st_to_string(astring_table_t* self, astring_ref_t ref)
{
    return strings(self) + ref + sizeof(uint32_t);
}

uint32_t any_st_to_hash(astring_table_t* self, astring_ref_t ref)
{
    return *(uint32_t*)(strings(self) + ref);
}