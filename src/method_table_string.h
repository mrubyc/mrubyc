/* Auto generated by make_method_table.rb */
#include "symbol_builtin.h"
struct RClass *mrbc_init_class_string(struct VM *vm)
{
  static const mrbc_sym method_symbols[] = {
    MRBC_SYMID_MUL,
    MRBC_SYMID_PLUS,
    MRBC_SYMID_LT_LT,
    MRBC_SYMID_BL_BR,
    MRBC_SYMID_BL_BR_EQ,
    MRBC_SYMID_b,
    MRBC_SYMID_chomp,
    MRBC_SYMID_chomp_E,
    MRBC_SYMID_clear,
    MRBC_SYMID_dup,
    MRBC_SYMID_empty_Q,
    MRBC_SYMID_end_with_Q,
    MRBC_SYMID_getbyte,
    MRBC_SYMID_include_Q,
    MRBC_SYMID_index,
    MRBC_SYMID_inspect,
    MRBC_SYMID_intern,
    MRBC_SYMID_length,
    MRBC_SYMID_lstrip,
    MRBC_SYMID_lstrip_E,
    MRBC_SYMID_new,
    MRBC_SYMID_ord,
    MRBC_SYMID_rstrip,
    MRBC_SYMID_rstrip_E,
    MRBC_SYMID_size,
    MRBC_SYMID_slice_E,
    MRBC_SYMID_split,
    MRBC_SYMID_start_with_Q,
    MRBC_SYMID_strip,
    MRBC_SYMID_strip_E,
#if MRBC_USE_FLOAT
    MRBC_SYMID_to_f,
#endif
    MRBC_SYMID_to_i,
    MRBC_SYMID_to_s,
    MRBC_SYMID_to_sym,
    MRBC_SYMID_tr,
    MRBC_SYMID_tr_E,
  };
  static const mrbc_func_t method_functions[] = {
    c_string_mul,
    c_string_add,
    c_string_append,
    c_string_slice,
    c_string_insert,
    c_ineffect,
    c_string_chomp,
    c_string_chomp_self,
    c_string_clear,
    c_string_dup,
    c_string_empty,
    c_string_end_with,
    c_string_getbyte,
    c_string_include,
    c_string_index,
    c_string_inspect,
    c_string_to_sym,
    c_string_size,
    c_string_lstrip,
    c_string_lstrip_self,
    c_string_new,
    c_string_ord,
    c_string_rstrip,
    c_string_rstrip_self,
    c_string_size,
    c_string_slice_self,
    c_string_split,
    c_string_start_with,
    c_string_strip,
    c_string_strip_self,
#if MRBC_USE_FLOAT
    c_string_to_f,
#endif
    c_string_to_i,
    c_ineffect,
    c_string_to_sym,
    c_string_tr,
    c_string_tr_self,
  };

  return mrbc_define_builtin_class("String", mrbc_class_object, method_symbols, method_functions, sizeof(method_symbols)/sizeof(mrbc_sym) );
}
