#pragma once

#include <any/platform.h>
#include <any/list.h>
#include <any/task.h>

// Primitive types.
typedef int64_t aint_t;
typedef double  areal_t;

// Specify the operation to be performed by the instructions.
typedef enum {
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
    AOC_RET = 41,
    AOC_SND = 50,
    AOC_RCV = 51,
    AOC_RMV = 52
} AOPCODE;

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
\brief Please refer \ref aprocess_t for the protocol.
\note Result will be placed on top of the stack.
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
\brief Top of the stack will be returned as the result.
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

/** Pop a target pid and next a message from the stack and send it.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_SND  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_snd_t;

/** Picks up next message in the queue and places it onto the stack.
\brief If there is no more message, jump to signed `displacement`.
A `timeout` value will be popped from the stack, that will be used by AVM to
determine if it must wait for this period before it decided that there is no
more message in the queue. This `timeout` is signed and in milliseconds, a
negative value means infinite waiting.
\note This instruction will not remove messages from the queue.
\rst
=======  ============
8 bits   24 bits
=======  ============
AOC_RCV  displacement
=======  ============
\endrst
*/
typedef struct {
    uint32_t _ : 8;
    int32_t displacement : 24;
} ai_rcv_t;

/** Remove current message which is previously peeked by \ref ai_rct_t.
\brief The peek pointer will be rewound to the front.
\rst
=======  =======
8 bits   24 bits
=======  =======
AOC_RMV  _
=======  =======
\endrst
*/
typedef struct {
    uint32_t _;
} ai_rmv_t;

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
    ai_snd_t snd;
    ai_rcv_t rcv;
    ai_rmv_t rmv;
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

AINLINE ainstruction_t ai_snd()
{
    ainstruction_t i;
    i.b.opcode = AOC_SND;
    return i;
}

AINLINE ainstruction_t ai_rcv(int32_t displacement)
{
    ainstruction_t i;
    i.b.opcode = AOC_RCV;
    i.rcv.displacement = displacement;
    return i;
}

AINLINE ainstruction_t ai_rmv()
{
    ainstruction_t i;
    i.b.opcode = AOC_RMV;
    return i;
}

/** Allocator interface.
\brief
`old` = 0 to malloc,
`sz`  = 0 to free,
otherwise realloc.
*/
typedef void* (*aalloc_t)(void* ud, void* old, int32_t sz);

/// Process identifier.
typedef uint32_t apid_t;
typedef uint32_t apid_idx_t;
typedef uint32_t apid_gen_t;
enum { APID_BITS = sizeof(apid_t)*8 };

/// Make a pid from index and generation.
AINLINE apid_t apid_from(
    int8_t idx_bits, int8_t gen_bits, apid_idx_t idx, apid_gen_t gen)
{
    return
        (idx << (APID_BITS - idx_bits)) |
        (gen << (APID_BITS - idx_bits - gen_bits));
}

/// Get index part.
AINLINE apid_idx_t apid_idx(int8_t idx_bits, apid_t pid)
{
    return pid >> (APID_BITS - idx_bits);
}

/// Get generation part.
AINLINE apid_gen_t apid_gen(int8_t idx_bits, int8_t gen_bits, apid_t pid)
{
    return (pid << idx_bits) >> (APID_BITS - gen_bits);
}

/// Reference to a string
typedef int32_t astring_ref_t;

/// Native function.
typedef void(*anative_func_t)(struct aprocess_t*);

/// Basic value tags.
typedef enum {
    /// No value.
    ABT_NIL,
    /// Process identifier.
    ABT_PID,
    /// Either `true`: 1 or `false`: 0.
    ABT_BOOL,
    /// Raw pointer.
    ABT_POINTER,
    /// Constant string.
    ABT_CONSTANT_STRING,
    /// Variant of number.
    ABT_NUMBER,
    /// Variant of function.
    ABT_FUNCTION
} ABT;

/// Variant tags for \ref ABT_NUMBER.
typedef enum {
    /// No fractional.
    AVTN_INTEGER,
    /// Floating-point number.
    AVTN_REAL
} AVT_NUMBER;

/// Variant tags for \ref ABT_FUNCTION.
typedef enum {
    /// C lib function.
    AVTF_NATIVE,
    /// Byte code function.
    AVTF_AVM
} AVT_FUNCTION;

/// Value tag.
typedef struct {
    int8_t b;
    int8_t variant;
    int8_t _[2];
} avalue_tag_t;

/// Tagged value.
typedef struct {
    avalue_tag_t tag;
    union {
        /// \ref ABT_PID.
        apid_t pid;
        /// \ref ABT_BOOL.
        int32_t boolean;
        /// \ref ABT_POINTER.
        void* ptr;
        /// \ref ABT_CONST_STRING.
        astring_ref_t const_string;
        /// \ref AVTN_INTEGER.
        aint_t integer;
        /// \ref AVTN_REAL.
        areal_t real;
        /// \ref AVTF_NATIVE.
        anative_func_t func;
        /// \ref AVTF_AVM.
        struct aprototype_t* avm_func;
    } v;
} avalue_t;

#pragma pack(push, 1)

/** Byte code chunk.
\brief Layout: [`header`] [`module prototype`].
AVM is dynamic, stack based abstract machine which rely on a strong coupled,
relocatable and recursive intermediate representation called `byte code chunk`.
That is designed to be loaded and executed directly from the storages with as
least effort as possible. The byte code chunk includes everything we need for
a module to be executed except the external value (called import). This chunk
is portable in theory. However, there is no need to actually do that in real-
life. AVM will not even try to make any effort to load an incompatible chunks.
Of course, if we really **need** this as a feature, byte code rewriter could
be implemented as an external tool.
\par Header format.
\rst
=====  ===================  ===================
bytes  description          value
=====  ===================  ===================
4      signature            0x41 0x6E 0x79 0x00
1      version number       0x{MAJOR}{MINOR}
1      is big endian?       1 = big, 0 = little
1      size of integer      default 8 bytes
1      size of float        default 8 bytes
1      size of instruction  always 4
3      _                    _
=====  ===================  ===================
\endrst
\note The header portion is not affected by endianess.
*/
typedef struct APACKED {
    uint8_t  signature[4];
    uint32_t major_ver       : 4;
    uint32_t minor_ver       : 4;
    uint32_t big_endian      : 8;
    uint32_t size_of_integer : 8;
    uint32_t size_of_float   : 8;
    uint8_t  size_of_instruction;
    uint8_t  _[3];
} achunk_header_t;

#pragma pack(pop)

ASTATIC_ASSERT(sizeof(achunk_header_t) == 12);

/// Function import.
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

/** Function prototype.
\brief
This struct represents the header portion of a function prototype. Prototype is
self-contained and relocatable, references are relative to their corresponding
header. That means we can safety copy a memory blob of prototype and move that
around, save to and loaded from disk without any need for patching.

The first prototype in chunk is `module prototype`, there are no global scope in
AVM. The most outer scope is `module`, which is only consists of nested function
and constant. There are no instruction, up-value, argument, local var and import
in this kind of prototype. Either that make non-sense or strictly prohibited to
prevent the possibility of race condition. Nested of module prototype is `module
function`. Which can be exported by the name specified in `symbol`. That allows
such functions to be imported to use by other modules.

In AVM, there are two kind of string, `static` and `collectable`. Compile time
string are `static` and must be referenced by `astring_ref_t` which is just an
offset to the dedicated **per prototype** string pool. We don't want to share
these pools between prototype to make relocating trivial. General speaking, the
whole prototype is read-only, which is only accessible using related instruction
to ensure the thread safety and security, we could easily mmap a prototype from
storages, as well as traditional OS executable.

\par Memory layout:
\rst
====================================  ======================
bytes                                 description
====================================  ======================
4 signed                              source file name
4 signed                              to be exported
4 signed                              num of string bytes
4 signed                              num of instructions
2 signed                              num of nesteds
1 unsigned                            num of up-values
1 unsigned                            num of arguments
1 unsigned                            num of constants
1 unsigned                            num of local variables
1 unsigned                            num of imports
1                                     _
num of string bytes                   strings
n * sizeof(:c:type:`ainstruction_t`)  instructions
n * sizeof(:c:type:`avalue_t`)        constants
n * sizeof(:c:type:`aimport_t`)       imports
_                                     nested prototypes
====================================  ======================
\endrst
*/
typedef struct {
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
} aprototype_header_t;

ASTATIC_ASSERT(sizeof(aprototype_header_t) == 24);

/// Lib function.
typedef struct {
    const char* name;
    anative_func_t func;
} alib_func_t;

/// Lib module.
typedef struct {
    const char* name;
    const alib_func_t* funcs;
    alist_node_t node;
} alib_t;

/// Runtime prototype.
typedef struct aprototype_t {
    struct achunk_t* chunk;
    aprototype_header_t* header;
    const char* strings;
    ainstruction_t* instructions;
    avalue_t* constants;
    aimport_t* imports;
    struct aprototype_t* nesteds;
    avalue_t* import_values;
} aprototype_t;

/// Runtime byte code chunk.
typedef struct achunk_t {
    achunk_header_t* header;
    aalloc_t alloc;
    void* alloc_ud;
    avalue_t* imports;
    aprototype_t* prototypes;
    alist_node_t node;
    int32_t retain;
} achunk_t;

/** Byte code loader.
\brief
AVM byte code loading and linking is done by `aloader_t`, with heavily focused
to be able to support `hot reloading`. That means we can introduce a new version
of already loaded module on-the-fly, without the need of shut down the system.
There are 3 state of chunk which is managed by loader, `pending`, `running` and
`garbage`. Newly added chunk is `pending`, which is invisible until successfully
be linked with other chunk and get its imports resolved. `running` is where the
latest chunks are stored, `aloader_find` will try to search in there. The last
state is `garbage`, as the name suggested, is out-of-date but still be there so
already referenced may continue to work. `aloader_sweep` could be used to free
these chunks.
*/
typedef struct {
    aalloc_t alloc;
    void* alloc_ud;
    alist_t pendings;
    alist_t runnings;
    alist_t garbages;
    alist_t libs;
} aloader_t;

/// Message box.
typedef struct {
    avalue_t* msgs;
    int32_t sz;
    int32_t cap;
} ambox_t;

/// Process flags.
typedef enum {
    APF_EXIT = 1 << 0
} APFLAGS;

/// Process stack frame.
typedef struct aframe_t {
    struct aframe_t* prev;
    int32_t bp;
    int32_t nargs;
} aframe_t;

/// Error catching points.
typedef struct acatch_t {
    struct acatch_t* prev;
    volatile int32_t status;
    jmp_buf jbuff;
} acatch_t;

/** Light weight processes.
\brief
AVM concurrency is rely on `actor model`, which is inspired by Erlang. In this
model, each concurrent task will be represented as a `aprocess_t`. Which is, in
general have isolated state, that means such state can not be touched by other
actor. The only way an actor can affect each others is through message, to tell
the owner of that state to make the modification by itself. Therefore, we avoid
the need for explicitly locking to the state and lot of consequence trouble like
deadlock. Well, frankly speaking, deadlock still be possible if there are actors
that is at the same time blocking wait for messages from each other, but mostly
that remains easier to deal with. AVM process is also light weight in compared
to most of OS threads. The overhead in memory footprint, creation, termination
and scheduling is low. Therefore, a massive amount of process could be spawned.

\par Virtual stack.
Besides of the native C stack, AVM also using a dedicated, dynamic sized stack
for `avalue_t`, which is subject of Garbage Collector. Indexing for this stack
is relative to current function frame. Negative value of the index is used to
point to the argument, that must be passed in reversed order. Underflow always
result as a nil value.

=====  ================
index  description
=====  ================
 4     overflow   (sp)
 3     local var3 (top)
 2     local var2
 1     local var1
 0     local var0 (bp)
-1     argument 1
-2     argument 2
-3     argument 3
-4     nil
-5     nil
=====  ================
*/
typedef struct aprocess_t {
    int32_t flags;
    apid_t pid;
    aalloc_t alloc;
    void* alloc_ud;
    struct ascheduler_t* owner;
    atask_t task;
    ambox_t mbox;
    acatch_t* error_jmp;
    aframe_t* frame;
    avalue_t* stack;
    int32_t stack_cap;
    int32_t sp;
} aprocess_t;

/** Process scheduler interface.
\brief
AVM is designed to be resumable, that sounds tricky and non-portable at first.
However, we believe that its worth. Generally, we don't want the users to worry
about whether C stack is resumable or not, and if not then what happen. Yielding
native function and across native function is just mandatory use cases in AVM.
A new \ref atask_t is required for each new process. That allows AVM to save the
context of a process and comeback later, in native side.
*/
typedef struct ascheduler_t {
    aalloc_t alloc;
    void* alloc_ud;
    struct avm_t* vm;
    atask_t task;
} ascheduler_t;

/// VM managed process.
typedef struct {
#ifdef ANY_SMP
    amutex_t mutex;
#endif
    volatile int32_t dead;
    aprocess_t p;
} avm_process_t;

/** VM Engine.
\par System architecture.
                                        +--------+           +----------+
            +-------+                   | alib_t <-----+-----> achunk_t |
            | avm_t +-------+           +--------+     |     +----------+
            +-------+       |                          |
                        `provides`               `load & link`
                            |                          |
+--------------+         +--v---------+          +-----+-----+
| ascheduler_t +--`run`--> aprocess_t +-`import`-> aloader_t |
+--------------+         +-------+----+          +-----------+
                                 |
                            `interpret`
                                 |     +---------------+
                                 +-----> adispatcher_t |
                                       +---------------+
*/
typedef struct avm_t {
#ifdef ANY_SMP
    amutex_t mutex;
#endif
    aalloc_t alloc;
    void* alloc_ud;
    avm_process_t* procs;
    aloader_t loader;
    int8_t idx_bits;
    int8_t gen_bits;
    apid_idx_t next_idx;
} avm_t;