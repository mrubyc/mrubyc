/*! @file
  @brief
  Hardware abstraction layer
        for PSoC5LP

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
/***** System headers *******************************************************/
#include <project.h>


/***** Local headers ********************************************************/
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
#if !defined(MRBC_TICK_UNIT)
#define MRBC_TICK_UNIT_1_MS   1
#define MRBC_TICK_UNIT_2_MS   2
#define MRBC_TICK_UNIT_4_MS   4
#define MRBC_TICK_UNIT_10_MS 10
// You may be able to reduce power consumption if you configure
// MRBC_TICK_UNIT_2_MS or larger.
#define MRBC_TICK_UNIT MRBC_TICK_UNIT_1_MS
// Substantial timeslice value (millisecond) will be
// MRBC_TICK_UNIT * MRBC_TIMESLICE_TICK_COUNT (+ Jitter).
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

#if !defined(MRBC_NO_TIMER)
# define mrbc_hal_init()        ((void)0)
# define mrbc_hal_enable_irq()  CyGlobalIntEnable
# define mrbc_hal_disable_irq() CyGlobalIntDisable
# define mrbc_hal_idle_cpu()    CyPmAltAct(PM_SLEEP_TIME_NONE, \
                                      PM_SLEEP_SRC_CTW | PM_SLEEP_SRC_PICU)

#else // MRBC_NO_TIMER
# define mrbc_hal_init()        ((void)0)
# define mrbc_hal_enable_irq()  ((void)0)
# define mrbc_hal_disable_irq() ((void)0)
# define mrbc_hal_idle_cpu()    (CyDelay(MRBC_TICK_UNIT), mrbc_tick())

#endif

int mrbc_hal_write(int fd, const void *buf, int nbytes);
int mrbc_hal_flush(int fd);
void mrbc_hal_abort(const char *s);


/***** Inline functions *****************************************************/

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
#endif // ifndef MRBC_HAL_H_
