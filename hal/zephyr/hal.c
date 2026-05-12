/*! @file
  @brief
  Hardware abstraction layer
        for Zephyr
        Board examples: Nordic nRF52/nRF54/nRF91, ST Nucleo/Discovery,
        Espressif ESP32 DevKit, NXP FRDM/i.MX RT EVK, Raspberry Pi Pico,
        SiFive HiFive, Microchip SAM/PIC32, Renesas RA/RZ, Silicon Labs EFR32,
        TI LaunchPad, and QEMU boards.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdint.h>
#include <string.h>
#include <zephyr/fatal.h>
#include <zephyr/irq.h>
#include <zephyr/sys/printk.h>

/***** Local headers ********************************************************/
#include "hal.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
static unsigned int hal_irq_lock_key;
static uint32_t hal_irq_lock_count;
static uint8_t hal_sched_locked;

/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
#ifndef MRBC_NO_TIMER
//================================================================
/*!@brief
  Timer handler

*/
static void mrubyc_haltimerhandler(struct k_timer *timer)
{
  (void)timer;
  mrbc_tick();
}

K_TIMER_DEFINE(mrubyc_haltimer, mrubyc_haltimerhandler, NULL);

/***** Global functions *****************************************************/
//================================================================
/*!@brief
  initialize

*/
void mrbc_hal_init(void)
{
  k_timer_start(&mrubyc_haltimer, K_MSEC(MRBC_TICK_UNIT), K_MSEC(MRBC_TICK_UNIT));
}
#else
/***** Global functions *****************************************************/
#endif

//================================================================
/*!@brief
  enable interrupt

*/
void mrbc_hal_enable_irq(void)
{
  if( hal_irq_lock_count == 0 ) return;

  hal_irq_lock_count--;
  if( hal_irq_lock_count == 0 ) {
    unsigned int key = hal_irq_lock_key;
    uint8_t sched_locked = hal_sched_locked;

    hal_irq_lock_key = 0;
    hal_sched_locked = 0;
    irq_unlock(key);
    if( sched_locked ) k_sched_unlock();
  }
}

//================================================================
/*!@brief
  disable interrupt

*/
void mrbc_hal_disable_irq(void)
{
  if( hal_irq_lock_count == 0 ) {
    if( !k_is_in_isr() ) {
      k_sched_lock();
      hal_sched_locked = 1;
    } else {
      hal_sched_locked = 0;
    }
    hal_irq_lock_key = irq_lock();
  }

  hal_irq_lock_count++;
}

//================================================================
/*!@brief
  Write

  @param  fd    dummy.
  @param  buf   pointer of buffer.
  @param  nbytes        output byte length.
*/
int mrbc_hal_write(int fd, const void *buf, int nbytes)
{
  (void)fd;

  if( nbytes < 0 ) return -1;
  if( nbytes == 0 ) return 0;
  if( buf == NULL ) return -1;

  const char *p = (const char *)buf;
  for( int i = 0; i < nbytes; i++ ) {
    printk("%c", (int)(unsigned char)p[i]);
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
  (void)fd;
  return 0;
}

//================================================================
/*!@brief
  abort program

  @param s      additional message.
*/
void mrbc_hal_abort(const char *s)
{
  if( s ) {
    mrbc_hal_write(2, s, strlen(s));
  }

  k_panic();
  k_fatal_halt(K_ERR_KERNEL_PANIC);
}
