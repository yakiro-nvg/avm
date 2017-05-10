=======
Runtime
=======

Bytecode Loader
===============
.. doxygenfunction:: any_link

Value Types
===========
.. doxygenstruct::  avalue_t
.. doxygenstruct::  avtag_t
.. doxygentypedef:: anative_func_t
.. doxygenenum::    ATB
.. doxygenenum::    AVT_NUMBER
.. doxygenenum::    AVT_FUNCTION

Multi-tasking
=============
.. doxygenstruct::   aprocess_t
.. doxygenstruct::   ascheduler_t
.. doxygenfunction:: any_sched_init
.. doxygenfunction:: any_sched_outgoing
.. doxygenfunction:: any_sched_empty_incoming
.. doxygenstruct::   avm_t
.. doxygenfunction:: any_vm_startup
.. doxygenfunction:: any_vm_shutdown
.. doxygenfunction:: any_vm_proc_lock
.. doxygenfunction:: any_vm_proc_lock_idx
.. doxygenfunction:: any_vm_proc_unlock
.. doxygenfunction:: any_vm_proc_allocate
.. doxygenfunction:: any_vm_proc_free
.. doxygenfunction:: any_vm_flush_outgoing
.. doxygentypedef::  adispatcher_t