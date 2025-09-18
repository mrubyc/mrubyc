/*! @file
  @brief
  mruby/c mrbc_value definition.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

//================================================================
/*!@brief
  Value object. Default version.
*/
typedef struct RObject {
  mrbc_vtype tt;
  union {
    mrbc_int_t i;		// MRBC_TT_INTEGER
#if MRBC_USE_FLOAT
    mrbc_float_t d;		// MRBC_TT_FLOAT
#endif
    mrbc_sym sym_id;		// MRBC_TT_SYMBOL
    struct RBasic *obj;		// use inc/dec ref only.
    struct RClass *cls;		// MRBC_TT_CLASS, MRBC_TT_MODULE
    struct RInstance *instance;	// MRBC_TT_OBJECT
    struct RProc *proc;		// MRBC_TT_PROC
    struct RArray *array;	// MRBC_TT_ARRAY
    struct RString *string;	// MRBC_TT_STRING
    struct RRange *range;	// MRBC_TT_RANGE
    struct RHash *hash;		// MRBC_TT_HASH
    struct RException *exception; // MRBC_TT_EXCEPTION
    void *handle;		// internal use only.
  };
} mrbc_value;



//================================================================
// getters
/*!
  @def mrbc_type(o)
  get the type (#mrbc_vtype) from mrbc_value.

  @def mrbc_integer(o)
  get int value from mrbc_value.

  @def mrbc_float(o)
  get float(double) value from mrbc_value.

  @def mrbc_symbol(o)
  get symbol value (#mrbc_sym) from mrbc_value.
*/
#define mrbc_type(o)		((o).tt)
#define mrbc_integer(o)		((o).i)
#define mrbc_float(o)		((o).d)
#define mrbc_symbol(o)		((o).sym_id)


// make immediate values.
#define mrbc_integer_value(n)	((mrbc_value){.tt = MRBC_TT_INTEGER, .i=(n)})
#define mrbc_float_value(vm,n)	((mrbc_value){.tt = MRBC_TT_FLOAT, .d=(n)})
#define mrbc_nil_value()	((mrbc_value){.tt = MRBC_TT_NIL})
#define mrbc_true_value()	((mrbc_value){.tt = MRBC_TT_TRUE})
#define mrbc_false_value()	((mrbc_value){.tt = MRBC_TT_FALSE})
#define mrbc_bool_value(n)	((mrbc_value){.tt = (n)?MRBC_TT_TRUE:MRBC_TT_FALSE})
#define mrbc_symbol_value(n)	((mrbc_value){.tt = MRBC_TT_SYMBOL, .sym_id=(n)})
#define mrbc_immediate_value(...) MRBC_arg_choice(__VA_ARGS__, mrbc_immediate_value2, mrbc_immediate_value1) (__VA_ARGS__)
#define mrbc_immediate_value1(type) \
  ((mrbc_value){.tt=type})			// internal use only.
#define mrbc_immediate_value2(type, v2) \
  ((mrbc_value){.tt=type, v2})			// internal use only.



//================================================================
// mrbc_value setters
static inline void mrbc_set_integer(mrbc_value *p, mrbc_int_t n)
{
  p->tt = MRBC_TT_INTEGER;
  p->i = n;
}

static inline void mrbc_set_float(mrbc_value *p, mrbc_float_t d)
{
  p->tt = MRBC_TT_FLOAT;
  p->d = d;
}

static inline void mrbc_set_nil(mrbc_value *p)
{
  p->tt = MRBC_TT_NIL;
}

static inline void mrbc_set_true(mrbc_value *p)
{
  p->tt = MRBC_TT_TRUE;
}

static inline void mrbc_set_false(mrbc_value *p)
{
  p->tt = MRBC_TT_FALSE;
}

static inline void mrbc_set_bool(mrbc_value *p, int n)
{
  p->tt = n ? MRBC_TT_TRUE: MRBC_TT_FALSE;
}

static inline void mrbc_set_symbol(mrbc_value *p, mrbc_sym sym_id)
{
  p->tt = MRBC_TT_SYMBOL;
  p->sym_id = sym_id;
}

static inline void mrbc_set_tt(mrbc_value *p, mrbc_vtype type)	// internal use only.
{
  p->tt = type;
}
