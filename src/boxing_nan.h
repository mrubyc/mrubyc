/*! @file
  @brief
  mruby/c NaN boxing mrbc_value definition.

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/


//@cond
#define MRBC_NAN_BITS 0xFFFF

//================================================================
/*!@brief
  Value object. NaN boxing version.

Bit field of:
 IEEE754 binary64
  SEEEEEEE EEEEFFFF  FFFFFFFF FFFFFFFF   FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
  b7*      b6*       b5*      b4*        b3*      b2*      b1*      b0*

 RObject
  11111111 11111111  00000000 TTTTTTTT   VVVVVVVV VVVVVVVV VVVVVVVV VVVVVVVV
  ~~~~~~~~~~~~~~nan  ~~~~~pad ~~~~~~tt
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~tt2

(note)
 This assumption is valid only when using gcc on a 32-bit little-endian machine.
*/
typedef struct RObject {
  union {
    struct {
      // LSB 32bit
      union {
        int32_t i;			// MRBC_TT_INTEGER
        mrbc_sym sym_id;		// MRBC_TT_SYMBOL

        struct RBasic *obj;		// use inc/dec ref only.
        struct RClass *cls;		// MRBC_TT_CLASS, MRBC_TT_MODULE
        struct RInstance *instance;	// MRBC_TT_OBJECT
        struct RProc *proc;		// MRBC_TT_PROC
        struct RArray *array;		// MRBC_TT_ARRAY
        struct RString *string;		// MRBC_TT_STRING
        struct RRange *range;		// MRBC_TT_RANGE
        struct RHash *hash;		// MRBC_TT_HASH
        struct RException *exception;	// MRBC_TT_EXCEPTION
        void *handle;			// internal use only.
      };

      // MSB 32bit
      union {
        struct {
          int8_t   tt;	// valid if nan==0xffff
          uint8_t  pad;
          uint16_t nan;
        };
        uint32_t tt2;
      };
    };

    double d;				// MRBC_TT_FLOAT
  };
} mrbc_value;



//================================================================
#define mrbc_type(v) ((v).nan == MRBC_NAN_BITS ? (v).tt : MRBC_TT_FLOAT)
#define mrbc_integer(o)		((o).i)
#define mrbc_float(o)		((o).d)
#define mrbc_symbol(o)		((o).sym_id)


// make immediate values.
#define mrbc_integer_value(n)	(mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_INTEGER, .i = (n)}
#define mrbc_float_value(vm,n)	(mrbc_value){.d=(n)}
#define mrbc_nil_value()	(mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_NIL}
#define mrbc_true_value()	(mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_TRUE}
#define mrbc_false_value()	(mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_FALSE}
#define mrbc_bool_value(n)	(mrbc_value){.tt2 = (n)? (MRBC_NAN_BITS << 16 | MRBC_TT_TRUE) : \
                                                         (MRBC_NAN_BITS << 16 | MRBC_TT_FALSE)}
#define mrbc_symbol_value(n)	(mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_SYMBOL, .sym_id=(n)}

#define mrbc_immediate_value(...) MRBC_arg_choice(__VA_ARGS__, mrbc_immediate_value2, mrbc_immediate_value1) (__VA_ARGS__)
#define mrbc_immediate_value1(type) \
  ((mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | (type)})		// internal use only.
#define mrbc_immediate_value2(type, content) \
  ((mrbc_value){.tt2 = MRBC_NAN_BITS << 16 | (type), content})	// internal use only.



//================================================================
static inline void mrbc_set_integer(mrbc_value *p, mrbc_int_t n)
{
  p->tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_INTEGER;
  p->i = n;
}

static inline void mrbc_set_float(mrbc_value *p, mrbc_float_t d)
{
  p->d = d;
}

static inline void mrbc_set_nil(mrbc_value *p)
{
  p->tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_NIL;
}

static inline void mrbc_set_true(mrbc_value *p)
{
  p->tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_TRUE;
}

static inline void mrbc_set_false(mrbc_value *p)
{
  p->tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_FALSE;
}

static inline void mrbc_set_bool(mrbc_value *p, int n)
{
  p->tt2 = n?
    (MRBC_NAN_BITS << 16 | MRBC_TT_TRUE):
    (MRBC_NAN_BITS << 16 | MRBC_TT_FALSE);
}

static inline void mrbc_set_symbol(mrbc_value *p, mrbc_sym sym_id)
{
  p->tt2 = MRBC_NAN_BITS << 16 | MRBC_TT_SYMBOL;
  p->sym_id = sym_id;
}

static inline void mrbc_set_tt(mrbc_value *p, mrbc_vtype type)
{
  p->tt2 = MRBC_NAN_BITS << 16 | type;
}

//@endcond
