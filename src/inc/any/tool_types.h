/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/rt_types.h>

#define WBY_UINT_PTR size_t
#include <any/wby.h>

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
need for pointer patching. Just make sure to call \ref astring_table_pack
before saving so that it uses as little memory as possible.

This structure representing a string table. The data for string table is
stored directly after this header in memory and consists of a hash table
followed by a string data block.
*/
typedef struct astring_table_s {
    /// The total size of the allocated data, including this header.
    aint_t allocated_bytes;
    /// The number of strings in the table.
    aint_t count;
    /// Total number of slots in the hash table.
    aint_t num_hash_slots;
    /// The current number of bytes used for string data.
    aint_t string_bytes;
} astring_table_t;

// We must have room for at least one hash slot and one string.
enum {
    ANY_ST_MIN_SIZE =
    sizeof(astring_table_t) + sizeof(uint32_t) + sizeof(aint_t) + 1
};

/** Byte code assembler prototype.
\warning `max_*` is **read-only**.
\ref aasm_reserve are required to extends these values.
*/
typedef struct aasm_prototype_s {
    aint_t source;
    aint_t symbol;
    aint_t num_local_vars;
    aint_t num_instructions;
    aint_t max_instructions;
    aint_t num_nesteds;
    aint_t max_nesteds;
    aint_t num_constants;
    aint_t max_constants;
    aint_t num_imports;
    aint_t max_imports;
} aasm_prototype_t;

/// Byte code assembler context.
typedef struct aasm_ctx_s {
    aint_t slot;
    aint_t idx;
} aasm_ctx_t;

/// Byte code assembler reserve sizes.
typedef struct aasm_reserve_s {
    aint_t max_instructions;
    aint_t max_constants;
    aint_t max_imports;
    aint_t max_nesteds;
} aasm_reserve_t;

/// Byte code assembler resolved prototype pointers.
typedef struct aasm_current_s {
    ainstruction_t* instructions;
    aconstant_t* constants;
    aimport_t* imports;
    aint_t* source_lines;
    aint_t* nesteds;
} aasm_current_t;

/** Byte code assembler.

\warning
\ref aasm_prototype_t and \ref aasm_current_t are reserved for advanced purpose
like optimization, which provides direct access to the assembler internal. The
content of these structure may be relocated after a call to these following
functions, use it at your own risk:
- \ref aasm_emit
- \ref aasm_add_constant
- \ref aasm_add_import
- \ref aasm_reserve
- \ref aasm_push

\brief
The assembler is context sensitive, which can be used to authoring multiple
prototypes with just only single instance, in nested manner. Generally, for
each \ref aasm_push or \ref aasm_open a new context will be created for
nested prototype, and that also saves the current context onto an internal
stack, which can be restored later by a corresponding \ref aasm_pop.

This struct itself is not POD, then must rely on \ref achunk_header_t as the
portable format for exchanges. That format enable assembler as a **framework**
to working with byte code between optimization passes.
*/
typedef struct aasm_s {
    // allocator.
    aalloc_t alloc;
    void* alloc_ud;
    // common string table.
    astring_table_t* st;
    // prototype buffers.
    aint_t* _slots;
    aint_t _num_slots;
    aint_t _max_slots;
    uint8_t* _buff;
    aint_t _buff_size;
    aint_t _buff_capacity;
    // contexts, limited nested level.
    aasm_ctx_t _context[24];
    aint_t _nested_level;
    // output binary chunk.
    achunk_header_t* chunk;
    aint_t chunk_size;
    aint_t _chunk_capacity;
} aasm_t;

// Number of nested level allowed for assembler.
enum {
    ANY_ASM_MAX_NESTED_LEVEL =
    (sizeof(((aasm_t*)0)->_context) / sizeof(((aasm_t*)0)->_context[0])) - 1
};

/** Debug service.
AVM debug service is the back-end that exposes debugging capabilities over wire.
Which can be used to debug AVM byte code function that is currently running by
the actor. Technically, debugger must touch the actor state, and vice versa. So
a dedicated debug service is required for each scheduler, to get rid of locking
and other threading headaches.
*/
typedef struct adb_s {
    aalloc_t alloc;
    void* alloc_ud;
    struct wby_server wby;
    void* wby_buff;
    ascheduler_t* target;
    void* res_buf;
    aint_t res_buf_sz;
    void* req_buf;
    aint_t req_buf_sz;
} adb_t;