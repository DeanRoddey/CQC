/*******************************  ZW_CONTROLLER_INSTALLER_API.H  **********************
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
 * Description: Z-Wave Controller node application interface
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.13 $
 * Last Changed:     $Date: 2006/12/01 12:31:37 $
 *
 ****************************************************************************/
#ifndef _ZW_CONTROLLER_INSTALLER_API_H_
#define _ZW_CONTROLLER_INSTALLER_API_H_

#ifndef ZW_INSTALLER
#define ZW_INSTALLER
#endif

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
/*These are a part of the standard controller API*/
#include <ZW_controller_api.h>

/****************************  Z-Wave API  **********************************
* Functionality specific for the Installer API.
****************************************************************************/
/*============================   ZW_TX_COUNTER   ============================
**    Function description
**    This variable returns the number of transmitted frames since last time
**    it was reset by the application. if 0xFF is returned 0xFF or more
**    frames have been transmitted
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_TX_COUNTER zwTransmitCount

/*==========================   ZW_STORE_NODE_INFO   ==========================
**
**    Stores Protocol Nodeinformation. The format should be equal to
**    the one gotten with ZW_GET_NODE_STATE
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_STORE_NODE_INFO(NodeID,NodeInfo,function) ZW_StoreNodeInfo(NodeID,NodeInfo,function)

/*============================   ZW_STORE_HOME_ID  ==========================
**    Stores the Home-ID and Node-ID in nonvolatile memory. Please consult
**    with Programming guide before using.
**      RET FALSE if write buffer full
**       IN BYTE pointer to Home-ID word as gotten with ZW_MEMORY_GET_ID.
**       IN BYTE Node-ID as gotten with ZW_MEMORY_GET_ID.
**
**--------------------------------------------------------------------------*/
#define ZW_STORE_HOME_ID(home,node) ZW_StoreHomeID(home,node)

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/*                 Implemented within the Z-Wave modules                    */
/****************************************************************************/

/*==========================   ZW_StoreNodeInfo   ===============================
**
**    Add node info to node info list in EEPROM
**
**    Side effects:
**
**-------------------------------------------------------------------------------*/
BOOL ZW_StoreNodeInfo(  /*  RET  TRUE if stored*/
  BYTE bNodeID,         /*  IN   Node ID */
  BYTE *pNodeInfo,      /*  IN   Pointer to node info frame */
  void (CODE *func)());  /* IN   callback function. Called when data has been stored*/

/*==========================   ZW_StoreHomeID   ===============================
**
**    Save the homeID and node ID in EEPROM
**
**    Side effects:
**
**-------------------------------------------------------------------------------*/BYTE               /*RET FALSE if write buffer full         */
ZW_StoreHomeID(
  BYTE *homeID,    /* IN Home-ID            */
  BYTE  nodeID );   /* IN Node-ID            */

extern BYTE zwTransmitCount;

#endif /* _ZW_CONTROLLER_INSTALLER_API_H_ */

