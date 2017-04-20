/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialization as a new assembler, 
// must be followed by an `any_asm_load`.
ANY_API void any_asm_init(aasm_t* self, arealloc_t realloc, void* realloc_ud);

// Load from an external `input` chunk. 
// This function will not update `self->chunk`.
// `input` can be NULL, to make an empty assembler.
ANY_API int32_t any_asm_load(aasm_t* self, achunk_t* input);

// Update `self->chunk` to reflect the current state.
ANY_API void any_asm_save(aasm_t* self);

// This will not free the `self` pointer itself.
// Release all internal allocated memory, result as a *fresh* assembler.
ANY_API void any_asm_cleanup(aasm_t* self);

// Emit instruction, 
// returns the index of emitted instruction.
ANY_API int32_t any_asm_emit(aasm_t* self, ainstruction_t instruction);

// Add constant,
// returns the index of added constant.
ANY_API int32_t any_asm_add_constant(aasm_t* self, aconstant_t constant);

// Add import,
// returns the index of added import.
ANY_API int32_t any_asm_add_import(aasm_t* self, aimport_t import);

// Create nested prototype,
// a new context will be pushed onto the stack,
// returns the index of added prototype. 
ANY_API int32_t any_asm_push(aasm_t* self);

// Open an existing nested prototype at `idx`,
// a new context will be pushed onto the stack.
ANY_API void any_asm_open(aasm_t* self, int32_t idx);

// Remove current context, go back to the parent prototype,
// returns the index of popped prototype.
ANY_API int32_t any_asm_pop(aasm_t* self);

// Wrapper of `any_st_to_ref` to internal string table, 
// automatically grow if neccessary.
ANY_API astring_ref_t any_asm_string_to_ref(aasm_t* self, const char* s);

// Extend prototype with more capacity.
ANY_API void any_asm_grow(
    aasm_t* self,
    uint32_t max_instructions,
    uint8_t max_constants,
    uint8_t max_imports,
    uint8_t max_nesteds);

// Get prototype header.
ANY_API aasm_prototype_t* any_asm_prototype(aasm_t* self);

// Resolve prototype pointers.
ANY_API aasm_current_t any_asm_resolve(aasm_t* self);

// Get prototype at `slot`.
inline aasm_prototype_t* any_asm_prototype_at(aasm_t* self, int32_t slot)
{
    return (aasm_prototype_t*)(self->buff + self->slots[slot]);
}

// Instruction constructors.
inline ainstruction_t ai_nop()
{
    ainstruction_t i;
    i.base.opcode = AOC_NOP;
    return i;
}

inline ainstruction_t ai_pop(int32_t n)
{
    ainstruction_t i;
    i.base.opcode = AOC_POP;
    i.pop.n = n;
    return i;
}

inline ainstruction_t ai_get_const(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_CONST;
    i.get_const.idx = idx;
    return i;
}

inline ainstruction_t ai_get_nil()
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_NIL;
    return i;
}

inline ainstruction_t ai_get_bool(int32_t val)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_BOOL;
    i.get_bool.val = val;
    return i;
}

inline ainstruction_t ai_get_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_LOCAL;
    i.get_local.idx = idx;
    return i;
}

inline ainstruction_t ai_set_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_SET_LOCAL;
    i.set_local.idx = idx;
    return i;
}

inline ainstruction_t ai_get_import(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_IMPORT;
    i.get_import.idx = idx;
    return i;
}

inline ainstruction_t ai_get_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_GET_UPVALUE;
    i.get_upvalue.idx = idx;
    return i;
}

inline ainstruction_t ai_set_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_SET_UPVALUE;
    i.set_upvalue.idx = idx;
    return i;
}

inline ainstruction_t ai_jump(int32_t displacement)
{
    ainstruction_t i;
    i.base.opcode = AOC_JUMP;
    i.jump.displacement = displacement;
    return i;
}

inline ainstruction_t ai_jump_if_not(int32_t displacement)
{
    ainstruction_t i;
    i.base.opcode = AOC_JUMP_IF_NOT;
    i.jump_if_not.displacement = displacement;
    return i;
}

inline ainstruction_t ai_call(int32_t nargs)
{
    ainstruction_t i;
    i.base.opcode = AOC_CALL;
    i.call.nargs = nargs;
    return i;
}

inline ainstruction_t ai_return()
{
    ainstruction_t i;
    i.base.opcode = AOC_RETURN;
    return i;
}

inline ainstruction_t ai_closure(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CLOSURE;
    i.closure.idx = idx;
    return i;
}

inline ainstruction_t ai_capture_local(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CAPTURE_LOCAL;
    i.capture_local.idx = idx;
    return i;
}

inline ainstruction_t ai_capture_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.base.opcode = AOC_CAPTURE_UPVALUE;
    i.capture_upvalue.idx = idx;
    return i;
}

inline ainstruction_t ai_close(int32_t offset)
{
    ainstruction_t i;
    i.base.opcode = AOC_CLOSE;
    i.close.offset = offset;
    return i;
}

#ifdef __cplusplus
} // extern "C"
#endif