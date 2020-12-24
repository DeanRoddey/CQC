/******************************* ZW_appltimer.h *******************************
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
 * Description:
 *          Header file for the PWM/Timer function
 * Author:   Henrik Holm
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.14 $
 * Last Changed:     $Date: 2007/01/26 15:27:13 $
 *
 ****************************************************************************/
#ifndef _ZW_APPLTIMER_H_
#define _ZW_APPLTIMER_H_
/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#ifdef ZW030x
#include <ZW030x.h>
#elif defined(ZW020x)
#include <ZW020x.h>
#elif defined(ZW010x)
#include <ZW010x.h>
#endif

#include <ZW_typedefs.h>
/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
#ifdef ZW010x
#define PWM_MODE_BIT            0x01
#define TIMER_RUN_BIT           0x02
#endif
#if defined(ZW020x) || defined(ZW030x)
#define RELOAD_BIT              0x08
#define PRESCALER_BIT           0x04
#define PWM_MODE_BIT            0x02
#define TIMER_RUN_BIT           0x01
#endif


#ifdef ZW010x
#define TIMER_INTERRUPT_VECTOR  INUM_TIMER2
#endif
#if defined(ZW020x) || defined(ZW030x)
#define TIMER_INTERRUPT_VECTOR  INUM_GP_TIMER
#endif


#ifdef ZW010x
/*============================   ZW_PWMSetup ===============================
**    Function description
**      Sets the TCON2 register to the value specified:
**      Bit 0     - 0 = TIMER mode
**                  1 = PWM mode
**      Bit 1     - 0 = Timer is inactive and counter is cleared.
**                  1 = Timer is active and counter is enabled.
**      Bit 2 to 7- Dont care
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*============================   ZW_PWMSetup ===============================
**    Function description
**      Sets the TCON2 register to the value specified:
**      Bit 0     - 0 = Timer is inactive and counter is cleared.
**                  1 = Timer is active and counter is enabled.
**      Bit 1     - 0 = TIMER mode
**                  1 = PWM mode
**      Bit 2     - 0 = Timer counter runs on fsystem/4.
**                - 1 = Timer counter runs on fsystem/512.
**      Bit 3     - 0 = The timer stops upon overflow.
**                - 1 = The timer reloads its counter registers upon overflow.
**      Bit 4  to 7- Dont care
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#define ZW_PWM_SETUP(value) ZW_PWMSetup(value)

#ifdef ZW010x
/*============================   ZW_PWMPrescale   ==========================
**    Function description
**      Sets the timeout or if running as PWM the period of the timer.
**      Timer mode (Interrupt period):
**        Tint = 255*(bValueMSB*256+bValueLSB+1)/CPU_FREQ
**      PWM mode:
**        Total period:
**          Tpwm = 255*(bValueMSB+1)/CPU_FREQ
**        High time of PWM:
**          ThPWM = (bValueMSB+1)*bValueLSB/CPU_FREQ
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*============================   ZW_PWMPrescale   ==========================
**    Function description
**      Sets the timeout or if running as PWM the period of the timer.
**      Timer mode (Interrupt period):
**        Tint = (bValueMSB*256+bValueLSB+1) * prescaler/CPU_FREQ
**      PWM mode:
**        Total period:
**          Tpwm = (bValueLSB * prescaler)/CPU_FREQ
**        High time of PWM:
**          ThPWM = (bValueMSB * prescaler)/CPU_FREQ
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#define ZW_PWM_PRESCALE(msb,lsb) ZW_PWMPrescale(msb,lsb)

/*==========================  ZW_PWM_CLEAR_INTERRUPT   ====================
**    Function description
**      Clears the Timer interrupt. Must be done by software when servicing
**      interrupt.
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_PWM_CLEAR_INTERRUPT()  ZW_PWMClearInterrupt()

/*============================   ZW_PWM_INT_ENABLE   ======================
**    Function description
**      Enables or disables the PWM interrupt:
**        TRUE = Interrupt enabled
**        FALSE = Interupt disabled
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_PWM_INT_ENABLE(value) ZW_PWMEnable(value) /*IN TRUE or FALSE*/



/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/
#ifdef ZW010x
/*============================   ZW_PWMSetup ===============================
**    Function description
**      Sets the TCON2 register to the value specified:
**      Bit 0     - 0 = TIMER mode
**                  1 = PWM mode
**      Bit 1     - 0 = Timer is inactive and counter is cleared.
**                  1 = Timer is active and counter is enabled.
**      Bit 2 to 7- Dont care
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*============================   ZW_PWMSetup ===============================
**    Function description
**      Sets the TCON2 register to the value specified:
**      Bit 0     - 0 = Timer is inactive and counter is cleared.
**                  1 = Timer is active and counter is enabled.
**      Bit 1     - 0 = TIMER mode
**                  1 = PWM mode
**      Bit 2     - 0 = Timer counter runs on fsystem/4.
**                - 1 = Timer counter runs on fsystem/512.
**      Bit 3     - 0 = The timer stops upon overflow.
**                - 1 = Tje timer reloads its counter registers upon overflow.
**      Bit 4  to 7- Dont care
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
void ZW_PWMSetup(BYTE bValue);         /*IN Value to set*/

#ifdef ZW010x
/*============================   ZW_PWMPrescale   ==========================
**    Function description
**      Sets the timeout or if running as PWM the period of the timer.
**      Timer mode (Interrupt period):
**        Tint = 255*(bValueMSB*256+bValueLSB+1)/CPU_FREQ
**      PWM mode:
**        Total period:
**          Tpwm = 255*(bValueMSB+1)/CPU_FREQ
**        High time of PWM:
**          ThPWM = (bValueMSB+1)*bValueLSB/CPU_FREQ
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*============================   ZW_PWMPrescale   ==========================
**    Function description
**      Sets the timeout or if running as PWM the period of the timer.
**      Timer mode (Interrupt period):
**        Tint = (bValueMSB*256+bValueLSB+1) * prescaler/CPU_FREQ
**      PWM mode:
**        Total period:
**          Tpwm = (bValueLSB * prescaler)/CPU_FREQ
**        High time of PWM:
**          ThPWM = (bValueMSB * prescaler)/CPU_FREQ
**    Side effects:
**
**--------------------------------------------------------------------------*/
#endif
void ZW_PWMPrescale(BYTE bValueMSB, BYTE bValueLSB);
/*============================   ZW_PWMClearInterrupt   ====================
**    Function description
**      Clears the Timer interrupt. Must be done on interrupt.
**    Side effects:
**
**--------------------------------------------------------------------------*/
void ZW_PWMClearInterrupt(void);

/*============================   ZW_PWMEnable   ======================
**    Function description
**      Enables or disables the ZW_PWMEnable:
**        TRUE = Interrupt enabled
**        FALSE = Interupt disabled
**    Side effects:
**
**--------------------------------------------------------------------------*/
void ZW_PWMEnable(BOOL bValue);            /*IN TRUE or FALSE*/
#endif /*_ZW_APPLTIMER_H_*/
