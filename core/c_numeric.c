/*! @file
  @brief
  mruby/c Fixnum and Float class

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/


#include "vm_config.h"
#include "mruby/opcode.h"
#include <stdio.h>
#include <limits.h>
#if MRBC_USE_FLOAT
#include <math.h>
#endif

#include "value.h"
#include "static.h"
#include "class.h"
#include "console.h"
#include "c_string.h"
#include "vm.h"


//================================================================
/*! (operator) [] bit reference
 */
mrbc_static_method(Fixnum, bitref)
{
  if( 0 <= v[1].i && v[1].i < 32 ) {
    SET_INT_RETURN( (v[0].i & (1 << v[1].i)) ? 1 : 0 );
  } else {
    SET_INT_RETURN( 0 );
  }
}


//================================================================
/*! (operator) unary -
*/
mrbc_static_method(Fixnum, negative)
{
  mrbc_int num = GET_INT_ARG(0);
  SET_INT_RETURN( -num );
}


//================================================================
/*! (operator) ** power
 */
mrbc_static_method(Fixnum, power)
{
  if( v[1].tt == MRBC_TT_FIXNUM ) {
    mrbc_int x = 1;
    int i;

    if( v[1].i < 0 ) x = 0;
    for( i = 0; i < v[1].i; i++ ) {
      x *= v[0].i;;
    }
    SET_INT_RETURN( x );
  }

#if MRBC_USE_FLOAT && MRBC_USE_MATH
  else if( v[1].tt == MRBC_TT_FLOAT ) {
    SET_FLOAT_RETURN( pow( v[0].i, v[1].d ) );
  }
#endif
}


//================================================================
/*! (operator) %
 */
mrbc_static_method(Fixnum, mod)
{
  mrbc_int num = GET_INT_ARG(1);
  SET_INT_RETURN( v->i % num );
}


//================================================================
/*! (operator) &; bit operation AND
 */
mrbc_static_method(Fixnum, and)
{
  mrbc_int num = GET_INT_ARG(1);
  SET_INT_RETURN(v->i & num);
}


//================================================================
/*! (operator) |; bit operation OR
 */
mrbc_static_method(Fixnum, or)
{
  mrbc_int num = GET_INT_ARG(1);
  SET_INT_RETURN(v->i | num);
}


//================================================================
/*! (operator) ^; bit operation XOR
 */
mrbc_static_method(Fixnum, xor)
{
  mrbc_int num = GET_INT_ARG(1);
  SET_INT_RETURN( v->i ^ num );
}


//================================================================
/*! (operator) ~; bit operation NOT
 */
mrbc_static_method(Fixnum, not)
{
  mrbc_int num = GET_INT_ARG(0);
  SET_INT_RETURN( ~num );
}


//================================================================
/*! x-bit left shift for x
 */
static mrbc_int shift(mrbc_int x, mrbc_int y)
{
  // Don't support environments that include padding in int.
  const int INT_BITS = sizeof(mrbc_int) * CHAR_BIT;

  if( y >= INT_BITS ) return 0;
  if( y >= 0 ) return x << y;
  if( y <= -INT_BITS ) return 0;
  return x >> -y;
}


//================================================================
/*! (operator) <<; bit operation LEFT_SHIFT
 */
mrbc_static_method(Fixnum, lshift)
{
  int num = GET_INT_ARG(1);
  SET_INT_RETURN( shift(v->i, num) );
}


//================================================================
/*! (operator) >>; bit operation RIGHT_SHIFT
 */
mrbc_static_method(Fixnum, rshift)
{
  int num = GET_INT_ARG(1);
  SET_INT_RETURN( shift(v->i, -num) );
}


//================================================================
/*! (method) abs
*/
mrbc_static_method(Fixnum, abs)
{
  if( v[0].i < 0 ) {
    v[0].i = -v[0].i;
  }
}


#if MRBC_USE_FLOAT
//================================================================
/*! (method) to_f
*/
mrbc_static_method(Fixnum, to_f)
{
  mrbc_float f = GET_INT_ARG(0);
  SET_FLOAT_RETURN( f );
}
#endif


#if MRBC_USE_STRING
//================================================================
/*! (method) chr
*/
mrbc_static_method(Fixnum, chr)
{
  char buf[2] = { GET_INT_ARG(0) };

  mrbc_value value = mrbc_string_new(vm, buf, 1);
  SET_RETURN(value);
}


//================================================================
/*! (method) to_s
*/
mrbc_static_method(Fixnum, to_s)
{
  int base = 10;
  if( argc ) {
    base = GET_INT_ARG(1);
    if( base < 2 || base > 36 ) {
      return;	// raise ? ArgumentError
    }
  }

  mrbc_printf pf;
  char buf[16];
  mrbc_printf_init( &pf, buf, sizeof(buf), NULL );
  pf.fmt.type = 'd';
  mrbc_printf_int( &pf, v->i, base );
  mrbc_printf_end( &pf );

  mrbc_value value = mrbc_string_new_cstr(vm, buf);
  SET_RETURN(value);
}
#endif



// Float
#if MRBC_USE_FLOAT

//================================================================
/*! (operator) unary -
*/
mrbc_static_method(Float, negative)
{
  mrbc_float num = GET_FLOAT_ARG(0);
  SET_FLOAT_RETURN( -num );
}


#if MRBC_USE_MATH
//================================================================
/*! (operator) ** power
 */
mrbc_static_method(Float, power)
{
  mrbc_float n = 0;
  switch( v[1].tt ) {
  case MRBC_TT_FIXNUM:	n = v[1].i;	break;
  case MRBC_TT_FLOAT:	n = v[1].d;	break;
  default:				break;
  }

  SET_FLOAT_RETURN( pow( v[0].d, n ));
}
#endif


//================================================================
/*! (method) abs
*/
mrbc_static_method(Float, abs)
{
  if( v[0].d < 0 ) {
    v[0].d = -v[0].d;
  }
}


//================================================================
/*! (method) to_i
*/
mrbc_static_method(Float, to_i)
{
  mrbc_int i = (mrbc_int)GET_FLOAT_ARG(0);
  SET_INT_RETURN( i );
}


#if MRBC_USE_STRING
//================================================================
/*! (method) to_s
*/
mrbc_static_method(Float, to_s)
{
  char buf[16];

  snprintf( buf, sizeof(buf), "%g", v->d );
  mrbc_value value = mrbc_string_new_cstr(vm, buf);
  SET_RETURN(value);
}
#endif

#endif
