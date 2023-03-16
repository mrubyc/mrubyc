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
  "<<",			// MRBC_SYMID_LT_LT = 12(0xc)
  "<=>",		// MRBC_SYMID_LT_EQ_GT = 13(0xd)
  "===",		// MRBC_SYMID_EQ_EQ_EQ = 14(0xe)
  ">>",			// MRBC_SYMID_GT_GT = 15(0xf)
  "ArgumentError",	// MRBC_SYMID_ArgumentError = 16(0x10)
  "Array",		// MRBC_SYMID_Array = 17(0x11)
  "E",			// MRBC_SYMID_E = 18(0x12)
  "Exception",		// MRBC_SYMID_Exception = 19(0x13)
  "FalseClass",		// MRBC_SYMID_FalseClass = 20(0x14)
  "Float",		// MRBC_SYMID_Float = 21(0x15)
  "Hash",		// MRBC_SYMID_Hash = 22(0x16)
  "IndexError",		// MRBC_SYMID_IndexError = 23(0x17)
  "Integer",		// MRBC_SYMID_Integer = 24(0x18)
  "MRUBYC_VERSION",	// MRBC_SYMID_MRUBYC_VERSION = 25(0x19)
  "MRUBY_VERSION",	// MRBC_SYMID_MRUBY_VERSION = 26(0x1a)
  "Math",		// MRBC_SYMID_Math = 27(0x1b)
  "NameError",		// MRBC_SYMID_NameError = 28(0x1c)
  "NilClass",		// MRBC_SYMID_NilClass = 29(0x1d)
  "NoMemoryError",	// MRBC_SYMID_NoMemoryError = 30(0x1e)
  "NoMethodError",	// MRBC_SYMID_NoMethodError = 31(0x1f)
  "NotImplementedError",	// MRBC_SYMID_NotImplementedError = 32(0x20)
  "Object",		// MRBC_SYMID_Object = 33(0x21)
  "PI",			// MRBC_SYMID_PI = 34(0x22)
  "Proc",		// MRBC_SYMID_Proc = 35(0x23)
  "RUBY_ENGINE",	// MRBC_SYMID_RUBY_ENGINE = 36(0x24)
  "RUBY_VERSION",	// MRBC_SYMID_RUBY_VERSION = 37(0x25)
  "Range",		// MRBC_SYMID_Range = 38(0x26)
  "RangeError",		// MRBC_SYMID_RangeError = 39(0x27)
  "RuntimeError",	// MRBC_SYMID_RuntimeError = 40(0x28)
  "StandardError",	// MRBC_SYMID_StandardError = 41(0x29)
  "String",		// MRBC_SYMID_String = 42(0x2a)
  "Symbol",		// MRBC_SYMID_Symbol = 43(0x2b)
  "TrueClass",		// MRBC_SYMID_TrueClass = 44(0x2c)
  "TypeError",		// MRBC_SYMID_TypeError = 45(0x2d)
  "ZeroDivisionError",	// MRBC_SYMID_ZeroDivisionError = 46(0x2e)
  "[]",			// MRBC_SYMID_BL_BR = 47(0x2f)
  "[]=",		// MRBC_SYMID_BL_BR_EQ = 48(0x30)
  "^",			// MRBC_SYMID_XOR = 49(0x31)
  "__ljust_rjust_argcheck",	// MRBC_SYMID___ljust_rjust_argcheck = 50(0x32)
  "abs",		// MRBC_SYMID_abs = 51(0x33)
  "acos",		// MRBC_SYMID_acos = 52(0x34)
  "acosh",		// MRBC_SYMID_acosh = 53(0x35)
  "all?",		// MRBC_SYMID_all_Q = 54(0x36)
  "all_symbols",	// MRBC_SYMID_all_symbols = 55(0x37)
  "any?",		// MRBC_SYMID_any_Q = 56(0x38)
  "asin",		// MRBC_SYMID_asin = 57(0x39)
  "asinh",		// MRBC_SYMID_asinh = 58(0x3a)
  "at",			// MRBC_SYMID_at = 59(0x3b)
  "atan",		// MRBC_SYMID_atan = 60(0x3c)
  "atan2",		// MRBC_SYMID_atan2 = 61(0x3d)
  "atanh",		// MRBC_SYMID_atanh = 62(0x3e)
  "attr_accessor",	// MRBC_SYMID_attr_accessor = 63(0x3f)
  "attr_reader",	// MRBC_SYMID_attr_reader = 64(0x40)
  "b",			// MRBC_SYMID_b = 65(0x41)
  "block_given?",	// MRBC_SYMID_block_given_Q = 66(0x42)
  "bytes",		// MRBC_SYMID_bytes = 67(0x43)
  "call",		// MRBC_SYMID_call = 68(0x44)
  "cbrt",		// MRBC_SYMID_cbrt = 69(0x45)
  "chomp",		// MRBC_SYMID_chomp = 70(0x46)
  "chomp!",		// MRBC_SYMID_chomp_E = 71(0x47)
  "chr",		// MRBC_SYMID_chr = 72(0x48)
  "clamp",		// MRBC_SYMID_clamp = 73(0x49)
  "class",		// MRBC_SYMID_class = 74(0x4a)
  "clear",		// MRBC_SYMID_clear = 75(0x4b)
  "collect",		// MRBC_SYMID_collect = 76(0x4c)
  "collect!",		// MRBC_SYMID_collect_E = 77(0x4d)
  "cos",		// MRBC_SYMID_cos = 78(0x4e)
  "cosh",		// MRBC_SYMID_cosh = 79(0x4f)
  "count",		// MRBC_SYMID_count = 80(0x50)
  "delete",		// MRBC_SYMID_delete = 81(0x51)
  "delete_at",		// MRBC_SYMID_delete_at = 82(0x52)
  "delete_if",		// MRBC_SYMID_delete_if = 83(0x53)
  "downto",		// MRBC_SYMID_downto = 84(0x54)
  "dup",		// MRBC_SYMID_dup = 85(0x55)
  "each",		// MRBC_SYMID_each = 86(0x56)
  "each_byte",		// MRBC_SYMID_each_byte = 87(0x57)
  "each_char",		// MRBC_SYMID_each_char = 88(0x58)
  "each_index",		// MRBC_SYMID_each_index = 89(0x59)
  "each_with_index",	// MRBC_SYMID_each_with_index = 90(0x5a)
  "empty?",		// MRBC_SYMID_empty_Q = 91(0x5b)
  "end_with?",		// MRBC_SYMID_end_with_Q = 92(0x5c)
  "erf",		// MRBC_SYMID_erf = 93(0x5d)
  "erfc",		// MRBC_SYMID_erfc = 94(0x5e)
  "exclude_end?",	// MRBC_SYMID_exclude_end_Q = 95(0x5f)
  "exp",		// MRBC_SYMID_exp = 96(0x60)
  "find_index",		// MRBC_SYMID_find_index = 97(0x61)
  "first",		// MRBC_SYMID_first = 98(0x62)
  "getbyte",		// MRBC_SYMID_getbyte = 99(0x63)
  "has_key?",		// MRBC_SYMID_has_key_Q = 100(0x64)
  "has_value?",		// MRBC_SYMID_has_value_Q = 101(0x65)
  "hypot",		// MRBC_SYMID_hypot = 102(0x66)
  "id2name",		// MRBC_SYMID_id2name = 103(0x67)
  "include?",		// MRBC_SYMID_include_Q = 104(0x68)
  "index",		// MRBC_SYMID_index = 105(0x69)
  "initialize",		// MRBC_SYMID_initialize = 106(0x6a)
  "inspect",		// MRBC_SYMID_inspect = 107(0x6b)
  "instance_methods",	// MRBC_SYMID_instance_methods = 108(0x6c)
  "instance_variables",	// MRBC_SYMID_instance_variables = 109(0x6d)
  "intern",		// MRBC_SYMID_intern = 110(0x6e)
  "is_a?",		// MRBC_SYMID_is_a_Q = 111(0x6f)
  "join",		// MRBC_SYMID_join = 112(0x70)
  "key",		// MRBC_SYMID_key = 113(0x71)
  "keys",		// MRBC_SYMID_keys = 114(0x72)
  "kind_of?",		// MRBC_SYMID_kind_of_Q = 115(0x73)
  "last",		// MRBC_SYMID_last = 116(0x74)
  "ldexp",		// MRBC_SYMID_ldexp = 117(0x75)
  "length",		// MRBC_SYMID_length = 118(0x76)
  "ljust",		// MRBC_SYMID_ljust = 119(0x77)
  "log",		// MRBC_SYMID_log = 120(0x78)
  "log10",		// MRBC_SYMID_log10 = 121(0x79)
  "log2",		// MRBC_SYMID_log2 = 122(0x7a)
  "loop",		// MRBC_SYMID_loop = 123(0x7b)
  "lstrip",		// MRBC_SYMID_lstrip = 124(0x7c)
  "lstrip!",		// MRBC_SYMID_lstrip_E = 125(0x7d)
  "map",		// MRBC_SYMID_map = 126(0x7e)
  "map!",		// MRBC_SYMID_map_E = 127(0x7f)
  "max",		// MRBC_SYMID_max = 128(0x80)
  "memory_statistics",	// MRBC_SYMID_memory_statistics = 129(0x81)
  "merge",		// MRBC_SYMID_merge = 130(0x82)
  "merge!",		// MRBC_SYMID_merge_E = 131(0x83)
  "message",		// MRBC_SYMID_message = 132(0x84)
  "min",		// MRBC_SYMID_min = 133(0x85)
  "minmax",		// MRBC_SYMID_minmax = 134(0x86)
  "new",		// MRBC_SYMID_new = 135(0x87)
  "nil?",		// MRBC_SYMID_nil_Q = 136(0x88)
  "object_id",		// MRBC_SYMID_object_id = 137(0x89)
  "ord",		// MRBC_SYMID_ord = 138(0x8a)
  "p",			// MRBC_SYMID_p = 139(0x8b)
  "pop",		// MRBC_SYMID_pop = 140(0x8c)
  "print",		// MRBC_SYMID_print = 141(0x8d)
  "printf",		// MRBC_SYMID_printf = 142(0x8e)
  "push",		// MRBC_SYMID_push = 143(0x8f)
  "puts",		// MRBC_SYMID_puts = 144(0x90)
  "raise",		// MRBC_SYMID_raise = 145(0x91)
  "reject",		// MRBC_SYMID_reject = 146(0x92)
  "reject!",		// MRBC_SYMID_reject_E = 147(0x93)
  "rjust",		// MRBC_SYMID_rjust = 148(0x94)
  "rstrip",		// MRBC_SYMID_rstrip = 149(0x95)
  "rstrip!",		// MRBC_SYMID_rstrip_E = 150(0x96)
  "shift",		// MRBC_SYMID_shift = 151(0x97)
  "sin",		// MRBC_SYMID_sin = 152(0x98)
  "sinh",		// MRBC_SYMID_sinh = 153(0x99)
  "size",		// MRBC_SYMID_size = 154(0x9a)
  "slice!",		// MRBC_SYMID_slice_E = 155(0x9b)
  "sort",		// MRBC_SYMID_sort = 156(0x9c)
  "sort!",		// MRBC_SYMID_sort_E = 157(0x9d)
  "split",		// MRBC_SYMID_split = 158(0x9e)
  "sprintf",		// MRBC_SYMID_sprintf = 159(0x9f)
  "sqrt",		// MRBC_SYMID_sqrt = 160(0xa0)
  "start_with?",	// MRBC_SYMID_start_with_Q = 161(0xa1)
  "strip",		// MRBC_SYMID_strip = 162(0xa2)
  "strip!",		// MRBC_SYMID_strip_E = 163(0xa3)
  "tan",		// MRBC_SYMID_tan = 164(0xa4)
  "tanh",		// MRBC_SYMID_tanh = 165(0xa5)
  "times",		// MRBC_SYMID_times = 166(0xa6)
  "to_a",		// MRBC_SYMID_to_a = 167(0xa7)
  "to_f",		// MRBC_SYMID_to_f = 168(0xa8)
  "to_h",		// MRBC_SYMID_to_h = 169(0xa9)
  "to_i",		// MRBC_SYMID_to_i = 170(0xaa)
  "to_s",		// MRBC_SYMID_to_s = 171(0xab)
  "to_sym",		// MRBC_SYMID_to_sym = 172(0xac)
  "tr",			// MRBC_SYMID_tr = 173(0xad)
  "tr!",		// MRBC_SYMID_tr_E = 174(0xae)
  "unshift",		// MRBC_SYMID_unshift = 175(0xaf)
  "upto",		// MRBC_SYMID_upto = 176(0xb0)
  "values",		// MRBC_SYMID_values = 177(0xb1)
  "|",			// MRBC_SYMID_OR = 178(0xb2)
  "~",			// MRBC_SYMID_NEG = 179(0xb3)
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
  MRBC_SYMID_LT_LT = 12,
  MRBC_SYMID_LT_EQ_GT = 13,
  MRBC_SYMID_EQ_EQ_EQ = 14,
  MRBC_SYMID_GT_GT = 15,
  MRBC_SYMID_ArgumentError = 16,
  MRBC_SYMID_Array = 17,
  MRBC_SYMID_E = 18,
  MRBC_SYMID_Exception = 19,
  MRBC_SYMID_FalseClass = 20,
  MRBC_SYMID_Float = 21,
  MRBC_SYMID_Hash = 22,
  MRBC_SYMID_IndexError = 23,
  MRBC_SYMID_Integer = 24,
  MRBC_SYMID_MRUBYC_VERSION = 25,
  MRBC_SYMID_MRUBY_VERSION = 26,
  MRBC_SYMID_Math = 27,
  MRBC_SYMID_NameError = 28,
  MRBC_SYMID_NilClass = 29,
  MRBC_SYMID_NoMemoryError = 30,
  MRBC_SYMID_NoMethodError = 31,
  MRBC_SYMID_NotImplementedError = 32,
  MRBC_SYMID_Object = 33,
  MRBC_SYMID_PI = 34,
  MRBC_SYMID_Proc = 35,
  MRBC_SYMID_RUBY_ENGINE = 36,
  MRBC_SYMID_RUBY_VERSION = 37,
  MRBC_SYMID_Range = 38,
  MRBC_SYMID_RangeError = 39,
  MRBC_SYMID_RuntimeError = 40,
  MRBC_SYMID_StandardError = 41,
  MRBC_SYMID_String = 42,
  MRBC_SYMID_Symbol = 43,
  MRBC_SYMID_TrueClass = 44,
  MRBC_SYMID_TypeError = 45,
  MRBC_SYMID_ZeroDivisionError = 46,
  MRBC_SYMID_BL_BR = 47,
  MRBC_SYMID_BL_BR_EQ = 48,
  MRBC_SYMID_XOR = 49,
  MRBC_SYMID___ljust_rjust_argcheck = 50,
  MRBC_SYMID_abs = 51,
  MRBC_SYMID_acos = 52,
  MRBC_SYMID_acosh = 53,
  MRBC_SYMID_all_Q = 54,
  MRBC_SYMID_all_symbols = 55,
  MRBC_SYMID_any_Q = 56,
  MRBC_SYMID_asin = 57,
  MRBC_SYMID_asinh = 58,
  MRBC_SYMID_at = 59,
  MRBC_SYMID_atan = 60,
  MRBC_SYMID_atan2 = 61,
  MRBC_SYMID_atanh = 62,
  MRBC_SYMID_attr_accessor = 63,
  MRBC_SYMID_attr_reader = 64,
  MRBC_SYMID_b = 65,
  MRBC_SYMID_block_given_Q = 66,
  MRBC_SYMID_bytes = 67,
  MRBC_SYMID_call = 68,
  MRBC_SYMID_cbrt = 69,
  MRBC_SYMID_chomp = 70,
  MRBC_SYMID_chomp_E = 71,
  MRBC_SYMID_chr = 72,
  MRBC_SYMID_clamp = 73,
  MRBC_SYMID_class = 74,
  MRBC_SYMID_clear = 75,
  MRBC_SYMID_collect = 76,
  MRBC_SYMID_collect_E = 77,
  MRBC_SYMID_cos = 78,
  MRBC_SYMID_cosh = 79,
  MRBC_SYMID_count = 80,
  MRBC_SYMID_delete = 81,
  MRBC_SYMID_delete_at = 82,
  MRBC_SYMID_delete_if = 83,
  MRBC_SYMID_downto = 84,
  MRBC_SYMID_dup = 85,
  MRBC_SYMID_each = 86,
  MRBC_SYMID_each_byte = 87,
  MRBC_SYMID_each_char = 88,
  MRBC_SYMID_each_index = 89,
  MRBC_SYMID_each_with_index = 90,
  MRBC_SYMID_empty_Q = 91,
  MRBC_SYMID_end_with_Q = 92,
  MRBC_SYMID_erf = 93,
  MRBC_SYMID_erfc = 94,
  MRBC_SYMID_exclude_end_Q = 95,
  MRBC_SYMID_exp = 96,
  MRBC_SYMID_find_index = 97,
  MRBC_SYMID_first = 98,
  MRBC_SYMID_getbyte = 99,
  MRBC_SYMID_has_key_Q = 100,
  MRBC_SYMID_has_value_Q = 101,
  MRBC_SYMID_hypot = 102,
  MRBC_SYMID_id2name = 103,
  MRBC_SYMID_include_Q = 104,
  MRBC_SYMID_index = 105,
  MRBC_SYMID_initialize = 106,
  MRBC_SYMID_inspect = 107,
  MRBC_SYMID_instance_methods = 108,
  MRBC_SYMID_instance_variables = 109,
  MRBC_SYMID_intern = 110,
  MRBC_SYMID_is_a_Q = 111,
  MRBC_SYMID_join = 112,
  MRBC_SYMID_key = 113,
  MRBC_SYMID_keys = 114,
  MRBC_SYMID_kind_of_Q = 115,
  MRBC_SYMID_last = 116,
  MRBC_SYMID_ldexp = 117,
  MRBC_SYMID_length = 118,
  MRBC_SYMID_ljust = 119,
  MRBC_SYMID_log = 120,
  MRBC_SYMID_log10 = 121,
  MRBC_SYMID_log2 = 122,
  MRBC_SYMID_loop = 123,
  MRBC_SYMID_lstrip = 124,
  MRBC_SYMID_lstrip_E = 125,
  MRBC_SYMID_map = 126,
  MRBC_SYMID_map_E = 127,
  MRBC_SYMID_max = 128,
  MRBC_SYMID_memory_statistics = 129,
  MRBC_SYMID_merge = 130,
  MRBC_SYMID_merge_E = 131,
  MRBC_SYMID_message = 132,
  MRBC_SYMID_min = 133,
  MRBC_SYMID_minmax = 134,
  MRBC_SYMID_new = 135,
  MRBC_SYMID_nil_Q = 136,
  MRBC_SYMID_object_id = 137,
  MRBC_SYMID_ord = 138,
  MRBC_SYMID_p = 139,
  MRBC_SYMID_pop = 140,
  MRBC_SYMID_print = 141,
  MRBC_SYMID_printf = 142,
  MRBC_SYMID_push = 143,
  MRBC_SYMID_puts = 144,
  MRBC_SYMID_raise = 145,
  MRBC_SYMID_reject = 146,
  MRBC_SYMID_reject_E = 147,
  MRBC_SYMID_rjust = 148,
  MRBC_SYMID_rstrip = 149,
  MRBC_SYMID_rstrip_E = 150,
  MRBC_SYMID_shift = 151,
  MRBC_SYMID_sin = 152,
  MRBC_SYMID_sinh = 153,
  MRBC_SYMID_size = 154,
  MRBC_SYMID_slice_E = 155,
  MRBC_SYMID_sort = 156,
  MRBC_SYMID_sort_E = 157,
  MRBC_SYMID_split = 158,
  MRBC_SYMID_sprintf = 159,
  MRBC_SYMID_sqrt = 160,
  MRBC_SYMID_start_with_Q = 161,
  MRBC_SYMID_strip = 162,
  MRBC_SYMID_strip_E = 163,
  MRBC_SYMID_tan = 164,
  MRBC_SYMID_tanh = 165,
  MRBC_SYMID_times = 166,
  MRBC_SYMID_to_a = 167,
  MRBC_SYMID_to_f = 168,
  MRBC_SYMID_to_h = 169,
  MRBC_SYMID_to_i = 170,
  MRBC_SYMID_to_s = 171,
  MRBC_SYMID_to_sym = 172,
  MRBC_SYMID_tr = 173,
  MRBC_SYMID_tr_E = 174,
  MRBC_SYMID_unshift = 175,
  MRBC_SYMID_upto = 176,
  MRBC_SYMID_values = 177,
  MRBC_SYMID_OR = 178,
  MRBC_SYMID_NEG = 179,
};

#define MRB_SYM(sym)  MRBC_SYMID_##sym
#define MRBC_SYM(sym) MRBC_SYMID_##sym
#endif
