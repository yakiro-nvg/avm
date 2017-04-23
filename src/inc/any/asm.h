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
ANY_API int32_t any_asm_load(aasm_t* self, const achunk_t* input);

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

// Wrapper of `any_st_to_ref` to `self->st` string table, 
// automatically grow if neccessary.
ANY_API astring_ref_t any_asm_string_to_ref(aasm_t* self, const char* s);

// Extend prototype with more capacity.
ANY_API void any_asm_reserve(
    aasm_t* self,
    int32_t max_instructions,
    uint8_t max_constants,
    uint8_t max_imports,
    int16_t max_nesteds);

// Get prototype header.
ANY_API aasm_prototype_t* any_asm_prototype(aasm_t* self);

// Resolve prototype pointers.
ANY_API aasm_current_t any_asm_resolve(aasm_t* self);

// Get prototype at `slot`.
AINLINE aasm_prototype_t* any_asm_prototype_at(aasm_t* self, int32_t slot)
{
    return (aasm_prototype_t*)(self->buff + self->slots[slot]);
}

#ifdef __cplusplus
} // extern "C"
#endif