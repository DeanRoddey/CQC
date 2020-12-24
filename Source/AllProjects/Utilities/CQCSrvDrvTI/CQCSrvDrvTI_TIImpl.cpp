//
// FILE NAME: CppDrvTest_SrvIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//  Implements our faux CQCServer driver admin interface, which we use to hard wire
//  the client driver interface to the server driver.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSrvDrvTI_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSrvDrvTI, TCQCSrvAdminServerBase)




// ---------------------------------------------------------------------------
//  TCQCSrvDrvTI: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCSrvDrvTI::TCQCSrvDrvTI() :

    m_psdrvTar(nullptr)
{
}

TCQCSrvDrvTI::TCQCSrvDrvTI(const TOrbObjId& ooidThis, TCQCServerBase* const psdrvTar) :

    TCQCSrvAdminServerBase(ooidThis)
    , m_psdrvTar(psdrvTar)
{

}

TCQCSrvDrvTI::~TCQCSrvDrvTI()
{
    // We don't own the driver object
}



// ---------------------------------------------------------------------------
//  TCQCSrvDrvTI: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass to the driver
tCIDLib::TBoolean
TCQCSrvDrvTI::bCheckFieldExists(const   TString&            strMoniker
                                ,       tCIDLib::TBoolean&  bDevExists
                                , const TString&            strField
                                ,       tCIDLib::TBoolean&  bFldExists)
{
    //
    //  Ask it if the field exists, and not to throw if not found. We can
    //  just do a simple id lookup to test this.
    //
    tCIDLib::TCard4 c4Id;
    if (!m_psdrvTar->bQueryFldId(strField, c4Id, kCIDLib::False))
        return kCIDLib::False;

    // Ok, we won the prize
    bFldExists = kCIDLib::True;
    return kCIDLib::True;

}


// Ignore this one
tCIDLib::TBoolean TCQCSrvDrvTI::
bCheckForAllNewFields(  const   tCIDLib::TCard4                 c4DriverListId
                        , const TFundVector<tCIDLib::TCard8>&   fcolIds)
{
    return kCIDLib::False;
}


//
//  The only one that matters is the field list id, the others don't even exist in
//  this case. There's only one driver.
//
tCIDLib::TBoolean
TCQCSrvDrvTI::bCheckForNewFields(const  tCIDLib::TCard4 c4DriverListId
                                , const tCIDLib::TCard4 c4DriverId
                                , const tCIDLib::TCard4 c4FieldListId)
{
    return (m_psdrvTar->c4FieldListId() != c4FieldListId);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bCheckForNewDrivers(const tCIDLib::TCard4 c4DriverListId)
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bHasQueuedTimedWrite( const   TString&            strMoniker
                                    , const TString&            strField
                                    ,       tCIDLib::TCard8&    c8EndTime)
{
    return kCIDLib::False;
}


// Load up drivers based on the passed list of monikers
tCIDLib::TBoolean
TCQCSrvDrvTI::bLoadDrivers( const   tCIDLib::TStrList&  colMonikers
                            , const TCQCSecToken&       sectUser)
{
    return kCIDLib::False;
}



//
//  Don't need this. The IDE/harness programs get this info locally.
//
tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryCommPorts(          tCIDLib::TStrList&  colToFill
                                , const tCIDLib::TBoolean   bLocalOnly
                                , const tCIDLib::TBoolean   bAvailOnly)
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryData(   const   TString&            strMoniker
                            , const TString&            strQueryType
                            , const TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        c4Bytes = 0;
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryData
        (
            strQueryType, strDataName, c4Bytes, mbufToFill, kCIDLib::False
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        bRet = pdcmdWait->m_bVal;
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


tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryData2(  const   TString&            strMoniker
                            , const TString&            strQueryType
                            ,       tCIDLib::TCard4&    c4IOBytes
                            ,       THeapBuf&           mbufIO)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryData
        (
            strQueryType, TString::strEmpty(), c4IOBytes, mbufIO, kCIDLib::True
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        bRet = pdcmdWait->m_bVal;
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


tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryDriverInfo( const   TString&            strMoniker
                                ,       TCQCDriverObjCfg&   cqcdcToFill)
{
    try
    {
        m_psdrvTar->QueryDrvConfigObj(cqcdcToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return kCIDLib::True;
}


// Ignore this one
tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryDriverStates(const  tCIDLib::TCard4
                                ,       tCIDLib::TCardList&
                                ,       TFundVector<tCQCKit::EDrvStates>&   )
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryTextVal( const   TString&    strMoniker
                                , const TString&    strQueryType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryTextVal
        (
            strQueryType, strDataName
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        bRet = pdcmdWait->m_bVal;
        strToFill = pdcmdWait->m_strVal;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


// We always make a direct call, so the 'no queue' parameter is ignored
tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryVal(const   TString&            strMoniker
                        , const TString&            strValId
                        , const tCIDLib::TBoolean   )
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryBoolVal
        (
            strValId, TString::strEmpty(), kCIDLib::False
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        bRet = pdcmdWait->m_bVal;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}

tCIDLib::TBoolean
TCQCSrvDrvTI::bQueryVal2(const  TString&            strMoniker
                        , const TString&            strValId
                        , const TString&            strValName
                        , const tCIDLib::TBoolean   bNoQueue)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryBoolVal
        (
            strValId, strValName, kCIDLib::True
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        bRet = pdcmdWait->m_bVal;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadBoolByName(          tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       tCIDLib::TBoolean&  bToFill)
{
    return m_psdrvTar->bReadBoolFld(strFldName, c4SerialNum, bToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadCardByName(          tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4ToFill)
{
    return m_psdrvTar->bReadCardFld(strFldName, c4SerialNum, c4ToFill);
}


// Driver list and driver ids don't matter in this scenario
tCIDLib::TBoolean
TCQCSrvDrvTI::bReadField(const  tCIDLib::TCard4
                        , const tCIDLib::TCard4
                        , const tCIDLib::TCard4     c4FieldListId
                        , const tCIDLib::TCard4     c4FieldId
                        ,       tCIDLib::TCard4&    c4SerialNum
                        ,       TString&            strValue
                        ,       tCQCKit::EFldTypes& eType)
{
    return m_psdrvTar->bReadField
    (
        c4FieldListId, c4FieldId, c4SerialNum, strValue, eType
    );
}

tCIDLib::TBoolean
TCQCSrvDrvTI::bReadField(   const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4     c4FieldListId
                            , const tCIDLib::TCard4     c4FieldId
                            ,       tCIDLib::TCard4&    c4SerialNum
                            ,       tCIDLib::TCard4&    c4RetBytes
                            ,       THeapBuf&           mbufValue
                            ,       tCQCKit::EFldTypes& eType)
{
    return m_psdrvTar->bReadField
    (
        c4FieldListId, c4FieldId, c4SerialNum, c4RetBytes, mbufValue, eType
    );
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadFieldByName(         tCIDLib::TCard4&    c4SerialNum
                                , const TString&            strMoniker
                                , const TString&            strFldName
                                ,       TString&            strToFill
                                ,       tCQCKit::EFldTypes& eType)
{
    return m_psdrvTar->bReadFieldByName(strFldName, c4SerialNum, strToFill, eType);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadFieldByName(         tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       tCIDLib::TCard4&    c4RetBytes
                                ,       THeapBuf&           mbufValue
                                ,       tCQCKit::EFldTypes& eType)
{
    return m_psdrvTar->bReadFieldByName
    (
        strFldName, c4SerialNum, c4RetBytes, mbufValue, eType
    );
}



tCIDLib::TBoolean
TCQCSrvDrvTI::bReadFloatByName(         tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       tCIDLib::TFloat8&   f8ToFill)
{
    return m_psdrvTar->bReadFloatFld(strFldName, c4SerialNum, f8ToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadIntByName(       tCIDLib::TCard4&    c4SerialNum
                            , const TString&
                            , const TString&            strFldName
                            ,       tCIDLib::TInt4&     i4ToFill)
{
    return m_psdrvTar->bReadIntFld(strFldName, c4SerialNum, i4ToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadSListByName(         tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       TVector<TString>&   colToFill)
{
    return m_psdrvTar->bReadSListFld(strFldName, c4SerialNum, colToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadStringByName(        tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       TString&            strToFill)
{
    return m_psdrvTar->bReadStringFld(strFldName, c4SerialNum, strToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bReadTimeByName(          tCIDLib::TCard4&    c4SerialNum
                                , const TString&
                                , const TString&            strFldName
                                ,       tCIDLib::TCard8&    c8ToFill)
{
    return m_psdrvTar->bReadTimeFld(strFldName, c4SerialNum, c8ToFill);
}


tCIDLib::TBoolean
TCQCSrvDrvTI::bSendData(const   TString&
                        , const TString&            strSendType
                        ,       TString&            strDataName
                        ,       tCIDLib::TCard4&    c4IOBytes
                        ,       THeapBuf&           mbufIO
                        , const TCQCSecToken&       )
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQSendData
        (
            strSendType
            , strDataName
            , c4IOBytes
            , mbufIO
            , tCQCKit::EDrvCmdWaits::Wait
        );

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
//  Returns a list of local audio devices on this machine. This one we can
//  handle ourself and doesn't require any synchronization.
//
tCIDLib::TCard4 TCQCSrvDrvTI::c4QueryAudioDevs(TVector<TString>& colToFill)
{
    colToFill.RemoveAll();
    return 0;
}


tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryDriverConfigObjs(  TVector<TCQCDriverObjCfg>& colToFill
                                        , tCIDLib::TCard4&         c4DriverListId
                                        , const TCQCSecToken&      )
{
    colToFill.RemoveAll();

    TCQCDriverObjCfg cqcdcCur;
    m_psdrvTar->QueryDrvConfigObj(cqcdcCur);
    colToFill.objAdd(cqcdcCur);

    // There is no driver list id in this case, so just always return 1
    c4DriverListId = 1;

    // And only one driver returned ever
    return 1;
}


// There is no driver id or driver list id, so just return 1
tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryDriverId(  const   TString&
                                ,       tCIDLib::TCard4&    c4DriverListId)
{
    c4DriverListId = 1;
    return 1;
}


tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryDriverIds( const tCIDLib::TStrList&
                                ,     tCIDLib::TCardList&   fcolIds
                                ,     tCIDLib::TKVPList&    colMakeModels)
{
    fcolIds.RemoveAll();
    colMakeModels.RemoveAll();
    return 0;
}


tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryDriverIdList(  tCIDLib::TStrList&      colMonikers
                                    , tCIDLib::TCardList&   fcolIds
                                    , tCIDLib::TKVPList&    colMakeModels
                                    , const TCQCSecToken&   )
{
    colMonikers.RemoveAll();
    fcolIds.RemoveAll();
    colMakeModels.RemoveAll();
    return 0;
}


tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryDriverIdList2( tCIDLib::TStrList&      colMonikers
                                    , tCIDLib::TCardList&   fcolIds
                                    , const TCQCSecToken&   )
{
    colMonikers.RemoveAll();
    fcolIds.RemoveAll();
    return 0;
}


//
//  Return the names of all fields of the driver that have the requested access
//  or access and a name that matches a given regular expression.
tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryFieldNames(const   TString&
                                ,       TVector<TString>&   colToFill
                                , const tCQCKit::EReqAccess eAccess)
{
    return m_psdrvTar->c4QueryFieldNames(colToFill, eAccess);
}

tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryFieldNamesRX(  const   TString&
                                    ,       tCQCKit::TFldDefList&   colToFill
                                    , const TString&                strRegEx
                                    , const tCQCKit::EReqAccess     eAccess)
{
    return m_psdrvTar->c4QueryFieldNamesRX(colToFill, strRegEx, eAccess);
}


// Return all the fields of the indicated driver
tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryFields(const   TString&
                            ,       tCQCKit::EDrvStates&    eState
                            ,       TVector<TCQCFldDef>&    colToFill
                            ,       tCIDLib::TCard4&        c4FldListId
                            ,       tCIDLib::TCard4&        c4DriverId
                            ,       tCIDLib::TCard4&        c4DriverListId)
{
    // These is no driver list or driver id so return 1
    c4DriverId = 1;
    c4DriverListId = 1;

    eState = m_psdrvTar->eState();
    return m_psdrvTar->c4QueryFields(colToFill, c4FldListId);
}


//
//  Returns all fields of a given semantic type and access type. For now, we don't
//  need this, so we just return nothing.
//
tCIDLib::TCard4 TCQCSrvDrvTI::
c4QuerySemFields(const  TFundVector<tCQCKit::EFldSTypes>&   fcolTypes
                , const tCQCKit::EReqAccess                 eReqAccess
                ,       TVector<TCQCDrvFldDef>&             colToFill
                , const TString&                            strMoniker)
{
    colToFill.RemoveAll();
    return 0;
}


//
//  Returns a list of local TTS voices on this machine. This one we can
//  handle ourself and doesn't require any synchronization.
//
tCIDLib::TCard4 TCQCSrvDrvTI::c4QueryTTSVoices(TVector<TString>& colToFill)
{
    TSpeech::QueryVoiceList(colToFill);
    return colToFill.c4ElemCount();
}


// Call the driver backdoor method to query a Card4 value
tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryVal(const  TString&
                        , const TString&            strValId
                        , const tCIDLib::TBoolean   )
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryCardVal
        (
            strValId, TString::strEmpty(), kCIDLib::False
        );

        // Wait for it to complete. Tell it not to release for us unless it fails
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the return value out
        c4Ret = pdcmdWait->m_c4Val;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}

tCIDLib::TCard4
TCQCSrvDrvTI::c4QueryVal2(  const   TString&
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   )
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryCardVal
        (
            strValId, strValId, kCIDLib::True
        );

        // Wait for it to complete. Tell it not to release for us unless if fails
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        c4Ret = pdcmdWait->m_c4Val;
    }


    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


// Call the driver backdoor method to send a driver command
tCIDLib::TCard4
TCQCSrvDrvTI::c4SendCmd(const   TString&
                        , const TString&        strCmdId
                        , const TString&        strParms
                        , const TCQCSecToken&   )
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQSendCmd
        (
            strCmdId, strParms, tCQCKit::EDrvCmdWaits::Wait
        );

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


// Cancel any outstanding timed write on the indicate field
tCIDLib::TVoid TCQCSrvDrvTI::CancelTimedWrite(const TString&, const TString&, const TCQCSecToken&)
{
    // Ignore
}


// Clear the stats for the indicated driver
tCIDLib::TVoid
TCQCSrvDrvTI::ClearDrvStats(const TString& strMoniker)
{
    m_psdrvTar->ClearStats();
}


// Queue up a timed write for the indicated field
tCIDLib::TVoid
TCQCSrvDrvTI::DoTimedWrite(  const  TString&
                            , const TString&
                            , const TString&
                            , const TString&
                            , const tCIDLib::TCard4
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits)
{
    // Ignore this one
}


// Make the driver backdoor call to query a signed value
tCIDLib::TInt4
TCQCSrvDrvTI::i4QueryVal(   const   TString&
                            , const TString&            strValId
                            , const tCIDLib::TBoolean   )
{
    // Lock and get the command queued up
    tCIDLib::TInt4 i4Ret = 0;
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    try
    {
        pdcmdWait = m_psdrvTar->pdcmdQQueryIntVal
        (
            strValId, TString::strEmpty(), kCIDLib::False
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        i4Ret = pdcmdWait->m_i4Val;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return i4Ret;
}

tCIDLib::TInt4
TCQCSrvDrvTI::i4QueryVal2(  const   TString&
                            , const TString&            strValId
                            , const TString&            strValName
                            , const tCIDLib::TBoolean   bNoQueue)
{
    tCIDLib::TInt4 i4Ret = 0;
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    try
    {
        pdcmdWait = m_psdrvTar->pdcmdQQueryIntVal
        (
            strValId, strValName, kCIDLib::True
        );

        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        TCQCSrvCmdJan janCmd(pdcmdWait);
        i4Ret = pdcmdWait->m_i4Val;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return i4Ret;
}


// Make the driver backdoor call to query the driver's configuration data
tCIDLib::TVoid
TCQCSrvDrvTI::QueryConfig(  const   TString&
                            ,       tCIDLib::TCard4&    c4BytesFilled
                            ,       THeapBuf&           mbufToFill
                            , const TCQCSecToken&       )
{
    try
    {
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQQueryConfig();

        // Wait for it to complete. If it fails, the cmd is released for us
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::False))
            throw(errFail);

        // Make sure it gets released
        TCQCSrvCmdJan janCmd(pdcmdWait);

        // Get the bytes returned out and copy the data bytes themselves
        c4BytesFilled = pdcmdWait->m_c4Count;
        mbufToFill.CopyIn(pdcmdWait->m_mbufVal, c4BytesFilled);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// Query some core info about a driver
tCIDLib::TVoid
TCQCSrvDrvTI::QueryDriverInfo(  const   TString&
                                ,       tCQCKit::EDrvStates&    eStatus
                                ,       tCIDLib::TCard4&        c4ArchVer
                                ,       tCQCKit::TDevClassList& fcolDevClasses
                                ,       TString&                strMake
                                ,       TString&                strModel
                                , const TCQCSecToken&           )
{
    m_psdrvTar->QueryDrvInfo(eStatus, c4ArchVer, fcolDevClasses, strMake, strModel);
}


// Query the basic state of the indicated driver
tCIDLib::TVoid
TCQCSrvDrvTI::QueryDriverState( const   TString&
                                ,       tCQCKit::EDrvStates&    eStatus
                                ,       tCQCKit::EVerboseLvls&  eVerbose
                                ,       tCIDLib::TCard4&        c4ThreadId)
{
    m_psdrvTar->QueryState(eStatus, eVerbose, c4ThreadId);
}


// Query the detailed driver stats of the indicated driver
tCIDLib::TVoid TCQCSrvDrvTI::
QueryDriverStats(const TString& strMoniker, TCQCDrvStats& cdstatsToFill)
{
    m_psdrvTar->QueryStats(cdstatsToFill);
}


// Query the field definition of the indicated field
tCIDLib::TVoid
TCQCSrvDrvTI::QueryFieldDef(const   TString&
                            , const TString&    strField
                            ,       TCQCFldDef& flddToFill)
{
    m_psdrvTar->QueryFldDef(strField, flddToFill);
}


// Query field info for the indicated driver
tCIDLib::TVoid
TCQCSrvDrvTI::QueryFieldInfo(const  TString&
                            , const TString&            strField
                            ,       tCIDLib::TCard4&    c4DriverListId
                            ,       tCIDLib::TCard4&    c4DriverId
                            ,       tCIDLib::TCard4&    c4FieldListId
                            ,       tCIDLib::TCard4&    c4FieldId
                            ,       tCQCKit::EFldTypes& eType)
{
    // These don't exist in this scenario so just return 1
    c4DriverListId = 1;
    c4DriverId = 1;

    m_psdrvTar->QueryFldInfo(strField, c4FieldListId, c4FieldId, eType);
}


tCIDLib::TVoid TCQCSrvDrvTI::
QueryPowerStates(const  tCIDLib::TKVPList&                  colDrivers
                ,       TFundVector<tCQCKit::EPowerStatus>& fcolStates)
{
    fcolStates.RemoveAll();
}


tCIDLib::TVoid TCQCSrvDrvTI::QueryStatusReport(TString& strErrList, const TCQCSecToken&)
{
    strErrList.Clear();
}


// Pause or resume the indicated driver
tCIDLib::TVoid
TCQCSrvDrvTI::PauseResumeDrv(const  TString&
                            , const tCIDLib::TBoolean
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   )
{
    // Ignore
}


tCIDLib::TVoid TCQCSrvDrvTI::
ReadDriverFields(const  tCIDLib::TCard4
                , const tCIDLib::TCard4
                , const tCIDLib::TCard4
                , const tCIDLib::TCardList&
                , const tCIDLib::TCardList&
                ,       tCIDLib::TCard4&    c4BufSz_mbufData
                ,       THeapBuf&           )
{
    // Do nothing, make sure no bogus data gets streamed back
    c4BufSz_mbufData = 0;
}


tCIDLib::TVoid
TCQCSrvDrvTI::ReadFields(const  TFldIOPacket&
                        ,       tCIDLib::TCard4&    c4BytesRead
                        ,       THeapBuf&           )
{
    // Not used in our scenario, make sure nothign gets streamed back if called
    c4BytesRead = 0;
}


tCIDLib::TVoid
TCQCSrvDrvTI::ReadFieldsByName( const   TString&
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

    //
    //  Loop through the field names. For each one, ask the driver to
    //  stream out the value of the field.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_psdrvTar->StreamValue(colFieldNames[c4Index], strmOut);

    // Flush all the data to the buffer and set the bytes parm
    strmOut.Flush();
    c4BytesRead = strmOut.c4CurSize();
}


tCIDLib::TVoid
TCQCSrvDrvTI::ReconfigDriver(const TString& strMoniker, const TCQCSecToken& sectUser)
{
    // Not used in this scenario
}


tCIDLib::TVoid TCQCSrvDrvTI::RefreshCfg(const TString& strCfgType)
{
    // Not used in this scenario
}


tCIDLib::TVoid
TCQCSrvDrvTI::SetConfig(const   TString&
                        , const tCIDLib::TCard4         c4Bytes
                        , const THeapBuf&               mbufNewCfg
                        , const TCQCSecToken&           sectUser
                        , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQSetConfig
    (
        c4Bytes, mbufNewCfg, eWait
    );

    if (pdcmdWait)
    {
        // Tell it to release the cmd for us. If it fails we'll just throw
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TCQCSrvDrvTI::SetVerboseLevel(const TString&, const tCQCKit::EVerboseLvls eToSet)
{
    m_psdrvTar->SetNewVerbosity(eToSet);
}


tCIDLib::TVoid TCQCSrvDrvTI::UnloadAllDrivers(const TCQCSecToken&)
{
    // Ignore
}


tCIDLib::TVoid TCQCSrvDrvTI::UnloadDriver(const TString&, const TCQCSecToken&)
{
    // Ignore
}


tCIDLib::TVoid TCQCSrvDrvTI::UpdateTriggers(const TString&, const TCQCSecToken&)
{
    // Ignore
}


tCIDLib::TVoid
TCQCSrvDrvTI::WriteBoolByName(  const   TString&
                                , const TString&                strFldName
                                , const tCIDLib::TBoolean       bValue
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQBoolFld(strFldName, 0, 0, bValue, eWait);

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteBoolFld( const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const tCIDLib::TBoolean       bValue
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQBoolFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, bValue, eWait
    );

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteCardByName(  const   TString&
                                , const TString&                strFldName
                                , const tCIDLib::TCard4         c4Value
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQCardFld(strFldName, 0, 0, c4Value, eWait);

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteCardFld( const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const tCIDLib::TCard4         c4Value
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQCardFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, c4Value, eWait
    );

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TCQCSrvDrvTI::WriteFieldByName( const   TString&
                                , const TString&                strFldName
                                , const TString&                strValue
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQWriteFld(strFldName, 0, 0, strValue, eWait);
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteFloatByName( const   TString&
                                , const TString&                strFldName
                                , const tCIDLib::TFloat8        f8Value
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQFloatFld(strFldName, 0, 0, f8Value, eWait);

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteFloatFld(const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const tCIDLib::TFloat8        f8Value
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQFloatFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, f8Value, eWait
    );

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteIntByName(const  TString&
                            , const TString&                strFldName
                            , const tCIDLib::TInt4          i4Value
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQIntFld(strFldName, 0, 0, i4Value, eWait);

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteIntFld(  const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const tCIDLib::TInt4          i4Value
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQIntFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, i4Value, eWait
    );

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TCQCSrvDrvTI::WriteStringByName(const   TString&
                                , const TString&                strFldName
                                , const TString&                strValue
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQStringFld(strFldName, 0, 0, strValue, eWait);

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteStringFld(const  tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const TString&                strValue
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQStringFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, strValue, eWait
    );

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


tCIDLib::TVoid
TCQCSrvDrvTI::WriteSListFld(const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const TVector<TString>&       colValue
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQSListFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, colValue, eWait
    );

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteSListByName( const   TString&
                                , const TString&                strFldName
                                , const TVector<TString>&       colValue
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQSListFld(strFldName, 0, 0, colValue, eWait);
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }

}


tCIDLib::TVoid
TCQCSrvDrvTI::WriteTimeByName(  const   TString&
                                , const TString&                strFldName
                                , const tCIDLib::TCard8         c8Value
                                , const TCQCSecToken&
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQTimeFld(strFldName, 0, 0, c8Value, eWait);

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}

tCIDLib::TVoid
TCQCSrvDrvTI::WriteTimeFld( const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4         c4FieldListId
                            , const tCIDLib::TCard4         c4FieldId
                            , const tCIDLib::TCard8         c8Value
                            , const TCQCSecToken&
                            , const tCQCKit::EDrvCmdWaits   eWait)
{
    TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTar->pdcmdQTimeFld
    (
        TString::strEmpty(), c4FieldListId, c4FieldId, c8Value, eWait
    );

    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}


// ---------------------------------------------------------------------------
//  TCQCSrvDrvTI: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSrvDrvTI::UpdateDrvPtr(TCQCServerBase* const psdrvTar)
{
    m_psdrvTar = psdrvTar;
}


// ---------------------------------------------------------------------------
//  TCQCSrvDrvTI: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSrvDrvTI::Initialize()
{
    TParent::Initialize();
}

tCIDLib::TVoid TCQCSrvDrvTI::Terminate()
{
    TParent::Terminate();
}

