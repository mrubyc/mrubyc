/*! @file
  @brief
  Hardware abstraction layer
        for Emscripten/WebAssembly

  @note Link applications with -sASYNCIFY because this HAL uses
        emscripten_sleep() to yield while mrbc_run() is idle.

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
#include <emscripten.h>

/***** Local headers ********************************************************/
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/
#ifndef MRBC_NO_TIMER
#define MRBC_NO_TIMER
#endif

#ifndef MRBC_SCHEDULER_EXIT
#define MRBC_SCHEDULER_EXIT 1
#endif

#ifndef MRBC_TICK_UNIT_1_MS
#define MRBC_TICK_UNIT_1_MS   1
#endif
#ifndef MRBC_TICK_UNIT_2_MS
#define MRBC_TICK_UNIT_2_MS   2
#endif
#ifndef MRBC_TICK_UNIT_4_MS
#define MRBC_TICK_UNIT_4_MS   4
#endif
#ifndef MRBC_TICK_UNIT_10_MS
#define MRBC_TICK_UNIT_10_MS 10
#endif

#ifndef MRBC_TICK_UNIT
#define MRBC_TICK_UNIT MRBC_TICK_UNIT_10_MS
#endif

#ifndef MRBC_TIMESLICE_TICK_COUNT
#define MRBC_TIMESLICE_TICK_COUNT 10
#endif

#define MRBC_OUT_OF_MEMORY() mrbc_out_of_memory_emscripten()

/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void mrbc_tick(void);

#define mrbc_hal_init()        ((void)0)
#define mrbc_hal_enable_irq()  ((void)0)
#define mrbc_hal_disable_irq() ((void)0)
void mrbc_hal_delay_ms(int ms);
#define mrbc_hal_idle_cpu()    (mrbc_hal_delay_ms(MRBC_TICK_UNIT), mrbc_tick())

int mrbc_hal_write(int fd, const void *buf, int nbytes);
int mrbc_hal_flush(int fd);
void mrbc_hal_abort(const char *s);
void mrbc_out_of_memory_emscripten(void);

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
