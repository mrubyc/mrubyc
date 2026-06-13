/*! @file
  @brief
  Hardware abstraction layer
        for Microchip AVR using XC8 Compiler

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_HAL_H_
#define MRBC_SRC_HAL_H_

/***** Feature test switches ************************************************/
#if !defined(MRBC_INT16)
#define MRBC_INT16
#endif

#if defined(MRBC_ALLOC_24BIT)
#undef MRBC_ALLOC_24BIT
#endif

#if !defined(MRBC_ALLOC_16BIT)
#define MRBC_ALLOC_16BIT
#endif

/***** System headers *******************************************************/
// F_CPU is used by <util/delay.h>. Define the actual CPU frequency in
// the build settings. This value is only a fallback.
// Common values are 16000000UL for ATmega128 and 24000000UL for AVR128Dx devices.
#ifndef F_CPU
#define F_CPU 24000000UL
#endif

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

/***** Local headers ********************************************************/
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
#if !defined(MRBC_TICK_UNIT_1_MS)
#define MRBC_TICK_UNIT_1_MS   1
#endif
#if !defined(MRBC_TICK_UNIT_2_MS)
#define MRBC_TICK_UNIT_2_MS   2
#endif
#if !defined(MRBC_TICK_UNIT_4_MS)
#define MRBC_TICK_UNIT_4_MS   4
#endif
#if !defined(MRBC_TICK_UNIT_10_MS)
#define MRBC_TICK_UNIT_10_MS 10
#endif

#if !defined(MRBC_TICK_UNIT)
#define MRBC_TICK_UNIT MRBC_TICK_UNIT_1_MS
#endif

#if !defined(MRBC_TIMESLICE_TICK_COUNT)
// Substantial timeslice value (millisecond) will be
// MRBC_TICK_UNIT * MRBC_TIMESLICE_TICK_COUNT (+ timer jitter).
// MRBC_TIMESLICE_TICK_COUNT must be natural number
// (recommended value is from 1 to 10).
#define MRBC_TIMESLICE_TICK_COUNT 10
#endif

/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void mrbc_tick(void);

#if !defined(MRBC_NO_TIMER)
# define mrbc_hal_init()        ((void)0)
# define mrbc_hal_enable_irq()  sei()
# define mrbc_hal_disable_irq() cli()
# define mrbc_hal_idle_cpu()    do { set_sleep_mode(SLEEP_MODE_IDLE); sleep_mode(); } while(0)

#else // MRBC_NO_TIMER
# define mrbc_hal_init()        ((void)0)
# define mrbc_hal_enable_irq()  ((void)0)
# define mrbc_hal_disable_irq() ((void)0)
# define mrbc_hal_idle_cpu()    (_delay_ms(MRBC_TICK_UNIT), mrbc_tick())
#endif

/***** Inline functions *****************************************************/
//================================================================
/*!@brief
  Write

  @param  fd      unused.
  @param  buf     unused.
  @param  nbytes  output byte length.
  @return nbytes on success, or -1 if nbytes is negative.
*/
inline static int mrbc_hal_write(int fd, const void *buf, int nbytes)
{
  (void)fd;
  (void)buf;

  if( nbytes < 0 ) {
    return -1;
  }

  return nbytes;
}


//================================================================
/*!@brief
  Flush write buffer

  @param  fd  unused.
  @return 0, because the default AVR HAL has no buffered output device.
*/
inline static int mrbc_hal_flush(int fd)
{
  (void)fd;

  return 0;
}


//================================================================
/*!@brief
  abort program

  @param s  additional message. It is ignored by this no-console
            implementation.
*/
inline static void mrbc_hal_abort(const char *s)
{
  (void)s;

  cli();
  while( 1 ) {
    __asm__ __volatile__("nop");
  }
}

/*
  for legacy compatibility.
*/
#define hal_init()               mrbc_hal_init()
#define hal_enable_irq()         mrbc_hal_enable_irq()
#define hal_disable_irq()        mrbc_hal_disable_irq()
#define hal_idle_cpu()           mrbc_hal_idle_cpu()
#define hal_write(fd,buf,nbytes) mrbc_hal_write(fd,buf,nbytes)
#define hal_flush(fd)            mrbc_hal_flush(fd)
#define hal_abort(s)             mrbc_hal_abort(s)

#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_SRC_HAL_H_
