/*! @file
  @brief
  mruby/c Math module

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
#include <math.h>
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
#if MRBC_USE_FLOAT && MRBC_USE_MATH
//================================================================
/*! convert mrbc_value to c double
*/
static double to_double( struct VM *vm, const mrbc_value *v )
{
  switch( mrbc_type(*v) ) {
  case MRBC_TT_INTEGER:	return (double)v->i;
  case MRBC_TT_FLOAT:	return (double)v->d;
  default: break;
  }

  mrbc_raise(vm, MRBC_CLASS(TypeError), 0);
  return 0.0;
}

//================================================================
/*! (method) acos
*/
static void c_math_acos(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( acos( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) acosh
*/
static void c_math_acosh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( acosh( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) asin
*/
static void c_math_asin(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( asin( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) asinh
*/
static void c_math_asinh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( asinh( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) atan
*/
static void c_math_atan(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atan( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) atan2
*/
static void c_math_atan2(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atan2( to_double(vm, &v[1]), to_double(vm, &v[2]) ));
}

//================================================================
/*! (method) atanh
*/
static void c_math_atanh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( atanh( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) cbrt
*/
static void c_math_cbrt(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cbrt( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) cos
*/
static void c_math_cos(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cos( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) cosh
*/
static void c_math_cosh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( cosh( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) erf
*/
static void c_math_erf(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( erf( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) erfc
*/
static void c_math_erfc(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( erfc( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) exp
*/
static void c_math_exp(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( exp( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) hypot
*/
static void c_math_hypot(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( hypot( to_double(vm, &v[1]), to_double(vm, &v[2]) ));
}

//================================================================
/*! (method) ldexp
*/
static void c_math_ldexp(struct VM *vm, mrbc_value v[], int argc)
{
  int exp;
  switch( mrbc_type(v[2]) ) {
  case MRBC_TT_INTEGER:	exp = v[2].i;		break;
  case MRBC_TT_FLOAT:	exp = (int)v[2].d;	break;
  default:
    mrbc_raise(vm, MRBC_CLASS(TypeError), 0);
    return;
  }

  v[0] = mrbc_float_value( ldexp( to_double(vm, &v[1]), exp ));
}

//================================================================
/*! (method) log
*/
static void c_math_log(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) log10
*/
static void c_math_log10(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log10( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) log2
*/
static void c_math_log2(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( log2( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) sin
*/
static void c_math_sin(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sin( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) sinh
*/
static void c_math_sinh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sinh( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) sqrt
*/
static void c_math_sqrt(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( sqrt( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) tan
*/
static void c_math_tan(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( tan( to_double(vm, &v[1]) ));
}

//================================================================
/*! (method) tanh
*/
static void c_math_tanh(struct VM *vm, mrbc_value v[], int argc)
{
  v[0] = mrbc_float_value( tanh( to_double(vm, &v[1]) ));
}

/***** Global functions *****************************************************/
//================================================================
/*! initialize
*/
void mrbc_init_module_math(void)
{
  static mrbc_value e = mrbc_float_value(M_E);
  mrbc_set_class_const( MRBC_CLASS(Math), MRBC_SYM(E), &e );

  static mrbc_value pi = mrbc_float_value(M_PI);
  mrbc_set_class_const( MRBC_CLASS(Math), MRBC_SYM(PI), &pi );
}

/* MRBC_AUTOGEN_METHOD_TABLE

  MODULE("Math")
  FILE("_autogen_module_math.h")

  METHOD( "acos",	c_math_acos )
  METHOD( "acosh",	c_math_acosh )
  METHOD( "asin",	c_math_asin )
  METHOD( "asinh",	c_math_asinh )
  METHOD( "atan",	c_math_atan )
  METHOD( "atan2",	c_math_atan2 )
  METHOD( "atanh",	c_math_atanh )
  METHOD( "cbrt",	c_math_cbrt )
  METHOD( "cos",	c_math_cos )
  METHOD( "cosh",	c_math_cosh )
  METHOD( "erf",	c_math_erf )
  METHOD( "erfc",	c_math_erfc )
  METHOD( "exp",	c_math_exp )
  METHOD( "hypot",	c_math_hypot )
  METHOD( "ldexp",	c_math_ldexp )
  METHOD( "log",	c_math_log )
  METHOD( "log10",	c_math_log10 )
  METHOD( "log2",	c_math_log2 )
  METHOD( "sin",	c_math_sin )
  METHOD( "sinh",	c_math_sinh )
  METHOD( "sqrt",	c_math_sqrt )
  METHOD( "tan",	c_math_tan )
  METHOD( "tanh",	c_math_tanh )
*/
#include "_autogen_module_math.h"

#endif  // MRBC_USE_FLOAT && MRBC_USE_MATH
