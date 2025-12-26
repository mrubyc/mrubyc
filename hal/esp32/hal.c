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

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdio.h>
#include <string.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/gptimer.h"


/***** Local headers ********************************************************/
#include "hal.h"


/***** Constat values *******************************************************/
#ifndef MRBC_NO_TIMER
#define GPTIMER_RESOLUTION (1000*1000) // 1 MHz
#endif

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
static bool on_timer(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    mrbc_tick();
    return false;
}
//================================================================
/*!@brief
  Converting ms value to timer ticks.

*/
#define GPTIMER_FROM_MS(ms) ((ms)*GPTIMER_RESOLUTION/1000)
#endif


/***** Global functions *****************************************************/
#ifndef MRBC_NO_TIMER

//================================================================
/*!@brief
  initialize

*/
void hal_init(void)
{
  gptimer_handle_t gptimer = NULL;
  gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = GPTIMER_RESOLUTION,
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
  gptimer_alarm_config_t alarm_config = {
    .reload_count = 0,
    .alarm_count = GPTIMER_FROM_MS(MRBC_TICK_UNIT),
    .flags.auto_reload_on_alarm = true
  };
  ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
  gptimer_event_callbacks_t cbs = {
    .on_alarm = on_timer
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  ESP_ERROR_CHECK(gptimer_start(gptimer));
}


//================================================================
/*!@brief
  enable interrupt

*/
void hal_enable_irq(void)
{
  portEXIT_CRITICAL(&mux);
}


//================================================================
/*!@brief
  disable interrupt

*/
void hal_disable_irq(void)
{
  portENTER_CRITICAL(&mux);
}


#endif /* ifndef MRBC_NO_TIMER */


//================================================================
/*!@brief
  abort program

  @param s	additional message.
*/
void hal_abort(const char *s)
{
  if( s ) {
    write(1, s, strlen(s));
  }

  abort();
}
