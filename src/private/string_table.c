/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/string_table.h>

#ifdef ANY_TOOL

#include <any/gc_string.h>

#define HASH_FACTOR (2.0f)

#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct {
    uint32_t offset;
} hash_slot_t;

static AINLINE hash_slot_t* hashtable(astring_table_t* self)
{
    return (hash_slot_t*)(self + 1);
}

static AINLINE char* strings(astring_table_t* self)
{
    return (char*)(hashtable(self) + self->num_hash_slots);
}

static AINLINE const hash_slot_t* hashtable_const(const astring_table_t* self)
{
    return (const hash_slot_t*)(self + 1);
}

static AINLINE const char* strings_const(const astring_table_t* self)
{
    return (const char*)(hashtable_const(self) + self->num_hash_slots);
}

static AINLINE int32_t available_string_bytes(astring_table_t* self)
{
    return self->allocated_bytes - sizeof(*self) -
        self->num_hash_slots * sizeof(hash_slot_t);
}

static void add_empty_string(astring_table_t* self)
{
    // Empty string is stored at index 0.
    // This way, we can use 0 as a marker for empty hash slots.
    const ahash_and_length_t hl = ahash_and_length("");
    char* const strs = strings(self);
    *(uint32_t*)strs = hl.hash;
    strs[sizeof(uint32_t)] = 0;
    self->string_bytes = sizeof(uint32_t) + 1;
}

static void recompute_num_hash_slots(astring_table_t* self, int32_t bytes)
{
    const float average_strlen = self->count > 0
        ? (float)self->string_bytes / (float)self->count
        : 15.0f;
    const float bytes_per_string = average_strlen + sizeof(uint32_t) + 1 +
        sizeof(hash_slot_t)*HASH_FACTOR;
    const float num_strings = (bytes - sizeof(*self)) / bytes_per_string;
    self->num_hash_slots = (int32_t)MAX(
        num_strings*HASH_FACTOR, self->num_hash_slots);
}

static void rebuild_hash_table(astring_table_t* self)
{
    const char* const strs = strings(self);
    const char* string = strs + sizeof(uint32_t) + 1;
    hash_slot_t* const ht = hashtable(self);
    memset(ht, 0, self->num_hash_slots * sizeof(hash_slot_t));

    while (string < strs + self->string_bytes) {
        const ahash_and_length_t hl = ahash_and_length(string + sizeof(uint32_t));
        int32_t i = hl.hash % self->num_hash_slots;
        while (ht[i].offset) i = (i + 1) % self->num_hash_slots;
        ht[i].offset = (uint32_t)(string - strs);
        string = string + sizeof(uint32_t) + hl.length + 1;
    }
}

void astring_table_init(
    astring_table_t* self, int32_t bytes, int32_t average_strlen)
{
    const float bytes_per_string = average_strlen + sizeof(uint32_t) + 1 +
        sizeof(hash_slot_t)*HASH_FACTOR;
    const float num_strings = (bytes - sizeof(*self)) / bytes_per_string;
    self->num_hash_slots = (int32_t)MAX(num_strings * HASH_FACTOR, 1);

    assert(bytes >= ANY_ST_MIN_SIZE);
    self->allocated_bytes = bytes;
    self->count = 0;

    memset(hashtable(self), 0, self->num_hash_slots * sizeof(hash_slot_t));

    add_empty_string(self);
}

void astring_table_grow(astring_table_t* self, int32_t bytes)
{
    const char* const old_strings = strings(self);

    assert(bytes >= self->allocated_bytes);
    self->allocated_bytes = bytes;

    recompute_num_hash_slots(self, bytes);

    memmove(strings(self), old_strings, self->string_bytes);
    rebuild_hash_table(self);
}

int32_t astring_table_pack(astring_table_t* self)
{
    const char* const old_strings = strings(self);

    self->num_hash_slots = (aint_t)(self->count*HASH_FACTOR);
    if (self->num_hash_slots < 1)
        self->num_hash_slots = 1;
    if (self->num_hash_slots < self->count + 1)
        self->num_hash_slots = self->count + 1;

    memmove(strings(self), old_strings, self->string_bytes);
    rebuild_hash_table(self);

    self->allocated_bytes = (int32_t)(
        (strings(self) + self->string_bytes) - (char*)self);
    return self->allocated_bytes;
}

astring_ref_t astring_table_to_ref(astring_table_t* self, const char* string)
{
    if (*string) {
        const ahash_and_length_t hl = ahash_and_length(string);
        char* const strs = strings(self);

        hash_slot_t* const ht = hashtable(self);
        int32_t i = hl.hash % self->num_hash_slots;
        while (ht[i].offset) {
            if (strcmp(string, strs + ht[i].offset + sizeof(uint32_t)) == 0)
                return ht[i].offset;
            i = (i + 1) % self->num_hash_slots;
        }

        if (self->count + 1 >= self->num_hash_slots)
            return AERR_FULL;

        if ((float)self->num_hash_slots /
            (float)(self->count + 1) < HASH_FACTOR)
            return AERR_FULL;

        if (self->string_bytes + (int)sizeof(uint32_t)+hl.length + 1 >
            available_string_bytes(self)) {
            return AERR_FULL;
        } else {
            char* const dest = strs + self->string_bytes;
            const astring_ref_t ref = self->string_bytes;
            ht[i].offset = ref;
            self->count++;
            *(uint32_t*)dest = hl.hash;
            memcpy(dest + sizeof(uint32_t), string, hl.length + 1);
            self->string_bytes += sizeof(uint32_t)+hl.length + 1;
            return ref;
        }
    } else {
        // "" maps to 0
        return 0;
    }
}

astring_ref_t astring_table_to_ref_const(
    const astring_table_t* self, const char* string)
{
    if (*string) {
        const ahash_and_length_t hl = ahash_and_length(string);
        const char* const strs = strings_const(self);

        int32_t i = 0;
        const hash_slot_t* const ht = hashtable_const(self);
        i = hl.hash % self->num_hash_slots;
        while (ht[i].offset) {
            if (strcmp(string, strs + ht[i].offset + sizeof(uint32_t)) == 0)
                return ht[i].offset;
            i = (i + 1) % self->num_hash_slots;
        }

        return AERR_FULL;
    } else {
        // "" maps to 0
        return 0;
    }
}

const char* astring_table_to_string(
    const astring_table_t* self, astring_ref_t ref)
{
    return strings_const(self) + ref + sizeof(uint32_t);
}

uint32_t astring_table_to_hash(const astring_table_t* self, astring_ref_t ref)
{
    return *(const uint32_t*)(strings_const(self) + ref);
}

#else // ANY_TOOL
static char non_empty_unit;
#endif
