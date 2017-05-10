#pragma once

#include <stdint.h>
#include <any/platform.h>
#include <any/fiber.h>

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
    AOC_RET = 41,
    AOC_SND = 50,
    AOC_RCV = 51,
    AOC_RMV = 52
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
Pop a closure and next `nargs` arguments from the stack
then call it. The result will be places onto the top of
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

/** Pop a pid and next a message from the stack and send this message to that pid.
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

/** Picks up next message in the queue and places it onto the top of stack.
\brief 
If there is no message, jump to signed `displacement`.

A `timeout` value will be popped from the stack, that will be used by AVM to 
determine if it must wait for this period before it decided that there is no 
more message in the queue. This `timeout` is signed and in milliseconds, a 
negative value means infinite waiting.

\note This instruction will not remove that message from the queue.
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
typedef void* (*arealloc_t)(void* userdata, void* old, int32_t sz);

/// Process identifier.
typedef uint32_t apid_t;

/// Pid from index and generation.
AINLINE apid_t apid_from(
    int32_t idx_bits, int32_t gen_bits, int32_t idx, int32_t gen)
{
    return (idx << (32 - idx_bits)) | (gen << (32 - idx_bits - gen_bits));
}

/// Pid index part.
AINLINE int32_t apid_idx(int32_t idx_bits, int32_t gen_bits, apid_t pid)
{
    AUNUSED(gen_bits);
    return pid >> (32 - idx_bits);
}

/// Pid generation part.
AINLINE int32_t apid_gen(int32_t idx_bits, int32_t gen_bits, apid_t pid)
{
    return (pid << idx_bits) >> (32 - gen_bits);
}

/// Reference to a string
typedef int32_t astring_ref_t;

/// Native function.
typedef int32_t(*anative_func_t)(void*);

/// Basic value tags.
enum ATB {
    /// No value.
    ATB_NIL,
    /// Process identifier.
    ATB_PID,
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
        /// \ref ATB_PID.
        apid_t pid;
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

/// Message box.
typedef struct {
    avalue_t* msgs;
    int32_t sz;
    int32_t cap;
} ambox_t;

/// Message envelope.
typedef struct {
    apid_t to;
    avalue_t payload;
} aenvelope_t;

/// Scheduler message box.
typedef struct {
    aenvelope_t* msgs;
    int32_t sz;
    int32_t cap;
} asmbox_t;

/** Process scheduler interface.
\brief
The heart of AVM is scheduler, which is where the process is going to run. There
are many schedulers that can be co-exists at the same time in single AVM, which is 
depends on the real use cases that you're facing. New scheduler may be added to 
take care about a lot more of processes for each CPU core, aka symmetric multiple
processing. You may also have a kernel mode scheduler if you are running on bare 
metal system with full access to the CPU. That can open a lot of interesting like 
preemptive and sandbox native processes. That kind of things may be used to replace
most of simple RTOS.

The actual dispatching function is depends on each scheduler, which may be a full
power worker which will be pined to a single core, or must cooperative with others
system like GUI (main thread). How these function should be scheduled, generally 
will not be defined by AVM at this level. Therefore, you must check the document of
the actual scheduler which you're using to make sure that actually does dispatches
\ref avm_t engine will stand above schedulers to handle cross-scheduler operations,
that includes message routing and process migration.

Scheduler itself only know about resources assigning to execute processes, besides
of that, there is another component that is responsible for actual executing called
dispatchers. As same as scheduler, there are also many kind of dispatchers which is
depends on the actual use cases. From naive portable interpreter to complex JIT/AOT
based, please refer to \ref adispatcher_t.

AVM is designed to be resumable, that sounds tricky and non-portable. However, we
believe that its worth. Generally, we don't want the users to worry about whether
C stack is resumable or not, and if not then what happen. Yielding native function
and across native function is just mandatory use cases in AVM. A new \ref afiber_t
is required for each new process. That allows AVM to save the context of a process
and comeback later, in native side.

\warning
To ensure the compatibility between platforms, although preemptive native function
is possible in some case like special duty scheduler, such schedulers should only
be used by special duty processes like driver or kernel mode service as well. You
shouldn't rely on that for application processes, which is portable and generally 
can't be preemptive by most of OS. If for some reasons, long running native code is
expected, you must yield the execution explicitly, otherwise that will impacts rest
of the scheduler.
*/
typedef struct {
    struct avm_s* vm;
    struct adispatcher_s* runner;
    arealloc_t realloc;
    void* realloc_ud;
#ifdef ANY_SMP
    amutex_t omutex;
    amutex_t imutex;
#endif
    asmbox_t oback;
    asmbox_t ofront;
    asmbox_t iback;
    asmbox_t ifront;
} ascheduler_t;

/// Process flags.
enum APFLAGS {
    APF_DEAD = 1 << 0,
    APF_BORROWED = 1 << 1
};

/// Process stack frames.
typedef struct {
    avalue_t* sp;
    avalue_t* bp;
    ainstruction_t* ip;
    aprototype_t* proto;
} aframe_t;

/** Light weight processes.
\brief
AVM concurrency is rely on `actor model`, which is inspired from Erlang. In this 
model, each concurrent thread will be represented by a `aprocess_t`. Which in
general have isolated state and can't be touched from other actor. The only way 
an actor can affect others is through message, to tell the owner of the state 
to modify that by itself. Therefore, we avoid explicitly locking to the state
and lot of consequence trouble like deadlock. Well, actually deadlock still be
possible if there are two actors which are blocking wait for messages from each
other, but that still remains easier.

AVM process is very light weight compared to the OS threads. The stack and heap
for each process is grow and shrink dynamically due to the usages. The overhead
in memory footprint, creation, terminate and scheduling is very low. All of that
ultimately means massive amount of process can be spawned for each asynchronous
operation. Then now you can forget about the messy callback hell. The scheduling
is mostly preemptive, which doesn't require cooperations from process to yield
for others. The exceptional case is native function. There is no portable way to 
save the context of native C functions. That means a naive AVM scheduler will not 
be able to interrupt processes that tends to block at the native side. Usually, 
this can only be handled by a special duty scheduler which is work as low level 
and known the CPU very well.

You may surprise about this next strange decision, but in AVM, process memory may
be static allocated by user and be borrowed by the scheduler. Which is an effort
to makes dynamic allocation be an optional as much as possible. Frankly speaking, 
dynamic allocation is good as it boost the productivity and may reduce the memory 
footprint. However, that usually be considered as luxury things in embedded world.
In this kind of system, non-predicted is bad, and they don't have too many things 
to spawns dynamically as well. These factors lead to usual design that estimates
the memory usage for each process, and just pre-allocate it statically. This kind
of process is called `borrowed`. Which memory, in short will not be allocated by 
AVM itself so there are no effort to grow and shrink that at all.
*/
typedef struct {
#ifdef ANY_SMP
    amutex_t mutex;
#endif
    volatile ascheduler_t* owner;
    volatile int32_t load;
    volatile int32_t flags;
    volatile apid_t pid;
    afiber_t fiber;
    ambox_t mbox;
    avalue_t* stack;
    aframe_t* frame;
    aframe_t* frames;
    int32_t stack_cap;
    int32_t max_frames;
} aprocess_t;

/// Bytecode dispatcher.
typedef struct adispatcher_s {
    void(*exec)(aprocess_t* p);
} adispatcher_t;

/// VM Engine.
typedef struct avm_s {
    aprocess_t* _procs;
    int32_t _idx_bits;
    int32_t _gen_bits;
    int32_t _next_idx;
#ifdef ANY_SMP
    amutex_t _next_idx_mutex;
#endif
} avm_t;