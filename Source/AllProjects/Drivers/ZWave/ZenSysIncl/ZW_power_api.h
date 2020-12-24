/******************************  ZW_power_api.c  *******************************
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
 *-----------------------------------------------------------------------------
 *
 * Description:  The ZW010x power managment library
 *
 * Author:Samer Seoud
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.17 $
 * Last Changed:     $Date: 2007/01/24 09:30:15 $
 *
 *****************************************************************************/


#ifndef _ZW_POWER_API_H_
#define _ZW_POWER_API_H_

/***************************************************************/
/* DEFINES                                                     */
/***************************************************************/

#ifdef ZW010x
#define PWR_MAIN_CLK          0
#define PWR_32K_CLK           1

#define SELECT_MAIN_CLK       0
#define SELECT_32K_INT        1
#define SELECT_32K_EXT        2

#define ZW_PWR_MAIN_CLK       (1<<PWR_MAIN_CLK)
#define ZW_PWR_32K_CLK        (1<<PWR_32K_CLK)

#define ZW_SELECT_MAIN_CLK    (1<<SELECT_MAIN_CLK)
#define ZW_SELECT_32K_CLK_INT     (1<<SELECT_32K_INT)
#define ZW_SELECT_32K_CLK_EXT     (1<<SELECT_32K_EXT)

#define ZW_PWR_SET_STOP_MODE               PWR_SetStopMode()
#define ZW_PWR_CLK_POWERDOWN(FLAGS)        PWR_Clk_PD(FLAGS)
#define ZW_PWR_CLK_POWERUP(FLAGS)          PWR_Clk_PUp(FLAGS)
#define ZW_PWR_SELECT_CLK(CLOCK)           PWR_Select_Clk(CLOCK)
#endif /*ZW010x*/

#ifdef ZW030x
#define  ZW_WUT_FAST_MODE  0x04
#endif
#if defined(ZW020x) || defined(ZW030x)
#define  ZW_STOP_MODE      0x01
#define  ZW_WUT_MODE       0x02



#ifdef ZW_BEAM_RX_WAKEUP
#define  ZW_SENSOR_MODE 3
#define  ZW_FREQUENTLY_LISTENING_MODE 5
#endif

#define ZW_PWR_SET_STOP_MODE              ZW_SetSleepMode(ZW_STOP_MODE,FALSE,0)
#define ZW_SET_SLEEP_MODE(MODE,MASK_INT)  ZW_SetSleepMode(MODE,MASK_INT,0)
#define ZW_SET_WUT_TIMEOUT(TIME)          ZW_SetWutTimeout(TIME)
#endif /*ZW020x*/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/
#ifdef ZW010x
/*==========================   ZW_SetStopMode   =======================
**    Let the CPU go into a power save state.
**    The CPU only will wake up by ether system reset or power recycle
**
**    This function is used to save (battery) power when nothing to do.
**    ALl the ZW0102 hardware (Except for the ADC) will be turned off
**    The Z-Wave main poll loop is stopped untill the CPU has been woken.
**
**    The application controlled hardware should be turned off before
**    returning from the application poll function.
**
**--------------------------------------------------------------------------*/
void              /*RET Nothing  */
PWR_SetStopMode( void );  /*IN  NONE*/
/*===============================   ZW_Clk_PD   =======================
**   Power Down a desired clock oscillator
**   Power Down Main crystal oscillator , 32k oscillator or both.
**--------------------------------------------------------------------------*/
void              /*RET Nothing  */
PWR_Clk_PD( BYTE flags );  /*IN  Bit fields indicating the clock oscillator to power down */

/*===============================   ZW_Clk_PUp   =======================
**   Power Up a desired clock oscillator
**   Power Up Main crystal oscillator , 32k oscillator or both.
**--------------------------------------------------------------------------*/
void              /*RET Nothing  */
PWR_Clk_PUp( BYTE flags );  /*IN  Bit fields indicating the clock oscillator to power up */

/*===============================   ZW_Select_Clk   =======================
**   Select the ZW0102 ASIC clock source
**   The source could be either MAIN clock oscillator, 32k clock osicllator or
**   32k external clock sginal.
**  The RTC will always use the 32K clock signal.
**--------------------------------------------------------------------------*/
void              /*RET Nothing  */
PWR_Select_Clk( BYTE clock );  /*IN  the clock source*/
#endif /*ZW010x*/

#if defined(ZW020x) || defined(ZW030x)
/*===============================   ZW_SetSleepMode   ===========================================
**    Set the Power Down mode to use.
**
**    This function is used to save (battery) power when nothing to do.
**    The ZW_WAVE power down system function in 4 different mode:
**
**    The STOP_MODE: where the All the ASIC is turn down. The ASIC can be waked up
**    again by Hardware reset or by the external interrupt INT1.
**
**    The WUT_MODE: The ASIC is powered down, and it can only be waked by the Wake Up Timer
**                  or the external interrupt INT1. The the time out value of the WUT
**                  be set by the API call ZW_SetWutTimeout. When the ASIC is waked from
**                  the WUT_MODE, the reason for wakeup will be given in the ApplicationInitHW.
**
**
**    IN interrupts Enable bitmask, Valid values are ZW_INT_MASK_EXT1 or 0x00 if no external
**                         Interrupts should wakeup
**
**    The Z-Wave RF is turned off in WUT and STOP mode so nothing can be received while in one
**     of these  modes.
**    In STOP and WUT modes the INT1 interrupt can be masked out so it cannot wake up the ASIC.
**    The Z-Wave main poll loop is stopped untill the CPU has been woken.
**
**--------------------------------------------------------------------------------------------------*/
void              /*RET Nothing  */
ZW_SetSleepMode(
  BYTE Mode,        /*IN the power down mode to use*/
  BYTE IntEnable,   /*IN interrupts Enable bitmask, Valid values are ZW_INT_MASK_EXT1 or 0x00 if no external
                         Interrupts should wakeup*/
  BYTE BeamCount);  /*IN The number of wakeup beam periodes before waking up the application */


/*===============================   ZW_SetWutTimeout   ===========================================
**    Set the time out value for the WUT.
**    This function set the WUT timeout value so the the ASIC wake up from the WUt power down
**    mode after the specified number of seconds.
**--------------------------------------------------------------------------------------------------*/
void              /*RET Nothing  */
ZW_SetWutTimeout(
  BYTE wutTimeout); /*the timeout value of the WUT timer, The WUT will wake up after (wutTimeout+ 1) seconds */


#endif /*ZW020x*/
#endif /* _ZW_POWER_API_H_ */
