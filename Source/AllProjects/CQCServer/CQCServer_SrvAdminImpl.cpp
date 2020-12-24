//
// FILE NAME: CQCServer_SrvAdminImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/29/2001
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
//  This file implements the CQCServer remote admin interface. We just
//  delegate almost everything to the facility object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCServer.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSrvAdminImpl,TCQCSrvAdminServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvAdminImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSrvAdminImpl: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCSrvAdminImpl::bCheckFieldExists(const   TString&            strMoniker
                                    ,       tCIDLib::TBoolean&  bDevExists
                                    , const TString&            strField
                                    ,       tCIDLib::TBoolean&  bFldExists)
{
    return facCQCServer.bCheckFieldExists(strMoniker, bDevExists, strField, bFldExists);
}


tCIDLib::TBoolean TCQCSrvAdminImpl::
bCheckForAllNewFields(  const   tCIDLib::TCard4                 c4DriverListId
                        , const TFundVector<tCIDLib::TCard8>&   fcolIds)
{
    return facCQCServer.bCheckForAllNewFields(c4DriverListId, fcolIds);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bCheckForNewFields(const  tCIDLib::TCard4 c4DriverListId
                                    , const tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldListId)
{
    return facCQCServer.bCheckForNewFields(c4DriverListId, c4DriverId, c4FieldListId);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bCheckForNewDrivers(const tCIDLib::TCard4 c4DriverListId)
{
    return facCQCServer.bCheckForNewDrivers(c4DriverListId);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bHasQueuedTimedWrite( const   TString&            strMoniker
                                        , const TString&            strField
                                        ,       tCIDLib::TCard8&    c8EndTime)
{
    return facCQCServer.bHasQueuedTimedWrite(strMoniker, strField, c8EndTime);
}


// Load up drivers based on the passed list of monikers
tCIDLib::TBoolean
TCQCSrvAdminImpl::bLoadDrivers( const   tCIDLib::TStrList&  colMonikers
                                , const TCQCSecToken&       sectUser)
{
    return facCQCServer.bRemoteLoadDrivers(colMonikers, sectUser);
}


//
//  Returns the list of available serial ports. We can optinally limit it to only
//  real local ports, or we can also return virtual ports that we support.
//
tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryCommPorts(          tCIDLib::TStrList&  colToFill
                                    , const tCIDLib::TBoolean   bLocalOnly
                                    , const tCIDLib::TBoolean   bAvailOnly)
{
    return facCQCServer.bQueryCommPorts(colToFill, bLocalOnly, bAvailOnly);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryData(const  TString&            strMoniker
                            , const TString&            strQueryType
                            , const TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufToFill)
{
    return facCQCServer.bQueryData
    (
        strMoniker, strQueryType, strDataName, c4Bytes, mbufToFill
    );
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryData2(  const   TString&            strMoniker
                                , const TString&            strQueryType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    return facCQCServer.bQueryData2(strMoniker, strQueryType, c4IOBytes, mbufIO);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryDriverInfo( const   TString&            strMoniker
                                    ,       TCQCDriverObjCfg&   cqcdcToFill)
{
    return facCQCServer.bQueryDriverInfo(strMoniker, cqcdcToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryDriverStates(const  tCIDLib::TCard4                 c4DriverListId
                                    ,       tCIDLib::TCardList&             fcolIds
                                    ,       TFundVector<tCQCKit::EDrvStates>& fcolStates)
{
    return facCQCServer.bQueryDriverStates(c4DriverListId, fcolIds, fcolStates);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryTextVal(const   TString&    strMoniker
                                , const TString&    strQueryType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    return facCQCServer.bQueryTextVal(strMoniker, strQueryType, strDataName, strToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryVal(const   TString&            strMoniker
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.bQueryVal(strMoniker, strValId, bNoQueue);
}

tCIDLib::TBoolean
TCQCSrvAdminImpl::bQueryVal2(const  TString&            strMoniker
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.bQueryVal2(strMoniker, strValId, strValName, bNoQueue);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadBoolByName(          tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       tCIDLib::TBoolean&  bToFill)
{
    return facCQCServer.bReadBoolByName(strMoniker, strFldName, c4SerialNum, bToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadCardByName(          tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       tCIDLib::TCard4&    c4ToFill)
{
    return facCQCServer.bReadCardByName(strMoniker, strFldName, c4SerialNum, c4ToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadField(const  tCIDLib::TCard4     c4DriverListId
                            , const tCIDLib::TCard4     c4DriverId
                            , const tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FieldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       TString&            strValue
                            ,       tCQCKit::EFldTypes& eType)
{
    return facCQCServer.bReadField
    (
        c4DriverListId
        , c4DriverId
        , c4FieldListId
        , c4FieldId
        , c4SerialNum
        , strValue
        , eType
    );
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadField(const  tCIDLib::TCard4     c4DriverListId
                            , const tCIDLib::TCard4     c4DriverId
                            , const tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FieldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4RetBytes
                            ,       THeapBuf&           mbufValue
                            ,       tCQCKit::EFldTypes& eType)
{
    return facCQCServer.bReadField
    (
        c4DriverListId
        , c4DriverId
        , c4FieldListId
        , c4FieldId
        , c4SerialNum
        , c4RetBytes
        , mbufValue
        , eType
    );
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadFieldByName(         tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       TString&            strToFill
                                    ,       tCQCKit::EFldTypes& eType)
{
    return facCQCServer.bReadFieldByName
    (
        strMoniker, strFldName, c4SerialNum, strToFill, eType
    );
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadFieldByName(         tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       tCIDLib::TCard4&    c4RetBytes
                                    ,       THeapBuf&           mbufValue
                                    ,       tCQCKit::EFldTypes& eType)
{
    return facCQCServer.bReadFieldByName
    (
        strMoniker, strFldName, c4SerialNum, c4RetBytes, mbufValue, eType
    );
}



tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadFloatByName(         tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       tCIDLib::TFloat8&   f8ToFill)
{
    return facCQCServer.bReadFloatByName(strMoniker, strFldName, c4SerialNum, f8ToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadIntByName(       tCIDLib::TCard4&    c4SerialNum
                                , const TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TInt4&     i4ToFill)
{
    return facCQCServer.bReadIntByName(strMoniker, strFldName, c4SerialNum, i4ToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadSListByName(         tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       TVector<TString>&   colToFill)
{
    return facCQCServer.bReadSListByName(strMoniker, strFldName, c4SerialNum, colToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadStringByName(        tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       TString&            strToFill)
{
    return facCQCServer.bReadStringByName(strMoniker, strFldName, c4SerialNum, strToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bReadTimeByName(          tCIDLib::TCard4&    c4SerialNum
                                    , const TString&            strMoniker
                                    , const TString&            strFldName
                                    ,       tCIDLib::TCard8&    c8ToFill)
{
    return facCQCServer.bReadTimeByName(strMoniker, strFldName, c4SerialNum, c8ToFill);
}


tCIDLib::TBoolean
TCQCSrvAdminImpl::bSendData(const   TString&            strMoniker
                            , const TString&            strSendType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4IOBytes
                            ,       THeapBuf&           mbufIO
                            , const TCQCSecToken&       sectUser)
{
    return facCQCServer.bSendData
    (
        strMoniker, strSendType, strDataName, c4IOBytes, mbufIO, sectUser
    );
}


//
//  Returns a list of local audio devices on this machine. This one we can
//  handle ourself and doesn't require any synchronization.
//
tCIDLib::TCard4 TCQCSrvAdminImpl::c4QueryAudioDevs(tCIDLib::TStrList& colToFill)
{
    tCIDLib::TKVPList colDevs;
    const tCIDLib::TCard4 c4Count = TAudio::c4EnumAudioDevices(colDevs);

    colToFill.RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.objAdd(colDevs[c4Index].strKey());

    return c4Count;
}


tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryDriverConfigObjs(TVector<TCQCDriverObjCfg>& colToFill
                                         , tCIDLib::TCard4&         c4DriverListId
                                        , const TCQCSecToken&       sectUser)
{
    return facCQCServer.c4QueryDriverConfigObjs(colToFill, c4DriverListId, sectUser);
}


tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryDriverId(  const   TString&            strMoniker
                                    ,       tCIDLib::TCard4&    c4DriverListId)
{
    return facCQCServer.c4QueryDriverId(strMoniker, c4DriverListId);
}


tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryDriverIds( const tCIDLib::TStrList&    colMonikers
                                    ,     tCIDLib::TCardList&   fcolIds
                                    ,     tCIDLib::TKVPList&    colMakeModels)
{
    return facCQCServer.c4QueryDriverIds(colMonikers, fcolIds, colMakeModels);
}


tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryDriverIdList(  tCIDLib::TStrList&      colMonikers
                                        , tCIDLib::TCardList&   fcolIds
                                        , tCIDLib::TKVPList&    colMakeModels
                                        , const TCQCSecToken&   sectUser)
{
    return facCQCServer.c4QueryDriverIdList(colMonikers, fcolIds, colMakeModels, sectUser);
}


tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryDriverIdList2( tCIDLib::TStrList&      colMonikers
                                        , tCIDLib::TCardList&   fcolIds
                                        , const TCQCSecToken&   sectUsers)
{
    return facCQCServer.c4QueryDriverIdList2(colMonikers, fcolIds, sectUsers);
}


// Return the names of all fields of the driver that have the requested access
tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryFieldNames(const   TString&            strMoniker
                                    ,       tCIDLib::TStrList&  colToFill
                                    , const tCQCKit::EReqAccess eAccess)
{
    return facCQCServer.c4QueryFieldNames(strMoniker, colToFill, eAccess);
}


// Same as above but with a field name reg ex
tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryFieldNamesRX(  const   TString&                strMoniker
                                        ,       tCQCKit::TFldDefList&   colToFill
                                        , const TString&                strNameRegEx
                                        , const tCQCKit::EReqAccess     eAccess)
{
    return facCQCServer.c4QueryFieldNamesRX(strMoniker, colToFill, strNameRegEx, eAccess);
}

// Return all the fields of the indicated driver
tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryFields(const   TString&                strMoniker
                                ,       tCQCKit::EDrvStates&    eState
                                ,       TVector<TCQCFldDef>&    colToFill
                                ,       tCIDLib::TCard4&        c4FldListId
                                ,       tCIDLib::TCard4&        c4DriverId
                                ,       tCIDLib::TCard4&        c4DriverListId)
{
    return facCQCServer.c4QueryFields
    (
        strMoniker, eState, colToFill, c4FldListId, c4DriverId, c4DriverListId
    );
}


// Returns all fields of a given semantic type and access type
tCIDLib::TCard4 TCQCSrvAdminImpl::
c4QuerySemFields(const  TFundVector<tCQCKit::EFldSTypes>&   fcolTypes
                , const tCQCKit::EReqAccess                 eReqAccess
                ,       TVector<TCQCDrvFldDef>&             colToFill
                , const TString&                            strMoniker)
{
    return facCQCServer.c4QuerySemFields(fcolTypes, eReqAccess, colToFill, strMoniker);
}


//
//  Returns a list of local TTS voices on this machine. This one we can
//  handle ourself and doesn't require any synchronization.
//
tCIDLib::TCard4 TCQCSrvAdminImpl::c4QueryTTSVoices(TVector<TString>& colToFill)
{
    TSpeech::QueryVoiceList(colToFill);
    return colToFill.c4ElemCount();
}


// Call the driver backdoor method to query a Card4 value
tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryVal(const  TString&            strMoniker
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.c4QueryVal(strMoniker, strValId, bNoQueue);
}

tCIDLib::TCard4
TCQCSrvAdminImpl::c4QueryVal2(const TString&            strMoniker
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.c4QueryVal2(strMoniker, strValId, strValName, bNoQueue);
}


// Call the driver backdoor method to send a driver command
tCIDLib::TCard4
TCQCSrvAdminImpl::c4SendCmd(const   TString&        strMoniker
                            , const TString&        strCmdId
                            , const TString&        strParms
                            , const TCQCSecToken&   sectUser)
{
    return facCQCServer.c4SendCmd(strMoniker, strCmdId, strParms, sectUser);
}


// Cancel any outstanding timed write on the indicate field
tCIDLib::TVoid
TCQCSrvAdminImpl::CancelTimedWrite( const   TString&        strMoniker
                                    , const TString&        strFld
                                    , const TCQCSecToken&   sectUser)
{
    facCQCServer.CancelTimedWrite(strMoniker, strFld, sectUser);
}


// Clear the stats for the indicated driver
tCIDLib::TVoid
TCQCSrvAdminImpl::ClearDrvStats(const TString& strMoniker)
{
    facCQCServer.ClearDrvStats(strMoniker);
}


// Queue up a timed write for the indicated field
tCIDLib::TVoid
TCQCSrvAdminImpl::DoTimedWrite( const   TString&                strMoniker
                                , const TString&                strFld
                                , const TString&                strNewVal
                                , const TString&                strEndVal
                                , const tCIDLib::TCard4         c4Secs
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.DoTimedWrite
    (
        strMoniker, strFld, strNewVal, strEndVal, c4Secs, sectUser, eWait
    );
}


// Make the driver backdoor call to query a signed value
tCIDLib::TInt4
TCQCSrvAdminImpl::i4QueryVal(const  TString&            strMoniker
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.i4QueryVal(strMoniker, strValId, bNoQueue);
}

tCIDLib::TInt4
TCQCSrvAdminImpl::i4QueryVal2(  const   TString&            strMoniker
                                , const TString&            strValId
                                , const TString&            strValName
                                , const tCIDLib::TBoolean   bNoQueue)
{
    return facCQCServer.i4QueryVal2(strMoniker, strValId, strValName, bNoQueue);
}


// Make the driver backdoor call to query the driver's configuration data
tCIDLib::TVoid
TCQCSrvAdminImpl::QueryConfig(  const   TString&            strMoniker
                                ,       tCIDLib::TCard4&    c4BytesFilled
                                ,       THeapBuf&           mbufToFill
                                , const TCQCSecToken&       sectUser)
{
    c4BytesFilled = facCQCServer.c4QueryDrvConfig(strMoniker, mbufToFill, sectUser);
}


// Query some core info about a driver
tCIDLib::TVoid
TCQCSrvAdminImpl::QueryDriverInfo(const TString&                strMoniker
                                ,       tCQCKit::EDrvStates&    eStatus
                                ,       tCIDLib::TCard4&        c4ArchVer
                                ,       tCQCKit::TDevClassList& fcolDevClasses
                                ,       TString&                strMake
                                ,       TString&                strModel
                                , const TCQCSecToken&           sectUser)
{
    facCQCServer.QueryDriverInfo
    (
        strMoniker
        , eStatus
        , c4ArchVer
        , fcolDevClasses
        , strMake
        , strModel
        , sectUser
    );
}


// Query the basic state of the indicated driver
tCIDLib::TVoid
TCQCSrvAdminImpl::QueryDriverState( const   TString&                strMoniker
                                    ,       tCQCKit::EDrvStates&    eStatus
                                    ,       tCQCKit::EVerboseLvls&  eVerbose
                                    ,       tCIDLib::TCard4&        c4ThreadId)
{
    facCQCServer.QueryDriverState(strMoniker, eStatus, eVerbose, c4ThreadId);
}


// Query the detailed driver stats of the indicated driver
tCIDLib::TVoid TCQCSrvAdminImpl::
QueryDriverStats(const TString& strMoniker, TCQCDrvStats& cdstatsToFill)
{
    facCQCServer.QueryDriverStats(strMoniker, cdstatsToFill);
}


// Query the field definition of the indicated field
tCIDLib::TVoid
TCQCSrvAdminImpl::QueryFieldDef(const   TString&    strMoniker
                                , const TString&    strField
                                ,       TCQCFldDef& flddToFill)
{
    facCQCServer.QueryFieldDef(strMoniker, strField, flddToFill);
}


// Query field info for the indicated driver
tCIDLib::TVoid
TCQCSrvAdminImpl::QueryFieldInfo(const  TString&            strMoniker
                                , const TString&            strField
                                ,       tCIDLib::TCard4&    c4DriverListId
                                ,       tCIDLib::TCard4&    c4DriverId
                                ,       tCIDLib::TCard4&    c4FieldListId
                                ,       tCIDLib::TCard4&    c4FieldId
                                ,       tCQCKit::EFldTypes& eType)
{
    facCQCServer.QueryFieldInfo
    (
        strMoniker
        , strField
        , c4DriverListId
        , c4DriverId
        , c4FieldListId
        , c4FieldId
        , eType
    );
}


// Query the power status of a list of drivers
tCIDLib::TVoid TCQCSrvAdminImpl::
QueryPowerStates(const  tCIDLib::TKVPList&                  colDrivers
                ,       TFundVector<tCQCKit::EPowerStatus>& fcolStates)
{
    facCQCServer.QueryPowerStates(colDrivers, fcolStates);
}


// Get a status report on this CQCServer instance
tCIDLib::TVoid TCQCSrvAdminImpl::QueryStatusReport(TString& strErrList, const TCQCSecToken& sectUser)
{
    facCQCServer.QueryStatusReport(strErrList, sectUser);
}


// Pause or resume the indicated driver
tCIDLib::TVoid
TCQCSrvAdminImpl::PauseResumeDrv(const  TString&                strMoniker
                                , const tCIDLib::TBoolean       bPause
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.PauseResumeDrv(strMoniker, bPause, sectUser, eWait);
}


tCIDLib::TVoid TCQCSrvAdminImpl::
ReadDriverFields(const  tCIDLib::TCard4     c4DriverListId
                , const tCIDLib::TCard4     c4DriverId
                , const tCIDLib::TCard4     c4FieldListId
                , const tCIDLib::TCardList& fcolFldIds
                , const tCIDLib::TCardList& fcolSerialNums
                ,       tCIDLib::TCard4&    c4BufSz_mbufData
                ,       THeapBuf&           mbufData)
{
    facCQCServer.ReadDriverFields
    (
        c4DriverListId
        , c4DriverId
        , c4FieldListId
        , fcolFldIds
        , fcolSerialNums
        , c4BufSz_mbufData
        , mbufData
    );
}


tCIDLib::TVoid
TCQCSrvAdminImpl::ReadFields(const  TFldIOPacket&       fiopToRead
                            ,       tCIDLib::TCard4&    c4BytesRead
                            ,       THeapBuf&           mbufData)
{
    facCQCServer.ReadFields(fiopToRead, c4BytesRead, mbufData);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::ReadFieldsByName( const   TString&            strMoniker
                                    , const TVector<TString>&   colFieldNames
                                    ,       tCIDLib::TCard4&    c4BytesRead
                                    ,       THeapBuf&           mbufData)
{
    facCQCServer.ReadFieldsByName(strMoniker, colFieldNames, c4BytesRead, mbufData);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::ReconfigDriver(const TString& strMoniker, const TCQCSecToken& sectUser)
{
    return facCQCServer.ReconfigDriver(strMoniker, sectUser);
}


tCIDLib::TVoid TCQCSrvAdminImpl::RefreshCfg(const TString& strCfgType)
{
    facCQCServer.RefreshCfg(strCfgType);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::SetConfig(const   TString&                strMoniker
                            , const tCIDLib::TCard4         c4Bytes
                            , const THeapBuf&               mbufNewCfg
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.SetConfig(strMoniker, c4Bytes, mbufNewCfg, sectUser, eWait);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::SetVerboseLevel(  const   TString&                strMoniker
                                    , const tCQCKit::EVerboseLvls   eToSet)
{
    facCQCServer.SetVerboseLevel(strMoniker, eToSet);
}


tCIDLib::TVoid TCQCSrvAdminImpl::UnloadAllDrivers(const TCQCSecToken& sectUser)
{
    facCQCServer.RemoteUnloadAllDrivers(sectUser);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::UnloadDriver( const   TString&        strMonikerToUnload
                                , const TCQCSecToken&   sectUser)
{
    facCQCServer.RemoteUnloadDriver(strMonikerToUnload, sectUser);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::UpdateTriggers(const  TString&        strMoniker
                                , const TCQCSecToken&   sectUser)
{
    facCQCServer.UpdateTriggers(strMoniker, sectUser);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::WriteBoolByName(  const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const tCIDLib::TBoolean       bValue
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteBoolByName(strMoniker, strFldName, bValue, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteBoolFld( const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TBoolean       bValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteBoolFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, bValue, sectUser, eWait
    );
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteCardByName(  const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const tCIDLib::TCard4         c4Value
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteCardByName(strMoniker, strFldName, c4Value, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteCardFld( const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TCard4         c4Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Delegate to the facility class
    facCQCServer.WriteCardFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, c4Value, sectUser, eWait
    );
}


tCIDLib::TVoid
TCQCSrvAdminImpl::WriteFieldByName( const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const TString&                strValue
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteFieldByName(strMoniker, strFldName, strValue, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteFloatByName( const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const tCIDLib::TFloat8        f8Value
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteFloatByName(strMoniker, strFldName, f8Value, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteFloatFld(const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TFloat8        f8Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteFloatFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, f8Value, sectUser, eWait
    );
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteIntByName(const  TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TInt4          i4Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteIntByName(strMoniker, strFldName, i4Value, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteIntFld(  const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TInt4          i4Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteIntFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, i4Value, sectUser, eWait
    );
}


tCIDLib::TVoid
TCQCSrvAdminImpl::WriteStringByName(const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const TString&                strValue
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteStringByName(strMoniker, strFldName, strValue, sectUser, eWait);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::WriteSListFld(const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TStrList&      colValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteSListFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, colValue, sectUser, eWait
    );
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteSListByName( const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const tCIDLib::TStrList&      colValue
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteSListByName(strMoniker, strFldName, colValue, sectUser, eWait);
}


tCIDLib::TVoid
TCQCSrvAdminImpl::WriteStringFld(const  tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const TString&                strValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteStringFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, strValue, sectUser, eWait
    );
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteTimeByName(  const   TString&                strMoniker
                                    , const TString&                strFldName
                                    , const tCIDLib::TCard8         c8Value
                                    , const TCQCSecToken&           sectUser
                                    , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteTimeByName(strMoniker, strFldName, c8Value, sectUser, eWait);
}

tCIDLib::TVoid
TCQCSrvAdminImpl::WriteTimeFld( const   tCIDLib::TCard4         c4DriverListId
                                , const tCIDLib::TCard4         c4DriverId
                                , const tCIDLib::TCard4         c4FieldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const tCIDLib::TCard8         c8Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    facCQCServer.WriteTimeFld
    (
        c4DriverListId, c4DriverId, c4FieldListId, c4FldId, c8Value, sectUser, eWait
    );
}


// ---------------------------------------------------------------------------
//  TCQCSrvAdminImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSrvAdminImpl::Initialize()
{
    TParent::Initialize();
}

tCIDLib::TVoid TCQCSrvAdminImpl::Terminate()
{
    TParent::Terminate();
}

