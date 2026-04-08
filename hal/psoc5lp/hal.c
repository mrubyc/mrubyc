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

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <project.h>

/***** Local headers ********************************************************/
#include "hal.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

#if 0
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
  UART_1_PutArray( buf, nbytes );
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
  if( s ) {
    UART_1_PutString(s);
  }

  // Select one
  CySoftwareReset();
  // or
  CyHalt(0);
}
#endif
