/*! @file
  @brief
  Hardware abstraction layer for STM32 using STM HAL library.

  <pre>
  Copyright (C) 2015-      Kyushu Institute of Technology.
  Copyright (C) 2015-2026  Shimane IT Open-Innovation Center.
  Copyright (C) 2026-      Shimane Institute for Industrial Technology.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/


#if !defined(UART_HANDLETYPEDEF_CONSOLE)
#define UART_HANDLETYPEDEF_CONSOLE huart2
#endif


// This is one sample implementation.
//================================================================
/*!@brief
  Write

  @param  fd		dummy, but 1.
  @param  buf		pointer to buffer.
  @param  nbytes	output byte length.
*/
int hal_write(int fd, const void *buf, int nbytes)
{
  extern UART_HandleTypeDef UART_HANDLETYPEDEF_CONSOLE;
  HAL_UART_Transmit(&UART_HANDLETYPEDEF_CONSOLE, buf, nbytes, HAL_MAX_DELAY);

  return nbytes;
}


//================================================================
/*!@brief
  Flush write buffer

  @param  fd	dummy, but 1.
*/
int hal_flush(int fd)
{
  return 0;
}

//================================================================
/*!@brief
  abort program

  @param s	additional message.
*/
void hal_abort(const char *s)
{
  hal_write(2, s, strlen(s));

  HAL_Delay(10000);
  HAL_NVIC_SystemReset();
}
