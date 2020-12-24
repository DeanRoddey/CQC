//
// FILE NAME: CQCServer_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/04/2000
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
//  This file implements the more generic parts of the facility class for the
//  CQC server. Some private methods, which implement various specific pieces
//  of the server, are implemented in other files.
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
RTTIDecls(TFacCQCServer,TCQCSrvCore)




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCServer
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCServer: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We ask server core for a login and to enable the sending of events. We need
//  the former since some drivers will do things that require credentials.
//
TFacCQCServer::TFacCQCServer() :

    TCQCSrvCore
    (
        L"CQCServer"
        , L"CQC Server"
        , kCQCKit::ippnCQCSrvDefPort
        , kCQCServer::pszEvCQCServer
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::LogIn | tCQCSrvFW::ESrvOpts::EventsOut
    )
    , m_c4DriverListId(1)
    , m_c4GC100SerialNum(0)
    , m_c4JAPSerialNum(0)
    , m_colDriverList(tCIDLib::EAdoptOpts::NoAdopt)
    , m_porbsAdmin(nullptr)
    , m_thrMaint
      (
        L"CQCSrvMaintenanceThread"
        , TMemberFunc<TFacCQCServer>(this, &TFacCQCServer::eMaintThread)
      )
{
}

TFacCQCServer::~TFacCQCServer()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCServer: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Just a simple test of whether a particular driver and field exists. Since
//  its a fast operation, we just block with the overall lock
//
tCIDLib::TBoolean
TFacCQCServer::bCheckFieldExists(const  TString&            strMoniker
                                ,       tCIDLib::TBoolean&  bDevExists
                                , const TString&            strField
                                ,       tCIDLib::TBoolean&  bFldExists)
{
    // Assume the worst till proven otherwise
    bDevExists = kCIDLib::False;
    bFldExists = kCIDLib::False;

    TLocker lockrSync(&m_mtxLock);

    // Find the driver, don't throw if not found
    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index, kCIDLib::False);

    // If not found, we are done. out parms are already false
    if (!psdiTar)
        return kCIDLib::False;

    // We can set the device flag now since it does
    bDevExists = kCIDLib::True;

    //
    //  Ask it if the field exists, and not to throw if not found. We can
    //  just do a simple id lookup to test this.
    //
    tCIDLib::TCard4 c4Id;
    if (!psdiTar->sdrvDriver().bQueryFldId(strField, c4Id, kCIDLib::False))
        return kCIDLib::False;

    // Ok, we won the prize
    bFldExists = kCIDLib::True;
    return kCIDLib::True;
}


//
//  We get a list of packed driver/field list ids so that we can see if any
//  of them have any field changes in one go.
//
tCIDLib::TBoolean TFacCQCServer::
bCheckForAllNewFields(  const   tCIDLib::TCard4                 c4DriverListId
                        , const TFundVector<tCIDLib::TCard8>&   fcolIds)
{
    // Look while we do this
    TLocker lockrSync(&m_mtxLock);

    // Make sure the driver list's are the same, then not need even checking
    CheckDrvListId(c4DriverListId);

    // Loop through the drivers passed
    const tCIDLib::TCard4 c4Count = fcolIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Break out the ids. Driver is in the high and field list low
        const tCIDLib::TCard8 c8Cur = fcolIds[c4Index];
        tCIDLib::TCard4 c4DrvId = TRawBits::c4High32From64(c8Cur);
        tCIDLib::TCard4 c4FldListId = TRawBits::c4Low32From64(c8Cur);

        // If it's different, then we can stop now
        const TCQCServerBase& sdrvCur = m_colDriverList[c4DrvId]->sdrvDriver();
        if (sdrvCur.c4FieldListId() != c4FldListId)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Thius one allows a client to check whether any field changes have happened
//  for a single driver.
//
tCIDLib::TBoolean
TFacCQCServer::bCheckForNewFields(  const   tCIDLib::TCard4 c4DriverListId
                                    , const tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldListId)
{
    TLocker lockrSync(&m_mtxLock);
    CheckDrvListId(c4DriverListId);

    const TCQCServerBase& sdrvCur = m_colDriverList[c4DriverId]->sdrvDriver();
    return (sdrvCur.c4FieldListId() != c4FieldListId);
}


//
//  This one allows a client to check whether any drivers have been added
//  or removed on this server.
//
tCIDLib::TBoolean
TFacCQCServer::bCheckForNewDrivers(const tCIDLib::TCard4 c4DriverListId)
{
    TLocker lockrSync(&m_mtxLock);
    return (c4DriverListId != m_c4DriverListId);
}


//
//  Thius one allows a client to check if a given driver moniker is hosted
//  on this server.
//
//  For this, we won't lock the driver, since we are just checking for the
//  presence of the driver. We just lokc the main mutex to keep list from
//  changing as we lok through it.
//
tCIDLib::TBoolean
TFacCQCServer::bDriverIsLoaded(const TString& strMoniker) const
{
    TLocker lockrSync(&m_mtxLock);
    tCIDLib::TCard4 c4Index;
    return (psdiFindDrv(strMoniker, c4Index, kCIDLib::False) != nullptr);
}


//
//  We just pass through a call to the driver
//
tCIDLib::TBoolean
TFacCQCServer::bHasQueuedTimedWrite(const   TString&            strMoniker
                                    , const TString&            strField
                                    ,       tCIDLib::TCard8&    c8End)
{
    TLocker lockrSync(&m_mtxLock);

    // Look it up, throw if not found
    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    // Ask the driver if it has one
    return psdiTar->sdrvDriver().bHasQueuedTimedWrite(strField, c8End);
}


tCIDLib::TBoolean
TFacCQCServer::bQueryCommPorts(         tCIDLib::TStrList&  colToFill
                                , const tCIDLib::TBoolean   bLocalOnly
                                , const tCIDLib::TBoolean   bAvailOnly)
{
    // Refresh the GC-100 config first if they aren't just getting locals
    if (!bLocalOnly)
    {
        //
        //  Don't lock across a remote call, get it into a temp. This will also update
        //  the GC-100 com port factory if new config is found, so that any changes will
        //  show up for use.
        //
        {
            TGC100CfgList gcclPorts;
            tCIDLib::TCard4 c4SerNum = m_c4GC100SerialNum;
            if (facGC100Ser().bRefreshFactory(gcclPorts, c4SerNum, cuctxToUse().sectUser()))
            {
                // We got new data, so store it as our latest stuff after locking
                TLocker lockrSync(&m_mtxLock);
                m_gcclPorts = gcclPorts;
                m_c4GC100SerialNum = c4SerNum;
            }
        }

        // Do the same for Just Add Power
        {
            TJAPwrCfgList japlPorts;
            tCIDLib::TCard4 c4SerNum = m_c4JAPSerialNum;
            if (facJAPwrSer().bRefreshFactory(japlPorts, c4SerNum, cuctxToUse().sectUser()))
            {
                // We got new data, so store it as our latest stuff after locking
                TLocker lockrSync(&m_mtxLock);
                m_japlPorts = japlPorts;
                m_c4JAPSerialNum = c4SerNum;
            }
        }
    }

    return facCIDComm().bQueryPorts(colToFill, bLocalOnly, bAvailOnly);
}


//
//  We just pass through a call to the driver's bQueryData() backdoor call.
//  Queries are always wait mode so the caller doesn't get to choose.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryData(  const   TString&            strMoniker
                            , const TString&            strQueryType
                            , const TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

            c4Bytes = 0;
            pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryData
            (
                strQueryType, strDataName, c4Bytes, mbufToFill, kCIDLib::False
            );
        }

        // Wait for it to complete. Tell it to only release for us if it fails
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the return value out
        bRet = pdcmdWait->m_bVal;

        // And get the data out
        c4Bytes = pdcmdWait->m_c4Count;
        if (c4Bytes)
            mbufToFill.CopyIn(pdcmdWait->m_mbufVal, c4Bytes);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

//
//  We just pass through a call to the driver's bQueryData2() backdoor call.
//  Queries are always wait mode, so the caller doesn't get to choose.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryData2( const   TString&            strMoniker
                            , const TString&            strQueryType
                            ,       tCIDLib::TCard4&    c4IOBytes
                            ,       THeapBuf&           mbufIO)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

            pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryData
            (
                strQueryType, TString::strEmpty(), c4IOBytes, mbufIO, kCIDLib::True
            );
        }

        // Wait for it to complete. Tell it not to release for us unless it fails
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the return value out
        bRet = pdcmdWait->m_bVal;

        // And get the data out
        c4IOBytes = pdcmdWait->m_c4Count;
        if (c4IOBytes)
            mbufIO.CopyIn(pdcmdWait->m_mbufVal, c4IOBytes);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  This one allows a client to query a driver's object config.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryDriverInfo(const   TString&            strMoniker
                                ,       TCQCDriverObjCfg&   cqcdcToFill)
{
    try
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, don't throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index, kCIDLib::False);
        if (!psdiTar)
            return kCIDLib::False;

        psdiTar->sdrvDriver().QueryDrvConfigObj(cqcdcToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return kCIDLib::True;
}


//
//  We return a list of driver monikers and the states of those drivers. Return
//  only those that are offline, the others being online implicitly.
//
//  The return indicates if the driver id passed in is different from the one we gave
//  back.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryDriverStates(  const   tCIDLib::TCard4                 c4DriverListId
                                    ,       tCIDLib::TCardList&             fcolIds
                                    ,       TFundVector<tCQCKit::EDrvStates>& fcolStates)
{
    // Make sure we don't stream back bogus info
    fcolIds.RemoveAll();
    fcolStates.RemoveAll();

    // If the driver id list is out of sync, return false
    if (c4DriverListId != m_c4DriverListId)
        return kCIDLib::False;

    // Looks ok so let's do it
    TLocker lockrSync(&m_mtxLock);

    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TServerDriverInfo* psdiTar = psdiFindDrvById(m_c4DriverListId, c4Index);

        const tCQCKit::EDrvStates eState = psdiTar->sdrvDriver().eState();
        if (eState != tCQCKit::EDrvStates::Connected)
        {
            fcolIds.c4AddElement(c4Index);
            fcolStates.c4AddElement(eState);
        }
    }
    return kCIDLib::True;
}


//
//  We just pass through a call to the driver's bQueryTextVal() backdoor call.
//  Queries are always wait mode, so the caller doesn't get to choose.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryTextVal(const  TString&    strMoniker
                            , const TString&    strQueryType
                            , const TString&    strDataName
                            ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

            pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryTextVal
            (
                strQueryType, strDataName
            );
        }

        // Wait for it to complete. Tell it to release for us if it fails
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the return value out
        bRet = pdcmdWait->m_bVal;

        // And get the data out
        strToFill = pdcmdWait->m_strVal;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  We just pass through a call to the driver's bQueryVal() backdoor method. If the caller
//  says don't queue, then we make a direct call, else we queue it up.
//
//  DO NOT do 'noqueue' unless the driver knows it's going to happen, and generally only in
//  a case where it's just returning some basic value that it already has, i.e. it doesn't
//  have to access any data structure (that could change) in order to return the value.
//
tCIDLib::TBoolean
TFacCQCServer::bQueryVal(const  TString&            strMoniker
                        , const TString&            strValId
                        , const tCIDLib::TBoolean   bNoQueue)

{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);

            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            bRet = psdiTar->sdrvDriver().bQueryVal(strValId);
        }
         else
        {
            // Lock and get the command queued up
            TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryBoolVal
                (
                    strValId, TString::strEmpty(), kCIDLib::False
                );
            }

            // Wait for it to complete. Tell it to release for us if it fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            bRet = pdcmdWait->m_bVal;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


// We just pass through a call to the driver's bQueryVal2() backdoor method
tCIDLib::TBoolean
TFacCQCServer::bQueryVal2(  const   TString&            strMoniker
                            , const TString&            strValId
                            , const  TString&           strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);

            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            bRet = psdiTar->sdrvDriver().bQueryVal2(strValId, strValName);
        }
         else
        {
            // Lock and get the command queued up
            TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                // Queue up the command indicate we are going to wait
                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryBoolVal
                (
                    strValId, strValName, kCIDLib::True
                );
            }

            // Wait for it to complete. Tell it to release for us if it fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            bRet = pdcmdWait->m_bVal;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  These methods allow easy, by name, reading of fields of the various
//  supported types.
//
tCIDLib::TBoolean
TFacCQCServer::bReadBoolByName( const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerlNum
                                ,       tCIDLib::TBoolean&  bToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadBoolFld(strFldName, c4SerlNum, bToFill);
}


tCIDLib::TBoolean
TFacCQCServer::bReadCardByName( const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       tCIDLib::TCard4&    c4ToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadCardFld(strFldName, c4SerNum, c4ToFill);
}


//
//  A couple generic field reading methods. We have two that use the ids
//  for efficiency, one that returns the value as a string and another that
//  returns the value as a polymorpyhically flattened field value object.
//  And we have another simple moniker/field name one that returns the value
//  in the same two ways.
//
tCIDLib::TBoolean
TFacCQCServer::bReadField(  const   tCIDLib::TCard4     c4DrvListId
                            , const tCIDLib::TCard4     c4DrvId
                            , const tCIDLib::TCard4     c4FldListId
                            , const tCIDLib::TCard4     c4FldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       TString&            strValue
                            ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxLock);
    TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId, c4FldListId);

    return psdiTar->sdrvDriver().bReadField
    (
        c4FldListId, c4FldId, c4SerialNum, strValue, eType
    );
}

tCIDLib::TBoolean
TFacCQCServer::bReadField(  const   tCIDLib::TCard4     c4DrvListId
                            , const tCIDLib::TCard4     c4DrvId
                            , const tCIDLib::TCard4     c4FldListId
                            , const tCIDLib::TCard4     c4FldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4RetBytes
                            ,       THeapBuf&           mbufValue
                            ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxLock);

    //
    //  We don't pass the field list id here, since it's getting passed to
    //  the  read command anyway and it'll check it there, and avoid an
    //  extra lock.
    //
    TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);
    return psdiTar->sdrvDriver().bReadField
    (
        c4FldListId, c4FldId, c4SerialNum, c4RetBytes, mbufValue, eType
    );
}

tCIDLib::TBoolean
TFacCQCServer::bReadFieldByName(const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       TString&            strToFill
                                ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadFieldByName
    (
        strFldName, c4SerialNum, strToFill, eType
    );
}

tCIDLib::TBoolean
TFacCQCServer::bReadFieldByName(const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TCard4&    c4RetBytes
                                ,       THeapBuf&           mbufValue
                                ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadFieldByName
    (
        strFldName, c4SerialNum, c4RetBytes, mbufValue, eType
    );
}


//
//  Some type specific by name field reading methods
//
tCIDLib::TBoolean
TFacCQCServer::bReadFloatByName(const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       tCIDLib::TFloat8&   f8ToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadFloatFld(strFldName, c4SerNum, f8ToFill);
}


tCIDLib::TBoolean
TFacCQCServer::bReadIntByName(  const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       tCIDLib::TInt4&     i4ToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadIntFld(strFldName, c4SerNum, i4ToFill);
}


tCIDLib::TBoolean
TFacCQCServer::bReadSListByName(const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       tCIDLib::TStrList&  colToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadSListFld(strFldName, c4SerNum, colToFill);
}


tCIDLib::TBoolean
TFacCQCServer::bReadStringByName(const  TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       TString&            strToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadStringFld(strFldName, c4SerNum, strToFill);
}


tCIDLib::TBoolean
TFacCQCServer::bReadTimeByName( const   TString&            strMoniker
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4SerNum
                                ,       tCIDLib::TCard8&    c8ToFill)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().bReadTimeFld(strFldName, c4SerNum, c8ToFill);
}


// We just pass through a call to the driver's bSendData() backdoor method
tCIDLib::TBoolean
TFacCQCServer::bSendData(const  TString&            strMoniker
                        , const TString&            strSendType
                        ,       TString&            strDataName
                        ,       tCIDLib::TCard4&    c4IOBytes
                        ,       THeapBuf&           mbufIO
                        , const TCQCSecToken&       sectUser)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            pdcmdWait = psdiTar->sdrvDriver().pdcmdQSendData
            (
                strSendType
                , strDataName
                , c4IOBytes
                , mbufIO
                , tCQCKit::EDrvCmdWaits::Wait
            );
        }

        //
        //  If we got a command back, then we are waiting, so wait and get
        //  the values out.
        //
        if (pdcmdWait)
        {
            // Wait for it to complete. If it fails, the cmd is released for us
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            bRet = pdcmdWait->m_bVal;

            // And get the data out
            c4IOBytes = pdcmdWait->m_c4Count;
            if (c4IOBytes)
                mbufIO.CopyIn(pdcmdWait->m_mbufVal, c4IOBytes);
            strDataName = pdcmdWait->m_strName;
        }
         else
        {
            // We aren't going to stream anything back in this case
            c4IOBytes = 0;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  We just pass through a call to the driver's config query.
//
tCIDLib::TCard4
TFacCQCServer::c4QueryDrvConfig(const   TString&        strMoniker
                                ,       THeapBuf&       mbufToFill
                                , const TCQCSecToken&   sectUser                                )
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

            // Queue up the command. Queries always expect us to wait
            pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryConfig();
        }

        // Wait for it to complete. If it fails, the cmd is released for us
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the return value out
        c4Ret = pdcmdWait->m_c4Count;

        // And get the data out
        mbufToFill.CopyIn(pdcmdWait->m_mbufVal, c4Ret);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


//
//  Queries the driver configuration objects for all the drivers installed
//  on this server.
//
tCIDLib::TCard4 TFacCQCServer::
c4QueryDriverConfigObjs(TCollection<TCQCDriverObjCfg>& colToFill
                        , tCIDLib::TCard4&             c4DriverListId
                        , const TCQCSecToken&           sectUser)
{
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxLock);

    // Get the count of drivers and iterate them
    TCQCDriverObjCfg cqcdcCur;
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TServerDriverInfo* psdiTar = psdiFindDrvById(m_c4DriverListId, c4Index);

        psdiTar->sdrvDriver().QueryDrvConfigObj(cqcdcCur);
        colToFill.objAdd(cqcdcCur);
    }

    c4DriverListId = m_c4DriverListId;
    return c4Count;
}


//
//  Returns the driver list id for this server and the driver id for the
//  indicated driver. Since we are just getting a fundamental value out of
//  the driver, which never changes once set, we don't need to lock the driver
//  to get it out, we just lock the main mutex to keep the list from changing
//  on us while we are doing this query.
//
tCIDLib::TCard4
TFacCQCServer::c4QueryDriverId( const   TString&            strMoniker
                                ,       tCIDLib::TCard4&    c4DriverListId)
{
    TLocker lockrSync(&m_mtxLock);
    tCIDLib::TCard4 c4Id;
    psdiFindDrv(strMoniker, c4Id);
    c4DriverListId = m_c4DriverListId;
    return c4Id;
}


//
//  Returns the driver ids for a list of passed drivers.
//
//  We also return the make/model of each of them, which is required by
//  some clients of the polling engine (the primary user of this interface)
//  for validation purposes.)
//
//  See the header file level comments for the special locking considerations
//  we have here. We have to get the list of driver ids while we have the
//  overall list locked, then we let the list go. We can then iterate via
//  the driver list id and the remember driver ids.
//
tCIDLib::TCard4 TFacCQCServer::
c4QueryDriverIds(const  tCIDLib::TStrList&  colMonikers
                ,       tCIDLib::TCardList& fcolIds
                ,       tCIDLib::TKVPList&  colMakeModels)
{
    // Flush the caller's vector in case he forgot
    fcolIds.RemoveAll();
    colMakeModels.RemoveAll();


    TLocker lockrSync(&m_mtxLock);

    const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
    const tCIDLib::TCard4 c4RetDrvListId = m_c4DriverListId;

    tCIDLib::TCard4 c4TmpInd;
    TCQCDriverObjCfg cqcdcCur;
    tCQCKit::EDevCats eDevCat;
    TString strMoniker;
    TString strMake;
    TString strModel;
    tCIDLib::TCard4 c4MajVer, c4MinVer;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Look it up, don't throw if not found
        const TServerDriverInfo* psdiCur = psdiFindDrv
        (
            colMonikers[c4Index], c4TmpInd, kCIDLib::False
        );

        if (psdiCur)
        {
            fcolIds.c4AddElement(c4TmpInd);
            psdiCur->sdrvDriver().QueryDrvInfo
            (
                strMoniker, strMake, strModel, c4MajVer, c4MinVer, eDevCat
            );
            colMakeModels.objAdd(TKeyValuePair(strMake, strModel));
        }
         else
        {
           fcolIds.c4AddElement(kCIDLib::c4MaxCard);
        }
    }
    return c4RetDrvListId;
}


//
//  Returns a set of lists of various bits of info for each of the drivers
//  we have installed.
//
tCIDLib::TCard4 TFacCQCServer::
c4QueryDriverIdList(tCIDLib::TStrList&      colMonikers
                    , tCIDLib::TCardList&   fcolIds
                    , tCIDLib::TKVPList&    colMakeModels
                    , const TCQCSecToken&   sectUser)
{
    // Flush the caller's vectors in case he forgot
    colMonikers.RemoveAll();
    colMakeModels.RemoveAll();
    fcolIds.RemoveAll();

    TLocker lockrSync(&m_mtxLock);

    const tCIDLib::TCard4 c4RetListId = m_c4DriverListId;
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();

    tCQCKit::EDevCats eDevCat;
    TString strMake;
    TString strModel;
    TString strMoniker;
    tCIDLib::TCard4 c4MinVer, c4MajVer;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Look it up, throw if not found
        const TServerDriverInfo* psdiCur = psdiFindDrvById(c4RetListId, c4Index);

        // Get some info from it, and store away
        psdiCur->sdrvDriver().QueryDrvInfo
        (
            strMoniker, strMake, strModel, c4MajVer, c4MinVer, eDevCat
        );

        fcolIds.c4AddElement(c4Index);
        colMonikers.objAdd(strMoniker);
        colMakeModels.objAdd(TKeyValuePair(strMake, strModel));
    }
    return c4RetListId;
}


tCIDLib::TCard4 TFacCQCServer::
c4QueryDriverIdList2(           tCIDLib::TStrList&  colMonikers
                        ,       tCIDLib::TCardList& fcolIds
                        , const TCQCSecToken&       sectUser)
{
    // Flush the caller's vectors in case he forgot
    colMonikers.RemoveAll();
    fcolIds.RemoveAll();

    TLocker lockrSync(&m_mtxLock);

    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    tCQCKit::EDevCats eDevCat;
    TString strMake;
    TString strModel;
    TString strMoniker;
    tCIDLib::TCard4 c4MinVer, c4MajVer;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Look it up, throw if not found
        const TServerDriverInfo* psdiCur = psdiFindDrvById(m_c4DriverListId, c4Index);

        // Get some info from it, and store away
        psdiCur->sdrvDriver().QueryDrvInfo
        (
            strMoniker, strMake, strModel, c4MajVer, c4MinVer, eDevCat
        );

        fcolIds.c4AddElement(c4Index);
        colMonikers.objAdd(strMoniker);
    }
    return m_c4DriverListId;
}


//
//  Queries each of our drivers for any fields that have both the indicated
//  semantic type and the indicated access type (at least, could allow for
//  more.) In this case we are doing driver/field definition, not field
//  definitions because we are returning a heterogenous list of fields from
//  potentially different drivers.
//
tCIDLib::TCard4
TFacCQCServer::c4QuerySemFields(const   tCQCKit::TFldSTypeList& fcolTypes
                                , const tCQCKit::EReqAccess     eReqAccess
                                ,       TVector<TCQCDrvFldDef>& colToFill
                                , const TString&                strMoniker)
{
    TVector<TCQCFldDef> colFlds;
    colToFill.RemoveAll();
    tCIDLib::TCard4 c4FCnt;

    // Lock while we do this
    TLocker lockrSync(&m_mtxLock);

    // If we are looking for a specific moniker, do that, else check them all
    if (strMoniker.bIsEmpty())
    {
        const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TServerDriverInfo* psdiCur = psdiFindDrvById
            (
                m_c4DriverListId, c4Index
            );
            const TCQCServerBase& sdrvCur = psdiCur->sdrvDriver();

            colFlds.RemoveAll();
            c4FCnt = sdrvCur.c4QuerySemFields(fcolTypes, eReqAccess, colFlds);
            for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FCnt; c4FInd++)
            {
                colToFill.objAdd
                (
                    TCQCDrvFldDef(sdrvCur.strMoniker(), colFlds[c4FInd])
                );
            }
        }
    }
     else
    {
        tCIDLib::TCard4 c4Index;
        const TServerDriverInfo* psdiCur = psdiFindDrv(strMoniker, c4Index);
        const TCQCServerBase& sdrvCur = psdiCur->sdrvDriver();

        c4FCnt = sdrvCur.c4QuerySemFields(fcolTypes, eReqAccess, colFlds);
        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FCnt; c4FInd++)
        {
            colToFill.objAdd
            (
                TCQCDrvFldDef(sdrvCur.strMoniker(), colFlds[c4FInd])
            );
        }
    }
    return colToFill.c4ElemCount();
}


//
//  We pass this through to the target driver. It returns the names of all of
//  its fields that have the indicated access type(s), and the other variation that
//  also takes a field name regular expression.
//
tCIDLib::TCard4
TFacCQCServer::c4QueryFieldNames(const  TString&            strMoniker
                                ,       TVector<TString>&   colToFill
                                , const tCQCKit::EReqAccess eAccess)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().c4QueryFieldNames(colToFill, eAccess);
}

tCIDLib::TCard4
TFacCQCServer::c4QueryFieldNamesRX( const   TString&                strMoniker
                                    ,       tCQCKit::TFldDefList&   colToFill
                                    , const TString&                strNameRegEx
                                    , const tCQCKit::EReqAccess     eAccess)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    return psdiTar->sdrvDriver().c4QueryFieldNamesRX(colToFill, strNameRegEx, eAccess);
}


//
//  We pass this call through to the target driver, which returns all his
//  info on the fields he is managing.
//
tCIDLib::TCard4
TFacCQCServer::c4QueryFields(const  TString&                strMoniker
                            ,       tCQCKit::EDrvStates&    eState
                            ,       tCQCKit::TFldDefList&   colToFill
                            ,       tCIDLib::TCard4&        c4FldListId
                            ,       tCIDLib::TCard4&        c4DriverId
                            ,       tCIDLib::TCard4&        c4DriverListId)
{
    TLocker lockrSync(&m_mtxLock);

    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4DriverId);
    c4DriverListId = m_c4DriverListId;

    eState = psdiTar->sdrvDriver().eState();
    return psdiTar->sdrvDriver().c4QueryFields(colToFill, c4FldListId);
}

//
//  We just pass through a call to the driver's c4QueryVal() backdoor method. If the caller
//  says don't queue, then we make a direct call, else we queue it up.
//
//  DO NOT do 'noqueue' unless the driver knows it's going to happen, and generally only in
//  a case where it's just returning some basic value that it already has, i.e. it doesn't
//  have to access any data structure (that could change) in order to return the value.
//
tCIDLib::TCard4
TFacCQCServer::c4QueryVal(  const   TString&            strMoniker
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   bNoQueue)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);

            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            c4Ret = psdiTar->sdrvDriver().c4QueryVal(strValId);
        }
         else
        {
            // Lock and get the command queued up
            TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                // Queue up the command indicate we are going to wait
                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryCardVal
                (
                    strValId, TString::strEmpty(), kCIDLib::False
                );
            }

            // Wait for it to complete. Tell it not to release for us unless it fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            c4Ret = pdcmdWait->m_c4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


// We just pass through a call to the driver's c4QueryVal2() backdoor method
tCIDLib::TCard4
TFacCQCServer::c4QueryVal2( const   TString&            strMoniker
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);

            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            c4Ret = psdiTar->sdrvDriver().c4QueryVal2(strValId, strValName);
        }
         else
        {
            // Lock and get the command queued up
            TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                // Queue up the command indicate we are going to wait
                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryCardVal
                (
                    strValId, strValId, kCIDLib::True
                );
            }

            // Wait for it to complete. Tell it not to release for us unless if fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            c4Ret = pdcmdWait->m_c4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


// Queue up a send command backdoor command
tCIDLib::TCard4
TFacCQCServer::c4SendCmd(const  TString&        strMoniker
                        , const TString&        strCmdId
                        , const TString&        strParms
                        , const TCQCSecToken&   sectUser)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        // Lock and get the command queued up
        TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
        {
            TLocker lockrSync(&m_mtxLock);

            // Look up the driver, throw if not found
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

            // Queue up the command, take the driver's async policy
            pdcmdWait = psdiTar->sdrvDriver().pdcmdQSendCmd
            (
                strCmdId, strParms, tCQCKit::EDrvCmdWaits::Wait
            );
        }

        // If we got a cmd back, then we are waiting
        if (pdcmdWait)
        {
            //
            //  Wait for it to complete. Tell it to release if it fails, giving
            //  us back the error which we'll throw.
            //
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);
            c4Ret = pdcmdWait->m_c4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


// Just asks the target driver to cancel a timed field write
tCIDLib::TVoid
TFacCQCServer::CancelTimedWrite(const   TString&        strMoniker
                                , const TString&        strFld
                                , const TCQCSecToken&   sectUser)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().CancelTimedWrite(strFld);
}


// Just asks the target driver to clear out his stats fields
tCIDLib::TVoid TFacCQCServer::ClearDrvStats(const TString& strMoniker)
{
    TLocker lockrSync(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().ClearStats();
}


// Queue up a time field write command
tCIDLib::TVoid
TFacCQCServer::DoTimedWrite(const   TString&                strMoniker
                            , const TString&                strFld
                            , const TString&                strNewVal
                            , const TString&                strEndVal
                            , const tCIDLib::TCard4         c4Secs
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQTimedFldWrt
        (
            strFld, strNewVal, strEndVal, c4Secs, eWait
        );
    }

    //
    //  If we got a command back, then we are waiting, so wait for it to
    //  complete.
    //
    if (pdcmdWait)
    {
        // Tell it to release for us. If it fails, we'll just throw
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


//
//  We just pass through a call to the driver's i4QueryVal() backdoor method. If the caller
//  says don't queue, then we make a direct call, else we queue it up.
//
//  DO NOT do no queue unless the driver knows it's going to happen, and generally only in
//  a case where it's just returning some basic value that it already has, i.e. it doesn't
//  have to access any data structure (that could change) in order to return the value.
//
tCIDLib::TInt4
TFacCQCServer::i4QueryVal(  const   TString&            strMoniker
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   bNoQueue)
{
    // Lock and get the command queued up
    tCIDLib::TInt4 i4Ret = 0;
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);

            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            i4Ret = psdiTar->sdrvDriver().i4QueryVal(strValId);
        }
         else
        {
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                // Queue up the command indicate we are going to wait
                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryIntVal
                (
                    strValId, TString::strEmpty(), kCIDLib::False
                );
            }

            // Wait for it to complete. Tell it not to release for us unless if fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            i4Ret = pdcmdWait->m_i4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return i4Ret;
}


// We just pass through a call to the driver's i4QueryVal2() backdoor method
tCIDLib::TInt4
TFacCQCServer::i4QueryVal2( const   TString&            strMoniker
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    // Lock and get the command queued up
    tCIDLib::TInt4 i4Ret = 0;
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    try
    {
        if (bNoQueue)
        {
            TLocker lockrSync(&m_mtxLock);
            tCIDLib::TCard4 c4Index;
            TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
            i4Ret = psdiTar->sdrvDriver().i4QueryVal2(strValId, strValName);
        }
         else
        {
            {
                TLocker lockrSync(&m_mtxLock);

                // Look up the driver, throw if not found
                tCIDLib::TCard4 c4Index;
                TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

                // Queue up the command indicate we are going to wait
                pdcmdWait = psdiTar->sdrvDriver().pdcmdQQueryIntVal
                (
                    strValId, strValName, kCIDLib::True
                );
            }

            // Wait for it to complete. Tell it not to release for us unless if fails
            TError errFail;
            if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
                throw(errFail);

            // Make sure it gets released
            TCQCSrvCmdJan janCmd(pdcmdWait);

            // Get the return value out
            i4Ret = pdcmdWait->m_i4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return i4Ret;
}


tCIDLib::TVoid
TFacCQCServer::QueryDriverInfo( const   TString&                strMoniker
                                ,       tCQCKit::EDrvStates&    eStatus
                                ,       tCIDLib::TCard4&        c4ArchVer
                                ,       tCQCKit::TDevClassList& fcolDevClasses
                                ,       TString&                strMake
                                ,       TString&                strModel
                                , const TCQCSecToken&           sectUser)
{
    TLocker lockrList(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().QueryDrvInfo
    (
        eStatus, c4ArchVer, fcolDevClasses, strMake, strModel
    );
}


//
//  Returns status informaiton on the indicated driver. Since the values
//  we are getting (though the driver changes them) are fundamental values,
//  we don't lock the driver to get them out. We just lock the driver list
//  so that the driver can't go away while we are getting the info out.
//
tCIDLib::TVoid
TFacCQCServer::QueryDriverState(const   TString&                strMoniker
                                ,       tCQCKit::EDrvStates&    eStatus
                                ,       tCQCKit::EVerboseLvls&  eVerbose
                                ,       tCIDLib::TCard4&        c4ThreadId)
{
    TLocker lockrList(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().QueryState(eStatus, eVerbose, c4ThreadId);
}


//
//  Returns driver statistics for the indicated driver. This we just pass
//  on to the driver after locking it.
//
tCIDLib::TVoid
TFacCQCServer::QueryDriverStats(const TString& strMoniker, TCQCDrvStats& cdstatsToFill)
{
    TLocker lockrList(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    // And ask it for it's stats
    psdiTar->sdrvDriver().QueryStats(cdstatsToFill);
}


//
//  Returns the field def info for the indicated field in the indicated driver.
//  We just lock the driver and ask it for the field def info.
//
tCIDLib::TVoid
TFacCQCServer::QueryFieldDef(const  TString&    strMoniker
                            , const TString&    strField
                            ,       TCQCFldDef& flddToFill)
{
    TLocker lockrList(&m_mtxLock);

    tCIDLib::TCard4 c4Index;
    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().QueryFldDef(strField, flddToFill);
}


//
//  Returns all the ids that uniquely identify the indicated field. This
//  can be used later to read the field and to know if any changes have
//  occured.
//
tCIDLib::TVoid
TFacCQCServer::QueryFieldInfo(  const   TString&            strMoniker
                                , const TString&            strField
                                ,       tCIDLib::TCard4&    c4DriverListId
                                ,       tCIDLib::TCard4&    c4DriverId
                                ,       tCIDLib::TCard4&    c4FieldListId
                                ,       tCIDLib::TCard4&    c4FieldId
                                ,       tCQCKit::EFldTypes& eType)
{
    TLocker lockrList(&m_mtxLock);

    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4DriverId);
    c4DriverListId = m_c4DriverListId;
    psdiTar->sdrvDriver().QueryFldInfo(strField, c4FieldListId, c4FieldId, eType);
}


tCIDLib::TVoid TFacCQCServer::
QueryPowerStates(const  tCIDLib::TKVPList&                  colDrivers
                ,       TFundVector<tCQCKit::EPowerStatus>& fcolStates)
{

    //
    //  We want to do the least amount of work while the list is locked, so we just
    //  read all the values in an array of strings, then we process them afterwards.
    //  We also pre-build the field names into another array.
    //
    TObjArray<TString> objaNames(colDrivers.c4ElemCount());
    TObjArray<TString> objaVals(colDrivers.c4ElemCount());

    TString strFldName;
    const tCIDLib::TCard4 c4Count = colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colDrivers[c4Index];
        strFldName = L"PWR#";
        if (!kvalCur.strValue().bIsEmpty())
        {
            strFldName.Append(kvalCur.strValue());
            strFldName.Append(kCIDLib::chTilde);
        }
        strFldName.Append(L"Status");
        objaNames[c4Index] = strFldName;
    }

    // OK, now read the fields
    {
        // Lock while we do this
        TLocker lockrList(&m_mtxLock);

        //
        //  Loop through, find each driver, and store their power status field values.
        //  if the field isn't found, we just keep the empty string.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValuePair& kvalCur = colDrivers[c4Index];
            tCIDLib::TCard4 c4DrvInd;
            TServerDriverInfo* psdiTar = psdiFindDrv(kvalCur.strKey(), c4DrvInd);

            try
            {
                tCIDLib::TCard4 c4SerNum = 0;
                psdiTar->sdrvDriver().bReadStringFld
                (
                    objaNames[c4Index], c4SerNum, objaVals[c4Index]
                );
            }

            catch(...)
            {
                // Just eat it, the empty value is left in this slot
            }
        }
    }

    // OK, and now we can process them and store the results
    fcolStates.RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        fcolStates.c4AddElement(tCQCKit::eXlatEPowerStatus(objaVals[c4Index]));
}


//
//  Returns a list of any errors in this CQCServer that would be useful
//  in a high level status report.
//
tCIDLib::TVoid TFacCQCServer::QueryStatusReport(TString& strErrs, const TCQCSecToken& sectUser)
{
}


//
//  This is the 'bulk field reader' method. The caller passes info that
//  indicates the driver list, driver, and field list states at the time he
//  got information about the drivers and fields. And he passes a lsit of
//  field ids of the fields he wants and the last serial numbers he received
//  for those fields.
//
//  We make sure his ids valid, and if not an out of sync error is thrown
//  back so that he can resync. If the ids are ok, we format into the passed
//  buffer the values of those fields that have changed since he last
//  checked, based on serial number.
//
//  All the fields have to be for a particular driver.
//
tCIDLib::TVoid
TFacCQCServer::ReadDriverFields(const   tCIDLib::TCard4     c4DriverListId
                                , const tCIDLib::TCard4     c4DriverId
                                , const tCIDLib::TCard4     c4FldListId
                                , const tCIDLib::TCardList& fcolFldIds
                                , const tCIDLib::TCardList& fcolSerialNums
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufData)
{
    // Lock while we do this
    TLocker lockrList(&m_mtxLock);


    const TServerDriverInfo* psdiTar = psdiFindDrvById
    (
        c4DriverListId, c4DriverId, c4FldListId
    );
    const TCQCServerBase& sdrvSrc = psdiTar->sdrvDriver();

    //
    //  Make sure the field id and serial number collections have the same
    //  number of elements.
    //
    const tCIDLib::TCard4 c4Count = fcolFldIds.c4ElemCount();
    if (c4Count != fcolSerialNums.c4ElemCount())
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCSErrs::errcDrv_FldSerialCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , sdrvSrc.strMoniker()
        );
    }

    // If the driver is offline, then throw
    if (sdrvSrc.eState() != tCQCKit::EDrvStates::Connected)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NotOnline
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , sdrvSrc.strMoniker()
        );
    }

    // Make sure our field list id is still valid
    if (c4FldListId != sdrvSrc.c4FieldListId())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadFldListId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutOfSync
            , sdrvSrc.strMoniker()
        );
    }

    // Create a stream over the output buffer
    TBinMBufOutStream strmOut(&mbufData);

    //
    //  First we put out a format version for future use. We start the whole
    //  thing with a start object stream marker.
    //
    strmOut << tCIDLib::EStreamMarkers::StartObject
            << kCQCKit::c1FldFmtVersion;

    //
    //  Ok, let's loop through all the fields in the read list and load up
    //  the results for each one into the output buffer.
    //
    for (tCIDLib::TCard4 c4FldIndex = 0; c4FldIndex < c4Count; c4FldIndex++)
    {
        const tCIDLib::TCard4 c4FldId = fcolFldIds[c4FldIndex];
        const tCIDLib::TCard4 c4SerialNum = fcolSerialNums[c4FldIndex];

        strmOut << tCIDLib::EStreamMarkers::Frame
                << tCIDLib::TCard2(c4FldId);
        sdrvSrc.StreamValue(c4FldId, c4SerialNum, strmOut);
    }

    strmOut.Flush();
    c4BufSz = strmOut.c4CurPos();
}


//
//  This method is similar to ReadDriverFields above, so see the comments
//  there. This one though takes a field I/O packet, which can refer to
//  fields across multiple drivers. So it's like above, but potentially
//  applied to more than one driver in a single call.
//
tCIDLib::TVoid
TFacCQCServer::ReadFields(  const   TFldIOPacket&       fiopToRead
                            ,       tCIDLib::TCard4&    c4BytesRead
                            ,       THeapBuf&           mbufData)
{
    TLocker lockrSync(&m_mtxLock);

    // Check the driver id list and if its out of sync, throw.
    CheckDrvListId(fiopToRead.c4DriverListId());

    // Create a stream over the output buffer
    TBinMBufOutStream strmOut(&mbufData);

    //
    //  First we put out a format version for future use. We start the whole
    //  thing with a start object stream marker.
    //
    strmOut << tCIDLib::EStreamMarkers::StartObject
            << kCQCKit::c1FldFmtVersion;

    // Loop through the drivers, and for each driver, loop through its fields
    const tCIDLib::TCard4 c4DrvCount = fiopToRead.c4DriverCount();
    for (tCIDLib::TCard4 c4DrvInd = 0; c4DrvInd < c4DrvCount; c4DrvInd++)
    {
        const tCIDLib::TCard4 c4DriverId = fiopToRead.c4DriverIdAt(c4DrvInd);
        const tCIDLib::TCard4 c4FldListId = fiopToRead.c4FieldListIdAt(c4DrvInd);

        {
            // Let it check the field list id for us in this case as well
            const TServerDriverInfo* psdiTar = psdiFindDrvById
            (
                m_c4DriverListId, c4DriverId, c4FldListId
            );
            const TCQCServerBase& sdrvCur = psdiTar->sdrvDriver();

            //
            //  Check the online state of this driver. If it's offline, then
            //  we can just put out a single driver item that indicates this
            //  driver is offline, and we pass the driver state.
            //
            if (sdrvCur.eState() != tCQCKit::EDrvStates::Connected)
            {
                strmOut << tCIDLib::EStreamMarkers::StartObject
                        << kCQCKit::c1FldType_DriverOffline
                        << tCIDLib::TCard2(c4DriverId)
                        << tCIDLib::TCard1(psdiTar->sdrvDriver().eState());
            }
             else
            {
                //
                //  It appears to be ready, so put out an online record for the
                //  driver. We don't need to pass the state in this case, since it
                //  is implied.
                //
                strmOut << tCIDLib::EStreamMarkers::StartObject
                        << kCQCKit::c1FldType_DriverOnline
                        << tCIDLib::TCard2(c4DriverId);

                const tCIDLib::TCard4 c4FldCount = fiopToRead.c4FieldCountAt(c4DrvInd);
                for (tCIDLib::TCard4 c4FldInd = 0; c4FldInd < c4FldCount; c4FldInd++)
                {
                    // Get the field I/O data for this field
                    const TFldIOData& fiodCur = fiopToRead.fiodAt(c4DrvInd, c4FldInd);

                    //
                    //  If the serial number has changed for this one, then we
                    //  have to give back new data, else we just give back an
                    //  'unchanged' marker.
                    //
                    tCIDLib::TCard4 c4SerialNum = fiodCur.c4SerialNum();
                    const tCIDLib::TCard4 c4FldId = fiodCur.c4FieldId();
                    strmOut << tCIDLib::EStreamMarkers::StartObject
                            << kCQCKit::c1FldType_Field
                            << tCIDLib::TCard2(c4FldId);

                    sdrvCur.StreamValue(c4FldId, c4SerialNum, strmOut);
                }
            }
        }
    }

    // Flush the data out to the buffer
    strmOut.Flush();

    // And give back the byte count
    c4BytesRead = strmOut.c4CurPos();
}


//
//  A convenience method that allows the caller to ask for the values of
//  multiple fields (unconditionally returned, no serial numbers inovlved in
//  this one) via their field names, so it's useful for one-shot reads of
//  sets of fields where the overhead of gathering up all the ids required
//  to do sustained polling woudln't be worth it.
//
tCIDLib::TVoid
TFacCQCServer::ReadFieldsByName(const   TString&            strMoniker
                                , const TVector<TString>&   colFieldNames
                                ,       tCIDLib::TCard4&    c4BytesRead
                                ,       THeapBuf&           mbufData)
{
    // Assume worst case of no data
    c4BytesRead = 0;

    // Make sure that there are some names, else nothing to do
    const tCIDLib::TCard4 c4Count = colFieldNames.c4ElemCount();
    if (!c4Count)
        return;

    //
    //  Make a binary stream over the passed buffer. Default is not to
    //  adopt, which is what we want.
    //
    TBinMBufOutStream strmOut(&mbufData);

    TLocker lockrList(&m_mtxLock);

    tCIDLib::TCard4 c4DriverId;
    const TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4DriverId);
    const TCQCServerBase& sdrvSrc = psdiTar->sdrvDriver();

    //
    //  Loop through the field names. For each one, ask the driver to
    //  stream out the value of the field.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        sdrvSrc.StreamValue(colFieldNames[c4Index], strmOut);

    // Flush all the data to the buffer and set the bytes parm
    strmOut.Flush();
    c4BytesRead = strmOut.c4CurSize();
}


//
//  This is called if some client changes stored configuration that may affect us.
//  They will call this on loaded CQCServers. For now we don't have any. It's just for
//  future use at this point.
//
tCIDLib::TVoid TFacCQCServer::RefreshCfg(const TString& strCfgType)
{
}


//
//  We queue up a set config command. The caller can choose to wait or not.
//
tCIDLib::TVoid
TFacCQCServer::SetConfig(const  TString&                strMoniker
                        , const tCIDLib::TCard4         c4Bytes
                        , const THeapBuf&               mbufNewCfg
                        , const TCQCSecToken&           sectUser
                        , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We require at least power user
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::PowerUser);

    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQSetConfig(c4Bytes, mbufNewCfg, eWait);
    }

    //
    //  If we get a command back, then we are waiting, so wait for it to
    //  complete.
    //
    if (pdcmdWait)
    {
        // Tell it to release the cmd for us. If it fails we'll just throw
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


//
//  Set a new verbosity level. This just stores the new level on the driver
//  and returns. The driver will process the change later on its own thread
//  when it sees a change.
//
tCIDLib::TVoid
TFacCQCServer::SetVerboseLevel( const   TString&                strMoniker
                                , const tCQCKit::EVerboseLvls   eToSet)
{
    // Find the driver, thrown if not found
    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);
    psdiTar->sdrvDriver().SetNewVerbosity(eToSet);
}


//
//  We go get the latest config for the indicated driver, then we set the new triggers
//  on the driver who will apply it to his fields as appropriate, and update his
//  copy of the config data.
//
tCIDLib::TVoid
TFacCQCServer::UpdateTriggers(const TString& strMoniker, const TCQCSecToken& sectUser)
{
    // We require at least power user rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::PowerUser);

    // Get the config for this driver, and make sure it's configured for us
    TString strHost;
    TCQCDriverObjCfg cqcdcNew;
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Ver = 0;
        orbcIS->bDrvCfgQueryConfig(c4Ver, strMoniker, strHost, cqcdcNew, cuctxToUse().sectUser());
    }
    CIDAssert
    (
        strHost.bCompareI(TSysInfo::strIPHostName())
        , L"The driver is not configured for this CQC host"
    );

    TLocker lockrSync(&m_mtxLock);

    // Find our driver by that moniker
    tCIDLib::TCard4 c4Index;
    TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

    // And set its triggers
    psdiTar->sdrvDriver().SetTriggers(cqcdcNew.colTriggers());
}


//
//  These methods allow remote clients to store values into fields. We have
//  by name and by id versions for each supported field data types.
//
//  Because of a potential deadlock, we have to get both a call and field
//  lock before we do the store. The problem happens when we lock for field
//  access, and do the store. In the meantime, the driver's poll thread calls
//  into the driver for a poll, and decides it needs to store. So it will try
//  to get a field lock but we have it. If the value we write is different, the
//  store method would try to get a call lock and call a field changed method,
//  and we deadlock.
//
//  Otherwise, we could just get a field lock and allow the called driver
//  method to decide if it needed to get a call lock and do the field changed
//  method call, which it often times won't need to because the new value is
//  the same as the existing value.
//
//  So, it's not as efficient, but such is life.
//
tCIDLib::TVoid
TFacCQCServer::WriteBoolByName( const   TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TBoolean       bValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQBoolFld
        (
            strFldName, 0, 0, bValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteBoolFld(const   tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TBoolean       bValue
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQBoolFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, bValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteCardByName( const   TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TCard4         c4Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQCardFld
        (
            strFldName, 0, 0, c4Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteCardFld(const   tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TCard4         c4Value
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actually happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQCardFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, c4Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteFieldByName(const   TString&                strMoniker
                                , const TString&                strFldName
                                , const TString&                strValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQWriteFld
        (
            strFldName, 0, 0, strValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteFloatByName(const   TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TFloat8        f8Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQFloatFld
        (
            strFldName, 0, 0, f8Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteFloatFld(const  tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TFloat8        f8Value
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQFloatFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, f8Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteIntByName(  const   TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TInt4          i4Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQIntFld
        (
            strFldName, 0, 0, i4Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteIntFld( const   tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TInt4          i4Value
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQIntFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, i4Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteStringByName(const  TString&                strMoniker
                                , const TString&                strFldName
                                , const TString&                strValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        // Queue up the command, take the driver async policy
        pdcmdWait = psdiTar->sdrvDriver().pdcmdQStringFld
        (
            strFldName, 0, 0, strValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteStringFld(  const   tCIDLib::TCard4         c4DrvListId
                                , const tCIDLib::TCard4         c4DrvId
                                , const tCIDLib::TCard4         c4FldListId
                                , const tCIDLib::TCard4         c4FldId
                                , const TString&                strValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQStringFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, strValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TFacCQCServer::WriteSListByName(const   TString&                strMoniker
                                , const TString&                strFldName
                                , const TVector<TString>&       colValue
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQSListFld
        (
            strFldName, 0, 0, colValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteSListFld(const  tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TStrList&      colValue
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQSListFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, colValue, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}



tCIDLib::TVoid
TFacCQCServer::WriteTimeByName( const   TString&                strMoniker
                                , const TString&                strFldName
                                , const tCIDLib::TCard8&        c8Value
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        // Queue up the command, take the driver async policy
        pdcmdWait = psdiTar->sdrvDriver().pdcmdQTimeFld
        (
            strFldName, 0, 0, c8Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TFacCQCServer::WriteTimeFld(const   tCIDLib::TCard4         c4DrvListId
                            , const tCIDLib::TCard4         c4DrvId
                            , const tCIDLib::TCard4         c4FldListId
                            , const tCIDLib::TCard4         c4FldId
                            , const tCIDLib::TCard8&        c8Value
                            , const TCQCSecToken&           sectUser
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    // Lock and get the command queued up
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    {
        TLocker lockrSync(&m_mtxLock);

        //
        //  Look up the driver, throw if not found. We don't pass the
        //  field list id here. We are going to queue it up, and it'll
        //  be checked when the field write actuall happens.
        //
        TServerDriverInfo* psdiTar = psdiFindDrvById(c4DrvListId, c4DrvId);

        // Queue up the command, take the driver async policy
        pdcmdWait = psdiTar->sdrvDriver().pdcmdQTimeFld
        (
            TString::strEmpty(), c4FldListId, c4FldId, c8Value, eWait
        );
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCServer: Protected, inherited methods
// ---------------------------------------------------------------------------

// We keep trying to load our driver config
tCQCSrvFW::EStateRes
TFacCQCServer::eLoadConfig(tCIDLib::TCard4& c4WaitNext, const tCIDLib::TCard4 c4Count)
{
    try
    {
        //
        //  Ask the installation server for all of the driver config objects for
        //  our host.
        //
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCardList fcolVers;
        tCIDLib::TCard4 c4Ver = 0;
        const tCIDLib::TBoolean bRes = orbcIS->bDrvCfgQueryHostConfigs
        (
            c4Ver, TSysInfo::strIPHostName(), m_colCfgObjs, fcolVers, cuctxToUse().sectUser()
        );

        // If not found or it's empty, say we have no initial config
        if (!bRes || m_colCfgObjs.bIsEmpty())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_NoServerConfig
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCServer.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Ask to try again in 7 seconds
        c4WaitNext = 7000;
        return tCQCSrvFW::EStateRes::Retry;
    }

    return tCQCSrvFW::EStateRes::Success;
}


// We need to wait for the installation service to be ready
tCQCSrvFW::EStateRes TFacCQCServer::eWaitPrereqs(tCIDLib::TCard4& c4WaitNext)
{
    tCQCSrvFW::EStateRes eRes = tCQCSrvFW::EStateRes::Retry;
    try
    {
        tCQCKit::TInstSrvProxy orbcInstSrv(facCQCKit().orbcInstSrvProxy(2500));
        eRes = tCQCSrvFW::EStateRes::Success;
    }

    catch(TError& errToCatch)
    {
        //
        //  If not already logged and not the same error as last time,
        //  log it and make it the new last error.
        //
        if (facCQCServer.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    // If not successful, indicate to try again in 4 seconds
    if (eRes != tCQCSrvFW::EStateRes::Success)
        c4WaitNext = 4000;

    return eRes;
}


tCIDLib::TVoid TFacCQCServer::PreRegInit()
{
    //
    //  We have to install some class loaders on the macro engine
    //  facility, so that the standard CQC runtime classes, media
    //  access classes, and driver classes are available.
    //
    facCQCMEng().InitCMLRuntime(cuctxToUse().sectUser());
    facCQCMedia().InitCMLRuntime();
    facCQCGenDrvS().InitCMLRuntime();

    //
    //  Register our COM port factories, so ports are available to drivers,
    //  and we have to report available ports as well.
    //
    RegisterPortFactories();
}


// WE don't use any of the extra strings
tCIDLib::TVoid
TFacCQCServer::QueryCoreAdminInfo(  TString&    strCoreAdminBinding
                                    , TString&  strCoreAdminDesc
                                    , TString&
                                    , TString&
                                    , TString&
                                    , TString&  )
{
    strCoreAdminBinding = TCQCSrvAdminClientProxy::strCoreAdminBinding;
    strCoreAdminDesc = L"CQC Server Core Admin Object";
}


tCIDLib::TVoid TFacCQCServer::RegisterSrvObjs()
{
    m_porbsAdmin = new TCQCSrvAdminImpl;
    facCIDOrb().RegisterObject(m_porbsAdmin, tCIDLib::EAdoptOpts::Adopt);

    // Register our admin object with the rebinder object
    const TString& strOurHost = TSysInfo::strIPHostName();
    facCIDOrbUC().RegRebindObj
    (
        m_porbsAdmin->ooidThis()
        , TString(TCQCSrvAdminServerBase::strAdminScope)
            + TString(kCIDLib::pszTreeSepChar)
            + strOurHost
        , strMsg(kCQCSMsgs::midScope_CQCSrvAdminsByHost, strOurHost)
    );

    // Store the admin interface's object id for later use
    m_ooidAdmin = m_porbsAdmin->ooidThis();
}


tCIDLib::TVoid TFacCQCServer::StopWorkerThreads()
{
    //
    //  Stop the maintenance thread first. We can't really do it in parallel. But
    //  it should stop really quickly.
    //
    try
    {
        m_thrMaint.ReqShutdownSync(5000);
        m_thrMaint.eWaitForDeath(5000);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }

    //
    //  And how let's unload the drivers. No need to lock at this point
    //  since clients can't get in anymore. Just on the off chance, bump
    //  the driver list id, so that they'll throw an exception back to
    //  caller. We first start them shutting down all in parallel.
    //
    m_c4DriverListId++;
    UnloadDrivers();
}


//
//  We use this to start our drivers, and some helper threads we use. We waiting
//  for the installation server to show up in our prerequisites wait callback, so
//  it should be there now.
//
tCIDLib::TVoid TFacCQCServer::StartWorkerThreads()
{
    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy(8000);
    const tCIDLib::TCard4 c4Count = m_colCfgObjs.c4ElemCount();
    TCQCDriverCfg cqcdcLatest;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get a copy of current config object and let's try to add it
        TCQCDriverObjCfg cqcdcCur = m_colCfgObjs[c4Index];
        try
        {
            //
            //  Query the original manifest info and see if there are any changes.
            //  If so, then update the base part of our configuration.
            //
            if (orbcIS->bQueryDrvManifest(cqcdcCur.strMake(), cqcdcCur.strModel(), cqcdcLatest))
            {
                if (!cqcdcCur.bSameBase(cqcdcLatest))
                {
                    // Delete any current driver files to make us download them again
                    DeleteDrvFiles(cqcdcCur);

                    // And now update the base content
                    cqcdcCur.UpdateBase(cqcdcLatest);
                }
            }
             else
            {
                // Log that we couldn't refresh the driver config
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCSMsgs::midStatus_DrvCfgRefresh
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , cqcdcCur.strMoniker()
                    , cqcdcCur.strMake()
                    , cqcdcCur.strModel()
                    , cqcdcCur.strVersion()
                );
            }

            LoadADriver(cqcdcCur);

            //
            //  Pause a little bit between each one, so that they all aren't trying
            //  to bang away at the same time getting started. Our parent class
            //  provides a helper to do this in a shutdown friendly way.
            //
            if (bWait(250))
                return;
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCSErrs::errcDrv_CouldNotLoad
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , cqcdcCur.strMoniker()
                );
            }
        }

        catch(...)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCSErrs::errcDrv_CouldNotLoad
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , cqcdcCur.strMoniker()
                );
            }
        }
    }

    // Start our maintenace thread which scavenges removed drivers
    m_thrMaint.Start();
}


//
//  We only do our non-driver stuff here. Each driver also has a faux binding that
//  provides info about it and that points to our server admin interface. But those
//  will get dropped when the drivers are unloaded.
//
tCIDLib::TVoid TFacCQCServer::UnbindSrvObjs(tCIDOrbUC::TNSrvProxy& orbcNS)
{
    // Build up a list of bindings to remove
    TString  strBinding;
    tCIDLib::TStrList colToRemove(64UL);

    // Do our driver admin interface
    strBinding = TCQCSrvAdminServerBase::strAdminScope;
    strBinding.Append(L'/');
    strBinding.Append(TSysInfo::strIPHostName());
}



// ---------------------------------------------------------------------------
//  TFacCQCServer: Private, non-virtual methods
// ---------------------------------------------------------------------------

// If the passed drv list id is not the same as ours, throw and out of sync
tCIDLib::TVoid
TFacCQCServer::CheckDrvListId(const tCIDLib::TCard4 c4DriverListId) const
{
    if (c4DriverListId != m_c4DriverListId)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCSErrs::errcFld_DrvListOutOfSync
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutOfSync
        );
    }
}


//
//  We start up the m_thrMaint thread on this method. It will wake up periodically and
//  look for any drivers that have reach terminated state. That means that they were asked
//  to stop and have completed that process.
//
//  We also use it to try to get non-local serial port config data loaded if we don't
//  get that stuff initially at startup, which could happen.
//
tCIDLib::EExitCodes
TFacCQCServer::eMaintThread(TThread& thrThis, tCIDLib::TVoid*)
{
    tCIDLib::TCard4 c4Rounds = 0;
    while(kCIDLib::True)
    {
        c4Rounds++;

        // Sleep a while. If asked to stop while sleeping, exit
        if (!thrThis.bSleep(2500))
            break;

        // Check for drivers to unload
        UnloadDeadDrivers();

        //
        //  If we are yet to be able to load our GC-100 config data from the MS, then
        //  try it again every 15'ish seconds.
        //
        if (!m_c4GC100SerialNum && !(c4Rounds % 7))
        {
            try
            {
                tCIDLib::TCard4 c4SerNum = m_c4GC100SerialNum;
                TGC100CfgList gcclPorts;
                if (facGC100Ser().bRefreshFactory(gcclPorts, c4SerNum, cuctxToUse().sectUser()))
                {
                    TLocker lockrSync(&m_mtxLock);
                    m_gcclPorts = gcclPorts;
                    m_c4GC100SerialNum = c4SerNum;
                }
            }

            catch(TError& errToCatch)
            {
                if (bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }

        // Same for Just Add Power
        if (!m_c4JAPSerialNum && !(c4Rounds % 5))
        {
            try
            {
                tCIDLib::TCard4 c4SerNum = m_c4JAPSerialNum;
                TJAPwrCfgList japlPorts;
                if (facJAPwrSer().bRefreshFactory(japlPorts, c4SerNum, cuctxToUse().sectUser()))
                {
                    TLocker lockrSync(&m_mtxLock);
                    m_japlPorts = japlPorts;
                    m_c4JAPSerialNum = c4SerNum;
                }
            }

            catch(TError& errToCatch)
            {
                if (bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
    }

    return tCIDLib::EExitCodes::Normal;
}



//
//  Finds the driver with the indicated moniker in our driver list and
//  returns the index of it as well as a pointer to it.
//
const TServerDriverInfo*
TFacCQCServer::psdiFindDrv( const   TString&            strMonikerToFind
                            ,       tCIDLib::TCard4&    c4Index
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    if (c4Count)
    {
        for (c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TServerDriverInfo* psdiCur = m_colDriverList[c4Index];

            // If this is our guy, return the pointer to it
            if (psdiCur->sdrvDriver().strMoniker() == strMonikerToFind)
                return psdiCur;
        }
    }

    if (bThrowIfNot)
    {
        // We throw a kit error here, since clients need to know about this one
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMonikerToFind
            , TSysInfo::strIPHostName()
        );
    }

    c4Index = kCIDLib::c4MaxCard;
    return nullptr;
}


TServerDriverInfo*
TFacCQCServer::psdiFindDrv( const   TString&            strMonikerToFind
                            ,       tCIDLib::TCard4&    c4Index
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
    if (c4Count)
    {
        for (c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TServerDriverInfo* psdiCur = m_colDriverList[c4Index];

            // If this is our guy, return the pointer to it
            if (psdiCur->sdrvDriver().strMoniker() == strMonikerToFind)
                return psdiCur;
        }
    }

    if (bThrowIfNot)
    {
        // We throw a kit error here, since clients need to know about this one
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMonikerToFind
            , TSysInfo::strIPHostName()
        );
    }

    c4Index = kCIDLib::c4MaxCard;
    return nullptr;
}


//
//  Finds the driver in our list indicated by the passed ids. If any of the
//  ids are out of sync, we throw an out of sync error. If the field list
//  id is bad, we through a bad field list error.
//
//  They can pass zero for the field list id if they don't want us to check
//  that one (because they are going to subsequently call into the driver
//  and that call will check it anyway, saving an extra trip/lock.)
//
TServerDriverInfo*
TFacCQCServer::psdiFindDrvById( const   tCIDLib::TCard4 c4DriverListId
                                , const tCIDLib::TCard4 c4DriverId
                                , const tCIDLib::TCard4 c4FieldListId)
{
    // Check the diver list id and throw if out of sync
    CheckDrvListId(c4DriverListId);

    TServerDriverInfo* psdiCur = m_colDriverList[c4DriverId];

    if (c4FieldListId
    &&  (c4FieldListId != psdiCur->sdrvDriver().c4FieldListId()))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadFldListId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutOfSync
            , psdiCur->sdrvDriver().strMoniker()
        );
    }

    return psdiCur;
}


//
//  Called on startup to register com port factories for anything other
//  than the default local ports factory.
//
tCIDLib::TVoid TFacCQCServer::RegisterPortFactories()
{
    //
    //  Watch for a CQC_REMPORTSRV= environment variable. This tells us to
    //  register a remote port server port factory, which will make those
    //  serial ports available for drivers. This is for development purposes
    //  so that a developer can connect to a remote machine via our port server.
    //
    TString strRemPort;
    if (TProcEnvironment::bFind(L"CQC_REMPORTSRV", strRemPort))
    {
        try
        {
            TIPEndPoint ipepRemSrv(strRemPort, tCIDSock::EAddrTypes::Unspec);
            facCIDComm().RegisterFactory(new TCQCRemSrvPortFactory(ipepRemSrv));

            tRPortClient::TSerialPortClient orbcSrv
                            = facCQCRPortClient().orbcMakePortProxy(ipepRemSrv);
            orbcSrv->CloseAllPorts();
        }

        catch(TError& errToCatch)
        {
            if (facCQCServer.bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }
        }
    }

    //
    //  Register a GC-100 port factory. It will get updated each time we are asked for our
    //  available serial ports, which is called when the user is installing a driver, so we
    //  will pick up any changes at that point (the only time we need to.)
    //
    //  We don't care if we get any config back or not. If not, then there just isn't any
    //  yet, or it cannot be gotten to because of some fundamental issue, so we want to set
    //  the initial, empty info. This means that any drivers that depend on GC-100 ports
    //  won't connect.
    //
    try
    {
        facGC100Ser().eQueryConfig(m_gcclPorts, m_c4GC100SerialNum, cuctxToUse().sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
    facGC100Ser().bRegisterFactory(m_gcclPorts);

    // Do the same for JAP
    try
    {
        facJAPwrSer().eQueryConfig(m_japlPorts, m_c4JAPSerialNum, cuctxToUse().sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
    facJAPwrSer().bRegisterFactory(m_japlPorts);
}

