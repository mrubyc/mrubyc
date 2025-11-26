/*! @file
  @brief
  Hardware abstraction layer
        for POSIX

  <pre>
  Copyright (C) 2015- Kyushu Institute of Technology.
  Copyright (C) 2015- Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>


/***** Local headers ********************************************************/
#include "hal.h"


/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
#if !defined(MRBC_NO_TIMER)
static sigset_t sigset_, sigset2_;
#endif


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
#if !defined(MRBC_NO_TIMER)
//================================================================
/*!@brief
  alarm signal handler

*/
static void sig_alarm(int dummy)
{
  mrbc_tick();
}

#endif


/***** Local functions ******************************************************/
/***** Global functions *****************************************************/
#if !defined(MRBC_NO_TIMER)

//================================================================
/*!@brief
  initialize

*/
void hal_init(void)
{
  sigemptyset(&sigset_);
  sigaddset(&sigset_, SIGALRM);

  // prepare signal handler for timer.
  struct sigaction sa;
  sa.sa_handler = sig_alarm;
  sa.sa_flags   = SA_RESTART;
  sa.sa_mask    = sigset_;
  sigaction(SIGALRM, &sa, 0);

#if 1
  /*
    For compatibility, use the setitimer function.
  */
  // start the timer.
  struct itimerval tval;
  int sec  = 0;
  int usec = MRBC_TICK_UNIT * 1000;
  tval.it_interval.tv_sec  = sec;
  tval.it_interval.tv_usec = usec;
  tval.it_value.tv_sec     = sec;
  tval.it_value.tv_usec    = usec;
  setitimer(ITIMER_REAL, &tval, 0);

#else
  /*
    Uses modern timer_* functions.
  */
  // create a timer using a signal (SIGALRM)
  timer_t timer_id;
  struct sigevent ev = {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo  = SIGALRM,
    .sigev_value = {.sival_ptr = &timer_id}};

  if( timer_create(CLOCK_REALTIME, &ev, &timer_id) != 0 ) {
    perror("timer_create");
    exit(1);
  }

  // start the timer.
  struct itimerspec ts = {
    .it_interval = {.tv_sec = 0, .tv_nsec = MRBC_TICK_UNIT * 1000000},
    .it_value    = {.tv_sec = 0, .tv_nsec = MRBC_TICK_UNIT * 1000000}};

  if( timer_settime(timer_id, 0, &ts, NULL) != 0 ) {
    perror("timer_settime");
    exit(1);
  }
#endif
}


//================================================================
/*!@brief
  enable interrupt

*/
void hal_enable_irq(void)
{
  sigprocmask(SIG_SETMASK, &sigset2_, 0);
}


//================================================================
/*!@brief
  disable interrupt

*/
void hal_disable_irq(void)
{
  sigprocmask(SIG_BLOCK, &sigset_, &sigset2_);
}

#endif /* if !defined(MRBC_NO_TIMER) */



//================================================================
/*!@brief
  abort program

*/
void hal_abort( const char *s )
{
  if( s ) {
    hal_write(1, s, strlen(s));
  }
  exit( 1 );
}


//================================================================
/*!@brief
  out of memory

*/
void mrbc_out_of_memory_posix( void )
{
  static const char msg[] = "Fatal error: Out of memory.\n";
  hal_write(2, msg, sizeof(msg)-1);

#if 0
  void mrbc_alloc_print_memory_pool(void);
  mrbc_alloc_print_memory_pool();
#endif

  hal_abort(0);
}
