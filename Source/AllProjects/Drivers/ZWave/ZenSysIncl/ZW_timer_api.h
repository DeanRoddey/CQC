/*******************************  ZW_TIMER_API.H  ***************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Z-Wave, the wireless lauguage.
 *
 *              Copyright (c) 2001
 *              Zensys A/S
 *              Denmark
 *
 *              All Rights Reserved
 *
 *    This source file is subject to the terms and conditions of the
 *    Zensys Software License Agreement which restricts the manner
 *    in which it may be used.
 *
 *---------------------------------------------------------------------------
 *
 * Description: Timer service functions that handle delayed functions calls.
 *              The time base is around 10 msec.
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.13 $
 * Last Changed:     $Date: 2007/01/24 09:25:46 $
 *
 ****************************************************************************/
#ifndef _ZW_TIMER_API_H_
#define _ZW_TIMER_API_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/


/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
/* One second timeout */
#define TIMER_ONE_SECOND   100

/* One time timer */
#define TIMER_ONE_TIME     0
#define TIMER_FOREVER      (BYTE)-1

/******************************  Timer **************************************
** Timer service functions that handle delayed functions calls.
** The time base is around 10 msec.
**/

/*================================   TimerStart   ============================
**    Register a function that is called when the specified time has elapsed.
**    The function is called "repeats" times before the timer is stopped.
**
** BYTE              RET Timer handle ( 1 to XX if success, 0xFF if failed)
** TimerStart(
** void (*func)(),   IN  Timeout function adddress
** BYTE timerTicks,  IN  Timeout value (value * 10 msec.)
** BYTE repeats);    IN  Number of function calls (-1: forever)
**--------------------------------------------------------------------------*/
#define ZW_TIMER_START(func,ticks,repeats) TimerStart(func,ticks,repeats)

/*================================   TimerRestart  ===========================
**    Set the specified timer back to the initial value.
**
** BYTE               RET TRUE if timer restarted
** TimerRestart(
** BYTE timerHandle); IN  Timer number to restart
**--------------------------------------------------------------------------*/
#define ZW_TIMER_RESTART(handle) TimerRestart(handle)

/*================================   TimerCancel   ===========================
**    Stop the specified timer.
**      0 and 0xFF indicates no timer running.. This is acceptable
** BYTE               RET TRUE if timer cancelled
** TimerCancel(
** BYTE timerHandle); IN  Timer number to stop
**--------------------------------------------------------------------------*/
#define ZW_TIMER_CANCEL(handle) TimerCancel(handle)


/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

#ifdef ZW_PRODUCTION_TEST_LIB
extern volatile IWORD tickTime; /* global counter that is incremented every 10 msec */
#else
#ifdef ZW010x
extern volatile PWORD tickTime; /* global counter that is incremented every 10 msec */
#endif
#if defined(ZW020x) || defined(ZW030x)
extern volatile WORD tickTime; /* global counter that is incremented every 10 msec */
#endif
#endif /*#ifdef ZW_PRODUCTION_TEST_LIB*/




/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

/*================================   TimerStart   ============================
**    Register a function that is called when the specified time has elapsed.
**    The function is called "repeats" times before the timer is stopped.
**
**--------------------------------------------------------------------------*/
extern  BYTE              /*RET Timer handle                     */
TimerStart(
void (CODE *func)(),      /*IN  Timeout function address          */
BYTE timerTicks,          /*IN  Timeout value (value * 10 msec.)  */
BYTE repeats);            /*IN  Number of function calls (-1: forever)  */

/*================================   TimerRestart  ===========================
**    Set the specified timer back to the initial value.
**
**--------------------------------------------------------------------------*/
extern  BYTE              /*RET TRUE if timer restarted   */
TimerRestart(
BYTE timerHandle);        /*IN  Timer number to restart   */

/*================================   TimerCancel   ===========================
**    Stop the specified timer.
**
**--------------------------------------------------------------------------*/
extern  BYTE              /*RET TRUE if timer cancelled   */
TimerCancel(
BYTE timerHandle);        /*IN  Timer number to stop      */

#endif /* _ZW_TIMER_API_H_ */
