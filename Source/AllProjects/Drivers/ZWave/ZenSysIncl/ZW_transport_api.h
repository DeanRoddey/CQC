/*******************************  ZW_TRANSPORT_API.H  ***********************
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
 * Description: Z-Wave Transport Application layer interface
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: efh $
 * Revision:         $Revision: 1.59 $
 * Last Changed:     $Date: 2007/03/05 11:18:40 $
 *
 ****************************************************************************/
#ifndef _ZW_TRANSPORT_API_H_
#define _ZW_TRANSPORT_API_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* Max number of nodes in a Z-wave system */
#define ZW_MAX_NODES        232

/************************************************************/
/* Node Information frame*/
/************************************************************/
#define NODEPARM_MAX  20   /* max. number of parameters */


/* Transmit frame option flags */
#define TRANSMIT_OPTION_ACK           0x01    /* request acknowledge from destination node */
#define TRANSMIT_OPTION_LOW_POWER     0x02    /* transmit at low output power level (1/3 of normal RF range)*/
#ifdef ZW_SLAVE
#define TRANSMIT_OPTION_RETURN_ROUTE  0x04    /* request transmission via return route */
#endif
#ifdef ZW_CONTROLLER
#define TRANSMIT_OPTION_AUTO_ROUTE    0x04    /* request retransmission via repeater nodes */
#endif
/* do not use response route - Even if available */
#define TRANSMIT_OPTION_NO_ROUTE      0x10
#define TRANSMIT_OPTION_EXPLORE     0x20;

/* speedoptions for ZW_SendData */
#define TRANSMIT_SPEED_OPTION_BAUD_AUTO         0x00
#define TRANSMIT_SPEED_OPTION_BAUD_9600         0x01
#define TRANSMIT_SPEED_OPTION_BAUD_40000        0x02
#define TRANSMIT_SPEED_OPTION_BAUD_RESERVED     0x04
#define TRANSMIT_SPEED_OPTION_BEAM_AUTO         0x08
#define TRANSMIT_SPEED_OPTION_BEAM_250MS        0x08
#define TRANSMIT_SPEED_OPTION_BEAM_1000MS       0x10
#define TRANSMIT_SPEED_OPTION_BEAM_RESERVED     0x18

/* Received frame status flags */
#define RECEIVE_STATUS_ROUTED_BUSY    0x01
#define RECEIVE_STATUS_LOW_POWER      0x02    /* received at low output power level, this must */
                                              /* have the same value as TRANSMIT_OPTION_LOW_POWER */
#define RECEIVE_STATUS_TYPE_MASK      0x0C    /* Mask for masking out the received frametype bits */
#define RECEIVE_STATUS_TYPE_SINGLE    0x00    /* Received frame is singlecast frame (rxOptions == xxxx00xx) */
#define RECEIVE_STATUS_TYPE_BROAD     0x04    /* Received frame is broadcast frame  (rxOptions == xxxx01xx) */
#define RECEIVE_STATUS_TYPE_MULTI     0x08    /* Received frame is multiecast frame (rxOptions == xxxx10xx) */
#define RECEIVE_STATUS_FOREIGN_FRAME   0x40  /* Received frame is not send t to me (useful only in promiscuous mode) */

/* Predefined Node ID's */
#define NODE_BROADCAST              0xFF    /* broadcast */
#define ZW_TEST_NOT_A_NODEID        0x00    /* */

/* Transmit complete codes */
#define TRANSMIT_COMPLETE_OK      0x00
#define TRANSMIT_COMPLETE_NO_ACK  0x01  /* retransmission error */
#define TRANSMIT_COMPLETE_FAIL    0x02  /* transmit error */
#define TRANSMIT_ROUTING_NOT_IDLE 0x03  /* transmit error */
#ifdef ZW_CONTROLLER
/* Assign route transmit complete but no routes was found */
#define TRANSMIT_COMPLETE_NOROUTE 0x04  /* no route found in assignroute */
                                        /* therefore nothing was transmitted */
#endif

/* ZW_REDISCOVERY_NEEDED callback values.
   Note that they are different from ZW_REQUEST_NETWORK_UPDATE callbacks*/
#define ZW_ROUTE_LOST_FAILED      0x04  /*Node Asked wont help us*/
#define ZW_ROUTE_LOST_ACCEPT      0x05  /*Accepted to help*/



#if defined(ZW_SLAVE_ROUTING) || defined(ZW_CONTROLLER)
#define ZW_MAX_RETURN_ROUTE_DESTINATIONS   5
#endif


#ifdef ZW_SLAVE_ROUTING
/*============================   ZW_RouteDestinations   ======================
**    Function description
**      This contains a list of nodes that currently can be reached via
**      return routes.
**      This list should not be altered by Applications.s
**
**--------------------------------------------------------------------------*/
extern IBYTE ZW_RouteDestinations[ZW_MAX_RETURN_ROUTE_DESTINATIONS];
#endif

/*===============================   ZW_SendData   ===========================
**    Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast).
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER   transmit at low output power level (1/3 of
**                                      normal RF range).
**          TRANSMIT_OPTION_ACK         the multicast frame will be followed by a
**                                      singlecast frame to each of the destination nodes
**                                      and request acknowledge from each destination node.
**          TRANSMIT_OPTION_AUTO_ROUTE  request retransmission on singlecast frames
**                                      via repeater nodes (at normal output power level).
**
** extern BYTE            RET  FALSE if transmitter queue overflow
** ZW_SendData(
** BYTE  nodeID,          IN  Destination node ID (0xFF == broadcast)
** BYTE *pData,           IN  Data buffer pointer
** BYTE  dataLength,      IN  Data buffer length
** BYTE  txOptions,       IN  Transmit option flags
** void (*completedFunc)( IN  Transmit completed call back function
**        BYTE txStatus));    IN Transmit status
**--------------------------------------------------------------------------*/
#define ZW_SEND_DATA(node,data,length,options,func) ZW_SendData(node,data,length,options,func)


#if defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING)
/*============================   ZW_SendDataAbort   ========================
**    Abort the ongoing transmit started with ZW_SendData()
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_SEND_DATA_ABORT() ZW_SendDataAbort()


/*===============================   ZW_SendDataMulti   ======================
**    Transmit data buffer to a list of Z-Wave Nodes (multicast frame).
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER   transmit at low output power level (1/3 of
**                                      normal RF range).
**          TRANSMIT_OPTION_ACK         the multicast frame will be followed by a
**                                      singlecast frame to each of the destination nodes
**                                      and request acknowledge from each destination node.
**          TRANSMIT_OPTION_AUTO_ROUTE  request retransmission on singlecast frames
**                                      via repeater nodes (at normal output power level).
**
** extern BYTE            RET  FALSE if transmitter queue overflow
** ZW_SendDataMulti(
**  BYTE *pNodeIDList,     IN  List of destination node ID's
**  BYTE  numberNodes,     IN  Number of Nodes
**  BYTE *pData,           IN  Data buffer pointer
**  BYTE  dataLength,      IN  Data buffer length
**  BYTE  txOptions,       IN  Transmit option flags
**  VOID_CALLBACKFUNC(completedFunc)( IN  Transmit completed call back function
**    BYTE txStatus)); IN Transmit status
**--------------------------------------------------------------------------*/
#ifdef EFH_MULTICAST_OPTIMIZATIONS
#define ZW_SEND_DATA_MULTI(nodelist,data,length,options,func) ZW_SendDataMulti(nodelist,data,length,options,func)
#else
#define ZW_SEND_DATA_MULTI(nodelist,numnodes,data,length,options,func) ZW_SendDataMulti(nodelist,numnodes,data,length,options,func)
#endif
#endif  /* defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING) */

/*============================   ZW_SEND_CONST =============================
**    Function description
**      If production test is enabled during start up.
**      Calling this function will transmit a constant signal until a new
**      RF function is called
**
**--------------------------------------------------------------------------*/
#define ZW_SEND_CONST() ZW_SendConst()

/*============================ ZW_LOCK_RESPONSE_ROUTE ========================
**    Function description
**      This function locks and unlocks all return routes
**      IN  nodeID  != 0x00 lock route to node
**          nodeDI == 0x00 unlock entry
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_LOCK_RESPONSE_ROUTE(node) ZW_LockRoute(node)

#if defined(ZW020x) || defined(ZW030x)

#if defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING)
/*===============================   ZW_SEND_DATA_META ==========================
**    Transmit Meta data buffer to a single Z-Wave node or all Z-Wave nodes
**    Only allowed for 40kbit nodes. If a 9600 destination is selected FALSE will
**    be returned. Parameters and return values identical with ZW_SEND_DATA
**    DOES NOT EXIST ON ZW010x targets
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_SEND_DATA_META(destId,data,len,txOptions,func) ZW_SendDataMeta(destId,data,len,txOptions,func)
#endif
#endif /*ZW020x*/

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/
/*============================   ZW_LockRoutes   ============================
**    Function description
**      This function locks and unlocks any temporary route to a specific nodeID
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_LockRoute(
  BYTE bNodeID);          /*IN TRUE lock nodeID entry, FALSE unlock entry*/


/*============================   ZW_SendConst  =============================
**    Function description
**      If production test is enabled it will transmit a constant signal
**      until a new RF function is called
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_SendConst(void);


/*===============================   ZW_SendData   ===========================
**    Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast).
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER     transmit at low output power level
**                                        (1/3 of normal RF range).
**          TRANSMIT_OPTION_ACK           the multicast frame will be followed
**                                        by a  singlecast frame to each of
**                                        the destination nodes
**                                        and request acknowledge from each
**                                        destination node.
**          TRANSMIT_OPTION_AUTO_ROUTE    request retransmission via repeater
**                                        nodes at normal output power level).
**          TRANSMIT_OPTION_RETURN_ROUTE  transmission via return route, if
**                                        no return route is registred then
**                                        the frame will be send as a
**                                        broadcast.
**
** extern BYTE            RET  FALSE if transmitter queue overflow
** ZW_SendData(
** BYTE  nodeID,          IN  Destination node ID (0xFF == broadcast)
** BYTE *pData,           IN  Data buffer pointer
** BYTE  dataLength,      IN  Data buffer length
** BYTE  txOptions,       IN  Transmit option flags
** void (*completedFunc)( IN  Transmit completed call back function
**        BYTE txStatus));    IN Transmit status
**--------------------------------------------------------------------------*/
extern BYTE            /*RET  FALSE if transmitter busy      */
ZW_SendData(
  BYTE  nodeID,               /*IN  Destination node ID (0xFF == broadcast) */
  BYTE *pData,                /*IN  Data buffer pointer           */
  BYTE  dataLength,           /*IN  Data buffer length            */
  BYTE  txOptions,            /*IN  Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /*IN  Transmit completed call back function  */


#if defined(ZW020x) || defined(ZW030x)
#if defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING)
/*===============================   ZW_SendDataMeta ==========================
**    Transmit Meta data buffer to a single Z-Wave node or all Z-Wave nodes
**    Only allowed for 40kbit nodes. If a 9600 destination is selected FALSE will
**    be returned. Parameters and return values identical with ZW_SEND_DATA
**    DOES NOT EXIST ON ZW010x targets
**    Side effects:
**
**--------------------------------------------------------------------------*/

BYTE ZW_SendDataMeta(  BYTE  destNodeID, /* IN Destination node ID - 0xFF == all nodes */
  BYTE *pData,      /* IN Data buffer pointer           */
  BYTE  dataLength, /* IN Data buffer length            */
  BYTE  txOptions,  /* IN Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /* IN Transmit completed call back function  */
#endif
#endif /*ZW020x*/

#if defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING)

/*============================   ZW_SendDataAbort   ========================
**    Abort the ongoing transmit started with ZW_SendData()
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void                /*RET FALSE if transmitter busy      */
ZW_SendDataAbort(void);

/*===============================   ZW_SendDataMulti   ======================
**    Transmit data buffer to a list of Z-Wave Nodes (multicast frame).
**
**
**    txOptions:
**          TRANSMIT_OPTION_LOW_POWER   transmit at low output power level (1/3 of
**                                      normal RF range).
**          TRANSMIT_OPTION_ACK         the multicast frame will be followed by a
**                                      singlecast frame to each of the destination nodes
**                                      and request acknowledge from each destination node.
**          TRANSMIT_OPTION_AUTO_ROUTE  request retransmission on singlecast frames
**                                      via repeater nodes (at normal output power level).
**
**--------------------------------------------------------------------------*/
extern BYTE            /*RET  FALSE if transmitter busy      */
ZW_SendDataMulti(
  BYTE *pNodeIDList,          /*IN  List of destination node ID's */
#ifndef EFH_MULTICAST_OPTIMIZATIONS
  BYTE  numberNodes,          /*IN  Number of Nodes               */
#endif
  BYTE *pData,                /*IN  Data buffer pointer           */
  BYTE  dataLength,           /*IN  Data buffer length            */
  BYTE  txOptions,            /*IN  Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /*IN  Transmit completed call back function  */
#endif /* defined(ZW_CONTROLLER) || defined(ZW_SLAVE_ROUTING) */


#endif /* _ZW_TRANSPORT_API_H_ */

