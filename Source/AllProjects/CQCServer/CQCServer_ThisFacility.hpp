//
// FILE NAME: CQCServer_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/05/2000
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This is the header for the facility class for the server. This program
//  is responsible for hosting and providing access to drivers. It has to
//  be very careful to do the right thing, so that no one gets hurt, and it
//  would be easy for someone to get hurt.
//
//  The rules/issues involved are:
//
//  1. There is a list of loaded drivers that we keep. We must of course sync
//     access to this list since drivers can be loaded and unloaded and client
//     drivers can make calls into drivers and they can't be allowed to step
//     on each other. We have a mutex to control access to this list.
//  2. There are two types of calls that can be made
//      - There are those that make quick calls to get already available
//        info from the base driver class (which we control.)
//      - There are those that cause a call into the derived driver class'
//        callback methods, which we don't control.
//  3. The first type of call we control by the same mutex that controls the
//     main driver list. These will call into the driver, which will lock it
//     self and give back the data. The driver's lock is only around the
//     updating of driver state in the base class.
//  4. Therefore we know that only one client can be in a driver at a time
//     for the first type of call. This isn't really a limitation since these
//     calls are quite fast. And it means that, if we currently have the lock
//     then no clients are in any drivers, which is an important thing to
//     know.
//  5. For the second type of call, these are queued up asynchronously on a
//     command queue, and the client can either choose to wait for it to
//     complete/fail or not. The driver doesn't care if it does or not. Since
//     these commands only ever run in the context of the driver's own thread,
//     we know no clients are ever involved in these. These can be concurrent
//     with the first type of call, since derived classes either are just
//     updating/returning their own data (which is always only accessed by
//     the driver thread) or if they do access the base class, the same mutex
//     is used to protect base class driver state calls. So there isn't any
//     danger of conflict.
//  6. Derived driver classes cannot lock the driver's mutex, so they can't
//     accidentally lock up the driver and hold up calls (which would in turn
//     lock up our list mutex, and block all clients.)
//  7. And this also means of course that the base driver class MUST insure
//     that all locks of its mutex are very short, and never around anything
//     that could potentially take more than a fraction of a second.
//
//  When a driver is to be unloaded we have the special concerns:
//
//  1.  Any clients for the second type of call are easy. We just error out
//      any queued commands, and they all wake up and exit.
//  2.  Since the driver removal command is of the second type, and the shut
//      down could take a while, what we do is lock our list, orphan the driver
//      from the list, which means no subsequent client calls will see it,
//      and then unlock. We can then take as much time as required to get the
//      driver shut down without worries.
//  3.  Since only one of the 2nd type of call can be in a driver at a time,
//      the shutdown call was the only one, and it hides the driver. So we
//      don't have to worry that we are going to destroy the driver while
//      client calls are referencing it.
//
//  Drivers with private ORB interfaces:
//
//  1.  If a driver has its own private interface that it exposes to a client
//      side driver or client side management program, it MUST provide the
//      means during termination to make sure that all such calls are
//      completed before the driver is released.
//  2.  Preferably, just use the generic backdoor driver call system to do
//      such things, since they are queued up async and can never get you
//      into trouble. Just flatten the info to a buffer and send it,
//      streaming it back out on the other side.
//
//  Startup issues:
//
//  1.  All registrations of ORB interfaces must be done in deferred mode,
//      so that they are just put on the rebinder's queue. The main thread
//      doesn't start the rebinder until all startup is complete. This way
//      we can get everything into place without worrying about locking
//      issues, then make everything visible at once.
//
//  Configuration:
//
//  1.  As of 5.0, all of the driver config was moved to the master server. So we
//      query our config data from it on startup. This provides us the list of
//      drivers to load.
//  2.  We never store the configuration anymore. The client will make changes to
//      driver config and store them to the master config server, then it will pass
//      the changes to us to update our live info. If we somehow fail, oh well, the
//      data is stored and will be used next time. This means we never have to worry
//      about locking wrt to the config, other than to just quickly update it. Such
//      config changes are protected by the main mutex.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCServer
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCServer : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCServer();

        TFacCQCServer(const TFacCQCServer&) = delete;
        TFacCQCServer(TFacCQCServer&&) = delete;

        ~TFacCQCServer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCServer& operator=(const TFacCQCServer&) = delete;
        TFacCQCServer& operator=(TFacCQCServer&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckFieldExists
        (
            const   TString&                strMoniker
            ,       tCIDLib::TBoolean&      bDevExists
            , const TString&                strField
            ,       tCIDLib::TBoolean&      bFldExists
        );

        tCIDLib::TBoolean bCheckForAllNewFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const TFundVector<tCIDLib::TCard8>& fcolIds
        );

        tCIDLib::TBoolean bCheckForNewFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
        );

        tCIDLib::TBoolean bCheckForNewDrivers
        (
            const   tCIDLib::TCard4         c4DriverListId
        );

        tCIDLib::TBoolean bDriverIsLoaded
        (
            const   TString&                strMonikerToCheck
        )   const;

        tCIDLib::TBoolean bHasQueuedTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard8&        c8EndTime
        );

        tCIDLib::TBoolean bQueryCommPorts
        (
                    tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bLocalOnly
            , const tCIDLib::TBoolean       bAvailOnly
        );

        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryDriverInfo
        (
            const   TString&                strMonikerToCheck
            ,       TCQCDriverObjCfg&       cqcdcToFill
        );

        tCIDLib::TBoolean bQueryDriverPauseState
        (
            const   TString&                strMoniker
        );

        tCIDLib::TBoolean bQueryDriverStates
        (
            const   tCIDLib::TCard4         c4DriverListId
            ,       tCIDLib::TCardList&     fcolIds
            ,       TFundVector<tCQCKit::EDrvStates>& fcolStates
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strMoniker
            , const TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TBoolean bQueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strValue
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       THeapBuf&               mbufValue
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TBoolean bReadFieldByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strToFill
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TBoolean bReadFieldByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       THeapBuf&               mbufValue
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TBoolean bReadBoolByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TBoolean&      bToFill
        );

        tCIDLib::TBoolean bReadCardByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4ToFill
        );

        tCIDLib::TBoolean bReadFloatByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TFloat8&       f8ToFill
        );

        tCIDLib::TBoolean bReadIntByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TInt4&         i4ToFill
        );

        tCIDLib::TBoolean bReadSListByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TStrList&      colToFill
        );

        tCIDLib::TBoolean bReadStringByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bReadTimeByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard8&        c8ToFill
        );

        tCIDLib::TBoolean bRemoteLoadDrivers
        (
            const   tCIDLib::TStrList&      colMonikers
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strMoniker
            , const TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryDrvConfig
        (
            const   TString&                strMoniker
            ,       THeapBuf&               mbufToFill
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryDriverConfigObjs
        (
                    TCollection<TCQCDriverObjCfg>& colToFill
            ,       tCIDLib::TCard4&        c4DriverListId
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryDriverId
        (
            const   TString&                strMoniker
            ,       tCIDLib::TCard4&        c4DriverListId
        );

        tCIDLib::TCard4 c4QueryDriverIds
        (
            const   tCIDLib::TStrList&      colMonikers
            ,       TFundVector<tCIDLib::TCard4>& fcolIds
            ,       TVector<TKeyValuePair>& colMakeModels
        );

        tCIDLib::TCard4 c4QueryDriverIdList
        (
                    tCIDLib::TStrList&      colMonikers
            ,       TFundVector<tCIDLib::TCard4>& fcolIds
            ,       TVector<TKeyValuePair>& colMakeModels
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryDriverIdList2
        (
                    tCIDLib::TStrList&      colMonikers
            ,       tCIDLib::TCardList&     fcolIds
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TCard4 c4QueryFields
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eState
            ,       TVector<TCQCFldDef>&    colToFill
            ,       tCIDLib::TCard4&        c4FldListId
            ,       tCIDLib::TCard4&        c4DriverId
            ,       tCIDLib::TCard4&        c4DriverListId
        );

        tCIDLib::TCard4 c4QueryFieldNames
        (
            const   TString&                strMoniker
            ,       TVector<TString>&       colToFill
            , const tCQCKit::EReqAccess     eAccess
        );

        tCIDLib::TCard4 c4QueryFieldNamesRX
        (
            const   TString&                strMoniker
            ,       tCQCKit::TFldDefList&   colToFill
            , const TString&                strNameRegEx
            , const tCQCKit::EReqAccess     eAccess
        );

        tCIDLib::TCard4 c4QuerySemFields
        (
            const   tCQCKit::TFldSTypeList& fcolTypes
            , const tCQCKit::EReqAccess     eReqAccess
            ,       TVector<TCQCDrvFldDef>& colToFill
            , const TString&                strMoniker
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TCard4 c4QueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strParms
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid CancelTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid ClearDrvStats
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid DoTimedWrite
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strNewValue
            , const TString&                strEndValue
            , const tCIDLib::TCard4         c4Seconds
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TInt4 i4QueryVal
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TInt4 i4QueryVal2
        (
            const   TString&                strMoniker
            , const TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bNoQueue
        );

        tCIDLib::TVoid QueryDriverInfo
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eStatus
            ,       tCIDLib::TCard4&        c4ArchVer
            ,       tCQCKit::TDevClassList& fcolDevClasses
            ,       TString&                strMake
            ,       TString&                strModel
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryDriverState
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eStatus
            ,       tCQCKit::EVerboseLvls&  eVerbose
            ,       tCIDLib::TCard4&        c4ThreadId
        );

        tCIDLib::TVoid QueryDriverStats
        (
            const   TString&                strMoniker
            ,       TCQCDrvStats&           cdstatsToFill
        );

        tCIDLib::TVoid QueryFieldDef
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
        );

        tCIDLib::TVoid QueryFieldInfo
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard4&        c4DriverListId
            ,       tCIDLib::TCard4&        c4DriverId
            ,       tCIDLib::TCard4&        c4FieldListId
            ,       tCIDLib::TCard4&        c4FieldId
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TVoid QueryPowerStates
        (
            const   tCIDLib::TKVPList&      colDrivers
            ,       TFundVector<tCQCKit::EPowerStatus>& fcolStates
        );

        tCIDLib::TVoid QueryStatusReport
        (
                    TString&                strErrs
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid PauseResumeDrv
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bPause
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid ReadDriverFields
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const TFundVector<tCIDLib::TCard4>& fcolFldIds
            , const TFundVector<tCIDLib::TCard4>& fcolSerialNums
            ,       tCIDLib::TCard4&        c4BufSz_mbufData
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TVoid ReadFields
        (
            const   TFldIOPacket&           fiopToRead
            ,       tCIDLib::TCard4&        c4BytesRead
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TVoid ReadFieldsByName
        (
            const   TString&                strMoniker
            , const tCIDLib::TStrList&      colFieldNames
            ,       tCIDLib::TCard4&        c4BytesRead
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TVoid RefreshCfg
        (
            const   TString&                strCfgType
        );

        tCIDLib::TVoid SetConfig
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid SetVerboseLevel
        (
            const   TString&                strMoniker
            , const tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid ReconfigDriver
        (
            const   TString&                strMoniker
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid RemoteUnloadAllDrivers
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid RemoteUnloadDriver
        (
            const   TString&                strMoniker
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid UpdateTriggers
        (
            const   TString&                strMoniker
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteBoolByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TBoolean       bValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteBoolFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteCardByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TCard4         c4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteCardFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteFieldByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteFloatByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TFloat8        f8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteFloatFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteIntByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TInt4          i4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteIntFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteStringByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteStringFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteSListByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TStrList&      colValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteSListFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TStrList&      colValue
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteTimeByName
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const tCIDLib::TCard8&        c8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        tCIDLib::TVoid WriteTimeFld
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8&        c8Value
            , const TCQCSecToken&           sectUser
            , const tCQCKit::EDrvCmdWaits   eWait
        );


    protected :
        // -------------------------------------------------------------------
        //  Our lock janitor is our friend
        // -------------------------------------------------------------------
        friend class TDrvLocker;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCQCSrvFW::EStateRes eLoadConfig
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        )   final;

        tCQCSrvFW::EStateRes eWaitPrereqs
        (
                    tCIDLib::TCard4&        c4WaitNext
        )   final;

        tCIDLib::TVoid PreRegInit() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   override;

        tCIDLib::TVoid RegisterSrvObjs() final;

        tCIDLib::TVoid StartWorkerThreads() final;

        tCIDLib::TVoid StopWorkerThreads() final;

        tCIDLib::TVoid UnbindSrvObjs
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bWaitDataServer();

        tCIDLib::TVoid CheckDrvListId
        (
            const   tCIDLib::TCard4         c4DriverListId
        )   const;

        tCIDLib::TVoid DeleteDrvFiles
        (
            const   TCQCDriverObjCfg&       cqcdcToDel
        );

        tCIDLib::EExitCodes eMaintThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid LoadADriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        tCIDLib::TVoid LoadDrivers
        (
            const   tCQCKit::TDrvCfgList&   colCfgObjs
        );

        const TServerDriverInfo* psdiFindDrv
        (
            const   TString&                strMonikerToFind
            ,       tCIDLib::TCard4&        c4Index
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        TServerDriverInfo* psdiFindDrv
        (
            const   TString&                strMonikerToFind
            ,       tCIDLib::TCard4&        c4Index
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        TServerDriverInfo* psdiFindDrvById
        (
            const   tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId = 0
        );

        tCIDLib::TVoid RegisterPortFactories();

        tCIDLib::TVoid UnloadDeadDrivers();

        tCIDLib::TVoid UnloadDrivers();

        tCIDLib::TVoid UnregDriverNS
        (
            const   TString&                strMoniker
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DriverListId
        //      This is updated every time a driver is removed, which will invalidate
        //      any driver ids that we've given to clients. If they call back in with
        //      an id, they must provide the id we gave to them last time. We see its
        //      different and throw an error back to them.
        //
        //  m_GC100SerialNum
        //  m_c4JAPSerialNum
        //      For reading GC-100 and Just Add Power serial port config. We keep the
        //      last serial number around so that we can pass it back in for the next
        //      read.
        //
        //  m_colCfgObjs
        //      The configuration we load. It's a list of driver config objects. We have
        //      to save it for later use when we do the actual driver loading. Once the
        //      drivers are loaded, we can flush this to save space.
        //
        //  m_colDriverList
        //      A list of driver information objects. These represent the drivers that
        //      we've been configured for. At startup we query the driver configs for
        //      our host and create this list. We also deal with requests from clients
        //      to add, remove, pause, etc... drivers and update this list accordingly.
        //
        //  m_gcclPorts
        //      This is the GC-100 port configuration data. We try to load it during
        //      init and register the GC-100 port factory. Any time a client updates
        //      the config it will send a notification to any loaded CQCServers and
        //      we'll update this, and pass it on to the GC-100 port factory we installed
        //      on startup.
        //
        //  m_japlPorts
        //      This is the Just Add Power port config data. We try to load it during
        //      init and register the JAP 100 port factory. Any time a client updates
        //      the config it will send a notification to any loaded CQCServers and
        //      we'll update this, and pass it on to the JAP port factory we installed
        //      on startup.
        //
        //  m_mtxLock
        //      This used for synchronization of the overall driver list or other
        //      things that require complete synchronization of all incoming calls.
        //
        //  m_ooidAdmin
        //      The object id of our admin object. We store it here because every driver
        //      we load gets an 'alias' entry in the name server that just maps back to
        //      our admin interface, so that clients can quickly map from moniker to the
        //      CQCServer that handles the driver with that moniker, so we have to have
        //      access to this each time we add a driver.
        //
        //  m_porbsAdmin
        //      Our server admin interface. We keep it around so that we can deregister
        //      it during shutdown, but the ORB owns it.
        //
        //  m_thrMaint
        //      This maintenance thread periodically calls UnloadDeadDrivers to remove
        //      any drivers that have been been asked to stop and have completed that
        //      process. And it will also continue to try to load our GC-100 and JAP port
        //      config if we didn't get it during initial startup. It looks as the
        //      serial numbers and, if they are zero, it knows it's not read anything
        //      yet.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4DriverListId;
        tCIDLib::TCard4         m_c4GC100SerialNum;
        tCIDLib::TCard4         m_c4JAPSerialNum;
        tCQCKit::TDrvCfgList    m_colCfgObjs;
        tCQCServer::TDrvList    m_colDriverList;
        TGC100CfgList           m_gcclPorts;
        TJAPwrCfgList           m_japlPorts;
        TMutex                  m_mtxLock;
        TOrbObjId               m_ooidAdmin;
        TCQCSrvAdminImpl*       m_porbsAdmin;
        TThread                 m_thrMaint;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCServer,TCQCSrvCore)
};

#pragma CIDLIB_POPPACK

