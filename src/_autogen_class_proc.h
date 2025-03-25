

/*===== Proc class =====*/
static const mrbc_sym method_symbols_Proc[] = {
  MRBC_SYM(call),
  MRBC_SYM(new),
};

static const mrbc_func_t method_functions_Proc[] = {
  c_proc_call,
  c_proc_new,
};

struct RBuiltinClass mrbc_class_Proc = {
  .sym_id = MRBC_SYM(Proc),
  .flag_builtin = 1,
  .num_builtin_method = sizeof(method_symbols_Proc) / sizeof(mrbc_sym),
  .super = MRBC_CLASS(Object),
#if defined(MRBC_DEBUG)
  .name = "Proc",
#endif
  .method_symbols = method_symbols_Proc,
  .method_functions = method_functions_Proc,
};
