/*! @file
  @brief
  mruby/c Integer and Float class

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

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
#include "mrubyc.h"

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

    if( mrbc_integer(v[1]) < 0 ) x = 0;
    for( int i = 0; i < mrbc_integer(v[1]); i++ ) {
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
  if( mrbc_type(v[1]) != MRBC_TT_INTEGER ) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), 0 );
    return;
  }

  mrbc_int_t v0 = v[0].i;
  mrbc_int_t v1 = v[1].i;

  if( v1 == 0 ) {
    mrbc_raise(vm, MRBC_CLASS(ZeroDivisionError), 0 );
    return;
  }

  mrbc_int_t ret = v0 % v1;

  if( (ret != 0) && ((v0 ^ v1) < 0) ) ret += v1;
  SET_INT_RETURN( ret );
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
    mrbc_raise(vm, MRBC_CLASS(ArgumentError), "wrong number of arguments");
    return;
  }
  mrbc_value min = v[1];
  mrbc_value max = v[2];
  if (
    (mrbc_type(min) != MRBC_TT_INTEGER && mrbc_type(min) != MRBC_TT_FLOAT) ||
    (mrbc_type(max) != MRBC_TT_INTEGER && mrbc_type(max) != MRBC_TT_FLOAT)
  ) {
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
  mrbc_int_t codepoint = mrbc_integer(v[0]);
  char buf[5];
  int len;

  if( codepoint < 0 ) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "negative integer");
    return;
  }
  if( codepoint <= 0x7F ) {
    buf[0] = (char)codepoint;
    len = 1;
  } else if( codepoint <= 0x7FF ) {
    buf[0] = (char)(0xC0 | (codepoint >> 6));
    buf[1] = (char)(0x80 | (codepoint & 0x3F));
    len = 2;
  } else if( codepoint <= 0xFFFF ) {
    buf[0] = (char)(0xE0 | (codepoint >> 12));
    buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    buf[2] = (char)(0x80 | (codepoint & 0x3F));
    len = 3;
  } else if( codepoint <= 0x10FFFF ) {
    buf[0] = (char)(0xF0 | (codepoint >> 18));
    buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
    buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    buf[3] = (char)(0x80 | (codepoint & 0x3F));
    len = 4;
  } else {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "codepoint too large");
    return;
  }

  mrbc_value value = mrbc_string_new(vm, buf, len);
  SET_RETURN(value);
#else
  char buf[2] = { mrbc_integer(v[0]) };

  mrbc_value value = mrbc_string_new(vm, buf, 1);
  SET_RETURN(value);
#endif
}


//================================================================
/*! (method) inspect, to_s
*/
static void c_integer_inspect(struct VM *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    mrbc_object_inspect(vm, v, argc);
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
  if( mrbc_type(v[0]) == MRBC_TT_CLASS ) {
    mrbc_object_inspect(vm, v, argc);
    return;
  }

  char buf[32];
  mrbc_format_float(buf, sizeof(buf), v->d);
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
