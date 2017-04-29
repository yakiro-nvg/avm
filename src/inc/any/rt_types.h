#pragma once

#include <any/platform.h>
#include <stdint.h>

// Primitive types.
typedef uint64_t asize_t;
typedef int64_t aint_t;
typedef double areal_t;

// Specify the operation to be performed by the instructions.
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
    AOC_INVOKE          = 40,
    AOC_RETURN          = 41,
    AOC_CLOSURE         = 42,
    AOC_CAPTURE_LOCAL   = 43,
    AOC_CAPTURE_UPVALUE = 44,
    AOC_CLOSE           = 45,
    AOC_LABEL           = 0xFF
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
=======  ========
8 bits   24 bits
=======  ========
AOC_NOP  reserved
=======  ========
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
=============  =======
8 bits         24 bits
=============  =======
AOC_GET_CONST  idx
=============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_get_const_t;

/** Push a nil value onto the stack.
\rst
===========  ========
8 bits       24 bits
===========  ========
AOC_GET_NIL  reserved
===========  ========
\endrst
*/
typedef struct {
    uint32_t _;
} ai_get_nil_t;

/** Push a bool value `val` onto the stack.
\rst
============  =======
8 bits        24 bits
============  =======
AOC_GET_BOOL  val
============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t val : 8;
} ai_get_bool_t;

/** Push a value from local pool at `idx` onto the stack.
\rst
=============  =======
8 bits         24 bits
=============  =======
AOC_GET_LOCAL  idx
=============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_get_local_t;

/** Pop a value from the stack and set it into the local pool at `idx`.
\rst
=============  =======
8 bits         24 bits
=============  =======
AOC_SET_LOCAL  idx
=============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_set_local_t;

/** Push a value from import pool at `idx` onto the stack.
\rst
==============  =======
8 bits          24 bits
==============  =======
AOC_GET_IMPORT  idx
==============  =======
\endrst
*/
typedef struct  {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_get_import_t;

/** Push a value from upvalue pool at `idx` onto the stack.
\rst
===============  =======
8 bits           24 bits
===============  =======
AOC_GET_UPVALUE  idx
===============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_get_upvalue_t;

/** Pop a value from the stack and set it into the upvalue pool at `idx`.
\rst
===============  =======
8 bits           24 bits
===============  =======
AOC_SET_UPVALUE  idx
===============  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_set_upvalue_t;

/** Unconditional jump, with signed `displacement`.
\rst
========  ============
8 bits    24 bits
========  ============
AOC_JUMP  displacement
========  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jump_t;

/** Conditional jump, with signed `displacement`.
\brief Pop a boolean value from the stack and jump if it's nil or false.
\rst
===============  ============
8 bits           24 bits
===============  ============
AOC_JUMP_IF_NOT  displacement
===============  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_jump_if_not_t;

/** Function call.

\brief 
Pop `nargs` arguments and next a closure from the stack
then call it. The result will be pushed back onto the
stack after that.

\rst
==========  =======
8 bits      24 bits
==========  =======
AOC_INVOKE  nargs
==========  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t nargs : 24;
} ai_invoke_t;

/** Returning from a function call.
\brief Top of the stack will be returned as result.
\rst
==========  ========
8 bits      24 bits
==========  ========
AOC_RETURN  reserved
==========  ========
\endrst
*/
typedef struct {
    uint32_t _;
} ai_return_t;

/** Create a closure of prototype at `idx` from pool.

\brief 
There are pseudo-instruction which follow this instruction,
that are used by VM to manage upvalues, either `AOC_CAPTURE_LOCAL`
or `AOC_CAPTURE_UPVALE`.

\rst
===========  =======
8 bits       24 bits
===========  =======
AOC_CLOSURE  idx
===========  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_closure_t;

/** Pseudo-instruction for closure creating.
\brief Capture a local value at `idx` from local pool.
\rst
=================  =======
8 bits             24 bits
=================  =======
AOC_CAPTURE_LOCAL  idx
=================  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_capture_local_t;

/** Pseudo-instruction for closure creating.
\brief Capture a upvalue at `idx` from upvalue pool.
\rst
===================  =======
8 bits               24 bits
===================  =======
AOC_CAPTURE_UPVALUE  idx
===================  =======
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t idx : 24;
} ai_capture_upvalue_t;

/** Closes all local variables in the stack from `offset` onwards (>=).

\brief 
That's required if there are upvalues present within 
those local vars, otherwise such values will go out 
of scope and disappear.

\rst
=========  =======
8 bits     24 bits
=========  =======
AOC_CLOSE  offset
=========  =======
\endrst

\note AOC_RETURN also does an implicit AOC_CLOSE.
*/
typedef struct {
    uint32_t _ : 8;
    int32_t offset : 24;
} ai_close_t;

// Variant of instruction types.
typedef union {
    ai_base_t            b;
    ai_nop_t             nop;
    ai_pop_t             pop;
    ai_get_const_t       ldk;
    ai_get_nil_t         nil;
    ai_get_bool_t        ldb;
    ai_get_local_t       llv;
    ai_set_local_t       slv;
    ai_get_import_t      imp;
    ai_get_upvalue_t     luv;
    ai_set_upvalue_t     suv;
    ai_jump_t            jmp;
    ai_jump_if_not_t     jin;
    ai_invoke_t          ivk;
    ai_return_t          ret;
    ai_closure_t         cls;
    ai_capture_local_t   clv;
    ai_capture_upvalue_t cuv;
    ai_close_t           clo;
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

AINLINE ainstruction_t ai_get_const(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_CONST;
    i.ldk.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_nil()
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_NIL;
    return i;
}

AINLINE ainstruction_t ai_get_bool(int32_t val)
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_BOOL;
    i.ldb.val = val;
    return i;
}

AINLINE ainstruction_t ai_get_local(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_LOCAL;
    i.llv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_set_local(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_SET_LOCAL;
    i.slv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_import(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_IMPORT;
    i.imp.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_get_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_GET_UPVALUE;
    i.luv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_set_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_SET_UPVALUE;
    i.suv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_jump(int32_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JUMP;
    i.jmp.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_jump_if_not(int32_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_JUMP_IF_NOT;
    i.jin.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_invoke(int32_t nargs)
{
    ainstruction_t i;
    i.b.opcode = AOC_INVOKE;
    i.ivk.nargs = nargs;
    return i;
}

AINLINE ainstruction_t ai_return()
{
    ainstruction_t i;
    i.b.opcode = AOC_RETURN;
    return i;
}

AINLINE ainstruction_t ai_closure(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_CLOSURE;
    i.cls.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_capture_local(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_CAPTURE_LOCAL;
    i.clv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_capture_upvalue(int32_t idx)
{
    ainstruction_t i;
    i.b.opcode = AOC_CAPTURE_UPVALUE;
    i.cuv.idx = idx;
    return i;
}

AINLINE ainstruction_t ai_close(int32_t offset)
{
    ainstruction_t i;
    i.b.opcode = AOC_CLOSE;
    i.clo.offset = offset;
    return i;
}

/** Allocator interface.
\brief
`old` = 0 to malloc,
`sz`  = 0 to free,
otherwise realloc.
*/
typedef void* (*arealloc_t)(void* userdata, void* old, int32_t sz);

// Used by string table
typedef struct {
    uint32_t hash;
    int32_t length;
} ahash_and_length_t;

// Reference to a string
typedef int32_t astring_ref_t;

// Constant types.
enum {
    ACT_INTEGER,
    ACT_STRING,
    ACT_REAL
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
    // ACT_REAL.
    struct {
        uint32_t _;
        areal_t val;
    } r;
} aconstant_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(aconstant_t) == 
    sizeof(uint32_t) + 
    (sizeof(aint_t) > sizeof(areal_t)
        ? sizeof(aint_t)
        : sizeof(areal_t)));

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

AINLINE aconstant_t ac_real(areal_t val)
{
    aconstant_t c;
    c.b.type = ACT_REAL;
    c.r.val = val;
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
4      signature            0x416E7900
1      is big endian?       1 = big, 0 = little
1      size of size_t       default 8 bytes
1      size of integer      default 8 bytes
1      size of float        default 8 bytes
1      size of instruction  always 4
2      reserved             always 0
=====  ===================  ===================
\endrst

\note The header portion is not affected by endianess.
*/
typedef struct APACKED {
    uint8_t header[12];
} achunk_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(achunk_t) == 12);

/** Function prototype.

\brief
This struct represents the header portion of a function prototype. Each
prototype is self-contained and relocatable, all references are relative
to their corresponding header. That means we can safety copy a memory blob 
of prototype and move it around, save to and loaded from disk without any 
need for patching.

The first prototype in chunk is `module prototype`, which is unable to be
executed. There are no instructions, upvalues, arguments, constants, local
variables and imports in this kind of prototype. All nesteds of that also
be treated as module level functions. Which may have the `exported` field 
as the symbol name, that allow this to be imported by other modules.

Memory layout:
- header 
- strings 
- instructions 
- constants 
- imports
- nested_offsets
- nesteds
*/
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

// Native function.
typedef int32_t(*anative_func_t)(void*);

// Collectable objects.
typedef struct {
    int32_t ref_count;
} agc_object_t;

// Basic value tags.
enum {
    AVT_NIL,
    AVT_BOOL,
    AVT_POINTER,
    AVT_NUMBER,
    AVT_STRING,
    AVT_FUNCTION
};

// Variant tags for AVT_FUNCTION.
enum {
    AVT_CLOSURE,
    AVT_NATIVE_FUNC,
    AVT_NATIVE_CLOSURE
};

// Variant tags for AVT_NUMBER.
enum {
    AVT_INTEGER,
    AVT_REAL
};

// Value tag.
typedef struct {
    int8_t tag;
    int8_t variant;
    int8_t collectable;
    int8_t _;
} avtag_t;

// Tagged value.
typedef struct {
    avtag_t tag;
    union {
        // AVT_STRING,
        // AVT_CLOSURE,
        // AVT_NATIVE_CLOSURE.
        agc_object_t* gc;
        // AVT_BOOL.
        int32_t b;
        // AVT_POINTER.
        void* p;
        // AVT_INTEGER.
        aint_t i;
        // AVT_REAL.
        areal_t r;
        // AVT_NATIVE_FUNC.
        anative_func_t f;
    } v;
} avalue_t;

// AVT_STRING.
typedef struct {
    agc_object_t gc;
    int32_t hash;
    char cstr[1];
} astring_t;

// AVT_CLOSURE.
typedef struct {
    agc_object_t gc;
    aprototype_t* proto;
    int32_t* imports;
} aclosure_t;

// AVT_NATIVE_CLOSURE.
typedef struct {
    agc_object_t gc;
    anative_func_t func;
} anative_closure_t;