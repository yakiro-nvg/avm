/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#include <any/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialization as a new assembler.
\note Must be followed by an \ref aasm_load.
*/
ANY_API void
aasm_init(
    aasm_t* self, aalloc_t alloc, void* alloc_ud);

/** Load from an external `input` chunk.
\brief This function will not update `self->chunk`.
\note `input` can be NULL, to make an empty assembler.
*/
ANY_API aerror_t
aasm_load(
    aasm_t* self, const achunk_header_t* input);

/// Update `self->chunk` to reflect the current state.
ANY_API void
aasm_save(
    aasm_t* self);

/// Release all internal allocated memory, result as a *fresh* assembler.
ANY_API void
aasm_cleanup(
    aasm_t* self);

/** Emit new instruction.
\return The index of emitted instruction.
*/
ANY_API aint_t
aasm_emit(
    aasm_t* self, ainstruction_t instruction, aint_t source_line);

/** Add new constant.
\return The index of added constant.
*/
ANY_API aint_t
aasm_add_constant(
    aasm_t* self, aconstant_t constant);

/** Add new import.
\return The index of added import.
*/
ANY_API aint_t
aasm_add_import(
    aasm_t* self, const char* module, const char* name);

/** Create nested prototype at top (module) level.
\see \ref aasm_push
*/
ANY_API aint_t
aasm_module_push(
    aasm_t* self, const char* name);

/** Create nested prototype.
\brief New context will be pushed onto the stack,
\return The index of added prototype.
*/
ANY_API aint_t
aasm_push(
    aasm_t* self);

/** Open an existing nested prototype at `idx`.
\brief New context will be pushed onto the stack.
*/
ANY_API void
aasm_open(
    aasm_t* self, aint_t idx);

/** Remove current context, go back to the parent prototype.
\return The index of popped prototype.
*/
ANY_API aint_t
aasm_pop(
    aasm_t* self);

/** Wrapper of \ref astring_table_to_ref to `self->st` string table.
\brief Automatically grow if necessary.
*/
ANY_API aint_t
aasm_string_to_ref(
    aasm_t* self, const char* string);

/// Extend prototype with more capacity.
ANY_API void
aasm_reserve(
    aasm_t* self, const aasm_reserve_t* sz);

/// Get prototype header.
ANY_API aasm_prototype_t*
aasm_prototype(
    aasm_t* self);

/// Resolve prototype pointers.
ANY_API aasm_current_t
aasm_resolve(
    aasm_t* self);

/// Get prototype at `slot`.
static AINLINE aasm_prototype_t*
aasm_prototype_at(
    aasm_t* self, aint_t slot)
{
    return (aasm_prototype_t*)(self->_buff + self->_slots[slot]);
}

#ifdef __cplusplus
} // extern "C"
#endif