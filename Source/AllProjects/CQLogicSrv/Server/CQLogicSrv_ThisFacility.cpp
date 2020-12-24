//
// FILE NAME: CQLogicSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2009
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
//  This file implements the facility class for the logic server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQLogicSrv,TCQCSrvCore)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQLogicSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQLogicSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We don't require any login since we don't do anything that requires credentials.
//  And we don't handle any driver related stuff here, there's a separate driver
//  that works via us, and it will handle things like sending field change events
//  and such.
//
TFacCQLogicSrv::TFacCQLogicSrv() :

    TCQCSrvCore
    (
        L"CQLogicSrv"
        , L"CQC Logic Server"
        , kCQCKit::ippnLogicSrvDefPort
        , kCQLogicSrv::pszEvLogicSrv
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::None
    )
    , m_c4CfgSerialNum(1)
    , m_colValues(tCIDLib::EAdoptOpts::Adopt)
    , m_porbsImpl(nullptr)
    , m_strFldCfgPath(L"/Cfg/FieldDefs")
{
}


// ---------------------------------------------------------------------------
//  TFacCQLogicSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TFacCQLogicSrv::bQueryFields(const  tCIDLib::TCard4     c4CfgSerialNum
                            ,       tCIDLib::TCard4&    c4BufSz
                            ,       THeapBuf&           mbufData
                            , const tCIDLib::TBoolean   bReturnAll)
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    //
    //  If the serial number is off, then return false now. Be sure to
    //  zero the buffer size, so it doesn't try to stream anything back.
    //
    if (m_c4CfgSerialNum != c4CfgSerialNum)
    {
        c4BufSz = 0;
        return kCIDLib::False;
    }

    //
    //  Run through the fields and any for any that have a non-zero
    //  serial number, it's changed since the last query. So grab the
    //  value and reset the serial number. Or, if they ask us to return
    //  all we just do them all regardless.
    //
    TBinMBufOutStream strmOut(&mbufData);

    tCQLogicSh::TFldList& colFlds = m_lscfgServer.colFldTypes();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQLSrvFldType* pclsftCur = colFlds[c4Index];

        // Some of them are not treated as normal fields
        if (!pclsftCur->bNormalFld())
            continue;

        TCQCFldValue& fldvCur = pclsftCur->fvCurrent();
        if (fldvCur.c4SerialNum() || bReturnAll)
        {
            // Format out a frame marker and then the current index
            strmOut << tCIDLib::EStreamMarkers::Frame << c4Index;

            //
            //  Now we put out a byte that indicates error state or a
            //  value. If a value, we then do the value.
            //
            if (fldvCur.bInError())
            {
                strmOut << tCIDLib::TCard1(1);
            }
             else
            {
                strmOut << tCIDLib::TCard1(0);
                fldvCur.StreamOutValue(strmOut);
            }

            // Now let's clear the serial number
            fldvCur.c4SerialNum(0);
        }
    }

    // And now give back the size
    strmOut.Flush();
    c4BufSz = strmOut.c4CurPos();

    return kCIDLib::True;
}


//
//  This is called when a client wants to get the sample data for a graph
//  type field.
//
tCIDLib::TBoolean TFacCQLogicSrv::
bQueryGraphSamples( const   TString&                        strFldName
                    ,       tCIDLib::TCard4&                c4CfgSerialNum
                    ,       tCIDLib::TCard4&                c4SerialNum
                    ,       tCIDLib::TCard4&                c4NewSamples
                    ,       TFundVector<tCIDLib::TFloat4>&  fcolNewSamples)
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    tCIDLib::TBoolean bRet = kCIDLib::False;

    //
    //  If the configuration serial number is off, force the serial number to
    //  zero, to force a full update and give him the new config serial number
    //  back.
    //
    if (c4CfgSerialNum != m_c4CfgSerialNum)
    {
        c4CfgSerialNum = m_c4CfgSerialNum;
        c4SerialNum = 0;
    }

    //
    //  Flush the output collection so that we don't stream anything if we
    //  never find the field or have no samples.
    //
    c4NewSamples = 0;
    fcolNewSamples.RemoveAll();

    // Look up a field of this name in our configuration
    const tCQLogicSh::TFldList& colFlds = m_lscfgServer.colFldTypes();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQLSrvFldType* pclsftCur = colFlds[c4Index];

        // It can't be a graph field if marked as a normal field so skip it
        if (pclsftCur->bNormalFld())
            continue;

        if (pclsftCur->strFldName() == strFldName)
        {
            // Make sure it's a graph field
            const TCQSLLDGraph* pclsftGraph = static_cast<const TCQSLLDGraph*>(pclsftCur);
            if (pclsftCur)
            {
                // Ask it for any new samples
                bRet = pclsftGraph->bQuerySamples(c4SerialNum, fcolNewSamples, c4NewSamples);
                break;
            }
        }
    }
    return bRet;
}


//
//  This is called when a client wants to know all of the graph type fields,
//  which is basically the Intf. Designer tab for the graph widget, to let
//  the user select a graph field, without having to suck down the whole
//  configuration.
//
tCIDLib::TCard4 TFacCQLogicSrv::c4QueryGraphFlds(TVector<TString>& colToFill)
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    colToFill.RemoveAll();

    const tCQLogicSh::TFldList& colFlds = m_lscfgServer.colFldTypes();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQLSrvFldType* pclsftCur = colFlds[c4Index];

        // It can't be a graph field if marked as a normal field so skip it
        if (pclsftCur->bNormalFld())
            continue;

        // See if it's of the type we want. If so, take it
        if (pclsftCur->bIsA(TCQSLLDGraph::clsThis()))
            colToFill.objAdd(pclsftCur->strFldName());
    }
    return colToFill.c4ElemCount();
}


//
//  This is called from our IDL interface class to query our current
//  configuration. We just lock to keep the main server thread from
//  ccessing the config and copy it over.
//
tCIDLib::TVoid
TFacCQLogicSrv::QueryConfig(TCQLogicSrvCfg&     lscfgToFill
                            , tCIDLib::TCard4&  c4SerialNum)
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    lscfgToFill = m_lscfgServer;
    c4SerialNum = m_c4CfgSerialNum;
}


//
//  We provide a means to reset elapsed time fields that are not auto-
//  reset mode.
//
tCIDLib::TVoid TFacCQLogicSrv::ResetElTimeFld(const TString& strFldName)
{
    // Lock while we do this
    {
        TLocker lockrLock(&m_mtxLock);

        // Look up a field of this name in our configuration
        tCQLogicSh::TFldList& colFlds = m_lscfgServer.colFldTypes();
        const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQLSrvFldType* pclsftCur = colFlds[c4Index];

            if (pclsftCur->strFldName() == strFldName)
            {
                // It has to be an elapsed time field
                if (!pclsftCur->bIsA(TCQSLLDElapsedTm::clsThis()))
                {
                    facCQLogicSh().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQLShErrs::errcCfg_NotLegalFor
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotSupported
                        , pclsftCur->strFldName()
                    );
                }

                // Tell it to reset its value and return
                static_cast<TCQSLLDElapsedTm*>(pclsftCur)->ResetTime();
                return;
            }
        }
    }

    // If we get here, we never found it
    facCQLogicSh().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kCQLShErrs::errcCfg_FldNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotSupported
        , strFldName
    );
}


//
//  This is called from our IDL interface class to set a new configuration.
//
tCIDLib::TVoid
TFacCQLogicSrv::SetConfig(  const   TCQLogicSrvCfg&     lscfgToSet
                            ,       tCIDLib::TCard4&    c4SerialNum)
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    // Bump the config serial number and give it back
    m_c4CfgSerialNum++;
    c4SerialNum = m_c4CfgSerialNum;

    // Store the new configuration
    m_lscfgServer = lscfgToSet;

    // And store it to disk
    StoreConfig();

    // Do the setup we do after we load/get new config
    InitFields();
}


// ---------------------------------------------------------------------------
//  TFacCQLogicSrv: Protected, inherited methods
// ---------------------------------------------------------------------------

// Just deregister our logic server interface
tCIDLib::TVoid TFacCQLogicSrv::DeregisterSrvObjs()
{
    facCIDOrb().DeregisterObject(m_porbsImpl);
}


// We are not exposed to clients yet, so no need to sync
tCQCSrvFW::EStateRes
TFacCQLogicSrv::eLoadConfig(tCIDLib::TCard4& c4WaitNext, const tCIDLib::TCard4 c4Count)
{
    // Reset to defaults if we don't get any
    m_lscfgServer.Reset();

    // Open or create our repository
    if (m_oseData.bInitialize(facCQCKit().strRepositoryDir(), L"LogicSrvCfg"))
    {
        // We created a new one. Let's write out an empty field config
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQLSrvMsgs::midStatus_NoServerConfig
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        m_oseData.AddObject(m_strFldCfgPath, m_lscfgServer, 2048);
    }
    else
    {
        tCIDLib::TCard4 c4Version = 0;
        if (m_oseData.eReadObject(m_strFldCfgPath, c4Version, m_lscfgServer, kCIDLib::False) != tCIDLib::ELoadRes::NewData)
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQLSrvMsgs::midStatus_NoServerConfig
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    return tCQCSrvFW::EStateRes::Success;
}


tCIDLib::TVoid TFacCQLogicSrv::PostDeregTerm()
{
    // Stop the polling engine
    m_polleSrv.StopEngine();
}


//
//  We use this to set our stats cache objects before the stuff that would use them
//  gets going, and we go ahead at this point and try to initialize the fields. The
//  config has already been loaded at this point.
//
tCIDLib::TVoid TFacCQLogicSrv::PreRegInit()
{
    // Initialize our stats cache items
    TStatsCache::RegisterItem
    (
        L"/Stats/LogicSrv/CalcErrors"
        , tCIDLib::EStatItemTypes::Counter
        , m_sciCalcErrors
    );

    TStatsCache::RegisterItem
    (
        L"/Stats/LogicSrv/FieldCount"
        , tCIDLib::EStatItemTypes::Value
        , m_sciFieldCnt
    );

    //
    //  And set up any fields we have defined. This depends on the config already
    //  having been loaded, which is the case here.
    //
    InitFields();

    // Start the polling engine
    m_polleSrv.StartEngine(cuctxToUse().sectUser());
}


// We don't use the extra values, so just return the binding and description
tCIDLib::TVoid
TFacCQLogicSrv::QueryCoreAdminInfo( TString&    strCoreAdminBinding
                                    , TString&  strCoreAdminDesc
                                    , TString&
                                    , TString&
                                    , TString&
                                    , TString&  )
{
    strCoreAdminBinding = TLogicSrvServerBase::strCoreAdminBinding;
    strCoreAdminDesc = L"CQC Logic Server Core Admin Object";
}


//
//  The server framework calls us here on startup to let us register any server side
//  objects. We just have to do our logic server interface implementation object. We
//  give him back our core binding info so he can do that for us.
//
tCIDLib::TVoid TFacCQLogicSrv::RegisterSrvObjs()
{
    m_porbsImpl = new TLogicSrvImpl;
    facCIDOrb().RegisterObject(m_porbsImpl, tCIDLib::EAdoptOpts::Adopt);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsImpl->ooidThis(), TLogicSrvServerBase::strBinding, L"CQC Logic Server"
    );
}


//
//  The server framework calls us here on shutdown to store the configuration one
//  last time if we can, then we close the repo.
//
tCIDLib::TVoid TFacCQLogicSrv::StoreConfig()
{
    // Lock while we do this
    TLocker lockrLock(&m_mtxLock);

    try
    {
        tCIDLib::TCard4 c4Version = 0;
        m_oseData.bAddOrUpdate(m_strFldCfgPath, c4Version, m_lscfgServer, 2048);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
    }

    // Close the repo
    m_oseData.Close();
}


//
//  We override this so that we can do other things while mostly blocking on the shutdown
//  event. Otherwise we'd have to create another thread to do this work.
//
tCIDLib::TVoid TFacCQLogicSrv::WaitForTerm(TEvent& evWait)
{
    while(kCIDLib::True)
    {
        try
        {
            //
            //  Pause a bit, and watch for a shutdown request. If it comes
            //  back true, then it was triggered, which means we were asked
            //  to shut down.
            //
            if (evWait.bWaitFor(250))
            {
                break;
            }
             else
            {
                // Get the hour and minute to pass in
                TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
                tCIDLib::TCard4 c4Hour;
                tCIDLib::TCard4 c4Minute;
                tCIDLib::TCard4 c4Second;
                tmNow.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

                // Lock while we do this
                TLocker lockrLock(&m_mtxLock);

                //
                //  And give each of our pseudo fields a chance to update itself.
                //  We pass it the polling engine so that it can look up any
                //  fields it needs to.
                //
                tCQLogicSh::TFldList& colFlds = m_lscfgServer.colFldTypes();
                const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    TCQLSrvFldType* pclsftCur = colFlds[c4Index];
                    pclsftCur->Evaluate(m_polleSrv, c4Hour, c4Minute);
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLSrvErrs::errcProc_ExceptIn
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            // Bump the calc error stats
            TStatsCache::c8IncCounter(m_sciCalcErrors);
        }

        catch(...)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLSrvErrs::errcProc_ExceptIn
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            // Bump the calc error stats
            TStatsCache::c8IncCounter(m_sciCalcErrors);
        }
    }
}


// We just need to unbind our logic server interface object
tCIDLib::TVoid TFacCQLogicSrv::UnbindSrvObjs(tCIDOrbUC::TNSrvProxy&  orbcNS)
{
    orbcNS->RemoveBinding(TLogicSrvServerBase::strBinding, kCIDLib::False);
}



// ---------------------------------------------------------------------------
//  TFacCQLogicSrv: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called after we load config or get new config from the admin
//  interface client, to get the fields set up.
//
tCIDLib::TVoid TFacCQLogicSrv::InitFields()
{
    tCQLogicSh::TFldList& colSrcFlds = m_lscfgServer.colFldTypes();
    const tCIDLib::TCard4 c4SrcCount = colSrcFlds.c4ElemCount();

    //
    //  We need a field info cache for this. There's not much filtering we
    //  can do since we don't know what the various types of fields will
    //  need, but clearly they have to be readable. So we filter for that.
    //
    TCQCFldCache cfcInit;
    cfcInit.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));

    //
    //  Once we've gotten our fields registered, then we need to go through
    //  and get them set up. We need to register them with the polling
    //  engine.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCount; c4Index++)
    {
        TCQLSrvFldType* pclsftCur = colSrcFlds[c4Index];
        pclsftCur->RegisterFields(m_polleSrv, cfcInit);
    }

    //
    //  And go back once again and give each of them a chance to do any
    //  initialization that might require knowing the field types or stuff
    //  that's runtime only, so it's not done on the client side and so
    //  not just automatically done when they are streamedin.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCount; c4Index++)
        colSrcFlds[c4Index]->Initialize(cfcInit);

    // Set the stats cache item
    TStatsCache::SetValue(m_sciFieldCnt, c4SrcCount);
}

