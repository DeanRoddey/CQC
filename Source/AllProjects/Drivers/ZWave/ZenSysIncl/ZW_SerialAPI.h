/*******************************  SERIALAPI.H  *******************************
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
 * Description:       Serial API
 *
 * Author:            Kim Jakobsen
 *
 * Last Changed By:  $Author: jfr $
 * Revision:         $Revision: 1.20 $
 * Last Changed:     $Date: 2007/01/04 14:59:36 $
 *
 ****************************************************************************/

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/


/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

#define SOF 0x01  /* Start Of Frame */
#define ACK 0x06  /* Acknowledge successfull frame reception */
#define NAK 0x15  /* Not Acknowledge successfull frame reception - please retransmit... */
#define CAN 0x18  /* Frame received (from host) was dropped - waiting for ACK */

/* Frame types */
#define REQUEST                                         0x00
#define RESPONSE                                        0x01

/* Flags used in SERIAL_API_GET_INIT_DATA */
#define GET_INIT_DATA_FLAG_SLAVE_API                    0x01
#define GET_INIT_DATA_FLAG_TIMER_SUPPORT                0x02
#define GET_INIT_DATA_FLAG_CONTROLLER_STATUS            0x04 /* Obsolete. USE next */
#define GET_INIT_DATA_FLAG_SECONDARY_CTRL               0x04
#define GET_INIT_DATA_FLAG_IS_SUC                       0x08

/* Function IDs */
#define FUNC_ID_SERIAL_API_GET_INIT_DATA                0x02
#define FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION        0x03
#define FUNC_ID_APPLICATION_COMMAND_HANDLER             0x04
#define FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES          0x05

/* SERIAL API ver 4 added - START */
#define FUNC_ID_SERIAL_API_SET_TIMEOUTS                 0x06
#define FUNC_ID_SERIAL_API_GET_CAPABILITIES             0x07
#define FUNC_ID_SERIAL_API_SOFT_RESET                   0x08
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_SET_RF_RECEIVE_MODE                  0x10
//#define FUNC_ID_ZW_SET_SLEEP_MODE                     0x11
#define FUNC_ID_ZW_SEND_NODE_INFORMATION                0x12
#define FUNC_ID_ZW_SEND_DATA                            0x13
#define FUNC_ID_ZW_SEND_DATA_MULTI                      0x14
#define FUNC_ID_ZW_GET_VERSION                          0x15

/* SERIAL API ver 4 added - START */
#define FUNC_ID_ZW_SEND_DATA_ABORT                      0x16
#define FUNC_ID_ZW_RF_POWER_LEVEL_SET                   0x17
#define FUNC_ID_ZW_SEND_DATA_META                       0x18
/* SERIAL API ver 4 added - END */

#define FUNC_ID_ZW_RESERVED_SD                          0x19
#define FUNC_ID_ZW_RESERVED_SDM                         0x1A
#define FUNC_ID_ZW_RESERVED_SRI                         0x1B

#define FUNC_ID_MEMORY_GET_ID                           0x20
#define FUNC_ID_MEMORY_GET_BYTE                         0x21
#define FUNC_ID_MEMORY_PUT_BYTE                         0x22
#define FUNC_ID_MEMORY_GET_BUFFER                       0x23
#define FUNC_ID_MEMORY_PUT_BUFFER                       0x24
/* SERIAL API ver 5 added - START */
#define FUNC_ID_SERIAL_API_GET_APPL_HOST_MEMORY_OFFSET  0x25
/* SERIAL API ver 5 added - END */

#define FUNC_ID_CLOCK_SET                               0x30
#define FUNC_ID_CLOCK_GET                               0x31
#define FUNC_ID_CLOCK_CMP                               0x32
#define FUNC_ID_RTC_TIMER_CREATE                        0x33
#define FUNC_ID_RTC_TIMER_READ                          0x34
#define FUNC_ID_RTC_TIMER_DELETE                        0x35
#define FUNC_ID_RTC_TIMER_CALL                          0x36

#define FUNC_ID_ZW_SET_LEARN_NODE_STATE                 0x40
#define FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO               0x41
#define FUNC_ID_ZW_SET_DEFAULT                          0x42
#define FUNC_ID_ZW_NEW_CONTROLLER                       0x43
#define FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE         0x44
#define FUNC_ID_ZW_REPLICATION_SEND_DATA                0x45
#define FUNC_ID_ZW_ASSIGN_RETURN_ROUTE                  0x46
#define FUNC_ID_ZW_DELETE_RETURN_ROUTE                  0x47
#define FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE         0x48
#define FUNC_ID_ZW_APPLICATION_UPDATE                   0x49

/*Obsolete use ZW_APPLICATION_UPDATE */
#define FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE        0x49

#define FUNC_ID_ZW_ADD_NODE_TO_NETWORK                  0x4A
#define FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK             0x4B
#define FUNC_ID_ZW_CREATE_NEW_PRIMARY                   0x4C
#define FUNC_ID_ZW_CONTROLLER_CHANGE                    0x4D

#define FUNC_ID_ZW_RESERVED_FN                          0x4E
#define FUNC_ID_ZW_RESERVED_AR                          0x4F

#define FUNC_ID_ZW_REQUEST_NODE_INFO                    0x60
#define FUNC_ID_ZW_REMOVE_FAILED_NODE_ID                0x61
#define FUNC_ID_ZW_IS_FAILED_NODE_ID                    0x62
#define FUNC_ID_ZW_REPLACE_FAILED_NODE                  0x63

/* Slave only */
#define FUNC_ID_ZW_SET_LEARN_MODE                       0x50
/* Slave only end */

#define FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE              0x51
#define FUNC_ID_ZW_ENABLE_SUC                           0x52
#define FUNC_ID_ZW_REQUEST_NETWORK_UPDATE               0x53
#define FUNC_ID_ZW_SET_SUC_NODE_ID                      0x54
#define FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE              0x55
#define FUNC_ID_ZW_GET_SUC_NODE_ID                      0x56
#define FUNC_ID_ZW_SEND_SUC_ID                          0x57

#define FUNC_ID_ZW_RESERVED_ASR                         0x58
#define FUNC_ID_ZW_REDISCOVERY_NEEDED                   0x59

#define FUNC_ID_TIMER_START                             0x70
#define FUNC_ID_TIMER_RESTART                           0x71
#define FUNC_ID_TIMER_CANCEL                            0x72
#define FUNC_ID_TIMER_CALL                              0x73

/* Installer API */
#define FUNC_ID_GET_ROUTING_TABLE_LINE                  0x80
#define FUNC_ID_GET_TX_COUNTER                          0x81
#define FUNC_ID_RESET_TX_COUNTER                        0x82
#define FUNC_ID_STORE_NODEINFO                          0x83
#define FUNC_ID_STORE_HOMEID                            0x84
/* Installer API only end */

#define FUNC_ID_LOCK_ROUTE_RESPONSE                     0x90

#ifdef ZW_CONTROLLER_SINGLE
#define FUNC_ID_SERIAL_API_TEST                         0x95
#endif

/* ZW_CONTROLLER_BRIDGE only START */
#define FUNC_ID_SERIAL_API_APPL_SLAVE_NODE_INFORMATION  0xA0
#define FUNC_ID_APPLICATION_SLAVE_COMMAND_HANDLER       0xA1
#define FUNC_ID_ZW_SEND_SLAVE_NODE_INFORMATION          0xA2
#define FUNC_ID_ZW_SEND_SLAVE_DATA                      0xA3
#define FUNC_ID_ZW_SET_SLAVE_LEARN_MODE                 0xA4
#define FUNC_ID_ZW_GET_VIRTUAL_NODES                    0xA5
#define FUNC_ID_ZW_IS_VIRTUAL_NODE                      0xA6
#define FUNC_ID_ZW_RESERVED_SSD                         0xA7
/* ZW_CONTROLLER_BRIDGE only END */

#define FUNC_ID_PWR_SETSTOPMODE                         0xB0    // ZW102 only
#define FUNC_ID_PWR_CLK_PD                              0xB1    // ZW102 only
#define FUNC_ID_PWR_CLK_PUP                             0xB2    // ZW102 only
#define FUNC_ID_PWR_SELECT_CLK                          0xB3    // ZW102 only
#define FUNC_ID_ZW_SET_WUT_TIMEOUT                      0xB4    // ZW201 only
#define FUNC_ID_ZW_IS_WUT_KICKED                        0xB5    // ZW201 only

#define FUNC_ID_ZW_WATCHDOG_ENABLE                      0xB6
#define FUNC_ID_ZW_WATCHDOG_DISABLE                     0xB7
#define FUNC_ID_ZW_WATCHDOG_KICK                        0xB8
#define FUNC_ID_ZW_SET_EXT_INT_LEVEL                    0xB9    // ZW201 only

#define FUNC_ID_ZW_RF_POWER_LEVEL_GET                   0xBA
#define FUNC_ID_ZW_GET_NEIGHBOR_COUNT                   0xBB
#define FUNC_ID_ZW_ARE_NODES_NEIGHBOURS                 0xBC

#define FUNC_ID_ZW_TYPE_LIBRARY                         0xBD
#define FUNC_ID_ZW_SEND_TEST_FRAME                      0xBE
#define FUNC_ID_ZW_GET_PROTOCOL_STATUS                  0xBF

#define FUNC_ID_ZW_SET_PROMISCUOUS_MODE                 0xD0

/* Allocated for proprietary serial API commands */
#define FUNC_ID_PROPRIETARY_0                           0xF0
#define FUNC_ID_PROPRIETARY_1                           0xF1
#define FUNC_ID_PROPRIETARY_2                           0xF2
#define FUNC_ID_PROPRIETARY_3                           0xF3
#define FUNC_ID_PROPRIETARY_4                           0xF4
#define FUNC_ID_PROPRIETARY_5                           0xF5
#define FUNC_ID_PROPRIETARY_6                           0xF6
#define FUNC_ID_PROPRIETARY_7                           0xF7
#define FUNC_ID_PROPRIETARY_8                           0xF8
#define FUNC_ID_PROPRIETARY_9                           0xF9
#define FUNC_ID_PROPRIETARY_A                           0xFA
#define FUNC_ID_PROPRIETARY_B                           0xFB
#define FUNC_ID_PROPRIETARY_C                           0xFC
#define FUNC_ID_PROPRIETARY_D                           0xFD
#define FUNC_ID_PROPRIETARY_E                           0xFE


/* Illegal function ID */
#define FUNC_ID_UNKNOWN                                 0xFF
