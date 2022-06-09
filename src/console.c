/*! @file
  @brief
  console output module. (not yet input)

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
#include "vm_config.h"

/***** System headers *******************************************************/
#include <stdint.h>
#include <stdarg.h>
#include <types.h>
#include <string.h>
#include <assert.h>
#if MRBC_USE_FLOAT
#include <stdio.h>
#endif

/***** Local headers ********************************************************/
#include "compat.h"
#include "hal_selector.h"
#include "value.h"
#include "class.h"
#include "console.h"
#include "symbol.h"
#include "error.h"
#include "c_string.h"
#include "c_array.h"
#include "c_hash.h"
#include "c_range.h"


/***** Constant values ******************************************************/
#define MRBC_PRINTF_MAX_WIDTH 82


/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! output a character

  @param  c	character
*/
void mrbc_putchar(char c)
{
#if defined(MRBC_CONVERT_CRLF)
  static const char CRLF[2] = "\r\n";
  if( c == '\n' ) {
    hal_write(1, CRLF, 2);
  } else {
    hal_write(1, &c, 1);
  }

#else
    hal_write(1, &c, 1);
#endif
}


//================================================================
/*! output string with length parameter.

  @param str	str
  @param size	byte length.
*/
void mrbc_nprint(const char *str, int size)
{
#if defined(MRBC_CONVERT_CRLF)
  static const char CRLF[2] = "\r\n";
  const char *p1 = str;
  const char *p2 = str;
  int i;

  for( i = 0; i < size; i++ ) {
    if( *p1++ == '\n' ) {
      hal_write(1, p2, p1 - p2 - 1);
      hal_write(1, CRLF, 2);
      p2 = p1;
    }
  }
  if( p1 != p2 ) {
    hal_write(1, p2, p1 - p2);
  }

#else
  hal_write(1, str, size);
#endif
}


//----------------------------------------------------------------
/* sub function for mrbc_snprintf, mrbc_printf
*/
static int mrbc_printf_sub_output_arg( mrbc_printf_t *pf, va_list *ap )
{
  int ret;

  switch(pf->fmt.type) {
  case 'c':
    ret = mrbc_printf_char( pf, va_arg(*ap, int) );
    break;

  case 's':
    ret = mrbc_printf_str( pf, va_arg(*ap, char *), ' ');
    break;

  case 'd':
  case 'i':
  case 'u':
    ret = mrbc_printf_int( pf, va_arg(*ap, int), 10);
    break;

  case 'D':	// for mrbc_int (see mrbc_print_sub)
    ret = mrbc_printf_int( pf, va_arg(*ap, mrbc_int), 10);
    break;

  case 'b':
  case 'B':
    ret = mrbc_printf_bit( pf, va_arg(*ap, unsigned int), 1);
    break;

  case 'x':
  case 'X':
    ret = mrbc_printf_bit( pf, va_arg(*ap, unsigned int), 4);
    break;

#if MRBC_USE_FLOAT
  case 'f':
  case 'e':
  case 'E':
  case 'g':
  case 'G':
    ret = mrbc_printf_float( pf, va_arg(*ap, double) );
    break;
#endif
  case 'p':
    ret = mrbc_printf_pointer( pf, va_arg(*ap, void *) );
    break;

  default:
    ret = 0;
    break;
  }

  return ret;
}


//================================================================
/*! formatted output conversion, output to fixed buffer.

  @param  buf		output buffer.
  @param  bufsiz	buffer size.
  @param  fstr		format string.
*/
void mrbc_snprintf( char *buf, int bufsiz, const char *fstr, ...)
{
  va_list ap;
  va_start(ap, fstr);

  mrbc_printf_t pf;
  mrbc_printf_init( &pf, buf, bufsiz, fstr );

  while( 1 ) {
    if( mrbc_printf_main( &pf ) <= 0 ) break;
				// normal end (==0) or buffer full (<0).
    if( mrbc_printf_sub_output_arg( &pf, &ap ) != 0 ) break;
  }

  mrbc_printf_end( &pf );
  va_end(ap);
}


//================================================================
/*! formatted output conversion, output to console.

  @param  fstr		format string.
*/
void mrbc_printf(const char *fstr, ...)
{
  va_list ap;
  va_start(ap, fstr);

  mrbc_printf_t pf;
  char buf[MRBC_PRINTF_MAX_WIDTH];
  mrbc_printf_init( &pf, buf, sizeof(buf), fstr );

  while( 1 ) {
    int ret = mrbc_printf_main( &pf );
    if( mrbc_printf_len( &pf ) ) {
      mrbc_nprint( buf, mrbc_printf_len( &pf ) );
      mrbc_printf_clear( &pf );
    }
    if( ret == 0 ) break;
    if( ret < 0 ) continue;

    mrbc_printf_sub_output_arg( &pf, &ap );
    mrbc_nprint( buf, mrbc_printf_len( &pf ) );
    mrbc_printf_clear( &pf );
  }

  va_end(ap);
}


//================================================================
/*! sprintf subcontract function

  @param  pf	pointer to mrbc_printf
  @retval 0	(format string) done.
  @retval 1	found a format identifier.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_main( mrbc_printf_t *pf )
{
  int ch = -1;
  pf->fmt = (struct RPrintfFormat){0};

  while( pf->p < pf->buf_end && (ch = *pf->fstr) != '\0' ) {
    pf->fstr++;
    if( ch == '%' ) {
      if( *pf->fstr == '%' ) {	// is "%%"
	pf->fstr++;
      } else {
	goto PARSE_FLAG;
      }
    }
    *pf->p++ = ch;
  }
  return -(ch != '\0');


 PARSE_FLAG:
  // parse format - '%' [flag] [width] [.precision] type
  //   e.g. "%05d"
  while( (ch = *pf->fstr) ) {
    switch( ch ) {
    case '+': pf->fmt.flag_plus = 1; break;
    case ' ': pf->fmt.flag_space = 1; break;
    case '-': pf->fmt.flag_minus = 1; break;
    case '0': pf->fmt.flag_zero = 1; break;
    default : goto PARSE_WIDTH;
    }
    pf->fstr++;
  }

 PARSE_WIDTH:
  while( (void)(ch = *pf->fstr - '0'), (0 <= ch && ch <= 9)) {	// isdigit()
    pf->fmt.width = pf->fmt.width * 10 + ch;
    pf->fstr++;
  }
  if( *pf->fstr == '.' ) {
    pf->fstr++;
    while( (void)(ch = *pf->fstr - '0'), (0 <= ch && ch <= 9)) {
      pf->fmt.precision = pf->fmt.precision * 10 + ch;
      pf->fstr++;
    }
  }
  if( *pf->fstr ) pf->fmt.type = *pf->fstr++;

  return 1;
}


//================================================================
/*! sprintf subcontract function for char '%c'

  @param  pf	pointer to mrbc_printf
  @param  ch	output character (ASCII)
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_char( mrbc_printf_t *pf, int ch )
{
  if( pf->fmt.flag_minus ) {
    if( pf->p == pf->buf_end ) return -1;
    *pf->p++ = ch;
  }

  int width = pf->fmt.width;
  while( --width > 0 ) {
    if( pf->p == pf->buf_end ) return -1;
    *pf->p++ = ' ';
  }

  if( !pf->fmt.flag_minus ) {
    if( pf->p == pf->buf_end ) return -1;
    *pf->p++ = ch;
  }

  return 0;
}


//================================================================
/*! sprintf subcontract function for byte array.

  @param  pf	pointer to mrbc_printf.
  @param  str	pointer to byte array.
  @param  len	byte length.
  @param  pad	padding character.
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_bstr( mrbc_printf_t *pf, const char *str, int len, int pad )
{
  int ret = 0;

  if( str == NULL ) {
    str = "(null)";
    len = 6;
  }
  if( pf->fmt.precision && len > pf->fmt.precision ) len = pf->fmt.precision;

  int tw = len;
  if( pf->fmt.width > len ) tw = pf->fmt.width;

  int remain = pf->buf_end - pf->p;
  if( len > remain ) {
    len = remain;
    ret = -1;
  }
  if( tw > remain ) {
    tw = remain;
    ret = -1;
  }

  int n_pad = tw - len;

  if( !pf->fmt.flag_minus ) {
    while( n_pad-- > 0 ) {
      *pf->p++ = pad;
    }
  }
  while( len-- > 0 ) {
    *pf->p++ = *str++;
  }
  while( n_pad-- > 0 ) {
    *pf->p++ = pad;
  }

  return ret;
}



//================================================================
/*! sprintf subcontract function for integer '%d', '%u'

  @param  pf	pointer to mrbc_printf.
  @param  value	output value.
  @param  base	n base.
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_int( mrbc_printf_t *pf, mrbc_int value, unsigned int base )
{
  int sign = 0;
  mrbc_uint v = value;
  char *pf_p_ini_val = pf->p;

  if( value < 0 ) {
    sign = '-';
    v = -value;
  } else if( pf->fmt.flag_plus ) {
    sign = '+';
  } else if( pf->fmt.flag_space ) {
    sign = ' ';
  }

  // disable zero padding if conflict parameters exists.
  if( pf->fmt.flag_minus || pf->fmt.width == 0 || pf->fmt.precision != 0 ) {
    pf->fmt.flag_zero = 0;
  }

  // create string to temporary buffer
  char buf[sizeof(mrbc_int) * 8];
  char *p = buf + sizeof(buf);

  do {
    unsigned int ch = v % base;
    *--p = ch + ((ch < 10)? '0' : 'a' - 10);
    v /= base;
  } while( v != 0 );

  int dig_width = buf + sizeof(buf) - p;

  // write padding character, if adjust right.
  if( !pf->fmt.flag_minus && pf->fmt.width ) {
    int pad = pf->fmt.flag_zero ? '0' : ' ';
    int pad_width = pf->fmt.width - !!sign;
    pad_width -= (pf->fmt.precision > dig_width)? pf->fmt.precision: dig_width;

    for( ; pad_width > 0; pad_width-- ) {
      *pf->p++ = pad;
      if( pf->p >= pf->buf_end ) return -1;
    }
  }

  // sign
  if( sign ) {
    *pf->p++ = sign;
    if( pf->p >= pf->buf_end ) return -1;
  }

  // precision
  int pre_width = pf->fmt.precision - dig_width;
  for( ; pre_width > 0; pre_width-- ) {
    *pf->p++ = '0';
    if( pf->p >= pf->buf_end ) return -1;
  }

  // digit
  for( ; dig_width > 0; dig_width-- ) {
    *pf->p++ = *p++;
    if( pf->p >= pf->buf_end ) return -1;
  }

  // write space, if adjust left.
  if( pf->fmt.flag_minus && pf->fmt.width ) {
    int pad_width = pf->fmt.width - (pf->p - pf_p_ini_val);
    for( ; pad_width > 0; pad_width-- ) {
      *pf->p++ = ' ';
      if( pf->p >= pf->buf_end ) return -1;
    }
  }

  return 0;
}



//================================================================
/*! sprintf subcontract function for '%x', '%o', '%b'

  @param  pf	pointer to mrbc_printf.
  @param  value	output value.
  @param  bit	n bit. (4=hex, 3=oct, 1=bin)
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_bit( mrbc_printf_t *pf, mrbc_int value, int bit )
{
  if( pf->fmt.flag_plus || pf->fmt.flag_space ) {
    return mrbc_printf_int( pf, value, 1 << bit );
  }

  if( pf->fmt.flag_minus || pf->fmt.width == 0 ) {
    pf->fmt.flag_zero = 0; // disable zero padding if left align or width zero.
  }
  pf->fmt.precision = 0;

  mrbc_int v = value;
  int offset_a = (pf->fmt.type == 'X') ? 'A' - 10 : 'a' - 10;
  int mask = (1 << bit) - 1;	// 0x0f, 0x07, 0x01
  int mchar = mask + ((mask < 10)? '0' : offset_a);

  // create string to local buffer
  char buf[sizeof(mrbc_int) * 8 + 5];
  assert( sizeof(buf) > (sizeof(mrbc_int) * 8 + 4) );
  char *p = buf + sizeof(buf) - 1;
  *p = '\0';
  int n;
  do {
    assert( p >= buf );
    n = v & mask;
    *--p = n + ((n < 10)? '0' : offset_a);
    v >>= bit;
  } while( v != 0 && v != -1 );

  // add "..f" for negative value?
  // (note) '0' flag such as "%08x" is incompatible with ruby.
  if( value < 0 && !pf->fmt.flag_zero ) {
    if( n != mask ) *--p = mchar;
    *--p = '.';
    *--p = '.';
    assert( p >= buf );
  }

  // decide pad character and output sign character
  int pad;
  if( pf->fmt.flag_zero ) {
    pad = (value < 0) ? mchar : '0';
  } else {
    pad = ' ';
  }

  return mrbc_printf_str( pf, p, pad );
}



#if MRBC_USE_FLOAT
//================================================================
/*! sprintf subcontract function for float(double) '%f'

  @param  pf	pointer to mrbc_printf.
  @param  value	output value.
  @retval 0	done.
  @retval -1	buffer full.
*/
int mrbc_printf_float( mrbc_printf_t *pf, double value )
{
  char fstr[16];
  const char *p1 = pf->fstr;
  char *p2 = fstr + sizeof(fstr) - 1;

  *p2 = '\0';
  while( (*--p2 = *--p1) != '%' )
    ;

  snprintf( pf->p, (pf->buf_end - pf->p + 1), p2, value );

  while( *pf->p != '\0' )
    pf->p++;

  return -(pf->p == pf->buf_end);
}
#endif



//================================================================
/*! sprintf subcontract function for pointer '%p'

  @param  pf	pointer to mrbc_printf.
  @param  ptr	output value.
  @retval 0	done.
  @retval -1	buffer full.

  @note
    display '$00000000' style only.
    up to 8 digits, even if 64bit machines.
    not support sign, width, precision and other parameters.
*/
int mrbc_printf_pointer( mrbc_printf_t *pf, void *ptr )
{
#if defined(UINTPTR_MAX)
  uintptr_t v = (uintptr_t)ptr;
#else
  int v = (int)ptr; // regal (void* to int), but implementation defined.
#endif
  int n = sizeof(ptr) * 2;
  if( n > 8 ) n = 8;

  // check buffer size.
  if( (pf->buf_end - pf->p) < n+1 ) return -1;

  // write pointer value.
  *pf->p++ = '$';
  pf->p += n;
  char *p = pf->p - 1;

  for(; n > 0; n-- ) {
    *p-- = (v & 0xf) < 10 ? (v & 0xf) + '0' : (v & 0xf) - 10 + 'a';
    v >>= 4;
  }

  return 0;
}



//================================================================
/*! replace output buffer

  @param  pf	pointer to mrbc_printf
  @param  buf	pointer to output buffer.
  @param  size	buffer size.
*/
void mrbc_printf_replace_buffer(mrbc_printf_t *pf, char *buf, int size)
{
  int p_ofs = pf->p - pf->buf;
  pf->buf = buf;
  pf->buf_end = buf + size - 1;
  pf->p = pf->buf + p_ofs;
}



//================================================================
/*! p - sub function

  @param  v	pointer to target value.
 */
int mrbc_p_sub(const mrbc_value *v)
{
  switch( mrbc_type(*v) ){
  case MRBC_TT_NIL:
    mrbc_print("nil");
    break;

  case MRBC_TT_SYMBOL:{
    const char *s = mrbc_symbol_cstr( v );
    char *fmt = strchr(s, ':') ? "\":%s\"" : ":%s";
    mrbc_printf(fmt, s);
  } break;

#if MRBC_USE_STRING
  case MRBC_TT_STRING:{
    mrbc_putchar('"');
    const unsigned char *s = (const unsigned char *)mrbc_string_cstr(v);
    int i;
    for( i = 0; i < mrbc_string_size(v); i++ ) {
      if( s[i] < ' ' || 0x7f <= s[i] ) {	// tiny isprint()
	mrbc_printf("\\x%02X", s[i]);
      } else {
	mrbc_putchar(s[i]);
      }
    }
    mrbc_putchar('"');
  } break;
#endif

  case MRBC_TT_RANGE:{
    mrbc_value v1 = mrbc_range_first(v);
    mrbc_p_sub(&v1);
    mrbc_print( mrbc_range_exclude_end(v) ? "..." : ".." );
    v1 = mrbc_range_last(v);
    mrbc_p_sub(&v1);
  } break;

  default:
    mrbc_print_sub(v);
    break;
  }

#if 0
  // display reference counter
  if( mrbc_type(*v) >= MRBC_TT_OBJECT ) {
    mrbc_printf("(%d)", v->instance->ref_count);
  }
#endif

  return 0;
}


//================================================================
/*! print - sub function

  @param  v	pointer to target value.
  @retval 0	normal return.
  @retval 1	already output LF.
*/
int mrbc_print_sub(const mrbc_value *v)
{
  int ret = 0;

  switch( mrbc_type(*v) ){
  case MRBC_TT_EMPTY:	mrbc_print("(empty)");	break;
  case MRBC_TT_NIL:					break;
  case MRBC_TT_FALSE:	mrbc_print("false");		break;
  case MRBC_TT_TRUE:	mrbc_print("true");		break;
  case MRBC_TT_INTEGER:	mrbc_printf("%D", v->i);	break;
#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:	mrbc_printf("%g", v->d);	break;
#endif
  case MRBC_TT_SYMBOL:
    mrbc_print(mrbc_symbol_cstr(v));
    break;

  case MRBC_TT_CLASS:
    mrbc_print(mrbc_symid_to_str(v->cls->sym_id));
    break;

  case MRBC_TT_OBJECT:
    mrbc_printf("#<%s:%08x>",
	mrbc_symid_to_str( find_class_by_object(v)->sym_id ), v->instance );
    break;

  case MRBC_TT_PROC:
    mrbc_printf("#<Proc:%08x>", v->proc );
    //mrbc_printf("#<Proc:%08x, callinfo=%p>", v->proc, v->proc->callinfo );
    break;

  case MRBC_TT_ARRAY:{
    mrbc_putchar('[');
    int i;
    for( i = 0; i < mrbc_array_size(v); i++ ) {
      if( i != 0 ) mrbc_print(", ");
      mrbc_value v1 = mrbc_array_get(v, i);
      mrbc_p_sub(&v1);
    }
    mrbc_putchar(']');
  } break;

#if MRBC_USE_STRING
  case MRBC_TT_STRING:
    mrbc_nprint( mrbc_string_cstr(v), mrbc_string_size(v) );
    if( mrbc_string_size(v) != 0 &&
	mrbc_string_cstr(v)[ mrbc_string_size(v) - 1 ] == '\n' ) ret = 1;
    break;
#endif

  case MRBC_TT_RANGE:{
    mrbc_value v1 = mrbc_range_first(v);
    mrbc_print_sub(&v1);
    mrbc_print( mrbc_range_exclude_end(v) ? "..." : ".." );
    v1 = mrbc_range_last(v);
    mrbc_print_sub(&v1);
  } break;

  case MRBC_TT_HASH:{
    mrbc_putchar('{');
    mrbc_hash_iterator ite = mrbc_hash_iterator_new(v);
    while( mrbc_hash_i_has_next(&ite) ) {
      mrbc_value *vk = mrbc_hash_i_next(&ite);
      mrbc_p_sub(vk);
      mrbc_print("=>");
      mrbc_p_sub(vk+1);
      if( mrbc_hash_i_has_next(&ite) ) mrbc_print(", ");
    }
    mrbc_putchar('}');
  } break;

  case MRBC_TT_HANDLE:
    mrbc_printf("#<Handle:%08x>", v->handle );
    break;

  case MRBC_TT_EXCEPTION:
    mrbc_printf("#<%s: %s>", mrbc_symid_to_str(v->exception->cls->sym_id),
		 v->exception->message ?
		   (const char *)v->exception->message :
		   mrbc_symid_to_str(v->exception->cls->sym_id) );
    break;

  default:
    mrbc_printf("Not support MRBC_TT_XX(%d)", mrbc_type(*v));
    break;
  }

  return ret;
}


//================================================================
/*! puts - sub function

  @param  v	pointer to target value.
  @retval 0	normal return.
  @retval 1	already output LF.
*/
int mrbc_puts_sub(const mrbc_value *v)
{
  if( mrbc_type(*v) == MRBC_TT_ARRAY ) {
    int i;
    for( i = 0; i < mrbc_array_size(v); i++ ) {
      if( i != 0 ) mrbc_putchar('\n');
      mrbc_value v1 = mrbc_array_get(v, i);
      mrbc_puts_sub(&v1);
    }
    return 0;
  }

  return mrbc_print_sub(v);
}


//================================================================
/*! p - print mrbc_value

  @param  v	pointer to target value.
*/
void mrbc_p(const mrbc_value *v)
{
  mrbc_p_sub( v );
  mrbc_putchar('\n');
}
