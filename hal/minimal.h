/*! @file
  @brief
  Hardware abstraction layer minimum set sample.

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

#define hal_init()        ((void)0)
#define hal_enable_irq()  ((void)0)
#define hal_disable_irq() ((void)0)
#define hal_idle_cpu()    (delay(MRBC_TICK_UNIT), mrbc_tick())  // delay 1ms

int hal_write(int fd, const void *buf, int nbytes);
int hal_flush(int fd);
void hal_abort(const char *s);

/*
  or define the empty macros instead of function.
#define hal_write(fd,buf,nbytes)  ((void)0)
#define hal_flush(fd)             ((void)0)
#define hal_abort(s)              ((void)0)
*/

#endif
