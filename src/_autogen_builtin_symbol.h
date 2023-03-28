/* Auto generated by make_symbol_table.rb */
#ifndef MRBC_SRC_AUTOGEN_BUILTIN_SYMBOL_H_
#define MRBC_SRC_AUTOGEN_BUILTIN_SYMBOL_H_

#if defined(MRBC_DEFINE_SYMBOL_TABLE)
static const char *builtin_symbols[] = {
  "",			// MRBC_SYMID_ = 0(0x0)
  "!",			// MRBC_SYMID_NOT = 1(0x1)
  "!=",			// MRBC_SYMID_NOT_EQ = 2(0x2)
  "%",			// MRBC_SYMID_MOD = 3(0x3)
  "&",			// MRBC_SYMID_AND = 4(0x4)
  "*",			// MRBC_SYMID_MUL = 5(0x5)
  "**",			// MRBC_SYMID_MUL_MUL = 6(0x6)
  "+",			// MRBC_SYMID_PLUS = 7(0x7)
  "+@",			// MRBC_SYMID_PLUS_AT = 8(0x8)
  "-",			// MRBC_SYMID_MINUS = 9(0x9)
  "-@",			// MRBC_SYMID_MINUS_AT = 10(0xa)
  "/",			// MRBC_SYMID_DIV = 11(0xb)
  "<",			// MRBC_SYMID_LT = 12(0xc)
  "<<",			// MRBC_SYMID_LT_LT = 13(0xd)
  "<=",			// MRBC_SYMID_LT_EQ = 14(0xe)
  "<=>",		// MRBC_SYMID_LT_EQ_GT = 15(0xf)
  "==",			// MRBC_SYMID_EQ_EQ = 16(0x10)
  "===",		// MRBC_SYMID_EQ_EQ_EQ = 17(0x11)
  ">",			// MRBC_SYMID_GT = 18(0x12)
  ">=",			// MRBC_SYMID_GT_EQ = 19(0x13)
  ">>",			// MRBC_SYMID_GT_GT = 20(0x14)
  "ArgumentError",	// MRBC_SYMID_ArgumentError = 21(0x15)
  "Array",		// MRBC_SYMID_Array = 22(0x16)
  "E",			// MRBC_SYMID_E = 23(0x17)
  "Exception",		// MRBC_SYMID_Exception = 24(0x18)
  "FalseClass",		// MRBC_SYMID_FalseClass = 25(0x19)
  "Float",		// MRBC_SYMID_Float = 26(0x1a)
  "Hash",		// MRBC_SYMID_Hash = 27(0x1b)
  "IndexError",		// MRBC_SYMID_IndexError = 28(0x1c)
  "Integer",		// MRBC_SYMID_Integer = 29(0x1d)
  "MRUBYC_VERSION",	// MRBC_SYMID_MRUBYC_VERSION = 30(0x1e)
  "MRUBY_VERSION",	// MRBC_SYMID_MRUBY_VERSION = 31(0x1f)
  "Math",		// MRBC_SYMID_Math = 32(0x20)
  "NameError",		// MRBC_SYMID_NameError = 33(0x21)
  "NilClass",		// MRBC_SYMID_NilClass = 34(0x22)
  "NoMemoryError",	// MRBC_SYMID_NoMemoryError = 35(0x23)
  "NoMethodError",	// MRBC_SYMID_NoMethodError = 36(0x24)
  "NotImplementedError",	// MRBC_SYMID_NotImplementedError = 37(0x25)
  "Object",		// MRBC_SYMID_Object = 38(0x26)
  "PI",			// MRBC_SYMID_PI = 39(0x27)
  "Proc",		// MRBC_SYMID_Proc = 40(0x28)
  "RUBY_ENGINE",	// MRBC_SYMID_RUBY_ENGINE = 41(0x29)
  "RUBY_VERSION",	// MRBC_SYMID_RUBY_VERSION = 42(0x2a)
  "Range",		// MRBC_SYMID_Range = 43(0x2b)
  "RangeError",		// MRBC_SYMID_RangeError = 44(0x2c)
  "RuntimeError",	// MRBC_SYMID_RuntimeError = 45(0x2d)
  "StandardError",	// MRBC_SYMID_StandardError = 46(0x2e)
  "String",		// MRBC_SYMID_String = 47(0x2f)
  "Symbol",		// MRBC_SYMID_Symbol = 48(0x30)
  "TrueClass",		// MRBC_SYMID_TrueClass = 49(0x31)
  "TypeError",		// MRBC_SYMID_TypeError = 50(0x32)
  "ZeroDivisionError",	// MRBC_SYMID_ZeroDivisionError = 51(0x33)
  "[]",			// MRBC_SYMID_BL_BR = 52(0x34)
  "[]=",		// MRBC_SYMID_BL_BR_EQ = 53(0x35)
  "^",			// MRBC_SYMID_XOR = 54(0x36)
  "__ljust_rjust_argcheck",	// MRBC_SYMID___ljust_rjust_argcheck = 55(0x37)
  "abs",		// MRBC_SYMID_abs = 56(0x38)
  "acos",		// MRBC_SYMID_acos = 57(0x39)
  "acosh",		// MRBC_SYMID_acosh = 58(0x3a)
  "all?",		// MRBC_SYMID_all_Q = 59(0x3b)
  "all_symbols",	// MRBC_SYMID_all_symbols = 60(0x3c)
  "any?",		// MRBC_SYMID_any_Q = 61(0x3d)
  "asin",		// MRBC_SYMID_asin = 62(0x3e)
  "asinh",		// MRBC_SYMID_asinh = 63(0x3f)
  "at",			// MRBC_SYMID_at = 64(0x40)
  "atan",		// MRBC_SYMID_atan = 65(0x41)
  "atan2",		// MRBC_SYMID_atan2 = 66(0x42)
  "atanh",		// MRBC_SYMID_atanh = 67(0x43)
  "attr_accessor",	// MRBC_SYMID_attr_accessor = 68(0x44)
  "attr_reader",	// MRBC_SYMID_attr_reader = 69(0x45)
  "b",			// MRBC_SYMID_b = 70(0x46)
  "block_given?",	// MRBC_SYMID_block_given_Q = 71(0x47)
  "bytes",		// MRBC_SYMID_bytes = 72(0x48)
  "call",		// MRBC_SYMID_call = 73(0x49)
  "cbrt",		// MRBC_SYMID_cbrt = 74(0x4a)
  "chomp",		// MRBC_SYMID_chomp = 75(0x4b)
  "chomp!",		// MRBC_SYMID_chomp_E = 76(0x4c)
  "chr",		// MRBC_SYMID_chr = 77(0x4d)
  "clamp",		// MRBC_SYMID_clamp = 78(0x4e)
  "class",		// MRBC_SYMID_class = 79(0x4f)
  "clear",		// MRBC_SYMID_clear = 80(0x50)
  "collect",		// MRBC_SYMID_collect = 81(0x51)
  "collect!",		// MRBC_SYMID_collect_E = 82(0x52)
  "cos",		// MRBC_SYMID_cos = 83(0x53)
  "cosh",		// MRBC_SYMID_cosh = 84(0x54)
  "count",		// MRBC_SYMID_count = 85(0x55)
  "delete",		// MRBC_SYMID_delete = 86(0x56)
  "delete_at",		// MRBC_SYMID_delete_at = 87(0x57)
  "delete_if",		// MRBC_SYMID_delete_if = 88(0x58)
  "downcase",		// MRBC_SYMID_downcase = 89(0x59)
  "downcase!",		// MRBC_SYMID_downcase_E = 90(0x5a)
  "downto",		// MRBC_SYMID_downto = 91(0x5b)
  "dup",		// MRBC_SYMID_dup = 92(0x5c)
  "each",		// MRBC_SYMID_each = 93(0x5d)
  "each_byte",		// MRBC_SYMID_each_byte = 94(0x5e)
  "each_char",		// MRBC_SYMID_each_char = 95(0x5f)
  "each_index",		// MRBC_SYMID_each_index = 96(0x60)
  "each_with_index",	// MRBC_SYMID_each_with_index = 97(0x61)
  "empty?",		// MRBC_SYMID_empty_Q = 98(0x62)
  "end_with?",		// MRBC_SYMID_end_with_Q = 99(0x63)
  "erf",		// MRBC_SYMID_erf = 100(0x64)
  "erfc",		// MRBC_SYMID_erfc = 101(0x65)
  "exclude_end?",	// MRBC_SYMID_exclude_end_Q = 102(0x66)
  "exp",		// MRBC_SYMID_exp = 103(0x67)
  "find_index",		// MRBC_SYMID_find_index = 104(0x68)
  "first",		// MRBC_SYMID_first = 105(0x69)
  "getbyte",		// MRBC_SYMID_getbyte = 106(0x6a)
  "has_key?",		// MRBC_SYMID_has_key_Q = 107(0x6b)
  "has_value?",		// MRBC_SYMID_has_value_Q = 108(0x6c)
  "hypot",		// MRBC_SYMID_hypot = 109(0x6d)
  "id2name",		// MRBC_SYMID_id2name = 110(0x6e)
  "include?",		// MRBC_SYMID_include_Q = 111(0x6f)
  "index",		// MRBC_SYMID_index = 112(0x70)
  "initialize",		// MRBC_SYMID_initialize = 113(0x71)
  "inspect",		// MRBC_SYMID_inspect = 114(0x72)
  "instance_methods",	// MRBC_SYMID_instance_methods = 115(0x73)
  "instance_variables",	// MRBC_SYMID_instance_variables = 116(0x74)
  "intern",		// MRBC_SYMID_intern = 117(0x75)
  "is_a?",		// MRBC_SYMID_is_a_Q = 118(0x76)
  "join",		// MRBC_SYMID_join = 119(0x77)
  "key",		// MRBC_SYMID_key = 120(0x78)
  "keys",		// MRBC_SYMID_keys = 121(0x79)
  "kind_of?",		// MRBC_SYMID_kind_of_Q = 122(0x7a)
  "last",		// MRBC_SYMID_last = 123(0x7b)
  "ldexp",		// MRBC_SYMID_ldexp = 124(0x7c)
  "length",		// MRBC_SYMID_length = 125(0x7d)
  "ljust",		// MRBC_SYMID_ljust = 126(0x7e)
  "log",		// MRBC_SYMID_log = 127(0x7f)
  "log10",		// MRBC_SYMID_log10 = 128(0x80)
  "log2",		// MRBC_SYMID_log2 = 129(0x81)
  "loop",		// MRBC_SYMID_loop = 130(0x82)
  "lstrip",		// MRBC_SYMID_lstrip = 131(0x83)
  "lstrip!",		// MRBC_SYMID_lstrip_E = 132(0x84)
  "map",		// MRBC_SYMID_map = 133(0x85)
  "map!",		// MRBC_SYMID_map_E = 134(0x86)
  "max",		// MRBC_SYMID_max = 135(0x87)
  "memory_statistics",	// MRBC_SYMID_memory_statistics = 136(0x88)
  "merge",		// MRBC_SYMID_merge = 137(0x89)
  "merge!",		// MRBC_SYMID_merge_E = 138(0x8a)
  "message",		// MRBC_SYMID_message = 139(0x8b)
  "min",		// MRBC_SYMID_min = 140(0x8c)
  "minmax",		// MRBC_SYMID_minmax = 141(0x8d)
  "new",		// MRBC_SYMID_new = 142(0x8e)
  "nil?",		// MRBC_SYMID_nil_Q = 143(0x8f)
  "object_id",		// MRBC_SYMID_object_id = 144(0x90)
  "ord",		// MRBC_SYMID_ord = 145(0x91)
  "p",			// MRBC_SYMID_p = 146(0x92)
  "pop",		// MRBC_SYMID_pop = 147(0x93)
  "print",		// MRBC_SYMID_print = 148(0x94)
  "printf",		// MRBC_SYMID_printf = 149(0x95)
  "push",		// MRBC_SYMID_push = 150(0x96)
  "puts",		// MRBC_SYMID_puts = 151(0x97)
  "raise",		// MRBC_SYMID_raise = 152(0x98)
  "reject",		// MRBC_SYMID_reject = 153(0x99)
  "reject!",		// MRBC_SYMID_reject_E = 154(0x9a)
  "rjust",		// MRBC_SYMID_rjust = 155(0x9b)
  "rstrip",		// MRBC_SYMID_rstrip = 156(0x9c)
  "rstrip!",		// MRBC_SYMID_rstrip_E = 157(0x9d)
  "shift",		// MRBC_SYMID_shift = 158(0x9e)
  "sin",		// MRBC_SYMID_sin = 159(0x9f)
  "sinh",		// MRBC_SYMID_sinh = 160(0xa0)
  "size",		// MRBC_SYMID_size = 161(0xa1)
  "slice!",		// MRBC_SYMID_slice_E = 162(0xa2)
  "sort",		// MRBC_SYMID_sort = 163(0xa3)
  "sort!",		// MRBC_SYMID_sort_E = 164(0xa4)
  "split",		// MRBC_SYMID_split = 165(0xa5)
  "sprintf",		// MRBC_SYMID_sprintf = 166(0xa6)
  "sqrt",		// MRBC_SYMID_sqrt = 167(0xa7)
  "start_with?",	// MRBC_SYMID_start_with_Q = 168(0xa8)
  "strip",		// MRBC_SYMID_strip = 169(0xa9)
  "strip!",		// MRBC_SYMID_strip_E = 170(0xaa)
  "tan",		// MRBC_SYMID_tan = 171(0xab)
  "tanh",		// MRBC_SYMID_tanh = 172(0xac)
  "times",		// MRBC_SYMID_times = 173(0xad)
  "to_a",		// MRBC_SYMID_to_a = 174(0xae)
  "to_f",		// MRBC_SYMID_to_f = 175(0xaf)
  "to_h",		// MRBC_SYMID_to_h = 176(0xb0)
  "to_i",		// MRBC_SYMID_to_i = 177(0xb1)
  "to_s",		// MRBC_SYMID_to_s = 178(0xb2)
  "to_sym",		// MRBC_SYMID_to_sym = 179(0xb3)
  "tr",			// MRBC_SYMID_tr = 180(0xb4)
  "tr!",		// MRBC_SYMID_tr_E = 181(0xb5)
  "unshift",		// MRBC_SYMID_unshift = 182(0xb6)
  "upcase",		// MRBC_SYMID_upcase = 183(0xb7)
  "upcase!",		// MRBC_SYMID_upcase_E = 184(0xb8)
  "upto",		// MRBC_SYMID_upto = 185(0xb9)
  "utf8_size",		// MRBC_SYMID_utf8_size = 186(0xba)
  "utf8_slice",		// MRBC_SYMID_utf8_slice = 187(0xbb)
  "values",		// MRBC_SYMID_values = 188(0xbc)
  "|",			// MRBC_SYMID_OR = 189(0xbd)
  "~",			// MRBC_SYMID_NEG = 190(0xbe)
};
#endif

enum {
  MRBC_SYMID_ = 0,
  MRBC_SYMID_NOT = 1,
  MRBC_SYMID_NOT_EQ = 2,
  MRBC_SYMID_MOD = 3,
  MRBC_SYMID_AND = 4,
  MRBC_SYMID_MUL = 5,
  MRBC_SYMID_MUL_MUL = 6,
  MRBC_SYMID_PLUS = 7,
  MRBC_SYMID_PLUS_AT = 8,
  MRBC_SYMID_MINUS = 9,
  MRBC_SYMID_MINUS_AT = 10,
  MRBC_SYMID_DIV = 11,
  MRBC_SYMID_LT = 12,
  MRBC_SYMID_LT_LT = 13,
  MRBC_SYMID_LT_EQ = 14,
  MRBC_SYMID_LT_EQ_GT = 15,
  MRBC_SYMID_EQ_EQ = 16,
  MRBC_SYMID_EQ_EQ_EQ = 17,
  MRBC_SYMID_GT = 18,
  MRBC_SYMID_GT_EQ = 19,
  MRBC_SYMID_GT_GT = 20,
  MRBC_SYMID_ArgumentError = 21,
  MRBC_SYMID_Array = 22,
  MRBC_SYMID_E = 23,
  MRBC_SYMID_Exception = 24,
  MRBC_SYMID_FalseClass = 25,
  MRBC_SYMID_Float = 26,
  MRBC_SYMID_Hash = 27,
  MRBC_SYMID_IndexError = 28,
  MRBC_SYMID_Integer = 29,
  MRBC_SYMID_MRUBYC_VERSION = 30,
  MRBC_SYMID_MRUBY_VERSION = 31,
  MRBC_SYMID_Math = 32,
  MRBC_SYMID_NameError = 33,
  MRBC_SYMID_NilClass = 34,
  MRBC_SYMID_NoMemoryError = 35,
  MRBC_SYMID_NoMethodError = 36,
  MRBC_SYMID_NotImplementedError = 37,
  MRBC_SYMID_Object = 38,
  MRBC_SYMID_PI = 39,
  MRBC_SYMID_Proc = 40,
  MRBC_SYMID_RUBY_ENGINE = 41,
  MRBC_SYMID_RUBY_VERSION = 42,
  MRBC_SYMID_Range = 43,
  MRBC_SYMID_RangeError = 44,
  MRBC_SYMID_RuntimeError = 45,
  MRBC_SYMID_StandardError = 46,
  MRBC_SYMID_String = 47,
  MRBC_SYMID_Symbol = 48,
  MRBC_SYMID_TrueClass = 49,
  MRBC_SYMID_TypeError = 50,
  MRBC_SYMID_ZeroDivisionError = 51,
  MRBC_SYMID_BL_BR = 52,
  MRBC_SYMID_BL_BR_EQ = 53,
  MRBC_SYMID_XOR = 54,
  MRBC_SYMID___ljust_rjust_argcheck = 55,
  MRBC_SYMID_abs = 56,
  MRBC_SYMID_acos = 57,
  MRBC_SYMID_acosh = 58,
  MRBC_SYMID_all_Q = 59,
  MRBC_SYMID_all_symbols = 60,
  MRBC_SYMID_any_Q = 61,
  MRBC_SYMID_asin = 62,
  MRBC_SYMID_asinh = 63,
  MRBC_SYMID_at = 64,
  MRBC_SYMID_atan = 65,
  MRBC_SYMID_atan2 = 66,
  MRBC_SYMID_atanh = 67,
  MRBC_SYMID_attr_accessor = 68,
  MRBC_SYMID_attr_reader = 69,
  MRBC_SYMID_b = 70,
  MRBC_SYMID_block_given_Q = 71,
  MRBC_SYMID_bytes = 72,
  MRBC_SYMID_call = 73,
  MRBC_SYMID_cbrt = 74,
  MRBC_SYMID_chomp = 75,
  MRBC_SYMID_chomp_E = 76,
  MRBC_SYMID_chr = 77,
  MRBC_SYMID_clamp = 78,
  MRBC_SYMID_class = 79,
  MRBC_SYMID_clear = 80,
  MRBC_SYMID_collect = 81,
  MRBC_SYMID_collect_E = 82,
  MRBC_SYMID_cos = 83,
  MRBC_SYMID_cosh = 84,
  MRBC_SYMID_count = 85,
  MRBC_SYMID_delete = 86,
  MRBC_SYMID_delete_at = 87,
  MRBC_SYMID_delete_if = 88,
  MRBC_SYMID_downcase = 89,
  MRBC_SYMID_downcase_E = 90,
  MRBC_SYMID_downto = 91,
  MRBC_SYMID_dup = 92,
  MRBC_SYMID_each = 93,
  MRBC_SYMID_each_byte = 94,
  MRBC_SYMID_each_char = 95,
  MRBC_SYMID_each_index = 96,
  MRBC_SYMID_each_with_index = 97,
  MRBC_SYMID_empty_Q = 98,
  MRBC_SYMID_end_with_Q = 99,
  MRBC_SYMID_erf = 100,
  MRBC_SYMID_erfc = 101,
  MRBC_SYMID_exclude_end_Q = 102,
  MRBC_SYMID_exp = 103,
  MRBC_SYMID_find_index = 104,
  MRBC_SYMID_first = 105,
  MRBC_SYMID_getbyte = 106,
  MRBC_SYMID_has_key_Q = 107,
  MRBC_SYMID_has_value_Q = 108,
  MRBC_SYMID_hypot = 109,
  MRBC_SYMID_id2name = 110,
  MRBC_SYMID_include_Q = 111,
  MRBC_SYMID_index = 112,
  MRBC_SYMID_initialize = 113,
  MRBC_SYMID_inspect = 114,
  MRBC_SYMID_instance_methods = 115,
  MRBC_SYMID_instance_variables = 116,
  MRBC_SYMID_intern = 117,
  MRBC_SYMID_is_a_Q = 118,
  MRBC_SYMID_join = 119,
  MRBC_SYMID_key = 120,
  MRBC_SYMID_keys = 121,
  MRBC_SYMID_kind_of_Q = 122,
  MRBC_SYMID_last = 123,
  MRBC_SYMID_ldexp = 124,
  MRBC_SYMID_length = 125,
  MRBC_SYMID_ljust = 126,
  MRBC_SYMID_log = 127,
  MRBC_SYMID_log10 = 128,
  MRBC_SYMID_log2 = 129,
  MRBC_SYMID_loop = 130,
  MRBC_SYMID_lstrip = 131,
  MRBC_SYMID_lstrip_E = 132,
  MRBC_SYMID_map = 133,
  MRBC_SYMID_map_E = 134,
  MRBC_SYMID_max = 135,
  MRBC_SYMID_memory_statistics = 136,
  MRBC_SYMID_merge = 137,
  MRBC_SYMID_merge_E = 138,
  MRBC_SYMID_message = 139,
  MRBC_SYMID_min = 140,
  MRBC_SYMID_minmax = 141,
  MRBC_SYMID_new = 142,
  MRBC_SYMID_nil_Q = 143,
  MRBC_SYMID_object_id = 144,
  MRBC_SYMID_ord = 145,
  MRBC_SYMID_p = 146,
  MRBC_SYMID_pop = 147,
  MRBC_SYMID_print = 148,
  MRBC_SYMID_printf = 149,
  MRBC_SYMID_push = 150,
  MRBC_SYMID_puts = 151,
  MRBC_SYMID_raise = 152,
  MRBC_SYMID_reject = 153,
  MRBC_SYMID_reject_E = 154,
  MRBC_SYMID_rjust = 155,
  MRBC_SYMID_rstrip = 156,
  MRBC_SYMID_rstrip_E = 157,
  MRBC_SYMID_shift = 158,
  MRBC_SYMID_sin = 159,
  MRBC_SYMID_sinh = 160,
  MRBC_SYMID_size = 161,
  MRBC_SYMID_slice_E = 162,
  MRBC_SYMID_sort = 163,
  MRBC_SYMID_sort_E = 164,
  MRBC_SYMID_split = 165,
  MRBC_SYMID_sprintf = 166,
  MRBC_SYMID_sqrt = 167,
  MRBC_SYMID_start_with_Q = 168,
  MRBC_SYMID_strip = 169,
  MRBC_SYMID_strip_E = 170,
  MRBC_SYMID_tan = 171,
  MRBC_SYMID_tanh = 172,
  MRBC_SYMID_times = 173,
  MRBC_SYMID_to_a = 174,
  MRBC_SYMID_to_f = 175,
  MRBC_SYMID_to_h = 176,
  MRBC_SYMID_to_i = 177,
  MRBC_SYMID_to_s = 178,
  MRBC_SYMID_to_sym = 179,
  MRBC_SYMID_tr = 180,
  MRBC_SYMID_tr_E = 181,
  MRBC_SYMID_unshift = 182,
  MRBC_SYMID_upcase = 183,
  MRBC_SYMID_upcase_E = 184,
  MRBC_SYMID_upto = 185,
  MRBC_SYMID_utf8_size = 186,
  MRBC_SYMID_utf8_slice = 187,
  MRBC_SYMID_values = 188,
  MRBC_SYMID_OR = 189,
  MRBC_SYMID_NEG = 190,
};

#define MRB_SYM(sym)  MRBC_SYMID_##sym
#define MRBC_SYM(sym) MRBC_SYMID_##sym
#endif
