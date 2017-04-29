/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/platform.h>
#include <stdint.h>

// Primitive types.
typedef uint64_t asize_t;
typedef int64_t aint_t;
typedef double afloat_t;

// Opcodes.
enum AOPCODE {
    AOC_NOP             = 0,
    AOC_POP             = 1,
    AOC_GET_CONST       = 10,
    AOC_GET_NIL         = 11,
    AOC_GET_BOOL        = 12,
    AOC_GET_LOCAL       = 13,
    AOC_SET_LOCAL       = 14,
    AOC_GET_IMPORT      = 15,
    AOC_GET_UPVALUE     = 16,
    AOC_SET_UPVALUE     = 17,
    AOC_JUMP            = 30,
    AOC_JUMP_IF_NOT     = 31,
    AOC_CALL            = 40,
    AOC_RETURN          = 41,
    AOC_CLOSURE         = 42,
    AOC_CAPTURE_LOCAL   = 43,
    AOC_CAPTURE_UPVALUE = 44,
    AOC_CLOSE           = 45,
    AOC_LABEL           = 0xFF
};

// Instruction types.
typedef union {
    // Base type.
    struct {
        uint32_t opcode : 8;
        uint32_t _ : 24;
    } base;
    // AOC_NOP
    // No operation.
    struct {
        uint32_t _;
    } nop;
    // AOC_POP
    // Pop `n` elements from the stack.
    struct {
        uint32_t _ : 8;
        int32_t n : 24;
    } pop;
    // AOC_GET_CONST
    // Push a constant from const pool at `idx` onto the stack.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } get_const;
    // AOC_GET_NIL
    // Push a nil value onto the stack.
    struct {
        uint32_t _;
    } get_nil;
    // AOC_GET_BOOL
    // Push a bool value `val` onto the stack.
    struct {
        uint32_t _ : 8;
        int32_t val : 8;
    } get_bool;
    // AOC_GET_LOCAL
    // Push a value from local pool at `idx` onto the stack.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } get_local;
    // AOC_SET_LOCAL
    // Pop a value from the stack and set it into the local pool at `idx`.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } set_local;
    // AOC_GET_IMPORT
    // Push a value from import pool at `idx` onto the stack.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } get_import;
    // AOC_GET_UPVALUE
    // Push a value from upvalue pool at `idx` onto the stack.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } get_upvalue;
    // AOC_SET_UPVALUE
    // Pop a value from the stack and set it into the upvalue pool at `idx`.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } set_upvalue;
    // AOC_JUMP
    // Unconditional jump, with signed `displacement`.
    struct {
        uint32_t _ : 8;
        int32_t displacement : 24;
    } jump;
    // AOC_JUMP_IF_NOT
    // Pop a boolean value from the stack and jump if it's nil or false,
    // with signed `displacement`.
    struct {
        uint32_t _ : 8;
        int32_t displacement : 24;
    } jump_if_not;
    // AOC_CALL
    // Pop `nargs` arguments and next a closure from the stack then call it,
    // the result will be pushed onto the stack after that.
    struct {
        uint32_t _ : 8;
        int32_t nargs : 24;
    } call;
    // AOC_RETURN
    // Returning from a function call,
    // top of the stack will be returned as result.
    struct {
        uint32_t _;
    } ret;
    // AOC_CLOSURE
    // Create a closure of prototype at `idx` from pool,
    // there are pseudo-instruction which follow AOC_CLOSURE that are used
    // by VM to manage upvalues, either AOC_CAPTURE_LOCAL or AOC_CAPTURE_UPVALE.  
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } closure;
    // AOC_CAPTURE_LOCAL
    // Pseudo-instruction for closure creating,
    // capture a local value at `idx` from local pool.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } capture_local;
    // AOC_CAPTURE_UPVALUE
    // Pseudo-instruction for closure creating,
    // capture a upvalue at `idx` from upvalue pool.
    struct {
        uint32_t _ : 8;
        int32_t idx : 24;
    } capture_upvalue;
    // AOC_CLOSE
    // Closes all local variables in the stack from `offset` onwards (>=),
    // that's required if there are upvalues present within those local vars,
    // otherwise it will go out of scope and disappear.
    // AOC_RETURN also does an implicit AOC_CLOSE.
    struct {
        uint32_t _ : 8;
        int32_t offset : 24;
    } close;
} ainstruction_t;

ASTATIC_ASSERT(sizeof(ainstruction_t) == 4);

// Instruction constructors.
AINLINE ainstruction_t ai_nop()
{
    ainstruction_t i;
    i.base.opcode = AOC_NOP;
    return i;
}

AINLINE ainstruction_t ai_pop(int32_t n)
{
    ainstruction_t i;
    i.base.opcode = AOC_POP;
    i.pop.n = n;
    return i;
}

AINLINE ainstruction_t ai_get_const(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_CONST;
    i.get_const.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_nil()
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_NIL;
    return i;
}

AINLINE ainstruction_t ai_get_bool(int32_t val)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_BOOL;
    i.get_bool.val = val;
    return i;
}

AINLINE ainstruction_t ai_get_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_LOCAL;
    i.get_local.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_set_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_SET_LOCAL;
    i.set_local.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_import(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_IMPORT;
    i.get_import.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_UPVALUE;
    i.get_upvalue.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_set_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_SET_UPVALUE;
    i.set_upvalue.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_jump(int32_t displacement)
{
    ainstruction_t i;
    i.base.opcode = AOC_JUMP;
    i.jump.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_jump_if_not(int32_t displacement)
{
    ainstruction_t i;
    i.base.opcode = AOC_JUMP_IF_NOT;
    i.jump_if_not.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_call(int32_t nargs)
{
    ainstruction_t i;
    i.base.opcode = AOC_CALL;
    i.call.nargs = nargs;
    return i;
}

AINLINE ainstruction_t ai_return()
{
    ainstruction_t i;
    i.base.opcode = AOC_RETURN;
    return i;
}

AINLINE ainstruction_t ai_closure(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CLOSURE;
    i.closure.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_capture_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CAPTURE_LOCAL;
    i.capture_local.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_capture_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CAPTURE_UPVALUE;
    i.capture_upvalue.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_close(int32_t offset)
{
    ainstruction_t i;
    i.base.opcode = AOC_CLOSE;
    i.close.offset = offset;
    return i;
}

// Allocator, 
// `old` = 0 to malloc,
// `sz`  = 0 to free,
// otherwise realloc.
typedef void* (*arealloc_t)(void* userdata, void* old, int32_t sz);

// Used by string table
typedef struct {
    uint32_t hash;
    int32_t length;
} ahash_and_length_t;

// Reference to a string
typedef int32_t astring_ref_t;

// String table.
//
// WARNING!!!: don't cache the char* pointer,
// the content of this table may be relocated.
//
// Borrowed from: https://github.com/niklasfrykholm/nflibs
// This implements a string table for *string interning*, i.e. converting
// back and forth between *strings* and `int` representations which we call
// *string_ref*. This can be used to compress data structures containing 
// repeated strings.
//
// For cache friendliness all data is stored in a single continuous buffer.
// You can move this buffer around freely in memory. You are responsible for
// allocating the buffer. If the buffer runs out of memory you are responsible
// for resizing it before you can add more strings.
//
// String table is POD, that can be saved to and loaded from disk without any
// need for pointer patching. Just make sure to call any_st_pack before saving
// so that it uses as little memory as possible.
//
// This structure representing a string table. The data for string table is 
// stored directly after this header in memory and consists of a hash table 
// followed by a string data block.
typedef struct {
    // The total size of the allocated data, including this header.
    int32_t allocated_bytes;
    // The number of strings in the table.
    int32_t count;
    // Total number of slots in the hash table.
    int32_t num_hash_slots;
    // The current number of bytes used for string data.
    int32_t string_bytes;
} astring_table_t;

// We must have room for at least one hash slot and one string.
enum {
    ANY_ST_MIN_SIZE =
    sizeof(astring_table_t) + sizeof(uint32_t) + sizeof(uint32_t) + 1
};

// Constant types.
enum {
    ACT_INTEGER,
    ACT_STRING,
    ACT_FLOAT
};

#pragma pack(push, 1)

// Function constant.
typedef union APACKED {
    // Base type.
    struct {
        uint32_t type;
    } b;
    // ACT_INTEGER.
    struct {
        uint32_t _;
        aint_t val;
    } i;
    //  ACT_STRING.
    struct {
        uint32_t _;
        astring_ref_t ref;
    } s;
    // ACT_FLOAT.
    struct {
        uint32_t _;
        afloat_t val;
    } f;
} aconstant_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(aconstant_t) == 
    sizeof(uint32_t) + 
    (sizeof(aint_t) > sizeof(afloat_t) 
        ? sizeof(aint_t)
        : sizeof(afloat_t)));

// Constant constructors.
AINLINE aconstant_t ac_integer(aint_t val)
{
    aconstant_t c;
    c.b.type = ACT_INTEGER;
    c.i.val = val;
    return c;
}

AINLINE aconstant_t ac_string(astring_ref_t s)
{
    aconstant_t c;
    c.b.type = ACT_STRING;
    c.s.ref = s;
    return c;
}

AINLINE aconstant_t ac_float(afloat_t val)
{
    aconstant_t c;
    c.b.type = ACT_FLOAT;
    c.f.val = val;
    return c;
}

// Function import.
typedef struct {
    astring_ref_t module;
    astring_ref_t name;
} aimport_t;

ASTATIC_ASSERT(sizeof(aimport_t) == 8);

AINLINE aimport_t aimport(astring_ref_t module, astring_ref_t name)
{
    aimport_t i;
    i.module = module;
    i.name = name;
    return i;
}

// Function prototype.
//
// This struct represents the header portion of a function prototype. Each
// prototype is self-contained and relocatable, all references are relative
// to their prototype header. That means we can safety copy a memory blob 
// of prototype and move it around, save to and loaded from disk without any 
// need for patching.
//
// The first prototype in chunk is *module* prototype, which is unable to be
// executed. There are no instructions, upvalues, arguments, constants, local
// variables and imports in this kind of prototype. All nesteds of that also
// be treated as module level functions. Which may have the `exported` field 
// as the symbol name, that allow this to be imported by other modules.
//
// Memory layout:
// - header 
// - strings 
// - instructions 
// - constants 
// - imports
// - nested_offsets
// - nesteds
typedef struct {
    astring_ref_t source_name;
    astring_ref_t module_name;
    astring_ref_t exported;
    int32_t num_instructions;
    int16_t strings_sz;
    int16_t num_nesteds;
    uint8_t num_upvalues;
    uint8_t num_arguments;
    uint8_t num_constants;
    uint8_t num_local_vars;
    uint8_t num_imports;
    uint8_t _;
} aprototype_t;

ASTATIC_ASSERT(sizeof(aprototype_t) == 28);

#pragma pack(push, 1)

// Bytecode chunk.
//
// Layout: [header] [module prototype].
//
// The header portion is not affected by endianess,
// 4 bytes - signature:           0x416E7900
// 1 byte  - version:             0x{MAJOR}{MINOR}
// 1 byte  - big_endian:          1 = big, 0 = little
// 1 byte  - size of size_t:      default 8 bytes
// 1 byte  - size of integer:     default 8 bytes
// 1 byte  - size of float:       default 8 bytes
// 1 byte  - size of instruction: always 4
// 1 byte  - reserved:            always 0
// 1 byte  - reserved:            always 0
typedef struct APACKED {
    uint8_t header[12];
} achunk_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(achunk_t) == 12);

// Bytecode assembler prototype.
// `max_*` is readonly, `any_asm_reserve` are required to extends these values, 
// after that, you need to call `any_asm_prototype` and `any_asm_resolve` again.
typedef struct {
    astring_ref_t source_name;
    astring_ref_t module_name;
    astring_ref_t exported;
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

// Bytecode assembler current resolved pointers.
typedef struct {
    ainstruction_t* instructions;
    aconstant_t* constants;
    aimport_t* imports;
    int32_t* nesteds;
} aasm_current_t;

// Bytecode assembler context.
typedef struct {
    int32_t slot;
    int32_t idx;
} aasm_ctx_t;

// Bytecode assembler.
//
// WARNING!!!: `aasm_prototype_t` and `aasm_current_t` are reserved for
// advanced purpose like optimization, which provides direct access to the 
// assembler internal. The content of these structure may be relocated after 
// a call to these following functions, use it at your own risk:
// - any_asm_emit
// - any_asm_add_constant
// - any_asm_add_import
// - any_asm_reserve
// - any_asm_push
//
// This implements an assembler which can be used to produce relocatable and
// recursive chunk of bytecode. The binary chunk is portable in theory, however 
// there's no need to do that in real-life, our VM will not even try to make any 
// effort to load an incompatible chunks. Of course, if we really *need* this 
// feature, a bytecode rewriter can always be added as an external tool.
//
// The assembler is context sensitive, which can be used to authoring multiple
// prototypes with just only one single instance, in nested manner. Generally, for
// each `any_asm_push` or `any_asm_open` a new context will be created for nested 
// prototype, and that also saves the current context onto an internal stack, 
// which can be restored later by a corresponding `any_asm_pop`.
//
// This struct itself isn't POD, then rely on `achunk_t` as the portable format
// for exchanges. That format enable assembler as a *framework* to working with 
// bytecode between optimization passes on various address space.
typedef struct {
    // Allocator.
    arealloc_t realloc;
    void* realloc_ud;
    // Common string table.
    astring_table_t* st;
    // Prototype buffers.
    int32_t* slots;
    int32_t num_slots;
    int32_t max_slots;
    uint8_t* buff;
    int32_t buff_size;
    int32_t buff_capacity;
    // Contexts, limited nested level.
    aasm_ctx_t context[24];
    int32_t nested_level;
    // Output binary chunk.
    achunk_t* chunk;
    int32_t chunk_size;
    int32_t chunk_capacity;
} aasm_t;

// Number of nested level allowed for assembler.
enum {
    ANY_ASM_MAX_NESTED_LEVEL =
    (sizeof(((aasm_t*)0)->context) / sizeof(((aasm_t*)0)->context[0])) - 1
};