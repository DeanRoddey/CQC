/*******************************  ZW_BASIS_API.H  *******************************
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
 * Description: Z-Wave common controller/slave application interface
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: heh $
 * Revision:         $Revision: 1.154 $
 * Last Changed:     $Date: 2007/03/20 16:43:03 $
 *
 ****************************************************************************/
#ifndef _ZW_BASIS_API_H_
#define _ZW_BASIS_API_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <ZW_typedefs.h>
#include <ZW_sysdefs.h>
#include <ZW_transport_api.h>
#include <ZW_timer_api.h>
#include <ZW_mem_api.h>
#include <ZW_classcmd.h>

/* ZW_GetProtocolStatus retrun value masks*/
#define ZW_PROTOCOL_STATUS_ROUTING  0x01
#define ZW_PROTOCOL_STATUS_SUC      0x02

/* ZW_LIBRARY_TYPEs one of these defines are returned when requesting */
/* Library type */
#define ZW_LIB_CONTROLLER_STATIC  0x01
#define ZW_LIB_CONTROLLER         0x02
#define ZW_LIB_SLAVE_ENHANCED     0x03
#define ZW_LIB_SLAVE              0x04
#define ZW_LIB_INSTALLER          0x05
#define ZW_LIB_SLAVE_ROUTING      0x06
#define ZW_LIB_CONTROLLER_BRIDGE  0x07
#define ZW_LIB_DUT                0x08

#if defined(ZW020x) || defined(ZW030x)
/* the input parameters */
#define ZW_INT0     0x00
#define ZW_INT1     0x01

/*This bit mask used as paramters for the API ZW_SetSleepMode to indicate which interrupt to mask out*/
#define ZW_INT_MASK_EXT1       0x02

#endif
/* Z-Wave Library version. Auto generated do not change! */
#if (defined(ZW010x) || defined(ZW020x) || defined(ZW030x))
#define ZW_VERSION_MAJOR          2
#define ZW_VERSION_MINOR          16
#endif

/* Z-Wave Library Chip type */
#ifdef ZW010x
#define ZW_CHIP_TYPE              1
#define ZW_CHIP_REVISION          2
#endif

#ifdef ZW020x
#define ZW_CHIP_TYPE              2
#define ZW_CHIP_REVISION          1
#endif

#ifdef ZW030x
#ifdef ZW020x
#undef ZW_CHIP_TYPE
#undef ZW_CHIP_REVISION
#endif
#define ZW_CHIP_TYPE              3
#define ZW_CHIP_REVISION          1
#endif

#ifdef ZW030x
/*The valid vlaues of teh ApplicationRfNotify API*/
#define ZW_RF_TX_MODE       1   /*RF switch from Rx to Tx mode, the modualtor is satrted and the PA is on*/
#define ZW_RF_RX_MODE       2   /*RF switch from Tx to Rx mode, the demodulator is started*/
#define ZW_RF_PA_ON         4   /*RF is in Tx mode, the PA is turned on*/
#define ZW_RF_PA_OFF        8   /*RF is in Tx mode, the PA is turned off   */
#endif

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* Node type structure used in ApplicationNodeInformation */
typedef struct _APPL_NODE_TYPE_
{
  BYTE generic;
  BYTE specific;
} APPL_NODE_TYPE;


/* valid powerlevel values used in ZW_RF_POWERLEVEL_SET and ZW_RF_POWERLEVEL_GET */

enum
{
  normalPower = 0,
  minus1dBm,
  minus2dBm,
  minus3dBm,
  minus4dBm,
  minus5dBm,
  minus6dBm,
  minus7dBm,
  minus8dBm,
  minus9dBm,
  miniumPower = minus9dBm
};

/*===============================   ZW_Poll   ================================
**    Z-Wave low level poll function.
**    This function must be called while waiting on HW ready states and
**    when executing time consuming functions.
**
**    Not to lose RF receive data the time limit is 3/4 bit time, which is
**    around 70 usec at 9600 bps (500 clockcycles). Interrupt must not be
**    disabled more than 1/12 bit time which is around 8 usec (40 clock cycles).
**
**
** void       RET Nothing
** ZW_Poll( void );  IN  Nothing
**--------------------------------------------------------------------------*/
#define ZW_POLL() ZW_Poll()


/*===============================   ZW_Version   ============================
**    Copy Z-Wave basis version from code memory (Flash) to data memory (SRAM)
**    destintaion should be in XDATA
** extern void         RET  Nothing
** ZW_Version(
** XBYTE *buffer);      IN Destination pointer in RAM
**--------------------------------------------------------------------------*/
#define ZW_VERSION(buf) ZW_Version(buf)


/*============================   ZW_Type_Library   ===========================
** Get Z-Wave library type
**
** extern BYTE            RET  Z-Wave library type (ZW_LIB_xxxx)
** ZW_Type_Library(void);
**--------------------------------------------------------------------------*/
#define ZW_TYPE_LIBRARY() ZW_Type_Library()


/*===========================   SendNodeInformation   =======================
**    Create and transmit a node informations frame
**
** void                           RET  FALSE if transmitter queue overflow
** SendNodeInformation(
** BYTE node                      IN  Destination Node ID (0xff = broadcast)
** BYTE txOptions,                IN  Transmit option flags
** void (*completedFunc)(BYTE));  IN  Transmit completed call back function
**--------------------------------------------------------------------------*/
#define ZW_SEND_NODE_INFO(node,option,func) ZW_SendNodeInformation(node,option,func)


/*===============================   ZW_SetRFReceiveMode   ===================
**    Initialize the Z-Wave RF chip.
**    Mode on:  Set the RF chip in receive mode and starts the data sampling.
**    Mode off: Set the RF chip in power down mode.
**
** BYTE           RET TRUE if operation was executed successfully, FALSE if not
** ZW_SetRFReceiveMode(
** BYTE mode );   IN  TRUE: On; FALSE: Off mode
**--------------------------------------------------------------------------*/
#define ZW_SET_RX_MODE(mode) ZW_SetRFReceiveMode(mode)

#ifdef ZW010x
/*===============================   ZW_SetSleepMode   =======================
**    Let the CPU go into a power save state, untill woken by an interrupt:
**    Z-Wave enable the external interrupt INT1 as low-level trigged and
**    the Real Time Clock timer (TIMER2).
**
**    This function is used to save (battery) power when nothing to do.
**    The Z-Wave RF is turned off so nothing can be received while in sleep mode.
**    The Z-Wave main poll loop is stopped untill the CPU has been woken.
**
**    The application controlled hardware should be turned off before returning
**    from the application poll function.
**
** void              RET Nothing
** ZW_SetSleepMode( void );  IN  Nothing
**--------------------------------------------------------------------------*/
#define ZW_SLEEP() ZW_SetSleepMode()
#endif /*ZW010x*/

/*===============================   ZW_Random   =============================
**  Pseudo-random number function
**
**--------------------------------------------------------------------------*/
#define ZW_RANDOM() ZW_Random()

#ifdef DYNAMICAL_POWERLEVEL
/*======================   ZW_RFPowerlevelRediscoverySet  ======================
**    Set the powerlevel used when doing neighbour discovery
**
**          normalPower : Max power possible
**          minus1dB   : normalPower - 1dB
**          minus2dB   : normalPower - 2dB
**          minus3dB   : normalPower - 3dB
**          minus4dB   : normalPower - 4dB
**          minus5dB   : normalPower - 5dB
**          minus6dB   : normalPower - 6dB
**          minus7dB   : normalPower - 7dB
**          minus8dB   : normalPower - 8dB
**          minus9dB   : normalPower - 9dB
**
**--------------------------------------------------------------------------*/
void ZW_RFPowerlevelRediscoverySet(BYTE bNewPower);
#endif

/*==========================   ZW_RFPowerLevelSet  ==========================
**    Set the powerlevel used in RF transmitting.
**    Valid powerlevel values are :
**
**          normalPower : Max power possible
**          minus1dBm   - normalPower - 1dB
**          minus2dBm   - normalPower - 2dB
**          minus3dBm   - normalPower - 3dB
**          minus4dBm   - normalPower - 4dB
**          minus5dBm   - normalPower - 5dB
**          minus6dBm   - normalPower - 6dB
**          minus7dBm   - normalPower - 7dB
**          minus8dBm   - normalPower - 8dB
**          minus9dBm   - normalPower - 9dB
**
**--------------------------------------------------------------------------*/
#define ZW_RF_POWERLEVEL_SET(POWERLEVEL) ZW_RFPowerLevelSet(POWERLEVEL)


/*==========================   ZW_RFPowerLevelGet  ==========================
**    Get the current powerlevel used in RF transmitting.
**    Possible powerlevel return values are :
**
**          normalPower : Max power possible
**          minus1dBm   - normalPower - 1dB
**          minus2dBm   - normalPower - 2dB
**          minus3dBm   - normalPower - 3dB
**          minus4dBm   - normalPower - 4dB
**          minus5dBm   - normalPower - 5dB
**          minus6dBm   - normalPower - 6dB
**          minus7dBm   - normalPower - 7dB
**          minus8dBm   - normalPower - 8dB
**          minus9dBm   - normalPower - 9dB
**
**--------------------------------------------------------------------------*/

#define ZW_RF_POWERLEVEL_GET() ZW_RFPowerLevelGet()


/*==========================   ZW_SEND_TEST_FRAME  ==========================
**    Send a test frame to specified nodeID, power is previously
**    set by calling ZW_RF_POWERLEVEL_SET.
**    Valid power index values are :
**
**          normalPower - Max power possible
**          minus1dBm   - normalPower - 1dB
**          minus2dBm   - normalPower - 2dB
**          minus3dBm   - normalPower - 3dB
**          minus4dBm   - normalPower - 4dB
**          minus5dBm   - normalPower - 5dB
**          minus6dBm   - normalPower - 6dB
**          minus7dBm   - normalPower - 7dB
**          minus8dBm   - normalPower - 8dB
**          minus9dBm   - normalPower - 9dB
**
**--------------------------------------------------------------------------*/

#define ZW_SEND_TEST_FRAME(nodeID, power, func) ZW_SendTestFrame(nodeID, power, func)


/*===========================   ZW_GetProtocolStatus   ==========================
**    Reprort the status of the protocol
**   The function return a mask telling which protocol function is currently running
**   possible MASK values:
**   ZW_PROTOCOL_STATUS_ROUTING:    protocol is analysing the routing table.
**   ZW_PROTOCOL_STATUS_SUC:        SUC is sending pending updates.
**
**   return value of ZERO means protocol is idle.
**--------------------------------------------------------------------------*/
#define ZW_GET_PROTOCOL_STATUS()   ZW_GetProtocolStatus()

#if defined(ZW020x) || defined(ZW030x)
/*===============================   ZW_SET_EXT_INT_LEVEL   =========================================
**    Set the trigger level for external interrupt 0 or 1
**---------------------------------------------------------------------------------------------*/
#define ZW_SET_EXT_INT_LEVEL(SRC, TRIGGER_LEVEL)  ZW_SetExtIntLevel(SRC, TRIGGER_LEVEL)


#endif

/*============================= ZW_WATCHDOG_ENABLE =========================================
**  Enable the watch dog.
**  parameters:
**----------------------------------------------------------------------------*/
#define ZW_WATCHDOG_ENABLE  ZW_WatchDogEnable()
/*============================= ZW_WATCHDOG_DISABLE =========================================
**  Disable the watch dog.
**  parameters:
**----------------------------------------------------------------------------*/
#define ZW_WATCHDOG_DISABLE ZW_WatchDogDisable()

/*============================= ZW_WATCHDOG_KICK =========================================
**  Kick the watch dog to restarted.
**  parameters:
**----------------------------------------------------------------------------*/
#define ZW_WATCHDOG_KICK  ZW_WatchDogKick()

/*============================   ZW_SET_PROMISCUOUS_MODE   ======================
**
**  Function description.
**   Enable / disable the installer library promiscuous mode.
**   When promiscuous mode is enabled, all application layer frames will be passed
**   to the application layer regardless if the frames are addressed to another node.
**   When promiscuous mode is disabled, only application frames addressed to the node will be passed
**   to the application layer.
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_SET_PROMISCUOUS_MODE(state)   ZW_SetPromiscuousMode(state)



/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

#ifdef ZW010x
/*==============================   ApplicationInitHW   ======================
**    Initialize non Z-Wave used hardware.
**    Called by the Z-Wave main function doing system startup
**
**    This is an application function
**
**--------------------------------------------------------------------------*/
extern  BYTE        /*RET TRUE if HW initialized */
ApplicationInitHW();

#else
/*==============================   ApplicationInitHW   ======================
**    Initialize non Z-Wave used hardware.
**    Called by the Z-Wave main function doing system startup
**
**    This is an application function
**
**--------------------------------------------------------------------------*/
extern  BYTE        /*RET TRUE if HW initialized */
ApplicationInitHW(
  BYTE bWakeupReason  /* Reason for the powerup of the chip */
);

/* bWakeupReason can have the following values */
#define ZW_WAKEUP_RESET   0   /* Woken up by reset or external int */
#define ZW_WAKEUP_WUT     1   /* Woken up by the WUT timer */
#define ZW_WAKEUP_SENSOR  2   /* Woken up by a wakeup beam */

#endif /* *ZW010x */

/*==========================   ApplicationInitSW   ==========================
**    Initialize application software
**    Called by the Z-Wave main function doing system startup
**
**--------------------------------------------------------------------------*/
extern  BYTE        /*RET TRUE if application software initialized */
ApplicationInitSW(
  void );           /* IN Nothing */


/*===========================   ApplicationPoll   ===========================
**    Application poll Function.
**    Called from the Z-Wave main loop.
**
**--------------------------------------------------------------------------*/
extern  void      /*RET Nothing */
ApplicationPoll(
  void);          /*  IN Nothing */


/*===========================   ApplicationTestPoll   ===========================
**    Application poll Function.
**    Called from the Z-Wave main loop when test mode is active.
**
**--------------------------------------------------------------------------*/
extern  void          /*RET Nothing */
ApplicationTestPoll(
  void);              /* IN Nothing */


/*===========================   ApplicationCommandHandler   =================
**    The Application command handler function will be called from the Z-Wave
**    command handler when an application command or request has been received.
**
**--------------------------------------------------------------------------*/
extern  void    /*RET Nothing  */
ApplicationCommandHandler(
  BYTE  rxStatus,                   /*IN  Frame header info */
  BYTE  sourceNode,                 /*IN  Command sender Node ID */
  ZW_APPLICATION_TX_BUFFER *pCmd,  /*IN  Payload from the received frame, the union
                                          should be used to access the fields*/
  BYTE   cmdLength);                /*IN  Number of command bytes including the command */

#ifdef ZW030x
/*===========================   ApplicationRfNotify   ===========================
**    Notify the application when the radio switch state
**    Called from the Z-Wave PROTOCOL When radio switch from Rx to Tx or from Tx to Rx
**    or when the modulator PA (Power Amplifier) turn on/off
**---------------------------------------------------------------------------------*/
extern  void          /*RET Nothing */
ApplicationRfNotify(
  BYTE rfState);         /* IN state of the RF, the available values is as follow:
                               ZW_RF_TX_MODE: The RF switch from the Rx to Tx mode, the modualtor is started and PA is on
                               ZW_RF_PA_ON: The RF in the Tx mode, the modualtor PA is turned on
                               ZW_RF_PA_OFF: the Rf in the Tx mode, the modulator PA is turned off
                               ZW_RF_RX_MODE: The RF switch from Tx to Rx mode, the demodulator is started.*/
#endif

/*============================================================================
**    Defines for ApplicationNodeInformation field deviceOptionMask
**--------------------------------------------------------------------------*/
#define APPLICATION_NODEINFO_NOT_LISTENING            0x00
#define APPLICATION_NODEINFO_LISTENING                0x01
#define APPLICATION_NODEINFO_OPTIONAL_FUNCTIONALITY   0x80
#define APPLICATION_FREQ_LISTENING_MODE_1000ms        0x10
#ifdef ZW030x
#define APPLICATION_FREQ_LISTENING_MODE_250ms         0x20
#endif

/*==========================   ApplicationNodeInformation   =================
**    Request Application Node information.
**    Called by the the Z-Wave application layer before transmitting a
**    "Node Information" frame.
**
**--------------------------------------------------------------------------*/
extern  void                /*RET Nothing */
ApplicationNodeInformation(
  BYTE      *deviceOptionsMask,  /*OUT Bitmask with application options     */
  APPL_NODE_TYPE *nodeType, /*OUT Generic and Specific Device Type          */
  BYTE      **nodeParm,     /*OUT Device parameter buffer pointer           */
  BYTE      *parmLength );  /*OUT Number of Device parameter bytes          */


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/*                 Implemented within the Z-Wave modules                    */
/****************************************************************************/

/*===============================   ZW_Poll   ================================
**    Z-Wave low level poll function.
**    This function must be called while waiting on HW ready states and
**    when executing time consuming functions.
**
**    Not to lose RF receive data the time limit is 3/4 bit time, which is
**    around 70 usec at 9600 bps (500 clockcycles). Interrupt must not be
**    disabled more than 1/12 bit time which is around 8 usec (40 clock cycles).
**
**
**--------------------------------------------------------------------------*/
extern void  /*RET Nothing  */
ZW_Poll(
  void );    /* IN Nothing */

#ifndef ZW_SNIFFER
/*===============================   ZW_SetRFReceiveMode   ===================
**    Initialize the Z-Wave RF chip.
**    Mode on:  Set the RF chip in receive mode and starts the data sampling.
**    Mode off: Set the RF chip in power down mode.
**
**--------------------------------------------------------------------------*/
extern BYTE         /*RET TRUE if operation was executed successfully, FALSE if not */
ZW_SetRFReceiveMode(
  BYTE mode);       /* IN TRUE: On; FALSE: Off mode */
#endif

#ifdef ZW010x
/*===============================   ZW_SetSleepMode   =======================
**    Let the CPU go into a power save state, until woken by an interrupt.
**
**    This function is used to save (battery) power when nothing to do.
**    The Z-Wave RF is turned off so nothing can be received while in sleep mode.
**    The Z-Wave main poll loop is stopped untill the CPU has been woken.
**
**    The application controlled hardware should be turned off before returning
**    from the application poll function.
**
**--------------------------------------------------------------------------*/
extern void       /*RET Nothing  */
ZW_SetSleepMode(
  void );         /* IN Nothing */
#endif /*ZW010x*/

/*===========================   ZW_SendNodeInformation   ====================
**    Create and transmit a node information broadcast frame
**    RET  FALSE if transmitter queue overflow
**--------------------------------------------------------------------------*/
extern BYTE                    /*RET  FALSE if transmitter queue overflow   */
ZW_SendNodeInformation(
  BYTE destNode,                 /*IN  Destination Node ID  */
  BYTE txOptions,                /*IN  Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE));  /*IN  Transmit completed call back function  */


/*===============================   ZW_Random   =============================
**  Pseudo-random number function
**
**--------------------------------------------------------------------------*/
extern BYTE /*RET Random number */
ZW_Random(
  void);    /*IN Nothing */


/*===============================   ZW_Version   ============================
**    Copy Z-Wave basis version from code memory (Flash) to data memory (SRAM)
**    destintaion should be in XDATA
**    Returns the
**--------------------------------------------------------------------------*/
extern BYTE      /*RET Library type ZW_LIB_xxx  */
ZW_Version(
  XBYTE *dst);   /* IN Destination pointer in RAM */


/*============================   ZW_Type_Library   ===========================
** Get Z-Wave library type
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern BYTE      /*RET  Z-Wave library type (ZW_LIB_xxxx) */
ZW_Type_Library(void);

#ifdef ZW010x
/*==========================   ZW_RFPowerLevelSet  ==========================
**    Set the powerlevel used in RF transmitting.
**    Valid powerlevel values are :
**
**          normalPower - Max power possible
**          minus1dBm   - normalPower - 1dBm
**          minus2dBm   - normalPower - 2dBm
**          minus3dBm   - normalPower - 3dBm
**          minus4dBm   - normalPower - 4dBm
**          minus5dBm   - normalPower - 5dBm
**          minus6dBm   - normalPower - 6dBm
**          minus7dBm   - normalPower - 7dBm
**          minus8dBm   - normalPower - 8dBm
**          minus9dBm   - normalPower - 9dBm
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*==========================   ZW_RFPowerLevelSet  ==========================
**    Set the powerlevel used in RF transmitting.
**    Valid powerlevel values are :
**
**       normalPower : Max power possible
**       minus2dBm    - normalPower - 2dBm
**       minus4dBm    - normalPower - 4dBm
**       minus6dBm    - normalPower - 6dBm
**       minus8dBm    - normalPower - 8dBm
**       minus10dBm   - normalPower - 10dBm
**       minus12dBm   - normalPower - 12dBm
**       minus14dBm   - normalPower - 14dBm
**       minus16dBm   - normalPower - 16dBm
**       minus18dBm   - normalPower - 18dBm
**
**--------------------------------------------------------------------------*/
#endif

BYTE                /*RET The powerlevel set */
ZW_RFPowerLevelSet(
  BYTE powerLevel); /* IN Powerlevel to set */

#ifdef ZW010x
/*==========================   ZW_RFPowerLevelGet  ==========================
**    Get the current powerlevel used in RF transmitting.
**    Possible powerlevel return values are :
**
**          normalPower - Max power possible
**          minus2dBm   - normalPower - 1dBm
**          minus4dBm   - normalPower - 2dBm
**          minus6dBm   - normalPower - 3dBm
**          minus4dBm   - normalPower - 4dBm
**          minus5dBm   - normalPower - 5dBm
**          minus6dBm   - normalPower - 6dBm
**          minus7dBm   - normalPower - 7dBm
**          minus8dBm   - normalPower - 8dBm
**          minus9dBm   - normalPower - 9dBm
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*==========================   ZW_RFPowerLevelGet  ==========================
**    Get the current powerlevel used in RF transmitting.
**    Possible powerlevel return values are :
**
**       normalPower : Max power possible
**       minus2dBm    - normalPower - 2dBm
**       minus4dBm    - normalPower - 4dBm
**       minus6dBm    - normalPower - 6dBm
**       minus8dBm    - normalPower - 8dBm
**       minus10dBm   - normalPower - 10dBm
**       minus12dBm   - normalPower - 12dBm
**       minus14dBm   - normalPower - 14dBm
**       minus16dBm   - normalPower - 16dBm
**       minus18dBm   - normalPower - 18dBm
**
**--------------------------------------------------------------------------*/
#endif

BYTE                /*RET The current powerlevel */
ZW_RFPowerLevelGet(
  void);            /* IN Nothing */

#ifdef ZW010x
/*===========================   ZW_SendTestFrame   ==========================
**    Send a test frame to specified nodeID, powerlevel is previously
**    set by calling ZW_RFPowerLevelSet.
**    Valid powerLevel index values are :
**
**          normalPower - Max power possible
**          minus2dBm   - normalPower - 1dBm
**          minus4dBm   - normalPower - 2dBm
**          minus6dBm   - normalPower - 3dBm
**          minus4dBm   - normalPower - 4dBm
**          minus5dBm   - normalPower - 5dBm
**          minus6dBm   - normalPower - 6dBm
**          minus7dBm   - normalPower - 7dBm
**          minus8dBm   - normalPower - 8dBm
**          minus9dBm   - normalPower - 9dBm
**
**--------------------------------------------------------------------------*/
#endif
#if defined(ZW020x) || defined(ZW030x)
/*===========================   ZW_SendTestFrame   ==========================
**    Send a test frame to specified nodeID, powerlevel is previously
**    set by calling ZW_RFPowerLevelSet.
**    Valid powerLevel index values are :
**
**       normalPower : Max power possible
**       minus2dBm    - normalPower - 2dBm
**       minus4dBm    - normalPower - 4dBm
**       minus6dBm    - normalPower - 6dBm
**       minus8dBm    - normalPower - 8dBm
**       minus10dBm   - normalPower - 10dBm
**       minus12dBm   - normalPower - 12dBm
**       minus14dBm   - normalPower - 14dBm
**       minus16dBm   - normalPower - 16dBm
**       minus18dBm   - normalPower - 18dBm
**
**--------------------------------------------------------------------------*/
#endif

BYTE               /*RET FALSE if transmitter busy else TRUE */
ZW_SendTestFrame(
  BYTE nodeID,     /* IN nodeID to transmit to */
  BYTE powerLevel, /* IN powerlevel index */
  VOID_CALLBACKFUNC(func)(BYTE txStatus)); /* Call back function called when done */

/*===========================   ZW_GetProtocolStatus   ==========================
**    Reprort the status of the protocol
**   The function return a mask telling which protocol function is currently running
**   possible MASK values:
**   ZW_PROTOCOL_STATUS_ROUTING:    protocol is analysing the routing table.
**   ZW_PROTOCOL_STATUS_SUC:        SUC is sending pending updates.
**
**   return value of ZERO means protocol is idle.
**--------------------------------------------------------------------------*/

BYTE
ZW_GetProtocolStatus(void);

#if defined(ZW020x) || defined(ZW030x)
/*===============================   ZW_SetExtIntLevel   =========================================
**    Set the trigger level for external interrupt 0 or 1
**---------------------------------------------------------------------------------------------*/
void
ZW_SetExtIntLevel(
  BYTE intSrc,        /*IN: The interrupt src to set its level ZW_INT0 or ZW_INT1*/
  BOOL triggerLevel); /*IN: The trigger level of the interrupt source, TRUE active high/rising edge,*/
                      /*     FALSE active low faling edge*/

#endif
/*============================= ZW_WatchDogEnable() =========================================
**  Enable the watch dog.
**  parameters:
**----------------------------------------------------------------------------*/
ZW_WatchDogEnable();
/*============================= ZW_WatchDogDisable() =========================================
**  Disable the watch dog.
**  parameters:
**----------------------------------------------------------------------------*/
ZW_WatchDogDisable();

/*============================= ZW_WatchDogKick() =========================================
**  Kick the watchdog to restarted.
**  parameters:
**----------------------------------------------------------------------------*/
ZW_WatchDogKick();

/*============================   ZW_SetPromiscuousMode   ======================
**
**  Function description.
**   Enable / disable the installer library promiscuous mode.
**   When promiscuous mode is enabled, all application layer frames will be passed
**   to the application layer regardless if the frames are addressed to another node.
**   When promiscuous mode is disabled, only application frames addressed to the node will be passed
**   to the application layer.
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
void                /*Nothing */
ZW_SetPromiscuousMode(
  BOOL state);     /* IN TRUE to enable the promiscuous mode, FALSE to disable it.*/



#endif /* _ZW_BASIS_API_H_ */
