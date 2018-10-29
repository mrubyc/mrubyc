/*! @file
  @brief
  Hardware abstraction layer
        for ESP32

  <pre>
  Copyright (C) 2016-2018 Kyushu Institute of Technology.
  Copyright (C) 2016-2018 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"


/***** Local headers ********************************************************/
#include "hal.h"


/***** Constat values *******************************************************/
#define TIMER_DIVIDER 80

/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
#ifndef MRBC_NO_TIMER
//================================================================
/*!@brief
  Timer ISR function

*/
static void on_timer(void *arg)
{
    TIMERG0.int_clr_timers.t0 = 1;
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
    mrbc_tick();
}


#endif


/***** Global functions *****************************************************/
#ifndef MRBC_NO_TIMER

//================================================================
/*!@brief
  initialize

*/
void hal_init(void)
{
  timer_config_t config;

  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = TIMER_AUTORELOAD_EN;

  timer_init(TIMER_GROUP_0, TIMER_0, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000); /* 1ms */
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, on_timer, NULL, 0, NULL);

  timer_start(TIMER_GROUP_0, TIMER_0);
}


//================================================================
/*!@brief
  enable interrupt

*/
void hal_enable_irq(void)
{
  taskEXIT_CRITICAL(&mux);
}


//================================================================
/*!@brief
  disable interrupt

*/
void hal_disable_irq(void)
{
  taskENTER_CRITICAL(&mux);
}


#endif /* ifndef MRBC_NO_TIMER */
