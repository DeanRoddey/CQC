/*******************************  ZW_SLAVE_ROUTING_API.H  *******************************
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
 * Description: Z-Wave Routing Slave node application interface
 *
 * Author:   Johann Sigfredsson
 *
 * Last Changed By:  $Author: sse $
 * Revision:         $Revision: 1.11 $
 * Last Changed:     $Date: 2006/05/02 11:53:10 $
 *
 ****************************************************************************/
#ifndef _ZW_SLAVE_ROUTING_API_H_
#define _ZW_SLAVE_ROUTING_API_H_

#ifndef ZW_SLAVE_ROUTING
#define ZW_SLAVE_ROUTING
#endif

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
/* Include the basic slave functionality */
#include <ZW_slave_api.h>

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/


/* ZW_REQUEST_NETWORK_UPDATE callback values */
#define ZW_ROUTE_UPDATE_DONE      0x00
#define ZW_ROUTE_UPDATE_ABORT     0x01
#define ZW_ROUTE_UPDATE_WAIT      0x02
#define ZW_ROUTE_UPDATE_DISABLED  0x03


/*==================   ZW_REQUEST_NEW_ROUTE_DESTINATIONS ====================
**    Request new destinations for return routes.
**      list = pointer to array of node ids
**      len = length of nodeID array (Max len =
**            ZW_MAX_RETURN_ROUTE_DESTINATIONS
**      func = callback, called with status when operation is done.
**             status can be one of the following:
**
**    ZW_ROUTE_UPDATE_DONE		  - The update process is ended successfully.
**    ZW_ROUTE_UPDATE_ABORT		  - The update process aborted because of error.
**    ZW_ROUTE_UPDATE_WAIT		  - The SUC node is busy.
**    ZW_ROUTE_UPDATE_DISABLED	- The SUC functionality is disabled.
**
**--------------------------------------------------------------------------*/
#define ZW_REQUEST_NEW_ROUTE_DESTINATIONS(list,len,func) ZW_RequestNewRouteDestinations(list,len,func)

/*==================   ZW_REQUEST_NEW_ROUTE_DESTINATIONS ====================
** This function request network update from the Static Update Controller
**  Callback status will be one of following
**    ZW_ROUTE_UPDATE_DONE		  - The update process is ended successfully.
**    ZW_ROUTE_UPDATE_ABORT		  - The update process aborted because of error.
**    ZW_ROUTE_UPDATE_WAIT		  - The SUC node is busy.
**    ZW_ROUTE_UPDATE_DISABLED	- The SUC functionality is disabled.

------------------------------------------------------------------------*/
#define ZW_REQUEST_NETWORK_UPDATE(FUNC) ZW_RequestNetWorkUpdate(FUNC)

/*============================   ZW_RediscoveryNeeded   ======================
**    Function description
**      Application want a rediscovery, but is lost.
**      Try to ask the nodeID supplied for help.
**      Callback function returns:
**          ZW_ROUTE_LOST_FAILED  - if node nodeid is unable to help
**          ZW_ROUTE_LOST_ACCEPT  - if node nodeid contacted the controller.
**          ZW_ROUTE_UPDATE_DONE  - the rediscovery ended successfully.
**          ZW_ROUTE_UPDATE_ABORT - the node nodeid did not answer.
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REDISCOVERY_NEEDED(nodeid,func) ZW_RediscoveryNeeded(nodeid,func)

/*====================  ZW_IS_NODE_WITHIN_DIRECT_RANGE =====================
**    Test if ReturnRouted indicate that bNodeID is within direct range.
**
**--------------------------------------------------------------------------*/
#define ZW_IS_NODE_WITHIN_DIRECT_RANGE(nid) ZW_IsNodeWithinDirectRange(nid)

/*============================   ZW_RequestNodeInfo   ======================
**    Function description.
**     Request a node to send it's node information.
**     Function return TRUE if the request is send, else it return FALSE.
**     FUNC is a callback function, which is called with the status of the
**     Request nodeinformation frame transmission.
**     If a node sends its node info, ApplicationSlaveUpdate will be called
**     with UPDATE_STATE_NODE_INFO_RECEIVED as status together with the received
**     nodeinformation.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REQUEST_NODE_INFO(NODEID, FUNC)  ZW_RequestNodeInfo(NODEID, FUNC)


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/*                 Implemented within the Z-Wave moduls                     */
/****************************************************************************/

/*======================  ZW_AreNodeWithinDirectRange   ======================
**    Test if ReturnRouted indicate that bNodeID is within direct range.
**
**--------------------------------------------------------------------------*/
BOOL                          /*RET TRUE if neighbours, FALSE if not*/
ZW_IsNodeWithinDirectRange(
  BYTE bNodeID);              /*IN nodeID to check*/
/*=======================   ZW_RequestNewRouteDestinations   =================
**    Request new destinations for return routes.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
BOOL                                                /*RET TRUE if SUC/SIS exist FALSE if not*/
ZW_RequestNewRouteDestinations(
  BYTE_P destList,                                  /*IN Pointer to new destinations*/
  BYTE destListLen,                                 /*IN len of buffer */
  VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus)); /* IN callback function called when completed*/

/*----------------------------------------------------------------------------
 This function resquest network update from the Static Update Controller
----------------------------------------------------------------------------*/
BYTE                      /* RET: True; SUC is known to the controller,  */
                          /*      FALSE; SUC not known to the controller */
ZW_RequestNetWorkUpdate(
  VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus)); /* IN call back function indicates of the update sucessed or failed*/

/*============================   ZW_RediscoveryNeeded   ======================
**    Function description
**      Application thinks that a rediscovery is needed.
**      Try to reach the nodeID supplied
**    Side effects:
**
**--------------------------------------------------------------------------*/
BYTE                                                /*RET TRUE if success*/
ZW_RediscoveryNeeded(
  BYTE bNodeID,                                     /*IN nodeID to ask for help*/
  VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus));  /*IN callback function*/

/*============================   ZW_RequestNodeInfo   ======================
**    Function description.
**     Request a node to send it's node information.
**     Function return TRUE if the request is send, else it return FALSE.
**     FUNC is a callback function, which is called with the status of the
**     Request nodeinformation frame transmission.
**     If a node sends its node info, ApplicationSlaveUpdate will be called
**     with UPDATE_STATE_NODE_INFO_RECEIVED as status together with the received
**     nodeinformation.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
BOOL                      /*RET FALSE if transmitter busy */
ZW_RequestNodeInfo(
  BYTE nodeID,                     /*IN: node id of the node to request node info from it.*/
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /* IN Callback function */


#endif /* _ZW_SLAVE_ROUTING_API_H_ */

