/***************************  ZW_CONTROLLER_BRIDGE_API.H  *******************
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
 * Description: Z-Wave Bridge Controller node application interface
 *
 * Author:   Johann Sigfredsson
 *
 * Last Changed By:  $Author: efh $
 * Revision:         $Revision: 1.13 $
 * Last Changed:     $Date: 2006/10/25 08:03:51 $
 *
 ****************************************************************************/
#ifndef _ZW_CONTROLLER_BRIDGE_API_H_
#define _ZW_CONTROLLER_BRIDGE_API_H_

#ifndef ZW_CONTROLLER_BRIDGE
#define ZW_CONTROLLER_BRIDGE
#endif

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
/* A Bridge Controller is a Static Controller with BRIDGE functionality but */
/* without repeater fundtionality */
#include <ZW_controller_static_api.h>

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/****************************************************************************
* Functionality specific for the Bridge Controller API.
****************************************************************************/

/*===============================   ZW_SendSlaveData   ===========================
**    Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast).
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER   transmit at low output power level (1/3 of
**                                      normal RF range).
**          TRANSMIT_OPTION_ACK         request acknowledge from destination node.
**
** BYTE                                 RET  FALSE if transmitter queue overflow
** ZW_SendSlaveData(
**    BYTE  srcNode                      IN  Source node ID
**    BYTE  destNode,                    IN  Destination node ID (0xFF == broadcast)
**    BYTE *pData,                       IN  Data buffer pointer
**    BYTE  dataLength,                  IN  Data buffer length
**    BYTE  txOptions,                   IN  Transmit option flags
**    VOID_CALLBACKFUNC(completedFunc)(  IN  Transmit completed call back function
**        BYTE txStatus));                  IN Transmit status
**--------------------------------------------------------------------------*/
#define ZW_SEND_SLAVE_DATA(srcnode,destnode,data,length,options,func) ZW_SendSlaveData(srcnode,destnode,data,length,options,func)


/*=========================   ZW_SEND_SLAVE_NODE_INFO   =====================
** Create and transmit a slave node informations frame
**
** void                                    RET FALSE if transmitter queue overflow
** ZW_SEND_SLAVE_NODE_INFO(
** BYTE sourcenode                         IN  Source Node ID - Who is transmitting
** BYTE destnode                           IN  Destination Node ID (0xff = broadcast)
** BYTE txOptions,                         IN  Transmit option flags
** VOID_CALLBACKFUNC(completedFunc)(BYTE)) IN  Transmit completed call back function
**--------------------------------------------------------------------------*/
#define ZW_SEND_SLAVE_NODE_INFO(sourcenode,destnode,option,func) ZW_SendSlaveNodeInformation(sourcenode,destnode,option,func)


/* Defines used to handle inclusion and exclusion of virtual slave nodes */
/* Are returned as callback parameter when callback, setup with */
/* ZW_SetSlaveLearnMode, is called during inclusion/exclusion process */
#define ASSIGN_COMPLETE             0x00
#define ASSIGN_NODEID_DONE          0x01  /* Node ID have been assigned */
#define ASSIGN_RANGE_INFO_UPDATE    0x02  /* Node is doing Neighbor discovery */

/* Defines defining modes possible in ZW_SetSlaveLearnMode : */

/* Disable SlaveLearnMode (disable possibility to add/remove Virtual Slave nodes) */
/* Allowed when bridge is a primary controller, an inclusion controller or a secondary controller */
#define VIRTUAL_SLAVE_LEARN_MODE_DISABLE  0x00

/* Enable SlaveLearnMode - Enable possibility for including/excluding a */
/* Virtual Slave node by an external primary/inclusion controller */
/* Allowed when bridge is an inclusion controller or a secondary controller */
#define VIRTUAL_SLAVE_LEARN_MODE_ENABLE   0x01

/* Add new Virtual Slave node if possible */
/* Allowed when bridge is a primary or an inclusion controller */
#define VIRTUAL_SLAVE_LEARN_MODE_ADD      0x02

/* Remove existing Virtual Slave node */
/* Allowed when bridge is a primary or an inclusion controller */
#define VIRTUAL_SLAVE_LEARN_MODE_REMOVE   0x03


/*===========================   ZW_SetSlaveLearnMode   =======================
**    Enable/Disable home/node ID learn mode.
**    When learn mode is enabled, received "Assign ID's Command" are handled:
**    If the current stored ID's are zero, the received ID's will be stored.
**    If the received ID's are zero the stored ID's will be set to zero.
**
**    The learnFunc is called when the received assign command has been handled.
**    The returned parameter is the learned Node ID.
**
** void           RET  Nothing
** ZW_SetSlaveLearnMode(
**   BYTE node,                IN  nodeID on node to set Learn Node Mode -
**                               ZERO if new node is to be learned
**   BYTE mode,                IN  VIRTUAL_SLAVE_LEARN_MODE_DISABLE: Disable
**                               VIRTUAL_SLAVE_LEARN_MODE_ENABLE:  Enable
**                               VIRTUAL_SLAVE_LEARN_MODE_ADD:     Create New Virtual Slave Node
**                               VIRTUAL_SLAVE_LEARN_MODE_REMOVE:  Remove Virtual Slave Node
**   void (*learnFunc)(BYTE bStatus, BYTE orgId, BYTE newID)); IN  Node learn call back function.
**--------------------------------------------------------------------------*/
#define ZW_SET_SLAVE_LEARN_MODE(node, mode, func) ZW_SetSlaveLearnMode(node, mode, func)


/*============================   ZW_IsVirtualNode   ======================
**    Function description.
**      Returns TRUE if bNodeID is a virtual slave node
**              FALSE if it is not a virtual slave node
**    Side effects:
**--------------------------------------------------------------------------*/
#define ZW_IS_VIRTUAL_NODE(bNodeID) ZW_IsVirtualNode(bNodeID)


/*===========================   ZW_GetVirtualNodes   =========================
**    Request nodemask containing virtual nodes in controller bridge.
**
**    The nodemask points to nodemask structure where the current virtual node
**    nodemask is to be copied.
**
**--------------------------------------------------------------------------*/
#define ZW_GET_VIRTUAL_NODES(nodemask) ZW_GetVirtualNodes(nodemask)


/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

/*===========================   ApplicationSlaveCommandHandler   =================
**    The Application command handler function will be called from the Z-Wave
**    command handler when an application command or request has been received.
**
**--------------------------------------------------------------------------*/
extern void    /*RET Nothing  */
ApplicationSlaveCommandHandler(
  BYTE  rxStatus,                   /*IN  Frame header info */
  BYTE  destNode,     /* To whom it might concern - which node is to receive the frame */
  BYTE  sourceNode,                 /*IN  Command sender Node ID */
  ZW_APPLICATION_TX_BUFFER *pCmd,  /*IN  Payload from the received frame, the union
                                          should be used to access the fields*/
  BYTE   cmdLength);                /*IN  Number of command bytes including the command */


/*==========================   ApplicationSlaveNodeInformation   =============
**    Request Application Node information.
**    Called by the the Z-Wave application layer before transmitting a
**    "Node Information" frame.
**
**--------------------------------------------------------------------------*/
extern void                 /*RET Nothing */
ApplicationSlaveNodeInformation(
  BYTE      destNode,       /* IN Which node do we want the nodeinfo on */
  BYTE      *listening,     /*OUT TRUE if this node is always on air */
  APPL_NODE_TYPE *nodeType, /*OUT Generic and Specific Device Type   */
  BYTE      **nodeParm,     /*OUT Device parameter buffer pointer    */
  BYTE      *parmLength );  /*OUT Number of Device parameter bytes   */


/*===============================   ZW_SendSlaveData   ===========================
**    Transmit data buffer to a single Z-Wave node or all Z-Wave nodes - broadcast
**    and transmit it from srcNodeID
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER   transmit at low output power level (1/3 of
**                                      normal RF range).
**          TRANSMIT_OPTION_ACK         the multicast frame will be followed by a
**                                      singlecast frame to each of the destination nodes
**                                      and request acknowledge from each destination node.
**
**    RET  TRUE  if data buffer was successfully put into the transmit queue
**         FALSE if transmitter queue overflow or if controller primary or srcNodeID invalid
**               then completedFunc will NOT be called
**
** BYTE                                 RET  FALSE if transmitter queue overflow or srcNodeID invalid or controller primary
** ZW_SendSlaveData(
**    BYTE  srcNodeID,                   IN  Source node ID - Virtuel nodeID
**    BYTE  destNodeID,                  IN  Destination node ID - 0xFF == Broadcast
**    BYTE *pData,                       IN  Data buffer pointer
**    BYTE  dataLength,                  IN  Data buffer length
**    BYTE  txOptions,                   IN  Transmit option flags
**    VOID_CALLBACKFUNC(completedFunc)(  IN  Transmit completed call back function
**        BYTE txStatus));                  IN Transmit status
**--------------------------------------------------------------------------------*/
BYTE                /*RET FALSE if transmitter busy or srcNodeID invalid or controller primary */
ZW_SendSlaveData(
  BYTE  srcNodeID,  /* IN Source node ID - Virtuel nodeID */
  BYTE  destNodeID, /* IN Destination node ID - 0xFF == all nodes */
  BYTE *pData,      /* IN Data buffer pointer           */
  BYTE  dataLength, /* IN Data buffer length            */
  BYTE  txOptions,  /* IN Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /* IN Transmit completed call back function  */


/*============================   ZW_SendSlaveNodeInformation   ============================
**    Create and transmit a slave node information frame
**    RET  TRUE  if Slave NodeInformation frame was successfully put into the transmit queue
**         FALSE if transmitter queue overflow or if controller primary or destNode invalid
**               then completedFunc will NOT be called
**---------------------------------------------------------------------------------------*/
BYTE                              /*RET FALSE if SlaveNodeinformation not in transmit queue */
ZW_SendSlaveNodeInformation(
  BYTE sourceNode,                /* IN Which node is to transmit the nodeinfo */
  BYTE destNode,                  /* IN Destination Node ID  */
  BYTE txOptions,                 /* IN Transmit option flags */
  VOID_CALLBACKFUNC(completedFunc)(BYTE));  /*IN  Transmit completed call back function  */


/*===========================   ZW_SetSlaveLearnMode   =======================
**    Enable/Disable home/node ID learn mode.
**    When learn mode is enabled, received "Assign ID's Command" are handled:
**    If the current stored ID's are zero, the received ID's will be stored.
**    If the received ID's are zero the stored ID's will be set to zero.
**
**    The learnFunc is called when the received assign command has been handled.
**    The returned parameter is the learned Node ID.
**
** void           RET  Nothing
** ZW_SetSlaveLearnMode(
**   BYTE node,                IN  nodeID on node to set Learn Node Mode -
**                               ZERO if new node is to be learned
**   BYTE mode,                IN  VIRTUAL_SLAVE_LEARN_MODE_DISABLE: Disable
**                               VIRTUAL_SLAVE_LEARN_MODE_ENABLE:  Enable
**                               VIRTUAL_SLAVE_LEARN_MODE_ADD:     Create New Virtual Slave Node
**                               VIRTUAL_SLAVE_LEARN_MODE_REMOVE:  Remove Virtual Slave Node
**   void (*learnFunc)(BYTE bStatus, BYTE orgId, BYTE newID)); IN  Node learn call back function.
**--------------------------------------------------------------------------*/
BYTE                  /*RET Returns TRUE if successful or FALSE if node invalid or controller is primary */
ZW_SetSlaveLearnMode(
  BYTE node,          /* IN nodeID on Virtual node to set in Learn Node Mode - if new node wanted then it must be ZERO */
  BYTE mode,          /* IN TRUE  Enable, FALSE  Disable */
  VOID_CALLBACKFUNC(learnFunc)(BYTE bStatus, BYTE orgID, BYTE newID));  /* IN Slave node learn call back function. */


/*============================   ZW_IsVirtualNode   ======================
**    Function description.
**      Returns TRUE if bNodeID is a virtual slave node
**              FALSE if it is not a virtual slave node
**    Side effects:
**--------------------------------------------------------------------------*/
BOOL                      /*RET TRUE if virtual slave node, FALSE if not */
ZW_IsVirtualNode(BYTE bNodeID);


/*===========================   ZW_GetVirtualNodes   =========================
**    Request nodemask containing virtual nodes in controller bridge.
**
**    The nodemask points to nodemask structure where the current virtual node
**    nodemask is to be copied.
**
**--------------------------------------------------------------------------*/
void
ZW_GetVirtualNodes(
  BYTE *nodeMask);
#endif /* _ZW_CONTROLLER_BRIDGE_API_H_ */

