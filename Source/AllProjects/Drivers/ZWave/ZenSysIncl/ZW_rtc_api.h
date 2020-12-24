/*******************************  RTC_API.H  *******************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Z-Wave, the wireless language.
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
 * Description: Real Time Clock Application Interface.
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: jsi $
 * Revision:         $Revision: 1.18 $
 * Last Changed:     $Date: 2005/10/24 12:48:17 $
 *
 ****************************************************************************/
#ifndef _RTC_API_H_
#define _RTC_API_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/


/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* One time timer */
#define RTC_TIMER_ONE_TIME  1
#define RTC_TIMER_FOREVER   (BYTE)-1

typedef struct _CLOCK_
{
  BYTE  weekday;  /* Weekday 1:monday...7:sunday   */
  BYTE  hour;     /* Hour 0...23                   */
  BYTE  minute;   /* Minute 0...59                 */
} CLOCK;

typedef struct _RTC_TIMER_
{
  BYTE  status;         /* Timer element status                                  */
  CLOCK timeOn;         /* Weekday, all days, work days or weekend callback time */
  CLOCK timeOff;        /*  , Hour callback time                                 */
                        /*  , Minute callback time                               */
  BYTE  repeats;        /* Number of callback times (-1: forever)                */
  VOID_CALLBACKFUNC(func)(BYTE status, BYTE parm);   /* Timer function address   */
  BYTE  parm;           /* Parameter that is returned to the timer function      */
} RTC_TIMER;

/* RTC_TIMER status */
#define RTC_STATUS_DELETED  0x00    /* Used to free the timer element from the EEPROM */
#define RTC_STATUS_ACTIVE   0x80    /* Set when the timer element is stored in the EEPROM */
#define RTC_STATUS_FIRED    0x40    /* Set when the "on" timer callback function is called */
                                    /* Cleared when the "off" timer callback function is called */
#define RTC_STATUS_APPL_MASK 0x0F   /* Bits reserved for the Application programs */
                                    /*   The appl. bits are stored when the timer is created */
/* day definitions */
#define RTC_ALLDAYS   0   /* monday...sunday */
#define RTC_MONDAY    1
#define RTC_TUESDAY   2
#define RTC_WEDNESDAY 3
#define RTC_THURSDAY  4
#define RTC_FRIDAY    5
#define RTC_SATURDAY  6
#define RTC_SUNDAY    7
#define RTC_WORKDAYS  8   /* monday...friday */
#define RTC_WEEKEND   9   /* saturday, sunday */

/****************************************************************************
** Z-Wave Real Timer Application layer interface
**/


/*==============================   ZW_ClockInit   ============================
**    Reset and activate the Real Time Clock.
**    The time is initiated to 1.00:00 (monday after midnight)
**
** void                  RET Nothing
** ZW_ClockInit( void )  IN  Nothing
**--------------------------------------------------------------------------*/
#define ZW_CLOCK_INIT() ZW_ClockInit()


/*==============================   ClockSet   ===============================
**    Set the Real Time Clock
**
** extern BYTE         RET  FALSE if invalid time value
** ClockSet(
** CLOCK *pNewTime);   IN   New time to set
**--------------------------------------------------------------------------*/
#define ZW_CLOCK_SET(pNewTime) ClockSet(pNewTime)


/*==============================   ClockGet   ===============================
**    Get the current Real Time Clock time
**
** extern void         RET  Nothing
** ClockGet(
** CLOCK *pTime);      OUT   Pointer to put the current time
**--------------------------------------------------------------------------*/
#define ZW_CLOCK_GET(pTime) ClockGet(pTime)


/*==============================   ClockCmp   ===============================
**    Compare the specified time with the current Real Time Clock time
**
** extern char         RET  -1:before; 0:equal; 1:past current time
** ClockCmp(
** CLOCK *pTime);      IN   Pointer to the time structure
**--------------------------------------------------------------------------*/
#define ZW_CLOCK_CMP(pTime) ClockCmp(pTime)


/*=============================   ZW_RTCInit   ===============================
**    Initialize the Real Timer functionality.
**
** void                RET Nothing
** ZW_RTCInit( void)   IN  Nothing
**--------------------------------------------------------------------------*/
#define ZW_RTC_INIT() ZW_RTCInit()


/*==============================   RTCTimerAction   =========================
**    If RTC time has been changed, run through the RTC timer list and call
**    the timer functions that fit the current time.
**
** extern void               RET Nothing
** ZW_RTCTimerAction(void);  In  Nothing
**--------------------------------------------------------------------------*/
#define ZW_RTC_TIMER_ACTION() ZW_RTCTimerAction()


/*==============================   RTCTimerCreate   =========================
**    Create a timer element that initiate a call to the specified function.
**
**    Side effects: The timer element is saved into the EEPROM, therefore the
**                the timer element must be in "static" memory. The timer
**                element data buffer can be reused when the completion
**                function is called.
** extern BYTE          RET  RTC timer handle (-1 if failed)
** RTCTimerCreate(
** RTC_TIMER *timer,       IN  Pointer to the new timer structure
** void (*func)( void) );  IN  Timer element read completed function pointer
**--------------------------------------------------------------------------*/
#define ZW_RTC_CREATE(timer, func) RTCTimerCreate(timer, func)


/*==============================   RTCTimerRead   ===========================
**    Copy a timer element to the specified memory buffer.
**
** extern BYTE         RET  Next RTC timer handle in the list (-1 if end of list)
** RTCTimerRead(
** BYTE *timerHandle,   IN/OUT  Timer handle (0 if start of list)/actual Timer handle
** RTC_TIMER *timer);   OUT  Pointer to the memory buffer
**--------------------------------------------------------------------------*/
#define ZW_RTC_READ(handle, timer) RTCTimerRead(handle, timer)

/*==============================   RTCTimerDelete   =========================
**    Remove the timer element from the RTC timer list.
**
** extern void         RET  Nothing
** RTCTimerDelete(
** BYTE timerHandle);  IN  Timer handle to remove
**--------------------------------------------------------------------------*/
#define ZW_RTC_DELETE(handle) RTCTimerDelete(handle)


/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/


/*==============================   ZW_ClockInit   ============================
**    Reset and activate the Real Time Clock.
**    The time is initiated to 1.00:00 (monday after midnight)
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void                 /*RET Nothing  */
ZW_ClockInit(void);  /* IN Nothing */


/*==============================   ClockSet   ===============================
**    Set the Real Time Clock
**
**    At system startup the RTC is initiated to 1.00:00 (monday after midnight)
**--------------------------------------------------------------------------*/
extern BYTE         /*RET  FALSE if invalid time value       */
ClockSet(
  CLOCK *pNewTime);   /*IN   New time to set   */


/*==============================   ClockGet   ===============================
**    Get the current Real Time Clock time
**
**--------------------------------------------------------------------------*/
extern void         /*RET  Nothing      */
ClockGet(
  CLOCK *pTime);      /*OUT   Pointer to put the current time   */


/*==============================   ClockCmp   ===============================
**    Compare the specified time with the current Real Time Clock time
**
**--------------------------------------------------------------------------*/
extern char         /*RET  1:before; 0:equal; -1:past current time */
ClockCmp(
  CLOCK *pTime);      /*IN   Pointer to the time structure  */


/*=============================   ZW_RTCInit   ===============================
**    Initialize the Real Timer functionality.
**
**
**--------------------------------------------------------------------------*/
void                /*RET Nothing   */
ZW_RTCInit(void);   /* IN Nothing */


/*==============================   RTCTimerAction   =========================
**    If RTC time has been changed, run through the RCT timer list and call
**    the timer functions that fit the current time.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern void               /*RET Nothing */
ZW_RTCTimerAction(void);  /* In Nothing */


/*==============================   RTCTimerCreate   =========================
**    Create a timer element that initiate a call to the specified function.
**
**    Side effects: The timer element is saved into the EEPROM, therefore
**                the timer element must be in "static" memory. The timer
**                element data buffer can be reused when the completion
**                function is called.
**--------------------------------------------------------------------------*/
extern BYTE          /*RET  RTC timer handle: -1 if failed    */
RTCTimerCreate(
  RTC_TIMER *timer,       /*IN  Pointer to the new timer structure */
  VOID_CALLBACKFUNC(func)( void) );  /*IN  Timer element read completed function pointer */


/*==============================   RTCTimerRead   ===========================
**    Copy a timer element to the specified memory buffer.
**
**--------------------------------------------------------------------------*/
extern BYTE         /*RET  Next RTC timer handle in the list: -1 if end of list */
RTCTimerRead(
  BYTE *timerHandle,   /*IN/OUT  Timer handle pointer: 0 if start of list/actual Timer handle  */
  RTC_TIMER *timer);   /*IN  Pointer to the memory buffer */


/*==============================   RTCTimerDelete   =========================
**    Remove the timer element from the RTC timer list.
**
**--------------------------------------------------------------------------*/
extern void         /*RET  Nothing */
RTCTimerDelete(
  BYTE timerHandle);  /*IN  Timer handle to remove  */


#endif /* _RTC_API_H_ */
