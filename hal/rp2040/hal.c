/*! @file
  @brief
  Hardware abstraction layer
        for RP2040

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***** Local headers ********************************************************/
#include "hal.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
struct repeating_timer timer;

/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/
#ifndef MRBC_NO_TIMER

//================================================================
/*!@brief
  timer alarm irq

*/
bool alarm_irq(struct repeating_timer *t) {
  mrbc_tick();
}

//================================================================
/*!@brief
  initialize

*/
void mrbc_hal_init(void){
  add_repeating_timer_ms(1, alarm_irq, NULL, &timer);
  clocks_hw->sleep_en0 = 0;
  clocks_hw->sleep_en1 = CLOCKS_SLEEP_EN1_CLK_SYS_TIMER_BITS
                        | CLOCKS_SLEEP_EN1_CLK_SYS_USBCTRL_BITS
                        | CLOCKS_SLEEP_EN1_CLK_USB_USBCTRL_BITS
                        | CLOCKS_SLEEP_EN1_CLK_SYS_UART0_BITS
                        | CLOCKS_SLEEP_EN1_CLK_PERI_UART0_BITS;
}

#endif /* ifndef MRBC_NO_TIMER */


//================================================================
/*!@brief
  Write

  @param  fd    dummy.
  @param  buf   pointer of buffer.
  @param  nbytes        output byte length.

  Memo: Steps to use uart_putc_raw() with mrbc_hal_write.
  1. Write in main function↓
    uart_init(uart0,115200);
    gpio_set_function(0,GPIO_FUNC_UART);
    gpio_set_function(1,GPIO_FUNC_UART);

  2. Comment out the putchar for mrbc_hal_write.
  3. Uncomment uart_putc_raw for mrbc_hal_write.
*/
int mrbc_hal_write(int fd, const void *buf, int nbytes)
{
  int i = nbytes;
  const uint8_t *p = buf;

  while( --i >= 0 ) {
    putchar( *p++ );
    // uart_putc_raw(uart0, *p++ );
  }

  return nbytes;
}

//================================================================
/*!@brief
  Flush write buffer

  @param  fd    dummy.
*/
int mrbc_hal_flush(int fd)
{
  return 0;
}

//================================================================
/*!@brief
  abort program

  @param s	additional message.
*/
void mrbc_hal_abort(const char *s)
{
  if( s ) {
    mrbc_hal_write(1, s, strlen(s));
  }

  abort();
}
