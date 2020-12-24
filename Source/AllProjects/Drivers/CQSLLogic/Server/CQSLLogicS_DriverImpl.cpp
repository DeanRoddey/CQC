//
// FILE NAME: CQSLLogicS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This is a driver which implements the CQSL Logic Server driver
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLLogicS_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLLogicSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQSLLogicS_DriverImpl
{
};



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLogicSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLLogicSDriver::TCQSLLogicSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4CfgSerialNum(0)
    , m_mbufUpdate(8192UL)
{
    //
    //  Make sure that we were configured for a other connection. Otherwise,
    //  its a bad configuration.
    //
    if (cqcdcToLoad.conncfgReal().clsIsA() != TCQCOtherConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcToLoad.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCOtherConnCfg::clsThis()
        );
    }
}

TCQSLLogicSDriver::~TCQSLLogicSDriver()
{
}



// ---------------------------------------------------------------------------
//  TVarDriverSDriver: Public, inherited methods
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  TCQSLLogicSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQSLLogicSDriver::bGetCommResource(TThread& thrThis)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        TLogicSrvClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TLogicSrvClientProxy>
        (
            TLogicSrvClientProxy::strBinding, 2000
        );

        if (porbcProxy)
        {
            m_orbcLS.SetPointer(porbcProxy);
            bRes = kCIDLib::True;

            // Reset our serial number
            m_c4CfgSerialNum = 0;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch, tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return bRes;
}


//
//  This one isn't relevant to us, because we get our config from the
//  server which requires that we get connected first. It's virtual so
//  we have to provided it, but just return true.
//
tCIDLib::TBoolean TCQSLLogicSDriver::bWaitConfig(TThread& thrThis)
{
    return kCIDLib::True;
}


//
//  To allow clients to send us backdoor commands.
//
tCIDLib::TCard4
TCQSLLogicSDriver::c4SendCmd(const  TString&    strCmd
                            , const TString&    strParms)
{
    if (strCmd == L"ResetElapsedTmFld")
    {
        // The parameter should be the name of the field
        m_orbcLS->ResetElTimeFld(strParms);
    }
    return 0;
}


//
//  At this point, we've got our client proxy object, so we need to query
//  the configuration from the server, and set up our fields and do an
//  initial query to get the values updated.
//
tCQCKit::ECommResults TCQSLLogicSDriver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        // Let's try to get the configuration
        m_orbcLS->QueryConfig(m_cfgCurrent, m_c4CfgSerialNum);

        // That didn't throw, so let's do our setup
        RegisterFields();
        eUpdateFields(kCIDLib::True);

        // Bump the reconfig field
        IncReconfigCounter();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch, tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Just assume we lost our connection and cycle
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  We just set our poll times and then indicate we are ready to get the
//  comm resource. We do have config, but it's gotten from the server so we
//  don't do wait config.
//
tCQCKit::EDrvInitRes TCQSLLogicSDriver::eInitializeImpl()
{
    SetPollTimes(500, 5000);
    return tCQCKit::EDrvInitRes::WaitCommRes;
}



tCQCKit::ECommResults TCQSLLogicSDriver::ePollDevice(TThread& thrThis)
{
    tCQSLLogicS::EUpdateRes eRes = eUpdateFields(kCIDLib::False);

    if (eRes == tCQSLLogicS::EUpdateRes::Success)
        return tCQCKit::ECommResults::Success;
    else if (eRes == tCQSLLogicS::EUpdateRes::Error)
        return tCQCKit::ECommResults::LostCommRes;

    //
    //  It's a resync response, so we've gotten out of sync. We need to
    //  get the config again and set up our fields again. We'll have to
    //  do a field lock here since we are doing this in the poll.
    //
    //  We can just call our own connect method, since it does what we
    //  need to do and returns the correct type of value.
    //
    return eConnectToDevice(thrThis);
}


tCIDLib::TVoid TCQSLLogicSDriver::ReleaseCommResource()
{
    // Let our client proxy go
    try
    {
        m_orbcLS.SetPointer(0);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch, tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


tCIDLib::TVoid TCQSLLogicSDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLogicSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called during connection and polling, to update our fields with
//  any changes from the server. The caller can indicate this is the initial
//  pass (done from the connection) so that we can tell the server to give
//  us all the field values, whether they've changed or not. Otherwise he
//  only sends back changes, which is commonly none.
//
tCQSLLogicS::EUpdateRes
TCQSLLogicSDriver::eUpdateFields(const tCIDLib::TBoolean bInitial)
{
    tCQSLLogicS::EUpdateRes eRes;
    try
    {
        tCIDLib::TCard4 c4Sz;
        if (m_orbcLS->bQueryFields(m_c4CfgSerialNum, c4Sz, m_mbufUpdate, bInitial))
        {
            //
            //  OK, we got it, so pull the data out and store it if it
            //  returned anything, which it might not if nothing has
            //  changed.
            //
            if (c4Sz)
                StoreFields(m_mbufUpdate, c4Sz);
            eRes = tCQSLLogicS::EUpdateRes::Success;
        }
         else
        {
            // We are out of sync, so re-query configuration
            eRes = tCQSLLogicS::EUpdateRes::Resync;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch, tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQSLLogicS::EUpdateRes::Error;
    }
    return eRes;
}


//
//  Gets our configured virtual fields registered. Each one contains a field
//  definition object for the field that they represent, so this is pretty
//  easy.
//
//  Note that not all the fields are normal fields. Some are not exposed as
//  normal fields, and we skip those.
//
tCIDLib::TVoid TCQSLLogicSDriver::RegisterFields()
{
    //
    //  Let's run through all of the configured virtual fields and get copies
    //  of the field def objects in a list we can pass in for registration.
    //
    tCQLogicSh::TFldList& colLSFlds = m_cfgCurrent.colFldTypes();
    const tCIDLib::TCard4 c4Count = colLSFlds.c4ElemCount();
    TVector<TCQCFldDef> colFlds(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQLSrvFldType* pclsftCur = colLSFlds[c4Index];
        if (pclsftCur->bNormalFld())
            colFlds.objAdd(pclsftCur->flddCfg());
    }

    // Register them with the base driver class
    SetFields(colFlds);

    // Set them all to error state until we values stored
    SetAllErrStates();

    // Now go back and store the field ids on them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQLSrvFldType* pclsftCur = colLSFlds[c4Index];
        if (pclsftCur->bNormalFld())
            pclsftCur->c4FldId(pflddFind(pclsftCur->flddCfg().strName(), kCIDLib::True)->c4Id());
    }
}


//
//  This is called when we get new field info to store. We stream in the
//  info from the returned buffer and store it in our fields. They are
//  identifed by their index in the list of fields in the config object.
//
tCIDLib::TVoid
TCQSLLogicSDriver::StoreFields( const   THeapBuf&       mbufUpdate
                                , const tCIDLib::TCard4 c4Size)
{
    tCQLogicSh::TFldList& colFlds = m_cfgCurrent.colFldTypes();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();

    // Put a stream over the buffer, and read until we hit the end
    tCIDLib::TCard1         c1Error;
    tCIDLib::TCard4         c4Index;
    tCIDLib::EStreamMarkers eMarker;
    TBinMBufInStream        strmSrc(&mbufUpdate, c4Size);

    while (!strmSrc.bEndOfStream())
    {
        // We should get a frame marker
        strmSrc >> eMarker;
        if (eMarker != tCIDLib::EStreamMarkers::Frame)
        {
            facCQSLLogicS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLSDErrs::errcProto_ExpectedMarker
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Next we should get an index. It has to be a valid one
        strmSrc >> c4Index;
        if (c4Index >= c4Count)
        {
            facCQSLLogicS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLSDErrs::errcProto_InvalidFldIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        //
        //  Get the field id out for this one, which we stored during
        //  the field registration.
        //
        TCQLSrvFldType* pclsftCur = colFlds[c4Index];
        const tCIDLib::TCard4 c4FldId = pclsftCur->c4FldId();

        // And next we get a byte that indicates error or value
        strmSrc >> c1Error;
        if (c1Error)
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            //
            //  Stream out the value. The field object has a field value
            //  object of the appropriate type, so we can use that to
            //  stream in the value appropriately.
            //
            pclsftCur->fvCurrent().StreamInValue(strmSrc);

            // And now we can store that on our field
            bStoreFldValue(pclsftCur->c4FldId(), pclsftCur->fvCurrent());
        }
    }
}


