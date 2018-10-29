/*! @file
  @brief
  Hardware abstraction layer
        for PSoC5LP

  <pre>
  Copyright (C) 2016 Kyushu Institute of Technology.
  Copyright (C) 2016 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

#ifndef MRBC_SRC_HAL_H_
#define MRBC_SRC_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <project.h>


/***** Local headers ********************************************************/
/***** Constant values ******************************************************/
/***** Macros ***************************************************************/

#ifndef MRBC_NO_TIMER
# define hal_init()        ((void)0)
# define hal_enable_irq()  CyGlobalIntEnable
# define hal_disable_irq() CyGlobalIntDisable
# define hal_idle_cpu()    CyPmAltAct(PM_SLEEP_TIME_NONE, \
                                      PM_SLEEP_SRC_CTW | PM_SLEEP_SRC_PICU)

#else // MRBC_NO_TIMER
# define hal_init()        ((void)0)
# define hal_enable_irq()  ((void)0)
# define hal_disable_irq() ((void)0)
# define hal_idle_cpu()    (CyDelay(1), mrbc_tick())

#endif


/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
int hal_write(int fd, const void *buf, int nbytes);
int hal_flush(int fd);


/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif // ifndef MRBC_HAL_H_
