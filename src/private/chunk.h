// Copyright (c) 2017-2018 Giang "Yakiro" Nguyen. All rights reserved.
#ifndef _AVM_CHUNK_H_
#define _AVM_CHUNK_H_

#include <avm/prereq.h>

#ifdef __cplusplus
extern "C" {
#endif

/// NaN boxed constant.
typedef union aconstant_u {
    f64 f;
    u64 u;
} aconstant_t;
AALIGNAS(aconstant_t, 8)

/// Constant size is fixed 8 bytes.
ASTATIC_ASSERT(sizeof(aconstant_t) == 8);

/// Function import.
typedef struct aimport_s {
    u16 module;
    u16 symbol;
} aimport_t;
AALIGNAS(aimport_t, 4)

/// Import size is fixed 4 bytes.
ASTATIC_ASSERT(sizeof(aimport_t) == 4);

/** Byte code chunk.
\brief Layout: [`header`] [`module prototypes`]
AVM is dynamic, stack based abstract machine which rely on a efficient byte code
representation called `code chunk`. That chunks is designed to be able to loads
and executes directly by as little effort as possible. The code chunk includes
everything AVM needs to execute a module, except external value (import). Code
chunk is `portable` in theory. However, there's no need to do that in real-life.
AVM will not even try to load an incompatible chunk. If we really **need** this
feature, considering a separated rewriter to transforms it first then feeds the
resulted compatible chunk to our poor AVM.

\par Header format:
\rst
=====  ===================  ===================
bytes  description          value
=====  ===================  ===================
4      signature            0x41 0x6E 0x79 0x00
1      version number       0x{MAJOR}{MINOR}
1      is big endian?       1 = big, 0 = little
1      num of modules       unsigned byte
1      is debug?            1 = yes, 0 = no
=====  ===================  ===================
\endrst
*/
typedef struct acode_chunk_s {
    u8 signature[4];
    u8 major_ver : 4;
    u8 minor_ver : 4;
    u8 big_endian;
    u8 num_modules;
    u8 debug;
} acode_chunk_t;

// Chunk header size is fixed 8 bytes.
ASTATIC_ASSERT(sizeof(acode_chunk_t) == 8);

/** Byte code prototype.
\brief
AVM byte code module, function and it's compiled data are represents by a simple
recursive structure named `prototype`. Prototype is self-contained, portable in
theory and relocatable. We can safely copy a memory blob of prototype, moving it
around, serialization by just a simple fread/fwrite, and mmap as well.

The root prototype must be a `module prototype`, there are no global function in
AVM, all functions must be defined inside a module. The `module` prototype only
consists of nested function and constant. That means the instruction, up-value,
local variable and import will not be allowed to be defined at this level. That
either don't make any sense or strictly prohibited to prevent the possibility of
race conditions.

Module level function may be exported by a symbolic name if `symbol to export`
isn't negative. Otherwise this means a private function, which can only be used
by other functions in the same module. The absolute value of symbol here is an
index that points to the `string pool` (more later). AVM treats module symbols
as `immutable` (however, can still be explicitly updated by loader). In other
words we can't refer to these function as a variable, there are no variable in
this situation as well. Therefore, to get reference to a private module function
we also need to `import`s it the same way as public exported. That sounds weird
at glance, but it makes our code chunk safe to shares in threaded environment.
Besides that, the nested, or to be more precise anonymous closure, which bounds
to a variable is private to lexical scope, and apparently private to the current
VM instance. We can't share that variable no anyone from the outside, so there
is no race condition to worry about. But all of that means it isn't exportable
and must have a zero symbol.

Small integer may be encoded directly in the `AOC_LSI` instruction, but for more
complex, non trivial data likes constant, string and import we need a dedicated
corresponding pool for each type. That must be referred using index as well. The
reason behind this decision is simple, we don't want to be messed with variable
sized instruction and unaligned access. Constant is IEEE 754 double that must be
8 bytes aligned. Constant string is boxed by NaN, with the payload is an index
that points to the string pool.

Debug information must be stripped if \ref acode_chunk_t `.debug` is `no`.

\par Memory layout:
\rst
===================================================  ====================
bytes                                                description
===================================================  ====================
sizeof(s16)                                          symbol to export
sizeof(u16)                                          source file name
sizeof(u16)                                          num string bytes
sizeof(u16)                                          num local variables
sizeof(u16)                                          num instructions
sizeof(u16)                                          num nesteds
sizeof(u16)                                          num constants
sizeof(u16)                                          num imports
num string bytes                                     strings
num instructions * sizeof(:c:type:`ainstruction_t`)  instructions
num constants * sizeof(:c:type:`aconstant_t`)        constants
num nesteds * sizeof(:c:type:`aimport_t`)            imports
num instructions * sizeof(u16)                       debug source lines
num instructions * sizeof(u16)                       debug source columns
num local variables * sizeof(u16)                    debug variable names
_                                                    nested prototypes
===================================================  ====================
\endrst
*/
typedef struct acode_proto_s {
    s16 symbol;
    u16 source;
    u16 string_bytes;
    u16 num_local_vars;
    u16 num_instructions;
    u16 num_nesteds;
    u16 num_constants;
    u16 num_imports;
} acode_proto_t;

// Prototype header size is fixed 16 bytes.
ASTATIC_ASSERT(sizeof(acode_proto_t) == 16);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !_AVM_CHUNK_H_