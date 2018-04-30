// Copyright (c) 2017-2018 Nguyen Viet Giang. All rights reserved.
#ifndef _AVM_ASM_H_
#define _AVM_ASM_H_

#include <avm/prereq.h>
#include "opcode.h"
#include "chunk.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Byte code assembler.
\brief
The assembler is context sensitive, to be used to authoring multiple prototypes
in nested manner. In general, each call to \ref aasm_push or \ref aasm_open
will create a dedicated context for that new prototype. While current context
will also be saved onto an internal stack, which can be restored later by a
corresponding call to \ref aasm_pop.
*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_ASM_H_