=======
Runtime
=======

Bytecode Loader
===============
.. doxygenstruct::   aloader_t
.. doxygenfunction:: aloader_init
.. doxygenfunction:: aloader_cleanup
.. doxygenfunction:: aloader_add_chunk
.. doxygenfunction:: aloader_add_lib
.. doxygenfunction:: aloader_link
.. doxygenfunction:: aloader_sweep
.. doxygenfunction:: aloader_find

Value Types
===========
.. doxygenstruct::  avalue_t
.. doxygenstruct::  avalue_tag_t
.. doxygentypedef:: anative_func_t
.. doxygenenum::    aabt_t
.. doxygenenum::    avt_number_t
.. doxygenenum::    avt_function_t

Light-weight Process
====================
.. doxygenstruct::   aprocess_t
.. doxygenfunction:: aprocess_init
.. doxygenfunction:: aprocess_start
.. doxygenfunction:: aprocess_cleanup
.. doxygenfunction:: aprocess_reserve
.. doxygenfunction:: aprocess_absidx

Native Library API
==================
.. doxygenfunction:: any_error
.. doxygenfunction:: any_find
.. doxygenfunction:: any_call
.. doxygenfunction:: any_pcall
.. doxygenfunction:: any_yield
.. doxygenfunction:: any_try
.. doxygenfunction:: any_throw
.. doxygenfunction:: any_type
.. doxygenfunction:: any_push_nil
.. doxygenfunction:: any_push_bool
.. doxygenfunction:: any_push_integer
.. doxygenfunction:: any_push_real
.. doxygenfunction:: any_push_pid
.. doxygenfunction:: any_push_idx
.. doxygenfunction:: any_to_bool
.. doxygenfunction:: any_to_integer
.. doxygenfunction:: any_to_real
.. doxygenfunction:: any_to_pid
.. doxygenfunction:: any_pop
.. doxygenfunction:: any_remove
.. doxygenfunction:: any_count
.. doxygenfunction:: any_spawn

Realtime-fair Scheduler
=======================
.. doxygenstruct::   ascheduler_t
.. doxygenfunction:: ascheduler_init
.. doxygenfunction:: ascheduler_cleanup
.. doxygenfunction:: ascheduler_run_once
.. doxygenfunction:: ascheduler_new_process

Virtual Machine
===============
.. doxygenstruct::   avm_t
.. doxygenfunction:: avm_startup
.. doxygenfunction:: avm_shutdown
.. doxygenfunction:: avm_lock_pid
.. doxygenfunction:: avm_unlock
.. doxygenfunction:: avm_alloc
.. doxygenfunction:: avm_free

Multi-tasking
=============
.. doxygenstruct::   atask_t
.. doxygentypedef::  atask_entry_t
.. doxygenfunction:: atask_shadow
.. doxygenfunction:: atask_create
.. doxygenfunction:: atask_delete
.. doxygenfunction:: atask_yield
.. doxygenfunction:: atask_sleep
