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
AVM is dynamic, stack based abstract machine which relies on an efficient byte
code representation called `code chunk`. That is just a binary / compiled form
of the source code. And that was designed to be able to load and execute by as
little effort as possible. Code chunk is `portable` in theory. However, there is
no need to do that in real-life. AVM will not even try to load an incompatible
chunk. If we really **need** this feature, considering a separated rewriter to
transforms it first, then feeds the resulted compatible chunk instead.

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
    u8 debug;
    u8 _;
} acode_chunk_t;

// Chunk header size is fixed 8 bytes.
ASTATIC_ASSERT(sizeof(acode_chunk_t) == 8);

/** Byte code prototype.
\brief
AVM program was structured as modules, which is just a named group of function,
constant and attribute. Module can **import** exported value from other module,
which will be resolved at the runtime by linker. These resolved values resides
in a dedicated pool, which can be pushed onto the stack by **imp** instruction.
Besides, by loading an already loaded module, we do hot-reloading, in this case,
the linker will try to update these imported pool to points to our new one. So
consecutive **imp** will uses that version. However, value that was pushed onto
the stack remains unchanged thought.

Byte code module, function and it's compiled data are represented by simple and
recursive structure named `prototype`. Prototype is self-contained, portable in
theory and relocation independent. We can safely copy a memory blob prototype,
moving it around, doing the serialization by just a simple fread / fwrite, and
mmap as well. Let's think about it as data, actually it is, we can even embed it
as a C header in the ROM section of embedded devices.

The root prototype must be a `module prototype`, each code chunk must contains
one and only one module, by convention. There are no non-module function in AVM,
all functions must be defined inside a module. `module` prototype (top-level)
only consists of nested function and constant. That means instruction, up-value,
variable and import will not be allowed to be defined at this level. Because it
either doesn't make any sense, or prohibited to get rid of race conditions. In
other words, there is no global mutable things in AVM. It isn't work as Lua, we
won't use byte code execution to update the module table. Let's think about it
as the traditional OS executable file, it is immutable, obvioulsy thread safe as
well. However, for AVM it still need to be modified dynammically, in the case of
hot reloading. When that happen the AVM must suspends its execution, then switch
to a special mode, rebuilding the table, and go back. In fact, it is mutable,
but that doesn't really matter from the byte code point of view.

Module level function can be exported by a symbolic name, if `symbol to export`
isn't negative. Otherwise this means a private function, which can only be used
by other function in the same module. The absolute value of symbol here is an
index that points to the `string pool` (more later). AVM modules are immutable.
That means we can't refer to these function as a variable, there are no variable
in this situation as well. To refer to the private module function we also need
to **imp** it as the same way as public exported values. It sounds pretty weird
at glance, but it makes code chunk safe to be shared. Besides that, the nested,
or to be more precise the anonymous closure, which eventually bound to variable
is private to lexical scope, and apparently private to the current VM instance.
We can't share that variable to the outside world, obviously in this case there
is no race condition to worry about. We have a dedicated instruction to refer to
these anonymous closure named **cls** also. For conclusion, module-function has
symbolic name, which can be referred by **imp**. While anonymous closure doesn't
and relies on **cls** instruction, the `symbol to export` in this case should be
zero, obviously. However, that doesn't really matter since AVM treat this field
as reserved (unused), it just simply ignores.

Small integer may be encoded directly into the `AOC_LSI` instruction. However,
there are more complex, non trivial data eg. constant, string, import etc. That
wasn't fit to a fixed 32 bit instruction. We need dedicated pool to hold these
values, which must be referred using index as well. We don't want to be messed
with variable sized instruction and unaligned access. That really pains in the
body to deal with. For constant, we got number and string, both of that will be
represented by an IEEE 754 double, 8 bytes aligned. Constant string was boxed by
NaN, with the payload part is an index that points to the string pool. If that
setup doesn't work, we may provides additional compile time option to switch to
another less efficient representation, but that wasn't present yet.

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