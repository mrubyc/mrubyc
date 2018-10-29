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
static void c_math_acos(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( acos( to_double(&v[1]) ));
}

//================================================================
/*! (method) acosh
*/
static void c_math_acosh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( acosh( to_double(&v[1]) ));
}

//================================================================
/*! (method) asin
*/
static void c_math_asin(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( asin( to_double(&v[1]) ));
}

//================================================================
/*! (method) asinh
*/
static void c_math_asinh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( asinh( to_double(&v[1]) ));
}

//================================================================
/*! (method) atan
*/
static void c_math_atan(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atan( to_double(&v[1]) ));
}

//================================================================
/*! (method) atan2
*/
static void c_math_atan2(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atan2( to_double(&v[1]), to_double(&v[2]) ));
}

//================================================================
/*! (method) atanh
*/
static void c_math_atanh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atanh( to_double(&v[1]) ));
}

//================================================================
/*! (method) cbrt
*/
static void c_math_cbrt(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cbrt( to_double(&v[1]) ));
}

//================================================================
/*! (method) cos
*/
static void c_math_cos(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cos( to_double(&v[1]) ));
}

//================================================================
/*! (method) cosh
*/
static void c_math_cosh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cosh( to_double(&v[1]) ));
}

//================================================================
/*! (method) erf
*/
static void c_math_erf(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( erf( to_double(&v[1]) ));
}

//================================================================
/*! (method) erfc
*/
static void c_math_erfc(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( erfc( to_double(&v[1]) ));
}

//================================================================
/*! (method) exp
*/
static void c_math_exp(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( exp( to_double(&v[1]) ));
}

//================================================================
/*! (method) hypot
*/
static void c_math_hypot(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( hypot( to_double(&v[1]), to_double(&v[2]) ));
}

//================================================================
/*! (method) ldexp
*/
static void c_math_ldexp(struct VM *vm, mrbc_value v[], int argc)
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
static void c_math_log(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log( to_double(&v[1]) ));
}

//================================================================
/*! (method) log10
*/
static void c_math_log10(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log10( to_double(&v[1]) ));
}

//================================================================
/*! (method) log2
*/
static void c_math_log2(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log2( to_double(&v[1]) ));
}

//================================================================
/*! (method) sin
*/
static void c_math_sin(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sin( to_double(&v[1]) ));
}

//================================================================
/*! (method) sinh
*/
static void c_math_sinh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sinh( to_double(&v[1]) ));
}

//================================================================
/*! (method) sqrt
*/
static void c_math_sqrt(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sqrt( to_double(&v[1]) ));
}

//================================================================
/*! (method) tan
*/
static void c_math_tan(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( tan( to_double(&v[1]) ));
}

//================================================================
/*! (method) tanh
*/
static void c_math_tanh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( tanh( to_double(&v[1]) ));
}


//================================================================
/*! initialize
*/
void mrbc_init_class_math(struct VM *vm)
{
  mrbc_class_math = mrbc_define_class(vm, "Math",	mrbc_class_object);

  mrbc_define_method(vm, mrbc_class_math, "acos",	c_math_acos);
  mrbc_define_method(vm, mrbc_class_math, "acosh",	c_math_acosh);
  mrbc_define_method(vm, mrbc_class_math, "asin",	c_math_asin);
  mrbc_define_method(vm, mrbc_class_math, "asinh",	c_math_asinh);
  mrbc_define_method(vm, mrbc_class_math, "atan",	c_math_atan);
  mrbc_define_method(vm, mrbc_class_math, "atan2",	c_math_atan2);
  mrbc_define_method(vm, mrbc_class_math, "atanh",	c_math_atanh);
  mrbc_define_method(vm, mrbc_class_math, "cbrt",	c_math_cbrt);
  mrbc_define_method(vm, mrbc_class_math, "cos",	c_math_cos);
  mrbc_define_method(vm, mrbc_class_math, "cosh",	c_math_cosh);
  mrbc_define_method(vm, mrbc_class_math, "erf",	c_math_erf);
  mrbc_define_method(vm, mrbc_class_math, "erfc",	c_math_erfc);
  mrbc_define_method(vm, mrbc_class_math, "exp",	c_math_exp);
  mrbc_define_method(vm, mrbc_class_math, "hypot",	c_math_hypot);
  mrbc_define_method(vm, mrbc_class_math, "ldexp",	c_math_ldexp);
  mrbc_define_method(vm, mrbc_class_math, "log",	c_math_log);
  mrbc_define_method(vm, mrbc_class_math, "log10",	c_math_log10);
  mrbc_define_method(vm, mrbc_class_math, "log2",	c_math_log2);
  mrbc_define_method(vm, mrbc_class_math, "sin",	c_math_sin);
  mrbc_define_method(vm, mrbc_class_math, "sinh",	c_math_sinh);
  mrbc_define_method(vm, mrbc_class_math, "sqrt",	c_math_sqrt);
  mrbc_define_method(vm, mrbc_class_math, "tan",	c_math_tan);
  mrbc_define_method(vm, mrbc_class_math, "tanh",	c_math_tanh);
}


#endif  // MRBC_USE_FLOAT && MRBC_USE_MATH
