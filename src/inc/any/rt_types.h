#pragma once

#include <any/platform.h>
#include <stdint.h>

// Primitive types.
typedef uint64_t asize_t;
typedef int64_t aint_t;
typedef double areal_t;

// Specify the operation to be performed by the instructions.
enum AOPCODE {
    AOC_NOP = 0,
    AOC_POP = 1,
    AOC_LDK = 10,
    AOC_NIL = 11,
    AOC_LDB = 12,
    AOC_LLV = 13,
    AOC_SLV = 14,
    AOC_IMP = 15,
    AOC_JMP = 30,
    AOC_JIN = 31,
    AOC_IVK = 40,
    AOC_RET = 41
};

/** Base type.
\rst
======  =======
8 bits  24 bits
======  =======
opcode  payload
======  =======
\endrst
*/ 
typedef struct {
    uint32_t opcode : 8;
    uint32_t _ : 24;
} ai_base_t;

/** No operation
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NOP  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_nop_t;

/** Pop `n` elements from the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_POP  n
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t n : 24;
} ai_pop_t;

/** Push a constant from const pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDK  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_ldk_t;

/** Push a nil value onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_NIL  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_nil_t;

/** Push a bool value `val` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LDB  val
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t val : 8;
} ai_ldb_t;

/** Push a value from local pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_LLV  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_llv_t;

/** Pop a value from the stack and set it into the local pool at `idx`.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_SLV  idx
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_slv_t;

/** Push a value from import pool at `idx` onto the stack.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IMP  idx
=======  =======
\endrst
*/
typedef struct  {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_imp_t;

/** Unconditional jump, with signed `displacement`.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JMP  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jmp_t;

/** Conditional jump, with signed `displacement`.
\brief Pop a boolean value from the stack and jump if it's nil or false.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_JIN  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jin_t;

/** Function call.

\brief 
Pop `nargs` arguments and next a closure from the stack
then call it. The result will be pushed back onto the
stack after that.

\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_IVK  nargs
=======  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t nargs : 24;
} ai_ivk_t;

/** Returning from a function call.
\brief Top of the stack will be returned as result.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RET  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_ret_t;

/// Variant of instruction types, instruction size is fixed 4 bytes.
typedef union {
    ai_base_t b;
    ai_nop_t nop;
    ai_pop_t pop;
    ai_ldk_t ldk;
    ai_nil_t nil;
    ai_ldb_t ldb;
    ai_llv_t llv;
    ai_slv_t slv;
    ai_imp_t imp;
    ai_jmp_t jmp;
    ai_jin_t jin;
    ai_ivk_t ivk;
    ai_ret_t ret;
} ainstruction_t;

ASTATIC_ASSERT(sizeof(ainstruction_t) == 4);

// Instruction constructors.
AINLINE ainstruction_t ai_nop()
{
    ainstruction_t i;
    i.b.opcode = AOC_NOP;
    return i;
}

AINLINE ainstruction_t ai_pop(int32_t n)
{
    ainstruction_t i;
    i.b.opcode = AOC_POP;
    i.pop.n = n;
    return i;
}

AINLINE ainstruction_t ai_ldk(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_LDK;
    i.ldk.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_nil()
{
    ainstruction_t i;
    i.b.opcode = AOC_NIL;
    return i;
}

AINLINE ainstruction_t ai_ldb(int32_t val)
{
    ainstruction_t i;
    i.b.opcode = AOC_LDB;
    i.ldb.val = val;
    return i;
}

AINLINE ainstruction_t ai_llv(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_LLV;
    i.llv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_slv(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_SLV;
    i.slv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_imp(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_IMP;
    i.imp.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_jmp(int32_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JMP;
    i.jmp.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_jin(int32_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JIN;
    i.jin.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_ivk(int32_t nargs)
{
    ainstruction_t i;
    i.b.opcode = AOC_IVK;
    i.ivk.nargs = nargs;
    return i;
}

AINLINE ainstruction_t ai_ret()
{
    ainstruction_t i;
    i.b.opcode = AOC_RET;
    return i;
}

/** Allocator interface.
\brief
`old` = 0 to malloc,
`sz`  = 0 to free,
otherwise realloc.
*/
typedef void* (*arealloc_t)(void* userdata, void* old, int32_t sz);

/// Reference to a string
typedef int32_t astring_ref_t;

/// Native function.
typedef int32_t(*anative_func_t)(void*);

/// Native module function.
typedef struct {
    const char* name;
    anative_func_t func;
} anative_module_func_t;

/// Native module.
typedef struct {
    const char* name;
    const anative_module_func_t* funcs;
} anative_module_t;

/// Basic value tags.
enum ATB {
    /// No value.
    ATB_NIL,
    /// Either `true`: 1 or `false`: 0.
    ATB_BOOL,
    /// Raw pointer.
    ATB_POINTER,
    /// Constant string.
    ATB_CONST_STRING,
    /// Variant of number.
    ATB_NUMBER,
    /// Variant of function.
    ATB_FUNCTION
};

/// Variant tags for \ref ATB_FUNCTION.
enum AVT_FUNCTION {
    AVTF_NATIVE,
    AVTF_PURE
};

/// Variant tags for \ref ATB_NUMBER.
enum AVT_NUMBER {
    /// No fractional.
    AVTN_INTEGER,
    /// Floating-point number.
    AVTN_REAL
};

/// Value tag.
typedef struct {
    int8_t tag;
    int8_t variant;
    int8_t _[2];
} avtag_t;

/** Tagged value.

\brief
AVM is dynamically typed. Each value carries its own type in `avtag_t`. There are
some values which are subject to automatic memory management, which are marked 
`collectable`. The scope of such values are not corresponding to the lexical 
scope, opposite to `non collectable` values which are placed on the stack and will 
be disappeared as soon as the stack grow back. 

\note `collectable` type is also called `reference type`. 
*/
typedef struct {
    avtag_t tag;
    union {
        /// \ref ATB_BOOL.
        int32_t b;
        /// \ref ATB_POINTER.
        void* p;
        /// \ref ATB_CONST_STRING.
        astring_ref_t cs;
        /// \ref AVT_INTEGER.
        aint_t i;
        /// \ref AVT_REAL.
        areal_t r;
        /// \ref AVT_NATIVE_FUNC.
        anative_func_t f;
        /// \ref AVT_MODULE_FUNC.
        struct aprototype_s* mf;
    } v;
} avalue_t;

#pragma pack(push, 1)

/** Bytecode chunk.

\brief Layout: [`header`] [`module prototype`].

AVM is dynamic, stack based abstract machine which rely on a strong coupled,
relocatable and recursive intermediate representation called `bytecode chunk`.
That designed to be loaded and executed directly from the storages with as 
least effort as possible.

The bytecode chunk includes everything we need for a module to be executed
except the imports. This chunk is portable in theory, however there's no need
to actually do that in real-life. AVM will not even try to make any effort to
load an incompatible chunks. Of course, if we really **need** this feature, a
bytecode rewriter can always be implemented as an external tool.

\par Header format.
\rst
=====  ===================  ===================
bytes  description          value
=====  ===================  ===================
4      signature            0x41 0x6E 0x79 0x00
1      version number       0x{MAJOR}{MINOR}
1      is big endian?       1 = big, 0 = little
1      size of size_t       default 8 bytes
1      size of integer      default 8 bytes
1      size of float        default 8 bytes
1      size of instruction  always 4
2      _                    _
=====  ===================  ===================
\endrst

\note The header portion is not affected by endianess.
*/
typedef struct APACKED {
    uint8_t  signature[4];

    uint32_t major_ver       : 4;
    uint32_t minor_ver       : 4;
    uint32_t big_endian      : 8;
    uint32_t size_of_size_t  : 8;
    uint32_t size_of_integer : 8;

    uint8_t  size_of_float;
    uint8_t  size_of_instruction;
    uint8_t  _[2];
} achunk_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(achunk_t) == 12);

/// Function import.
typedef struct {
    astring_ref_t module;
    astring_ref_t name;
    avalue_t resolved;
} aimport_t;

ASTATIC_ASSERT(sizeof(aimport_t) == 8 + sizeof(avalue_t));

AINLINE aimport_t aimport(astring_ref_t module, astring_ref_t name)
{
    aimport_t i;
    i.module = module;
    i.name = name;
    return i;
}

/// Resolved prototype pointers.
typedef struct {
    ainstruction_t* instructions;
    avalue_t* constants;
    aimport_t* imports;
    struct aprototype_s** nesteds;
} acurrent_t;

/** Function prototype.

\brief
This struct represents the header portion of a function prototype. Each
prototype is self-contained and relocatable, all references are relative
to their corresponding header. That means we can safety copy a memory blob 
of prototype and move it around, save to and loaded from disk without any 
need for patching.

The first prototype in chunk is `module prototype`, which is unable to be
executed. There are no instructions, upvalues, arguments, constants, local
variables and imports in this kind of prototype. All nesteds of that will
be treated as `module function`. Which can't accept upvalues and will be 
exported as the name specified in the `symbol` field. That allow such 
functions to be imported later by other modules.

In AVM, there are two kind of string, `static` and `collectable`. Compile
time strings are `static` and must be referenced by `astring_ref_t` which
is just an offset to the dedicated **per prototype** string pool. We don't
want to share these pools between prototype to make relocating trivial.

To reduce the loading time and make memory management simpler, prototype
must be able to loaded from the storages as-is by just a single read. No 
more memory allocation are required. The prototype format itself reserved 
few bytes to be used by the runtime. Remember that, despite being called 
`bytecode`, we don't prefer polite and portable bytecode over performance 
and simplicity. For each platform, you are expected to run the compiler 
or rewriter again.The runtime is stupid and don't want to take challenges. 
The best things that AVM runtime provides is just sandbox guarantees, by 
prevent malformed bytecode to crash the system, nothing more.

\par Memory layout:
\rst
====================================  =========================  =======
bytes                                 description                scope
====================================  =========================  =======
4 signed                              source file name           _
4 signed                              to be exported             _
4 signed                              num of string bytes        _
4 signed                              num of instructions        _
2 signed                              num of nesteds             _
1 unsigned                            num of upvalues            _
1 unsigned                            num of arguments           _
1 unsigned                            num of constants           _
1 unsigned                            num of local variables     _
1 unsigned                            num of imports             _
1                                     _                          _
sizeof(:c:type:`acurrent_t`)          resolved pointers          runtime
num of string bytes                   pool of strings            _
n * sizeof(:c:type:`ainstruction_t`)  pool of instructions       _
n * sizeof(:c:type:`avalue_t`)        pool of constants          _
n * sizeof(:c:type:`aimport_t`)       pool of imports            _
n * sizeof(void*)                     nested prototype pointers  runtime
_                                     nested prototypes          _
====================================  =========================  =======
\endrst
\note Portions with `runtime scope` are reversed in compile time.
*/
typedef struct aprototype_s {
    astring_ref_t source;
    astring_ref_t symbol;
    int32_t strings_sz;
    int32_t num_instructions;
    int16_t num_nesteds;
    uint8_t num_upvalues;
    uint8_t num_arguments;
    uint8_t num_constants;
    uint8_t num_local_vars;
    uint8_t num_imports;
    uint8_t _;
    acurrent_t resolved;
} aprototype_t;

ASTATIC_ASSERT(sizeof(aprototype_t) == 24 + sizeof(acurrent_t));