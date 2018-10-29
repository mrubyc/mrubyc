/*! @file
  @brief
  console output module. (not yet input)

  <pre>
  Copyright (C) 2015-2018 Kyushu Institute of Technology.
  Copyright (C) 2015-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#include "vm_config.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#if MRBC_USE_FLOAT
#include <stdio.h>
#endif
#include "value.h"
#include "console.h"



//================================================================
/*! output formatted string

  @param  fstr		format string.
*/
void console_printf(const char *fstr, ...)
{
  va_list ap;
  va_start(ap, fstr);

  mrbc_printf pf;
  char buf[82];
  mrbc_printf_init( &pf, buf, sizeof(buf), fstr );

  int ret;
  while( 1 ) {
    ret = mrbc_printf_main( &pf );
    if( mrbc_printf_len( &pf ) ) {
      hal_write(1, buf, mrbc_printf_len( &pf ));
      mrbc_printf_clear( &pf );
    }
    if( ret == 0 ) break;
    if( ret < 0 ) continue;
    if( ret > 0 ) {
      switch(pf.fmt.type) {
      case 'c':
	ret = mrbc_printf_char( &pf, va_arg(ap, int) );
	break;

      case 's':
	ret = mrbc_printf_str( &pf, va_arg(ap, char *), ' ');
	break;

      case 'd':
      case 'i':
      case 'u':
	ret = mrbc_printf_int( &pf, va_arg(ap, unsigned int), 10);
	break;

      case 'b':
      case 'B':
	ret = mrbc_printf_int( &pf, va_arg(ap, unsigned int), 2);
	break;

      case 'x':
      case 'X':
	ret = mrbc_printf_int( &pf, va_arg(ap, unsigned int), 16);
	break;

#if MRBC_USE_FLOAT
      case 'f':
      case 'e':
      case 'E':
      case 'g':
      case 'G':
	ret = mrbc_printf_float( &pf, va_arg(ap, double) );
	break;
#endif

      default:
	break;
      }

      hal_write(1, buf, mrbc_printf_len( &pf ));
      mrbc_printf_clear( &pf );
    }
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
int mrbc_printf_main( mrbc_printf *pf )
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
  while( (ch = *pf->fstr - '0'), (0 <= ch && ch <= 9)) {	// isdigit()
    pf->fmt.width = pf->fmt.width * 10 + ch;
    pf->fstr++;
  }
  if( *pf->fstr == '.' ) {
    pf->fstr++;
    while( (ch = *pf->fstr - '0'), (0 <= ch && ch <= 9)) {
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
int mrbc_printf_char( mrbc_printf *pf, int ch )
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
int mrbc_printf_bstr( mrbc_printf *pf, const char *str, int len, int pad )
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
/*! sprintf subcontract function for integer '%d' '%x' '%b'

  @param  pf	pointer to mrbc_printf.
  @param  value	output value.
  @param  base	n base.
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
int mrbc_printf_int( mrbc_printf *pf, mrbc_int value, int base )
{
  int sign = 0;
  uint32_t v = value;	// (note) Change this when supporting 64 bit.

  if( pf->fmt.type == 'd' || pf->fmt.type == 'i' ) {	// signed.
    if( value < 0 ) {
      sign = '-';
      v = -value;
    } else if( pf->fmt.flag_plus ) {
      sign = '+';
    } else if( pf->fmt.flag_space ) {
      sign = ' ';
    }
  }

  if( pf->fmt.flag_minus || pf->fmt.width == 0 ) {
    pf->fmt.flag_zero = 0; // disable zero padding if left align or width zero.
  }
  pf->fmt.precision = 0;

  int bias_a = (pf->fmt.type == 'X') ? 'A' - 10 : 'a' - 10;

  // create string to local buffer
  char buf[32+2];	// int32 + terminate + 1
  char *p = buf + sizeof(buf) - 1;
  *p = '\0';
  do {
    int i = v % base;
    *--p = (i < 10)? i + '0' : i + bias_a;
    v /= base;
  } while( v != 0 );

  // decide pad character and output sign character
  int pad;
  if( pf->fmt.flag_zero ) {
    pad = '0';
    if( sign ) {
      *pf->p++ = sign;
      if( pf->p >= pf->buf_end ) return -1;
      pf->fmt.width--;
    }
  } else {
    pad = ' ';
    if( sign ) *--p = sign;
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
int mrbc_printf_float( mrbc_printf *pf, double value )
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
/*! replace output buffer

  @param  pf	pointer to mrbc_printf
  @param  buf	pointer to output buffer.
  @param  size	buffer size.
*/
void mrbc_printf_replace_buffer(mrbc_printf *pf, char *buf, int size)
{
  int p_ofs = pf->p - pf->buf;
  pf->buf = buf;
  pf->buf_end = buf + size - 1;
  pf->p = pf->buf + p_ofs;
}
