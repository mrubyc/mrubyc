/*! @file
  @brief
  mruby/c Math class

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include "vm_config.h"
#include <math.h>

#include "value.h"
#include "static.h"
#include "class.h"


#if MRBC_USE_FLOAT && MRBC_USE_MATH

//================================================================
/*! convert mrbc_value to c double
*/
static double to_double( const mrbc_value *v )
{
  switch( v->tt ) {
  case MRBC_TT_FIXNUM:	return (double)v->i;
  case MRBC_TT_FLOAT:	return (double)v->d;
  default:		return 0;	// TypeError. raise?
  }
}



//================================================================
/*! (method) acos
*/
mrbc_static_method(Math, acos)
{
  v[0] = mrbc_float_value( acos( to_double(&v[1]) ));
}

//================================================================
/*! (method) acosh
*/
mrbc_static_method(Math, acosh)
{
  v[0] = mrbc_float_value( acosh( to_double(&v[1]) ));
}

//================================================================
/*! (method) asin
*/
mrbc_static_method(Math, asin)
{
  v[0] = mrbc_float_value( asin( to_double(&v[1]) ));
}

//================================================================
/*! (method) asinh
*/
mrbc_static_method(Math, asinh)
{
  v[0] = mrbc_float_value( asinh( to_double(&v[1]) ));
}

//================================================================
/*! (method) atan
*/
mrbc_static_method(Math, atan)
{
  v[0] = mrbc_float_value( atan( to_double(&v[1]) ));
}

//================================================================
/*! (method) atan2
*/
mrbc_static_method(Math, atan2)
{
  v[0] = mrbc_float_value( atan2( to_double(&v[1]), to_double(&v[2]) ));
}

//================================================================
/*! (method) atanh
*/
mrbc_static_method(Math, atanh)
{
  v[0] = mrbc_float_value( atanh( to_double(&v[1]) ));
}

//================================================================
/*! (method) cbrt
*/
mrbc_static_method(Math, cbrt)
{
  v[0] = mrbc_float_value( cbrt( to_double(&v[1]) ));
}

//================================================================
/*! (method) cos
*/
mrbc_static_method(Math, cos)
{
  v[0] = mrbc_float_value( cos( to_double(&v[1]) ));
}

//================================================================
/*! (method) cosh
*/
mrbc_static_method(Math, cosh)
{
  v[0] = mrbc_float_value( cosh( to_double(&v[1]) ));
}

//================================================================
/*! (method) erf
*/
mrbc_static_method(Math, erf)
{
  v[0] = mrbc_float_value( erf( to_double(&v[1]) ));
}

//================================================================
/*! (method) erfc
*/
mrbc_static_method(Math, erfc)
{
  v[0] = mrbc_float_value( erfc( to_double(&v[1]) ));
}

//================================================================
/*! (method) exp
*/
mrbc_static_method(Math, exp)
{
  v[0] = mrbc_float_value( exp( to_double(&v[1]) ));
}

//================================================================
/*! (method) hypot
*/
mrbc_static_method(Math, hypot)
{
  v[0] = mrbc_float_value( hypot( to_double(&v[1]), to_double(&v[2]) ));
}

//================================================================
/*! (method) ldexp
*/
mrbc_static_method(Math, ldexp)
{
  int exp;
  switch( v[2].tt ) {
  case MRBC_TT_FIXNUM:	exp = v[2].i;		break;
  case MRBC_TT_FLOAT:	exp = (int)v[2].d;	break;
  default:		exp = 0;	// TypeError. raise?
  }

  v[0] = mrbc_float_value( ldexp( to_double(&v[1]), exp ));
}

//================================================================
/*! (method) log
*/
mrbc_static_method(Math, log)
{
  v[0] = mrbc_float_value( log( to_double(&v[1]) ));
}

//================================================================
/*! (method) log10
*/
mrbc_static_method(Math, log10)
{
  v[0] = mrbc_float_value( log10( to_double(&v[1]) ));
}

//================================================================
/*! (method) log2
*/
mrbc_static_method(Math, log2)
{
  v[0] = mrbc_float_value( log2( to_double(&v[1]) ));
}

//================================================================
/*! (method) sin
*/
mrbc_static_method(Math, sin)
{
  v[0] = mrbc_float_value( sin( to_double(&v[1]) ));
}

//================================================================
/*! (method) sinh
*/
mrbc_static_method(Math, sinh)
{
  v[0] = mrbc_float_value( sinh( to_double(&v[1]) ));
}

//================================================================
/*! (method) sqrt
*/
mrbc_static_method(Math, sqrt)
{
  v[0] = mrbc_float_value( sqrt( to_double(&v[1]) ));
}

//================================================================
/*! (method) tan
*/
mrbc_static_method(Math, tan)
{
  v[0] = mrbc_float_value( tan( to_double(&v[1]) ));
}

//================================================================
/*! (method) tanh
*/
mrbc_static_method(Math, tanh)
{
  v[0] = mrbc_float_value( tanh( to_double(&v[1]) ));
}


#endif  // MRBC_USE_FLOAT && MRBC_USE_MATH
