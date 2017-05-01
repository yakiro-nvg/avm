=============
API Reference
=============

String Table
============
.. doxygenstruct::   astring_table_t
.. doxygenfunction:: any_st_init
.. doxygenfunction:: any_st_grow
.. doxygenfunction:: any_st_pack
.. doxygenfunction:: any_st_to_ref
.. doxygenfunction:: any_st_to_ref_const
.. doxygenfunction:: any_st_to_string
.. doxygenfunction:: any_st_to_hash
.. doxygenfunction:: ahash_and_length

Bytecode Assembler
==================
.. doxygenstruct::   aasm_t
.. doxygenstruct::   aasm_prototype_t
.. doxygenstruct::   aasm_reserve_t
.. doxygenunion::    aasm_constant_t
.. doxygenfunction:: any_asm_init
.. doxygenfunction:: any_asm_load
.. doxygenfunction:: any_asm_save
.. doxygenfunction:: any_asm_cleanup
.. doxygenfunction:: any_asm_emit
.. doxygenfunction:: any_asm_add_constant
.. doxygenfunction:: any_asm_add_import
.. doxygenfunction:: any_asm_module_push
.. doxygenfunction:: any_asm_push
.. doxygenfunction:: any_asm_open
.. doxygenfunction:: any_asm_pop
.. doxygenfunction:: any_asm_string_to_ref
.. doxygenfunction:: any_asm_reserve
.. doxygenfunction:: any_asm_prototype
.. doxygenfunction:: any_asm_resolve
.. doxygenfunction:: any_asm_prototype_at