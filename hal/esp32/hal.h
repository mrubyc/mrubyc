/*! @file
  @brief
  Hardware abstraction layer
        for ESP32

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_HAL_H_
#define MRBC_SRC_HAL_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/***** Local headers ********************************************************/
#include "vm_config.h"

/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
#ifndef MRBC_SCHEDULER_EXIT
#define MRBC_SCHEDULER_EXIT 1
#endif

#if !defined(MRBC_TICK_UNIT)
#define MRBC_TICK_UNIT_1_MS   1
#define MRBC_TICK_UNIT_2_MS   2
#define MRBC_TICK_UNIT_4_MS   4
#define MRBC_TICK_UNIT_10_MS 10
// portTICK_PERIOD_MS is 10 in ESP-IDF by default.
// You can configure MRBC_TICK_UNIT less than 10 ms
// under the understanding of FreeRTOS's tick.
#define MRBC_TICK_UNIT MRBC_TICK_UNIT_10_MS
// Substantial timeslice value (millisecond) will be
// MRBC_TICK_UNIT * MRBC_TIMESLICE_TICK_COUNT (+ Jitter).
// MRBC_TIMESLICE_TICK_COUNT must be natural number
// (recommended value is from 1 to 10).
#define MRBC_TIMESLICE_TICK_COUNT 1
#endif


/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void mrbc_tick(void);

#if !defined(MRBC_NO_TIMER)
void mrbc_hal_init(void);
void mrbc_hal_enable_irq(void);
void mrbc_hal_disable_irq(void);
                           // Note: argument of vTaskDelay() should be 1+
# define mrbc_hal_idle_cpu()    vTaskDelay(MRBC_TICK_UNIT / portTICK_PERIOD_MS)

#else // MRBC_NO_TIMER
# define mrbc_hal_init()        ((void)0)
# define mrbc_hal_enable_irq()  ((void)0)
# define mrbc_hal_disable_irq() ((void)0)
                           // Note: argument of vTaskDelay() should be 1+
# define mrbc_hal_idle_cpu()    (vTaskDelay(MRBC_TICK_UNIT / portTICK_PERIOD_MS), \
                             mrbc_tick())

#endif

void mrbc_hal_abort(const char *s);


/***** Inline functions *****************************************************/

//================================================================
/*!@brief
  Write

  @param  fd    dummy, but 1.
  @param  buf   pointer of buffer.
  @param  nbytes        output byte length.
*/
inline static int mrbc_hal_write(int fd, const void *buf, int nbytes)
{
  return write(fd, buf, nbytes);
}

//================================================================
/*!@brief
  Flush write buffer

  @param  fd    dummy, but 1.
*/
inline static int mrbc_hal_flush(int fd)
{
  return fsync(1);
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
#endif // ifndef MRBC_HAL_H_
