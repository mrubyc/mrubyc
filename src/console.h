/*! @file
  @brief
  console output module. (not yet input)

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_CONSOLE_H_
#define MRBC_SRC_CONSOLE_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "vm_config.h"
#include <types.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

/***** Local headers ********************************************************/
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
// For compatibility. Not recommended for use.
#define mrb_p(vm, v)	mrbc_p(&v)
#define console_print mrbc_print
#define console_nprint mrbc_nprint
#define console_putchar mrbc_putchar
#define console_printf mrbc_printf


/***** Typedefs *************************************************************/
//================================================================
/*! printf tiny (mruby/c) version data container.
*/
struct RPrintfFormat {
  char type;			//!< format char. (e.g. 'd','f','x'...)
  unsigned int flag_plus : 1;
  unsigned int flag_minus : 1;
  unsigned int flag_space : 1;
  unsigned int flag_zero : 1;
  int16_t width;		//!< display width. (e.g. %10d as 10)
  int16_t precision;		//!< precision (e.g. %5.2f as 2)
};
typedef struct RPrintf {
  char *buf;			//!< output buffer.
  const char *buf_end;		//!< output buffer end point.
  char *p;			//!< output buffer write point.
  const char *fstr;		//!< format string. (e.g. "%d %03x")
  struct RPrintfFormat fmt;
} mrbc_printf_t;


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
void mrbc_putchar(char c);
void mrbc_nprint(const char *str, int size);
void mrbc_snprintf(char *buf, int bufsiz, const char *fstr, ...);
void mrbc_printf(const char *fstr, ...);
int mrbc_printf_main(mrbc_printf_t *pf);
int mrbc_printf_char(mrbc_printf_t *pf, int ch);
int mrbc_printf_bstr(mrbc_printf_t *pf, const char *str, int len, int pad);
int mrbc_printf_int(mrbc_printf_t *pf, mrbc_int value, unsigned int base);
int mrbc_printf_bit(mrbc_printf_t *pf, mrbc_int value, int bit);
int mrbc_printf_float(mrbc_printf_t *pf, double value);
int mrbc_printf_pointer(mrbc_printf_t *pf, void *ptr);
void mrbc_printf_replace_buffer(mrbc_printf_t *pf, char *buf, int size);
int mrbc_p_sub(const mrbc_value *v);
int mrbc_print_sub(const mrbc_value *v);
int mrbc_puts_sub(const mrbc_value *v);
void mrbc_p(const mrbc_value *v);


/***** Inline functions *****************************************************/

//================================================================
/*! output string

  @param str	str
*/
static inline void mrbc_print(const char *str)
{
  mrbc_nprint( str, strlen(str) );
}


//================================================================
/*! initialize data container.

  @param  pf	pointer to mrbc_printf
  @param  buf	pointer to output buffer.
  @param  size	buffer size.
  @param  fstr	format string.
*/
static inline void mrbc_printf_init( mrbc_printf_t *pf, char *buf, int size,
				     const char *fstr )
{
  pf->p = pf->buf = buf;
  pf->buf_end = buf + size - 1;
  pf->fstr = fstr;
  pf->fmt = (struct RPrintfFormat){0};
}


//================================================================
/*! clear output buffer in container.

  @param  pf	pointer to mrbc_printf
*/
static inline void mrbc_printf_clear( mrbc_printf_t *pf )
{
  pf->p = pf->buf;
}


//================================================================
/*! terminate ('\0') output buffer.

  @param  pf	pointer to mrbc_printf
*/
static inline void mrbc_printf_end( mrbc_printf_t *pf )
{
  *pf->p = '\0';
}


//================================================================
/*! return string length in buffer

  @param  pf	pointer to mrbc_printf
  @return	length
*/
static inline int mrbc_printf_len( mrbc_printf_t *pf )
{
  return pf->p - pf->buf;
}


//================================================================
/*! sprintf subcontract function for char '%s'

  @param  pf	pointer to mrbc_printf.
  @param  str	output string.
  @param  pad	padding character.
  @retval 0	done.
  @retval -1	buffer full.
  @note		not terminate ('\0') buffer tail.
*/
static inline int mrbc_printf_str( mrbc_printf_t *pf, const char *str, int pad )
{
  return mrbc_printf_bstr( pf, str, strlen(str), pad );
}

#ifdef __cplusplus
}
#endif
#endif
