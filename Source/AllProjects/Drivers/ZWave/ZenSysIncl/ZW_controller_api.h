/*******************************  ZW_CONTROLLER_API.H  **********************
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
 * Last Changed By:  $Author: efh $
 * Revision:         $Revision: 1.123 $
 * Last Changed:     $Date: 2007/03/05 11:18:40 $
 *
 ****************************************************************************/
#ifndef _ZW_CONTROLLER_API_H_
#define _ZW_CONTROLLER_API_H_

#ifndef ZW_CONTROLLER
#define ZW_CONTROLLER
#endif

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
/* These are a part of the standard controller API */
#ifndef WIN32
#include <ZW_basis_api.h>
#include <ZW_nodemask_api.h>
#ifdef ZW010x
#ifndef ZW_CONTROLLER_BRIDGE
#ifndef ZW_REDUCED_STATIC
#include <ZW_rtc_api.h>
#endif
#endif
#endif
#endif

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/* Mode parameters to ZW_AddNodeToNetwork */
#define ADD_NODE_ANY          1
#define ADD_NODE_CONTROLLER   2
#define ADD_NODE_SLAVE        3
#define ADD_NODE_EXISTING     4
#define ADD_NODE_STOP         5
#define ADD_NODE_STOP_FAILED  6

#define ADD_NODE_MODE_MASK                   0x0F
#define ADD_NODE_OPTION_HIGH_POWER           0x80

/* Callback states from ZW_AddNodeToNetwork */
#define ADD_NODE_STATUS_LEARN_READY          1
#define ADD_NODE_STATUS_NODE_FOUND           2
#define ADD_NODE_STATUS_ADDING_SLAVE         3
#define ADD_NODE_STATUS_ADDING_CONTROLLER    4
#define ADD_NODE_STATUS_PROTOCOL_DONE        5
#define ADD_NODE_STATUS_DONE                 6
#define ADD_NODE_STATUS_FAILED               7

/* Mode parameters to ZW_RemoveNodeFromNetwork */
#define REMOVE_NODE_ANY                     ADD_NODE_ANY
#define REMOVE_NODE_CONTROLLER              ADD_NODE_CONTROLLER
#define REMOVE_NODE_SLAVE                   ADD_NODE_SLAVE
#define REMOVE_NODE_STOP                    ADD_NODE_STOP

/* Callback states from ZW_RemoveNodeFromNetwork */
#define REMOVE_NODE_STATUS_LEARN_READY          ADD_NODE_STATUS_LEARN_READY
#define REMOVE_NODE_STATUS_NODE_FOUND           ADD_NODE_STATUS_NODE_FOUND
#define REMOVE_NODE_STATUS_REMOVING_SLAVE       ADD_NODE_STATUS_ADDING_SLAVE
#define REMOVE_NODE_STATUS_REMOVING_CONTROLLER  ADD_NODE_STATUS_ADDING_CONTROLLER
#define REMOVE_NODE_STATUS_DONE                 ADD_NODE_STATUS_DONE
#define REMOVE_NODE_STATUS_FAILED               ADD_NODE_STATUS_FAILED

/* Mode parameters to ZW_CreateNewPrimary */
#define CREATE_PRIMARY_START                ADD_NODE_CONTROLLER
#define CREATE_PRIMARY_STOP                 ADD_NODE_STOP
#define CREATE_PRIMARY_STOP_FAILED          ADD_NODE_STOP_FAILED

/* Mode parameters to ZW_ControllerChange */
#define CONTROLLER_CHANGE_START             ADD_NODE_CONTROLLER
#define CONTROLLER_CHANGE_STOP              ADD_NODE_STOP
#define CONTROLLER_CHANGE_STOP_FAILED       ADD_NODE_STOP_FAILED

/* Callback states from ZW_SetLearnMode */
#define LEARN_MODE_STARTED                  ADD_NODE_STATUS_LEARN_READY
#define LEARN_MODE_DONE                     ADD_NODE_STATUS_DONE
#define LEARN_MODE_FAILED                   ADD_NODE_STATUS_FAILED
#define LEARN_MODE_DELETED                  0x80

/* Callback states from ZW_REQUEST_NODE_NEIGHBOR_UPDATE */
#define REQUEST_NEIGHBOR_UPDATE_STARTED     0x21
#define REQUEST_NEIGHBOR_UPDATE_DONE        0x22
#define REQUEST_NEIGHBOR_UPDATE_FAILED      0x23

/* ApplicationcControllerUpdate status */
#define UPDATE_STATE_NODE_INFO_RECEIVED     0x84
#define UPDATE_STATE_NODE_INFO_REQ_DONE     0x82
#define UPDATE_STATE_NODE_INFO_REQ_FAILED   0x81
#define UPDATE_STATE_ROUTING_PENDING        0x80
#define UPDATE_STATE_ADD_DONE               0x40
#define UPDATE_STATE_DELETE_DONE            0x20
#define UPDATE_STATE_SUC_ID                 0x10

/* ZW_GetNeighborCount special return values */
#define NEIGHBORS_ID_INVALID          0xFE
#define NEIGHBORS_COUNT_FAILED        0xFF  /* Could not access routing info try again later */

/* RemoveFailedNode returns values */
#define  NOT_PRIMARY_CONTROLLER             1 /* The removing process was */
                                              /* aborted because the controller */
                                              /* is not the primary one */

#define  NO_CALLBACK_FUNCTION               2 /* The removing process was */
                                              /* aborted because no call back */
                                              /* function is used */
#define  FAILED_NODE_NOT_FOUND              3 /* The removing process aborted */
                                              /* because the node was node */
                                              /* found */
#define  FAILED_NODE_REMOVE_PROCESS_BUSY    4 /* The removing process is busy */
#define  FAILED_NODE_REMOVE_FAIL            5 /* The removing process could not */
                                              /* be started */

#define ZW_FAILED_NODE_REMOVE_STARTED       0 /* The removing process started */
#define ZW_NOT_PRIMARY_CONTROLLER           (1 << NOT_PRIMARY_CONTROLLER)
#define ZW_NO_CALLBACK_FUNCTION             (1 << NO_CALLBACK_FUNCTION)
#define ZW_FAILED_NODE_NOT_FOUND            (1 << FAILED_NODE_NOT_FOUND)
#define ZW_FAILED_NODE_REMOVE_PROCESS_BUSY  (1 << FAILED_NODE_REMOVE_PROCESS_BUSY)
#define ZW_FAILED_NODE_REMOVE_FAIL          (1 << FAILED_NODE_REMOVE_FAIL)


/*     RemoveFailed node call back status*/
#define ZW_NODE_OK                          0 /* The node is working properly (removed from the failed nodes list ) */
#define ZW_FAILED_NODE_REMOVED              1 /* The failed node was removed from the failed nodes list */
#define ZW_FAILED_NODE_NOT_REMOVED          2 /* The failed node was not removed from the failing nodes list */
#define ZW_FAILED_NODE_REPLACE              3 /* The failed node are ready to be replaced and controller */
                                              /* is ready to add new node with nodeID of the failed node */
#define ZW_FAILED_NODE_REPLACE_DONE         4 /* The failed node has been replaced */
#define ZW_FAILED_NODE_REPLACE_FAILED       5 /* The failed node has not been replaced */

/* ZW_RequestNetworkUpdate callback values*/
#define ZW_SUC_UPDATE_DONE      0x00
#define ZW_SUC_UPDATE_ABORT     0x01
#define ZW_SUC_UPDATE_WAIT      0x02


#define ZW_SUC_UPDATE_DISABLED  0x03
#define ZW_SUC_UPDATE_OVERFLOW  0x04

#define ZW_SUC_SET_SUCCEEDED    0x05
#define ZW_SUC_SET_FAILED       0x06

/* SUC capabilities used in ZW_EnableSUC and ZW_SetSUCNodeID */
#define ZW_SUC_FUNC_BASIC_SUC       0x00
#define ZW_SUC_FUNC_NODEID_SERVER   0x01

/* Defines for ZW_GetControllerCapabilities */
#define CONTROLLER_IS_SECONDARY                 0x01
#define CONTROLLER_ON_OTHER_NETWORK             0x02
#define CONTROLLER_NODEID_SERVER_PRESENT        0x04
#define CONTROLLER_IS_REAL_PRIMARY              0x08
#define CONTROLLER_IS_SUC                       0x10

#ifndef WIN32
/* Node type structure present in NODEINFO */
typedef struct _NODE_TYPE_
{
  BYTE basic;                 /* Basic Device Type, is it a Controller, Controller_Static, */
                              /* Slave or a Slave_Routing Device Type */
  BYTE generic;               /* Generic Device Type */
  BYTE specific;              /* Specific Device Type */
} NODE_TYPE;


/* Node info stored within the non-volatile memory */
/* This are the first (protocol part) payload bytes from the Node Infomation frame */
typedef struct _NODEINFO_
{
  BYTE        capability;     /* Network capabilities */
  BYTE        security;       /* Network security */
  BYTE        reserved;
  NODE_TYPE   nodeType;       /* Basic, Generic and Specific Device types - Basic is generated... */
} NODEINFO;
#endif

/* Listening bit in the NODEINFO capability byte */
#define NODEINFO_LISTENING_SUPPORT          0x80
/* Routing bit in the NODEINFO capability byte */
#define NODEINFO_ROUTING_SUPPORT            0x40

/* Optional functionality bit in the NODEINFO security byte*/
#define NODEINFO_OPTIONAL_FUNC_SUPPORT      0x10

/* Learn node state information passed by the call back function */
typedef struct _LEARN_INFO_
{
  BYTE  bStatus;      /* Status of learn mode */
  BYTE  bSource;      /* Node id of the node that send node info */
  BYTE  *pCmd;        /* Pointer to Application Node information */
  BYTE  bLen;         /* Node info length                        */
} LEARN_INFO;

#ifndef WIN32
/****************************  Z-Wave Basis API *****************************
* Functionality specific for the controller API.
****************************************************************************/

/*========================   ZW_GetNodeProtocolInfo   =======================
**
**    Copy the Node's current protocol information from the non-volatile
**    memory.
**
** void           RET  Nothing
**  ZW_GetNodeProtocolInfo(
**    BYTE  nodeID,        IN Node ID
**    NODEINFO *nodeInfo); OUT Node info buffer
**--------------------------------------------------------------------------*/
#define ZW_GET_NODE_STATE(nodeID, nodeInfo) ZW_GetNodeProtocolInfo(nodeID, nodeInfo)


/*========================   ZW_AssignReturnRoute   =========================
**
**    Assign static return routes within a Routing Slave node.
**    Calculate the shortest transport routes from the Routing Slave node
**    to the route destination node and
**    transmit the return routes to the Routing Slave node.
**
** BOOL                       RET TRUE if assign return route was initiated.
**                                FALSE if a return route assign/delete is allready active
** ZW_AssignReturnRoute(
**  BYTE  bSrcNodeID,         IN Routing Slave Node ID
**  BYTE  bDstNodeID,         IN Route destination Node ID
**  VOID_CALLBACKFUNC(completedFunc)(
**    BYTE bStatus)); IN  Status of process
**--------------------------------------------------------------------------*/
#define ZW_ASSIGN_RETURN_ROUTE(routingNodeID, destNodeID, func) ZW_AssignReturnRoute(routingNodeID, destNodeID, func)


/*========================   ZW_AssignSUCReturnRoute   =========================
**
**    Assign static return routes within a Routing Slave node.
**    Calculate the shortest transport routes to a Routing Slave node
**    from the Static Update Controller Node and
**    transmit the return routes to the Routing Slave node.
**
** BOOL                       RET TRUE if assign SUC return route was initiated.
**                                FALSE if a return route assign/delete is allready active
** ZW_AssignSUCReturnRoute(
**  BYTE  bSrcNodeID,         IN Routing Slave Node ID
**  VOID_CALLBACKFUNC(completedFunc)(
**    BYTE bStatus)); IN  Status of process
**--------------------------------------------------------------------------*/
#define ZW_ASSIGN_SUC_RETURN_ROUTE(routingNodeID, func) ZW_AssignSUCReturnRoute(routingNodeID, func)

/*========================   ZW_DeleteSUCReturnRoute   =========================
**
**    Delete the (Static Update Controller - SUC-) static return routes
**    within a Routing Slave node.
**    Transmit "NULL" return routes to the Routing Slave node.
**
** BOOL                       RET TRUE if delete SUC return route was initiated.
**                                FALSE if a return route assign/delete is allready active
** ZW_DeleteSUCReturnRoute(
**  BYTE  nodeID,         IN Routing Slave
**  VOID_CALLBACKFUNC(completedFunc)(
**    BYTE bStatus)); IN  Transmit complete status
**--------------------------------------------------------------------------*/
#define ZW_DELETE_SUC_RETURN_ROUTE(nodeID, func) ZW_DeleteSUCReturnRoute(nodeID, func)


/*========================   ZW_DeleteReturnRoute   =========================
**
**    Delete static return routes within a Routing Slave node.
**    Transmit "NULL" return routes to the Routing Slave node.
**
** BOOL                       RET TRUE if delete return route was initiated.
**                                FALSE if a return route assign/delete is allready active
** ZW_DeleteReturnRoute(
**  BYTE  nodeID,         IN Routing Slave
**  VOID_CALLBACKFUNC(completedFunc)(
**    BYTE bStatus)); IN  Transmit complete status
**--------------------------------------------------------------------------*/
#define ZW_DELETE_RETURN_ROUTE(nodeID, func) ZW_DeleteReturnRoute(nodeID, func)


/*===========================   ZW_SetDefault   ================================
**    Remove all Nodes and timers from the EEPROM memory.
**
** void           RET  Nothing
** SetDefault(
**  VOID_CALLBACKFUNC(completedFunc)(void)); IN  Command completed call back function
**--------------------------------------------------------------------------*/
#define ZW_SET_DEFAULT(func) ZW_SetDefault(func)


/****************************************************************************
** Z-Wave Transport Application layer interface functions specific for the
** controller.
**/

/*===================   ZW_REPLICATION_COMMAND_COMPLETE =====================
**    Sends command completed to master remote. Called in replication mode
**    when a command from the sender has been processed.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REPLICATION_COMMAND_COMPLETE() ZW_ReplicationReceiveComplete()


/*======================   ZW_REPLICATION_SEND_DATA   ======================
**    Used when the controller is replication mode.
**    It sends the payload and expects the receiver to respond with a
**    command complete message.
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REPLICATION_SEND_DATA(node,data,length,options,func) ZW_ReplicationSend(node,data,length,options,func)


/*==========================   ZW_GetFailedNode   ===============================
**
**    Check if a node failed is in the failed nodes table
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_IS_FAILED_NODE_ID(NODEID)       (ZW_isFailedNode(NODEID) )


/*==========================   ZW_RemoveFailedNode   ===============================
**
**    remove a node from the failed node list, if it already exist.
**    A call back function should be provided otherwise the function will return
**    without removing the node.
**    If the removing process started successfully then the function will return
**    ZW_FAILED_NODE_REMOVE_STARTED        The removing process started
**
**    If the removing process can not be started then the API function will return
**    on or more of the following flags
**    ZW_NOT_PRIMARY_CONTROLLER             The removing process was aborted because the controller is not the primaray one
**    ZW_NO_CALLBACK_FUNCTION              The removing process was aborted because no call back function is used
**    ZW_FAILED_NODE_NOT_FOUND             The removing process aborted because the node was node found
**    ZW_FAILED_NODE_REMOVE_PROCESS_BUSY   The removing process is busy
**
**    The call back function parameter value is:
**
**    ZW_NODE_OK                     The node is working proppely (removed from the failed nodes list )
**    ZW_FAILED_NODE_REMOVED         The failed node was removed from the failed nodes list
**    ZW_FAILED_NODE_NOT_REMOVED     The failed node was not
**    Side effects:
**--------------------------------------------------------------------------*/
#define ZW_REMOVE_FAILED_NODE_ID(NODEID,FUNC)  (ZW_RemoveFailedNode(NODEID,FUNC))


/*=========================   ZW_ReplaceFailedNode   ==========================
**
**    Replace a node from the failed node list.
**    A call back function should be provided otherwise the function will return
**    without replacing the node.
**    If the replacing process started successfully then the function will return
**    ZW_FAILED_NODE_REPLACE         The replacing process started and now the new
**                                   node must emit its nodeinformation frame to
**                                   start the assign process
**
**    If the replace process can not be started then the API function will return
**    on or more of the following flags
**
**    ZW_NOT_PRIMARY_CONTROLLER           The replacing process was aborted because
**                                        the controller is not the primary controller
**    ZW_NO_CALLBACK_FUNCTION             The replacing process was aborted because no
**                                        call back function is used
**    ZW_FAILED_NODE_NOT_FOUND            The replacing process aborted because
**                                        the node was node found
**    ZW_FAILED_NODE_REMOVE_PROCESS_BUSY  The replacing process is busy
**    ZW_FAILED_NODE_REMOVE_FAIL          The replacing process could not be started
**                                        because of
**
**    The call back function parameter value is:
**
**    ZW_FAILED_NODE_REPLACE         The failed node are ready to be replaced and controller
**                                   is ready to add new node with nodeID of the failed node
**    ZW_FAILED_NODE_REPLACE_DONE    The failed node has been replaced
**    ZW_FAILED_NODE_REPLACE_FAILED  The failed node has not been replaced
**    Side effects:
**--------------------------------------------------------------------------*/
#define ZW_REPLACE_FAILED_NODE(NODEID,FUNC)  (ZW_ReplaceFailedNode(NODEID,FUNC))


/*============================   ZW_SetSUCNodeID  ===========================
**    Function description
**    This function enables/disables a specified static controllers
**    Static Update Controller functionality
**
**--------------------------------------------------------------------------*/
#define ZW_SET_SUC_NODEID(NODEID, SUC_STATE, TX_OPTION, CAPABILITIES, FUNC) (ZW_SetSUCNodeID(NODEID, SUC_STATE, TX_OPTION, CAPABILITIES, FUNC))


/*============================   ZW_SendSUCID   ===============================
**    Function description
**      Transmits SUC node id to the node specified. Only allowed from Primary or SUC
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_SEND_SUC_ID(node,txOption, callback) ZW_SendSUCID(node,txOption, callback)



/*===========================   ZW_GET_SUC_NODEID  ===========================
**    Function description
**    This function gets the nodeID of the current Static Update Controller
**    if ZERO then no SUC is available
**
**--------------------------------------------------------------------------*/
#define ZW_GET_SUC_NODEID() ZW_GetSUCNodeID()


/*========================   ZW_REQUEST_NETWORK_UPDATE   ===================
**    Function description
**      This function resquest network update from the SUC
**      Returns: TRUE - SUC is known to this controller, FALSE - SUC unknown
**      FUNC is a call back function indicates of the update was a success
**      or failed
**    The call back function parameter value is:
**        ZW_SUC_UPDATE_DONE        The update process ended successfully
**        ZW_SUC_UPDATE_ABORT       The update process was aborted
**        ZW_SUC_UPDATE_WAIT        The SUC node is busy, try again later
**        ZW_SUC_UPDATE_DISABLED    The SUC functionality have been disabled
**        ZW_SUC_UPDATE_OVERFLOW    Too many changes to handle by automatic update
**--------------------------------------------------------------------------*/
#define ZW_REQUEST_NETWORK_UPDATE(FUNC) (ZW_RequestNetWorkUpdate(FUNC))


/*====================   ZW_RequestNodeNeighborUpdate  =======================
**
**    Request for an update of NodeID neighbors
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REQUEST_NODE_NEIGHBOR_UPDATE(NODEID,FUNC) ZW_RequestNodeNeighborUpdate(NODEID,FUNC)

/*============================   ZW_PRIMARYCTRL   =========================
**    Function description
**      Returns TRUE When the controller is a primary.
**              FALSE if it is a slave
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_PRIMARYCTRL() ZW_IsPrimaryCtrl()


/*========================   GetNeighborCount   =============================
**
**  Get number of neighbors the specified Node ID has registered
**
**  Returns:
**    0x00-0xE7               - Number of neighbors registered
**    NEIGHBORS_ID_INVALID    - specified node ID invalid
**    NEIGHBORS_COUNT_FAILED  - Could not access routeing information - try again later
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_GET_NEIGHBOR_COUNT(NODEID) ZW_GetNeighborCount(NODEID)

/*=====================   ZW_ARE_NODES_NEIGHBOURS   ============================
**
**  Are two specific nodes neighbours
**  returns TRUE if they are FALSE if not
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_ARE_NODES_NEIGHBOURS(a,b) ZW_AreNodesNeighbours(a,b)

/*============================   ZW_RequestNodeInfo   ======================
**    Function description.
**     Request a node to send it's node information.
**     Function return TRUE if the request is send, else it return FALSE.
**     FUNC is a callback function, which is called with the status of the
**     Request nodeinformation frame transmission.
**     If a node sends its node info, ApplicationControllerUpdate will be called
**     with UPDATE_STATE_NODE_INFO_RECEIVED as status together with the received
**     nodeinformation.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REQUEST_NODE_INFO(NODEID, FUNC)  ZW_RequestNodeInfo(NODEID, FUNC)


/*=====================   ZW_GetControllerCapabilities  ======================
**    Function description
**      Returns the SUC state
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_GET_CONTROLLER_CAPABILITIES ZW_GetControllerCapabilities

/*===========================   SetLearnMode   ==============================
**    Enable/Disable home/node ID learn mode.
**    When learn mode is enabled, received "Assign ID's Command" are handled:
**    If the current stored ID's are zero, the received ID's will be stored.
**    If the received ID's are zero the stored ID's will be set to zero.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_SET_LEARN_MODE(mode, func) ZW_SetLearnMode(mode, func)

/*==========================   ZW_AddNodeToNetwork   ========================
**
**    Add any type of node to the network
**
**    The modes are:
**
**    ADD_NODE_ANY            Add any node to the network
**    ADD_NODE_CONTROLLER     Add a controller to the network
**    ADD_NODE_SLAVE          Add a slaev node to the network
**    ADD_NODE_STOP           Stop learn mode without reporting an error.
**    ADD_NODE_STOP_FAILED    Stop learn mode and report an error to the
**                            new controller.
**
**    ADD_NODE_OPTION_HIGH_POWER    Set this flag in bMode for High Power inclusion.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_ADD_NODE_TO_NETWORK(mode, func) ZW_AddNodeToNetwork(mode, func)

/*==========================   ZW_RemoveNodeFromNetwork   ========================
**
**    Remove any type of node from the network
**
**    The modes are:
**
**    REMOVE_NODE_ANY            Remove any node from the network
**    REMOVE_NODE_CONTROLLER     Remove a controller from the network
**    REMOVE_NODE_SLAVE          Remove a slaev node from the network
**
**    REMOVE_NODE_STOP           Stop learn mode without reporting an error.
**
**    ADD_NODE_OPTION_HIGH_POWER    Set this flag in bMode for High Power exclusion.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_REMOVE_NODE_FROM_NETWORK(mode, func) ZW_RemoveNodeFromNetwork(mode, func)

/*========================   ZW_ControllerChange   ======================
**
**    Transfer the role as primary controller to another controller
**
**    The modes are:
**
**    CONTROLLER_CHANGE_START          Start the creation of a new primary
**    CONTROLLER_CHANGE_STOP           Stop the creation of a new primary
**    CONTROLLER_CHANGE_STOP_FAILED    Report that the replication failed
**
**    ADD_NODE_OPTION_HIGH_POWER       Set this flag in bMode for High Power exchange.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_CONTROLLER_CHANGE(mode, func) ZW_ControllerChange(mode, func)

/*========================   ZW_GET_ROUTING_INFO   ==========================
**
**  Get a list of neighbour information for a node in the routing table.
**
** IN   bNodeID         - NodeID to get the neighbour information on
** OUT  pMask           - Pointer to output byte buffer the buffer should be
**                        MAX_NODEMASK_LENGTH bytes long
** IN   bRemoveBad      - If TRUE Remove bad links from routing info
** IN   bRemoveNonReps  - If TRUE Remove non-repeaters from the routing info
**  Side effects:
**
**--------------------------------------------------------------------------*/
#define ZW_GET_ROUTING_INFO(bNodeID, pMask, bRemoveBad, bRemoveNonReps) ZW_GetRoutingInfo(bNodeID, pMask, bRemoveBad, bRemoveNonReps)

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/*                 Implemented within the application moduls                */
/****************************************************************************/


/*=====================   ApplictionControllerUpdate   =================
**   Inform a controller application that a node information is updated.
**   Called from the controller command handler when new node information
**   was added.
**
**--------------------------------------------------------------------------*/
extern void
ApplicationControllerUpdate(
  BYTE bStatus,     /*IN  Status of learn mode */
  BYTE bNodeID,     /*IN  Node id of the node that send node info */
  BYTE* pCmd,       /*IN  Pointer to Application Node information */
  BYTE bLen);       /*IN  Node info length                        */


/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/*                 Implemented within the Z-Wave controller modules         */
/****************************************************************************/
/*=========================   ZW_GetRoutingInfo   ==========================
**
**  Get a list of routing information for a node from the routing table.
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
void ZW_GetRoutingInfo( /* RET  Nothing */
  BYTE   bLine,           /* IN   Line number in routing table (node-1) */
  BYTE_P pMask,           /* OUT  Pointer where routing info should be put */
  BYTE   bRemoveBad,      /* IN   TRUE - Remove bad link from routing info */
  BYTE   bRemoveNonReps); /* IN   TRUE - Remove non-repeaters from the routing info */

/*===================   ZW_ReplicationReceiveComplete   ========================
**    Sends command completed to primary controller. Called in replication mode
**    when a command from the sender has been processed.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern void
ZW_ReplicationReceiveComplete(
  void);


/*============================ ZW_ReplicationSend   ======================
**    Used when the controller is replication mode.
**    It sends the payload and expects the receiver to respond with a
**    command complete message.
**    Side effects:
**
**--------------------------------------------------------------------------*/
extern BYTE                   /*RET  FALSE if transmitter busy      */
ZW_ReplicationSend(
  BYTE  destNodeID,             /*IN  Destination node ID. Only single cast allowed*/
  BYTE *pData,                  /*IN  Data buffer pointer           */
  BYTE  dataLength,             /*IN  Data buffer length            */
  BYTE  txOptions,              /*IN  Transmit option flags         */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /*IN  Transmit completed call back function  */


/*========================   ZW_GetNodeProtocolInfo   =======================
**
**    Copy the Node's current protocol information from the non-volatile
**    memory.
**
**--------------------------------------------------------------------------*/
extern void             /*RET Nothing        */
ZW_GetNodeProtocolInfo(
  BYTE  bNodeID,        /* IN Node ID */
  NODEINFO *nodeInfo);  /*OUT Node info buffer */


/*========================   ZW_AssignReturnRoute   =========================
**
**    Assign static return routes within a Routing Slave node.
**    Calculate the shortest transport routes from the Routing Slave node
**    to the route destination node and
**    transmit the return routes to the Routing Slave node.
**
**--------------------------------------------------------------------------*/
extern BOOL                         /*RET TRUE if assign was initiated. FALSE if not */
ZW_AssignReturnRoute(
  BYTE  bSrcNodeID,                 /* IN Routing Slave Node ID */
  BYTE  bDstNodeID,                 /* IN Route destination Node ID */
  VOID_CALLBACKFUNC(completedFunc)( /* IN Callback function called when done */
    BYTE bStatus));


/*========================   ZW_DeleteReturnRoute   =========================
**
**    Delete static return routes within a Routing Slave node.
**    Transmit "NULL" routes to the Routing Slave node.
**
**--------------------------------------------------------------------------*/
extern BOOL                  /*RET TRUE if delete return routes was initiated. FALSE if not */
ZW_DeleteReturnRoute(
  BYTE  nodeID,        /*IN Routing Slave */
  VOID_CALLBACKFUNC(completedFunc)(
    BYTE bStatus));     /*IN  Transmit complete status */


/*===========================   ZW_SetDefault   =============================
**    Remove all Nodes and timers from the EEPROM memory.
**
**--------------------------------------------------------------------------*/
extern void                         /*RET Nothing */
ZW_SetDefault(
  VOID_CALLBACKFUNC(completedFunc)( /* IN Command completed call back function */
    void));


/*==========================   ZW_GetFailedNode   ===============================
**
**    Check if a node failed is in the failed nodes table
**    Side effects:
**
**--------------------------------------------------------------------------*/
BYTE              /*RET true if node in failed node table, else false */
ZW_isFailedNode(
  BYTE nodeID);   /* IN the failed node ID */


/*==========================   ZW_RemoveFailedNode   ===============================
**
**    remove a node from the failed node list, if it already exist.
**    A call back function should be provided otherwise the function will return
**    without removing the node.
**    If the removing process started successfully then the function will return
**    ZW_FAILED_NODE_REMOVE_STARTED        The removing process started
**
**    If the removing process can not be started then the API function will return
**    on or more of the following flags
**    ZW_NOT_PRIMARY_CONTROLLER             The removing process was aborted because the controller is not the primaray one
**    ZW_NO_CALLBACK_FUNCTION              The removing process was aborted because no call back function is used
**    ZW_FAILED_NODE_NOT_FOUND             The removing process aborted because the node was node found
**    ZW_FAILED_NODE_REMOVE_PROCESS_BUSY   The removing process is busy
**
**    The call back function parameter value is:
**
**    ZW_NODE_OK                     The node is working proppely (removed from the failed nodes list )
**    ZW_FAILED_NODE_REMOVED         The failed node was removed from the failed nodes list
**    ZW_FAILED_NODE_NOT_REMOVED     The failed node was not
**    Side effects:
**--------------------------------------------------------------------------*/
BYTE                                /*RET function return code */
ZW_RemoveFailedNode(
  BYTE NodeID,                      /* IN the failed nodeID */
  VOID_CALLBACKFUNC(completedFunc)( /* IN callback function to be called */
    BYTE));                         /*    when the remove process end. */


/*=========================   ZW_ReplaceFailedNode   ==========================
**
**    Replace a node from the failed node list.
**    A call back function should be provided otherwise the function will return
**    without replacing the node.
**    If the replacing process started successfully then the function will return
**    ZW_FAILED_NODE_REMOVE_STARTED  The replacing process started and now the new
**                                   node must emit its nodeinformation frame to
**                                   start the assign process
**
**    If the replace process can not be started then the API function will return
**    on or more of the following flags
**
**    ZW_NOT_PRIMARY_CONTROLLER           The replacing process was aborted because
**                                        the controller is not the primary controller
**    ZW_NO_CALLBACK_FUNCTION             The replacing process was aborted because no
**                                        call back function is used
**    ZW_FAILED_NODE_NOT_FOUND            The replacing process aborted because
**                                        the node was node found
**    ZW_FAILED_NODE_REMOVE_PROCESS_BUSY  The replacing process is busy
**    ZW_FAILED_NODE_REMOVE_FAIL          The replacing process could not be started
**                                        because of
**
**    The call back function parameter value is:
**
**    ZW_NODE_OK                     The node is working proppely (removed from the failed nodes list )
**    ZW_FAILED_NODE_REPLACE         The failed node are ready to be replaced and controller
**                                   is ready to add new node with nodeID of the failed node
**    ZW_FAILED_NODE_REPLACE_DONE    The failed node has been replaced
**    ZW_FAILED_NODE_REPLACE_FAILED  The failed node has not been replaced
**    Side effects:
**--------------------------------------------------------------------------*/
BYTE                                       /*RET return the result of the function call */
ZW_ReplaceFailedNode(
  BYTE bNodeID,                            /* IN the nodeID on the failed node to replace */
  VOID_CALLBACKFUNC(completedFunc)(BYTE)); /* IN call back function to be called when the */
                                           /*    the replace process end. */


/*============================   ZW_SetSUCNodeID  ===========================
**    Function description
**    This function enable /disable a specified static controller
**    of functioning as the Static Update Controller
**
**--------------------------------------------------------------------------*/
BYTE                 /*RET TRUE target is a static controller*/
                     /*    FALSE if the target is not a static controller,  */
                     /*    the source is not primary or the SUC functinality is not enabled.*/
ZW_SetSUCNodeID(
  BYTE nodeID,       /* IN the node ID of the static controller to be a SUC */
  BYTE SUCState,     /* IN TRUE enable SUC, FALSE disable */
  BYTE bTxOption,    /* IN TRUE if to use low poer transmition, FALSE for normal Tx power */
  BYTE bCapabilities,             /* The capabilities of the new SUC */
  VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus)); /* IN a call back function */


/*============================   ZW_SendSUCID   =============================
**    Function description
**      Transmits SUC node id to the node specified. Only allowed from Primary or SUC
**    Side effects:
**
**--------------------------------------------------------------------------*/
BYTE ZW_SendSUCID(
  BYTE node,
  BYTE txOption,
  VOID_CALLBACKFUNC(callfunc)(BYTE));


/*============================   ZW_GetSUCNodeID  ===========================
**    Function description
**    This function gets the nodeID of the current Static Update Controller
**    if ZERO then no SUC is available
**
**--------------------------------------------------------------------------*/
BYTE                     /*RET nodeID on SUC, if ZERO -> no SUC */
ZW_GetSUCNodeID( void ); /* IN Nothing */


/*========================   ZW_RequestNetWorkUpdate   ======================
**    Function description
**      This function resquest network update from the SUC
**      Returns: TRUE - SUC is known to this controller, FALSE - SUC unknown
**      FUNC is a call back function indicates of the update was a success
**      or failed
**    The call back function parameter value is:
**        ZW_SUC_UPDATE_DONE        The update process ended successfully
**        ZW_SUC_UPDATE_ABORT       The update process was aborted
**        ZW_SUC_UPDATE_WAIT        The SUC node is busy, try again later
**        ZW_SUC_UPDATE_DISABLED    The SUC functionality have been disabled
**        ZW_SUC_UPDATE_OVERFLOW    Too many changes to handle by automatic update
**--------------------------------------------------------------------------*/
BYTE                    /* RET:  TRUE - SUC is known to this controller, FALSE - SUC unknown*/
ZW_RequestNetWorkUpdate(
  VOID_CALLBACKFUNC(completedFunc)(BYTE txStatus)); /* call back function indicates of the update sucessed or failed*/

/*=======================   ZW_AssignReturnRoute   =========================
**
**    Assign static return routes within a Routing Slave node.
**    Calculate the shortest transport routes to a Routing Slave node
**    from the Static Update Controller Node and
**    transmit the return routes to the Routing Slave node.
**
** void           RET  TRUE if process is started. FALSE if not
** ZW_AssignSUCReturnRoute(
**  BYTE  bSrcNodeID,        IN Routing Slave Node ID
**  VOID_CALLBACKFUNC(completedFunc)(
**    BYTE bStatus)); IN  Status of process
**--------------------------------------------------------------------------*/
extern BOOL                         /*RET TRUE if process is started. FALSE if not*/
ZW_AssignSUCReturnRoute(
  BYTE  bSrcNodeID,                 /* IN Routing Slave Node ID */
  VOID_CALLBACKFUNC(completedFunc)( /* IN Callback function called when done */
    BYTE bStatus));


/*========================   ZW_DeleteSUCReturnRoute   =======================
**
**    Delete Static Update Controller (SUC) static return routes within a
**    Routing Slave node. Transmit "NULL" routes to the Routing Slave node.
**
**--------------------------------------------------------------------------*/
extern BOOL             /*RET TRUE if delete SUC return routes was initiated. */
                        /*    FALSE if a return route assign/delete is allready active */
ZW_DeleteSUCReturnRoute(
  BYTE  bNodeID,        /*IN Routing Slave Node ID */
  VOID_CALLBACKFUNC(completedFunc)(BYTE bStatus)); /*IN  Status of process */


/*=====================   ZW_RequestNodeNeighborUpdate  ======================
**
**    Start neighbor discovery for bNodeID, if any other nodes present.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
BYTE                                /*RET TRUE neighbor discovery started */
ZW_RequestNodeNeighborUpdate(
  BYTE bNodeID,                     /* IN Node id */
  VOID_CALLBACKFUNC(completedFunc)( /* IN Function to be called when the done */
    BYTE));

/*============================   ZW_IsPrimaryCtrl   =========================
**    Function description
**      Returns TRUE When the controller is a primary.
**              FALSE if it is a slave
**    Side effects:
**
**--------------------------------------------------------------------------*/
BOOL ZW_IsPrimaryCtrl(void);

/*============================   ZW_IsSUCActive  =============================
**    Function description
**      Returns the SUC state
**    Side effects:
**
**--------------------------------------------------------------------------*/
BYTE
ZW_GetControllerCapabilities(void);

/*========================   ZW_GetNeighborCount   ===========================
**
**  Get number of neighbors the specified Node ID has registered
**
**  Returns:
**    0x00-0xE7               - Number of neighbors registered
**    NEIGHBORS_ID_INVALID    - specified node ID invalid
**    NEIGHBORS_COUNT_FAILED  - Could not access routeing information - try again later
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
BYTE                  /*RET Number of neighbors */
ZW_GetNeighborCount(
  BYTE bNodeID);      /* IN Node ID on node to count neighbors on */


/*============================   ZW_RequestNodeInfo   ======================
**    Function description.
**     Request a node to send it's node information.
**     Function return TRUE if the request is send, else it return FALSE.
**     FUNC is a callback function, which is called with the status of the
**     Request nodeinformation frame transmission.
**     If a node sends its node info, ApplicationControllerUpdate will be called
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


/*===========================   SetLearnMode   ==============================
**    Enable/Disable home/node ID learn mode.
**    When learn mode is enabled, received "Assign ID's Command" are handled:
**    If the current stored ID's are zero, the received ID's will be stored.
**    If the received ID's are zero the stored ID's will be set to zero.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_SetLearnMode( /*RET  Nothing        */
  BOOL mode,                                       /*IN  TRUE or FALSE */
  VOID_CALLBACKFUNC(completedFunc)(LEARN_INFO*));  /* IN Callback function */

/*==========================   ZW_AddNodeToNetwork   ========================
**
**    Add any type of node to the network
**
**    The modes are:
**
**    ADD_NODE_ANY            Add any node to the network
**    ADD_NODE_CONTROLLER     Add a controller to the network
**    ADD_NODE_SLAVE          Add a slaev node to the network
**    ADD_NODE_STOP           Stop learn mode without reporting an error.
**    ADD_NODE_STOP_FAILED    Stop learn mode and report an error to the
**                            new controller.
**
**    ADD_NODE_OPTION_HIGH_POWER    Set this flag in bMode for High Power inclusion.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_AddNodeToNetwork(BYTE bMode,
                    VOID_CALLBACKFUNC(completedFunc)(LEARN_INFO*));

/*==========================   ZW_RemoveNodeFromNetwork   ========================
**
**    Remove any type of node from the network
**
**    The modes are:
**
**    REMOVE_NODE_ANY            Remove any node from the network
**    REMOVE_NODE_CONTROLLER     Remove a controller from the network
**    REMOVE_NODE_SLAVE          Remove a slaev node from the network
**
**    REMOVE_NODE_STOP           Stop learn mode without reporting an error.
**
**    ADD_NODE_OPTION_HIGH_POWER    Set this flag in bMode for High Power exclusion.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_RemoveNodeFromNetwork(BYTE bMode,
                    VOID_CALLBACKFUNC(completedFunc)(LEARN_INFO*));

/*========================   ZW_ControllerChange   ======================
**
**    Transfer the role as primary controller to another controller
**
**    The modes are:
**
**    CONTROLLER_CHANGE_START          Start the creation of a new primary
**    CONTROLLER_CHANGE_STOP           Stop the creation of a new primary
**    CONTROLLER_CHANGE_STOP_FAILED    Report that the replication failed
**
**    ADD_NODE_OPTION_HIGH_POWER       Set this flag in bMode for High Power exchange.
**
**    Side effects:
**
**--------------------------------------------------------------------------*/
void
ZW_ControllerChange(BYTE bMode,
                        VOID_CALLBACKFUNC(completedFunc)(LEARN_INFO*));

/*==========================   ZW_AreNodesNeighbours   ============================
**
**  Are two specific nodes neighbours
**
**
**  Side effects:
**
**--------------------------------------------------------------------------*/
BOOL ZW_AreNodesNeighbours( /*RET NONE ZERO if nodes are neighbours else ZERO */
BYTE bNodeA,           /* IN first node id */
BYTE bNodeB);          /* IN second node id */
#endif /* WIN32 */

#endif /* _ZW_CONTROLLER_API_H_ */

