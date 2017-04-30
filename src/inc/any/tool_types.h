#pragma once

#ifdef ANY_TOOL

#include <any/rt_types.h>

// Used by string table.
typedef struct {
    uint32_t hash;
    int32_t length;
} ahash_and_length_t;

/** String table.

\warning
Don't cache the char* pointer, the content of this table may be relocated.

\note
Borrowed from: https://github.com/niklasfrykholm/nflibs

\brief
This implements a string table for **string interning**, i.e. converting
back and forth between **strings** and **int** representations which we call
`astring_ref_t`. This can be used to compress data structures containing
repeated strings.

For cache friendliness all data is stored in a single continuous buffer.
You can move this buffer around freely in memory. You are responsible for
allocating the buffer. If the buffer runs out of memory you are responsible
for resizing it before you can add more strings.

String table is POD, that can be saved to and loaded from disk without any
need for pointer patching. Just make sure to call \ref any_st_pack before 
saving so that it uses as little memory as possible.

This structure representing a string table. The data for string table is
stored directly after this header in memory and consists of a hash table
followed by a string data block.
*/
typedef struct {
    /// The total size of the allocated data, including this header.
    int32_t allocated_bytes;
    /// The number of strings in the table.
    int32_t count;
    /// Total number of slots in the hash table.
    int32_t num_hash_slots;
    /// The current number of bytes used for string data.
    int32_t string_bytes;
} astring_table_t;

// We must have room for at least one hash slot and one string.
enum {
    ANY_ST_MIN_SIZE =
    sizeof(astring_table_t) + sizeof(uint32_t) + sizeof(uint32_t) + 1
};

/** Bytecode assembler prototype.
\warning `max_*` is **readonly**.
\ref any_asm_reserve are required to extends these values.
*/
typedef struct {
    astring_ref_t source_name;
    astring_ref_t module_name;
    astring_ref_t symbol;
    int32_t num_instructions;
    int32_t max_instructions;
    int16_t num_nesteds;
    int16_t max_nesteds;
    uint8_t num_upvalues;
    uint8_t num_arguments;
    uint8_t num_local_vars;
    uint8_t num_constants;
    uint8_t max_constants;
    uint8_t num_imports;
    uint8_t max_imports;
} aasm_prototype_t;

/// Bytecode assembler context.
typedef struct {
    int32_t slot;
    int32_t idx;
} aasm_ctx_t;

/// Bytecode assembler reserve sizes.
typedef struct {
    int32_t max_instructions;
    uint8_t max_constants;
    uint8_t max_imports;
    int16_t max_nesteds;
} aasm_reserve_t;

/** Bytecode assembler.

\warning
\ref aasm_prototype_t and \ref acurrent_t are reserved for advanced purpose 
like optimization, which provides direct access to the assembler internal. The 
content of these structure may be relocated after a call to these following 
functions, use it at your own risk:
- \ref any_asm_emit
- \ref any_asm_add_constant
- \ref any_asm_add_import
- \ref any_asm_reserve
- \ref any_asm_push

\brief
The assembler is context sensitive, which can be used to authoring multiple
prototypes with just only one single instance, in nested manner. Generally, for
each \ref any_asm_push or \ref any_asm_open a new context will be created for 
nested prototype, and that also saves the current context onto an internal stack, 
which can be restored later by a corresponding \ref any_asm_pop.

This struct itself isn't POD, then rely on \ref achunk_t as the portable format
for exchanges. That format enable assembler as a **framework** to working with 
bytecode between optimization passes on various address space.
*/
typedef struct {
    // Allocator.
    arealloc_t realloc;
    void* realloc_ud;
    // Common string table.
    astring_table_t* st;
    // Prototype buffers.
    int32_t* _slots;
    int32_t _num_slots;
    int32_t _max_slots;
    uint8_t* _buff;
    int32_t _buff_size;
    int32_t _buff_capacity;
    // Contexts, limited nested level.
    aasm_ctx_t _context[24];
    int32_t _nested_level;
    // Output binary chunk.
    achunk_t* chunk;
    int32_t chunk_size;
    int32_t _chunk_capacity;
} aasm_t;

// Number of nested level allowed for assembler.
enum {
    ANY_ASM_MAX_NESTED_LEVEL =
    (sizeof(((aasm_t*)0)->_context) / sizeof(((aasm_t*)0)->_context[0])) - 1
};

#endif //! ANY_TOOL