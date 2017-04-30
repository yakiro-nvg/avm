========
Bytecode
========

Code Chunk
==========
.. doxygenstruct:: achunk_t

Function Prototype
==================
.. doxygenstruct:: aprototype_t
.. doxygenunion::  aconstant_t
.. doxygenstruct:: aimport_t
.. doxygenstruct:: acurrent_t

Instruction Set
===============
.. doxygenunion::  ainstruction_t
.. doxygenstruct:: ai_base_t
.. doxygenstruct:: ai_nop_t
.. doxygenstruct:: ai_pop_t
.. doxygenstruct:: ai_get_const_t
.. doxygenstruct:: ai_get_nil_t
.. doxygenstruct:: ai_get_bool_t
.. doxygenstruct:: ai_get_local_t
.. doxygenstruct:: ai_set_local_t
.. doxygenstruct:: ai_get_import_t
.. doxygenstruct:: ai_get_upvalue_t
.. doxygenstruct:: ai_set_upvalue_t
.. doxygenstruct:: ai_jump_t
.. doxygenstruct:: ai_jump_if_not_t
.. doxygenstruct:: ai_invoke_t
.. doxygenstruct:: ai_return_t
.. doxygenstruct:: ai_closure_t
.. doxygenstruct:: ai_capture_local_t
.. doxygenstruct:: ai_capture_upvalue_t
.. doxygenstruct:: ai_close_t