/*! @file
  @brief
  mruby/c Integer and Float class

  <pre>
  Copyright (C) 2015-2023 Kyushu Institute of Technology.
  Copyright (C) 2015-2023 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/


/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdio.h>
#include <limits.h>
#if MRBC_USE_FLOAT
#include <math.h>
#endif
//@endcond

/***** Local headers ********************************************************/
#include "value.h"
#include "symbol.h"
#include "error.h"
#include "class.h"
#include "c_string.h"
#include "console.h"


/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/

/***** Integer class ********************************************************/
//================================================================
//
#if MRBC_USE_STRING
mrbc_value mrbc_integer_chr(struct VM *vm, mrbc_value num)
{
#if MRBC_USE_STRING_UTF8
  int codepoint = mrbc_integer(num);
  unsigned char bytes[4];
  int len = 0;

  // NOTE: Table 3-6. UTF-8 Bit Distribution https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf
  if (codepoint <= 0x7F ) {
    len = 1;
    bytes[0] = codepoint;
  } else if (codepoint <= 0x7FF) {
    len = 2;
    bytes[0] = 0xC0 | (codepoint >> 6);
    bytes[1] = 0x80 | (codepoint & 0xBF);
  } else if (codepoint <= 0xFFFF) {
    len = 3;
    bytes[0] = 0xE0 | (codepoint >> 12);
    bytes[1] = 0x80 | (codepoint >> 6 & 0x3F);
    bytes[2] = 0x80 | (codepoint & 0x3F);
  } else if (codepoint <= 0x10FFFF) {
    len = 4;
    bytes[0] = 0xF0 | (codepoint >> 18);
    bytes[1] = 0x80 | (codepoint >> 12 & 0x3F);
    bytes[2] = 0x80 | (codepoint >> 6 & 0x3F);
    bytes[3] = 0x80 | (codepoint & 0x3F);
  }

  mrbc_value value = mrbc_string_new(vm, bytes, len);
#else
  char buf[2] = { mrbc_integer(num) };

  mrbc_value value = mrbc_string_new(vm, buf, 1);
#endif
  return value;
}
#endif

/*! (operator) [] bit reference
 */
static void c_integer_bitref(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_integer(v[1]) < 0 ) {
    SET_INT_RETURN( 0 );
  } else {
    mrbc_int_t mask = (argc == 1) ? 1 : (1 << mrbc_integer(v[2])) - 1;
    SET_INT_RETURN( (mrbc_integer(v[0]) >> mrbc_integer(v[1])) & mask );
  }
}

//================================================================
/*! (operator) unary +
*/
static void c_integer_positive(struct VM *vm, mrbc_value v[], int argc)
{
  // do nothing
}


//================================================================
/*! (operator) unary -
*/
static void c_integer_negative(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[0]);
  SET_INT_RETURN( -num );
}


//================================================================
/*! (operator) ** power
 */
static void c_integer_power(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) == MRBC_TT_INTEGER ) {
    mrbc_int_t x = 1;
    int i;

    if( mrbc_integer(v[1]) < 0 ) x = 0;
    for( i = 0; i < mrbc_integer(v[1]); i++ ) {
      x *= mrbc_integer(v[0]);
    }
    SET_INT_RETURN( x );
  }

#if MRBC_USE_FLOAT && MRBC_USE_MATH
  else if( mrbc_type(v[1]) == MRBC_TT_FLOAT ) {
    SET_FLOAT_RETURN( pow( mrbc_integer(v[0]), mrbc_float(v[1])));
  }
#endif
}


//================================================================
/*! (operator) %
 */
static void c_integer_mod(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[1]);
  SET_INT_RETURN( v->i % num );
}


//================================================================
/*! (operator) &; bit operation AND
 */
static void c_integer_and(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[1]);
  SET_INT_RETURN(v->i & num);
}


//================================================================
/*! (operator) |; bit operation OR
 */
static void c_integer_or(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[1]);
  SET_INT_RETURN(v->i | num);
}


//================================================================
/*! (operator) ^; bit operation XOR
 */
static void c_integer_xor(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[1]);
  SET_INT_RETURN( v->i ^ num );
}


//================================================================
/*! (operator) ~; bit operation NOT
 */
static void c_integer_not(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t num = mrbc_integer(v[0]);
  SET_INT_RETURN( ~num );
}


//================================================================
/*! x-bit left shift for x
 */
static mrbc_int_t shift(mrbc_int_t x, mrbc_int_t y)
{
  // Don't support environments that include padding in int.
  const int INT_BITS = sizeof(mrbc_int_t) * CHAR_BIT;

  if( y >= INT_BITS ) return 0;
  if( y >= 0 ) return x << y;
  if( y <= -INT_BITS ) return 0;
  return x >> -y;
}


//================================================================
/*! (operator) <<; bit operation LEFT_SHIFT
 */
static void c_integer_lshift(struct VM *vm, mrbc_value v[], int argc)
{
  int num = mrbc_integer(v[1]);
  SET_INT_RETURN( shift(v->i, num) );
}


//================================================================
/*! (operator) >>; bit operation RIGHT_SHIFT
 */
static void c_integer_rshift(struct VM *vm, mrbc_value v[], int argc)
{
  int num = mrbc_integer(v[1]);
  SET_INT_RETURN( shift(v->i, -num) );
}


//================================================================
/*! (method) abs
*/
static void c_integer_abs(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_integer(v[0]) < 0 ) {
    mrbc_integer(v[0]) = -mrbc_integer(v[0]);
  }
}


//================================================================
/*! (method) clamp
 *
 * Note: Does not support Range object as the argument
 *       like `3.clamp(1..2) #=> 2`
*/
static void c_numeric_clamp(struct VM *vm, mrbc_value v[], int argc)
{
  if (argc != 2) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments (expected 2)");
    return;
  }
  mrbc_value min = v[1];
  mrbc_value max = v[2];
  if (
    (mrbc_type(min) != MRBC_TT_INTEGER && mrbc_type(min) != MRBC_TT_FLOAT) ||
    (mrbc_type(max) != MRBC_TT_INTEGER && mrbc_type(max) != MRBC_TT_FLOAT)
  ){
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "comparison failed");
    return;
  }
  if (mrbc_compare(&max, &min) < 0) {
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "min argument must be smaller than max argument");
    return;
  }
  if (mrbc_compare(&v[0], &min) < 0) {
    SET_RETURN(min);
    return;
  }
  if (mrbc_compare(&max, &v[0]) < 0) {
    SET_RETURN(max);
    return;
  }
  SET_RETURN(v[0]); /* return self */
}


#if MRBC_USE_FLOAT
//================================================================
/*! (method) to_f
*/
static void c_integer_to_f(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_float_t f = mrbc_integer(v[0]);
  SET_FLOAT_RETURN( f );
}
#endif


#if MRBC_USE_STRING
//================================================================
/*! (method) chr
*/
static void c_integer_chr(struct VM *vm, mrbc_value v[], int argc)
{
#if MRBC_USE_STRING_UTF8
  int codepoint = mrbc_integer(v[0]);

  if (codepoint > 0x10FFFF || codepoint < 0) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "out of char range");
    return;
  }

  if (0xD800 <= codepoint && codepoint <= 0xDFFF) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "invalid codepoint in UTF-8");
    return;
  }
#endif
  mrbc_value value = mrbc_integer_chr(vm, v[0]);
  SET_RETURN(value);
}


//================================================================
/*! (method) inspect, to_s
*/
static void c_integer_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  if( v[0].tt == MRBC_TT_CLASS ) {
    v[0] = mrbc_string_new_cstr(vm, mrbc_symid_to_str( v[0].cls->sym_id ));
    return;
  }

  int base = 10;
  if( argc ) {
    base = mrbc_integer(v[1]);
    if( base < 2 || base > 36 ) {
      mrbc_raisef(vm, MRBC_CLASS(ArgumentError), "invalid radix %d", base);
      return;
    }
  }

  mrbc_printf_t pf;
  char buf[16];
  mrbc_printf_init( &pf, buf, sizeof(buf), NULL );
  pf.fmt.type = 'd';
  mrbc_printf_int( &pf, v->i, base );
  mrbc_printf_end( &pf );

  mrbc_value value = mrbc_string_new_cstr(vm, buf);
  SET_RETURN(value);
}
#endif


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Integer")
  FILE("_autogen_class_integer.h")

  METHOD( "[]",		c_integer_bitref )
  METHOD( "+@",		c_integer_positive )
  METHOD( "-@",		c_integer_negative )
  METHOD( "**",		c_integer_power )
  METHOD( "%",		c_integer_mod )
  METHOD( "&",		c_integer_and )
  METHOD( "|",		c_integer_or )
  METHOD( "^",		c_integer_xor )
  METHOD( "~",		c_integer_not )
  METHOD( "<<",		c_integer_lshift )
  METHOD( ">>",		c_integer_rshift )
  METHOD( "abs",	c_integer_abs )
  METHOD( "to_i",	c_ineffect )
  METHOD( "clamp",	c_numeric_clamp )
#if MRBC_USE_FLOAT
  METHOD( "to_f",	c_integer_to_f )
#endif
#if MRBC_USE_STRING
  METHOD( "chr",	c_integer_chr )
  METHOD( "inspect",	c_integer_inspect )
  METHOD( "to_s",	c_integer_inspect )
#endif
*/
#include "_autogen_class_integer.h"



/***** Float class **********************************************************/
#if MRBC_USE_FLOAT

//================================================================
/*! (operator) unary +
*/
static void c_float_positive(struct VM *vm, mrbc_value v[], int argc)
{
  // do nothing
}


//================================================================
/*! (operator) unary -
*/
static void c_float_negative(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_float_t num = mrbc_float(v[0]);
  SET_FLOAT_RETURN( -num );
}


#if MRBC_USE_MATH
//================================================================
/*! (operator) ** power
 */
static void c_float_power(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_float_t n = 0;
  switch( mrbc_type(v[1]) ) {
  case MRBC_TT_INTEGER:	n = mrbc_integer(v[1]);	break;
  case MRBC_TT_FLOAT:	n = mrbc_float(v[1]);	break;
  default:					break;
  }

  SET_FLOAT_RETURN( pow( mrbc_float(v[0]), n ));
}
#endif


//================================================================
/*! (method) abs
*/
static void c_float_abs(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_float(v[0]) < 0 ) {
    mrbc_float(v[0]) = -mrbc_float(v[0]);
  }
}


//================================================================
/*! (method) to_i
*/
static void c_float_to_i(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_int_t i = (mrbc_int_t)mrbc_float(v[0]);
  SET_INT_RETURN( i );
}


#if MRBC_USE_STRING
//================================================================
/*! (method) inspect, to_s
*/
static void c_float_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  if( v[0].tt == MRBC_TT_CLASS ) {
    v[0] = mrbc_string_new_cstr(vm, mrbc_symid_to_str( v[0].cls->sym_id ));
    return;
  }

  char buf[16];

  snprintf( buf, sizeof(buf), "%g", v->d );
  mrbc_value value = mrbc_string_new_cstr(vm, buf);
  SET_RETURN(value);
}
#endif


/* MRBC_AUTOGEN_METHOD_TABLE

  CLASS("Float")
  FILE("_autogen_class_float.h")

  METHOD( "+@",		c_float_positive )
  METHOD( "-@",		c_float_negative )
#if MRBC_USE_MATH
  METHOD( "**",		c_float_power )
#endif
  METHOD( "abs",	c_float_abs )
  METHOD( "to_i",	c_float_to_i )
  METHOD( "to_f",	c_ineffect )
  METHOD( "clamp",	c_numeric_clamp )
#if MRBC_USE_STRING
  METHOD( "inspect",	c_float_inspect )
  METHOD( "to_s",	c_float_inspect )
#endif
*/
#include "_autogen_class_float.h"

#endif  // MRBC_USE_FLOAT
