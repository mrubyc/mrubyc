/*! @file
  @brief
  Hardware abstraction layer minimum set sample.

  Copy this file as hal.h for use.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_HAL_H_
#define MRBC_SRC_HAL_H_

#define MRBC_TICK_UNIT 1
#define MRBC_TIMESLICE_TICK_COUNT 10

#ifdef __cplusplus
extern "C" {
#endif

void mrbc_tick(void);


/*
  If you use the rrt0 scheduler, implement the following four macros.
*/
#define mrbc_hal_init()        ((void)0)
#define mrbc_hal_enable_irq()  ((void)0)
#define mrbc_hal_disable_irq() ((void)0)
#define mrbc_hal_idle_cpu()    (delay(MRBC_TICK_UNIT), mrbc_tick())  // delay 1ms

/*
  Implement the following three functions:
*/
int mrbc_hal_write(int fd, const void *buf, int nbytes);
int mrbc_hal_flush(int fd);
void mrbc_hal_abort(const char *s);

/*
  or define the empty macros instead of function.

#define mrbc_hal_write(fd,buf,nbytes)  ((void)0)
#define mrbc_hal_flush(fd)             ((void)0)
#define mrbc_hal_abort(s)              ((void)0)
*/

#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_HAL_H_
