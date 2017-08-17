=============
API Reference
=============

String Table
============
.. doxygenstruct::   astring_table_t
.. doxygenfunction:: astring_table_init
.. doxygenfunction:: astring_table_grow
.. doxygenfunction:: astring_table_pack
.. doxygenfunction:: astring_table_to_ref
.. doxygenfunction:: astring_table_to_ref_const
.. doxygenfunction:: astring_table_to_string
.. doxygenfunction:: astring_table_to_hash

Bytecode Assembler
==================
.. doxygenstruct::   aasm_t
.. doxygenstruct::   aasm_prototype_t
.. doxygenstruct::   aasm_reserve_t
.. doxygenfunction:: aasm_init
.. doxygenfunction:: aasm_load
.. doxygenfunction:: aasm_save
.. doxygenfunction:: aasm_cleanup
.. doxygenfunction:: aasm_emit
.. doxygenfunction:: aasm_add_constant
.. doxygenfunction:: aasm_add_import
.. doxygenfunction:: aasm_module_push
.. doxygenfunction:: aasm_push
.. doxygenfunction:: aasm_open
.. doxygenfunction:: aasm_pop
.. doxygenfunction:: aasm_string_to_ref
.. doxygenfunction:: aasm_reserve
.. doxygenfunction:: aasm_prototype
.. doxygenfunction:: aasm_resolve
.. doxygenfunction:: aasm_prototype_at
