//
// FILE NAME: IntfVCtrlS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2004
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
//  This is is the implementation for the CQC User Interface driver.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "IntfVCtrlS_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfVCtrlSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//  Local constants
// ---------------------------------------------------------------------------
namespace IntfVCtrlS_DriverImpl
{
}



// ---------------------------------------------------------------------------
//   CLASS: TIntfVCtrlSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfVCtrlSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfVCtrlSDriver::TIntfVCtrlSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_Action(kCIDLib::c4MaxCard)
    , m_c4FldId_CurBaseTmpl(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTopTmpl(kCIDLib::c4MaxCard)
    , m_c4FldId_LayerCount(kCIDLib::c4MaxCard)
    , m_c4FldId_LoadOverlay(kCIDLib::c4MaxCard)
    , m_c4FldId_LoadTemplate(kCIDLib::c4MaxCard)
    , m_c4FldId_Maximized(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayWAV(kCIDLib::c4MaxCard)
    , m_c4FldId_SetVariable(kCIDLib::c4MaxCard)
{
    //
    //  Make sure that we were configured for an 'other' connection. Otherwise,
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

TIntfVCtrlSDriver::~TIntfVCtrlSDriver()
{
}


// ---------------------------------------------------------------------------
//  TIntfVCtrlSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIntfVCtrlSDriver::bGetCommResource(TThread& thrThis)
{
    // We don't need to do anything here
    return kCIDLib::True;
}


tCIDLib::TBoolean TIntfVCtrlSDriver::bWaitConfig(TThread& thrThis)
{
    // We don't need to do anything here
    return kCIDLib::True;
}


tCQCKit::ECommResults
TIntfVCtrlSDriver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        TString strBinding(TIntfCtrlClientProxy::strBinding);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);

        TIntfCtrlClientProxy* porbcProxy
        (
            facCIDOrbUC().porbcMakeClient<TIntfCtrlClientProxy>(strBinding, 2000)
        );

        if (!porbcProxy)
            return tCQCKit::ECommResults::LostConnection;

        // Set this object id on the proxy object
        m_orbcProxy.SetPointer(porbcProxy);
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TIntfVCtrlSDriver::eInitializeImpl()
{
    // Load up the list of fields and register them
    TVector<TCQCFldDef> colFlds(32);
    TCQCFldDef flddCur;

    const tCIDLib::TCh* const pszFld_Action = L"Action";
    const tCIDLib::TCh* const pszFld_CurBaseTmpl = L"CurBaseTmpl";
    const tCIDLib::TCh* const pszFld_CurTopTmpl = L"CurTopTmpl";
    const tCIDLib::TCh* const pszFld_LayerCount = L"LayerCount";
    const tCIDLib::TCh* const pszFld_LoadOverlay = L"LoadOverlay";
    const tCIDLib::TCh* const pszFld_LoadTemplate = L"LoadTemplate";
    const tCIDLib::TCh* const pszFld_Maximized = L"Maximized";
    const tCIDLib::TCh* const pszFld_PlayWAV = L"PlayWAV";
    const tCIDLib::TCh* const pszFld_SetVariable = L"SetVariable";

    flddCur.Set
    (
        pszFld_Action
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: Blanker, Blank Display, Blank and Blanker, Clock Blanker, Down, End, "
          L"Enter, FSMode, Hide, Home, Left, Maximize, Minimize, Next, NextPage, Prev, "
          L"PrevPage, Restore, Right, Show, Slideshow Blanker, To Back, To Front, "
          L"Up, Unblank"
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_CurBaseTmpl, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_CurTopTmpl, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_LayerCount, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_LoadOverlay, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_LoadTemplate, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_Maximized, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_PlayWAV, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddCur);

    flddCur.Set
    (
        pszFld_SetVariable, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddCur);

    // Set the fields and look up any we want to do id based access of
    SetFields(colFlds);
    m_c4FldId_Action = pflddFind(pszFld_Action, kCIDLib::True)->c4Id();
    m_c4FldId_CurBaseTmpl = pflddFind(pszFld_CurBaseTmpl, kCIDLib::True)->c4Id();
    m_c4FldId_CurTopTmpl = pflddFind(pszFld_CurTopTmpl, kCIDLib::True)->c4Id();
    m_c4FldId_LayerCount = pflddFind(pszFld_LayerCount, kCIDLib::True)->c4Id();
    m_c4FldId_LoadOverlay = pflddFind(pszFld_LoadOverlay, kCIDLib::True)->c4Id();
    m_c4FldId_LoadTemplate = pflddFind(pszFld_LoadTemplate, kCIDLib::True)->c4Id();
    m_c4FldId_Maximized = pflddFind(pszFld_Maximized, kCIDLib::True)->c4Id();
    m_c4FldId_PlayWAV = pflddFind(pszFld_PlayWAV, kCIDLib::True)->c4Id();
    m_c4FldId_SetVariable = pflddFind(pszFld_SetVariable, kCIDLib::True)->c4Id();

    // Set a moderately fast poll time
    SetPollTimes(200, 5000);

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TIntfVCtrlSDriver::ePollDevice(TThread& thrThis)
{
    try
    {
        // Poll the IV and store the results
        tCIDLib::TBoolean bMaximized;
        const tCIDLib::TCard4 c4LayerCnt = m_orbcProxy->c4Poll
        (
            m_strPollTmp1, m_strPollTmp2, bMaximized
        );

        // Store the data
        bStoreStringFld(m_c4FldId_CurBaseTmpl, m_strPollTmp1, kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurTopTmpl, m_strPollTmp2, kCIDLib::True);
        bStoreCardFld(m_c4FldId_LayerCount, c4LayerCnt, kCIDLib::True);
        bStoreBoolFld(m_c4FldId_Maximized, bMaximized, kCIDLib::True);
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        // Just give up and make us reconnect
        m_orbcProxy->ResetProxy();
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TIntfVCtrlSDriver::eStringFldValChanged(const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_Action)
        {
            if ((strNewValue == L"Blanker")
            ||  (strNewValue == L"Blank Display")
            ||  (strNewValue == L"Blank and Blanker")
            ||  (strNewValue == L"Clock Blanker")
            ||  (strNewValue == L"Slideshow Blanker")
            ||  (strNewValue == L"Unblank"))
            {
                // These all translate to the IV misc ops enum
                try
                {
                    m_orbcProxy->DoMiscOp(tCQCKit::eAltXlatEIVMiscOps(strNewValue));
                }

                catch(...)
                {
                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::BadValue;
                }
            }
             else if (strNewValue == L"Enter")
            {
                m_orbcProxy->Invoke();
            }
             else if (strNewValue == L"FSMode")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::FSMode);
            }
             else if (strNewValue == L"Hide")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::Hide);
            }
             else if (strNewValue == L"Maximize")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::Maximize);
            }
             else if (strNewValue == L"Minimize")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::Minimize);
            }
             else if (strNewValue == L"Restore")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::Restore);
            }
             else if (strNewValue == L"Show")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::Show);
            }
             else if (strNewValue == L"ToBack")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::ToBack);
            }
             else if (strNewValue == L"ToFront")
            {
                m_orbcProxy->DoFrameOp(tCQCKit::EIVFrmOps::ToFront);
            }
             else
            {
                tCQCKit::EScrNavOps eOp;
                if (strNewValue == L"Down")
                    eOp = tCQCKit::EScrNavOps::Down;
                else if (strNewValue == L"End")
                    eOp = tCQCKit::EScrNavOps::End;
                else if (strNewValue == L"Home")
                    eOp = tCQCKit::EScrNavOps::Home;
                else if (strNewValue == L"Left")
                    eOp = tCQCKit::EScrNavOps::Left;
                else if (strNewValue == L"Next")
                    eOp = tCQCKit::EScrNavOps::Next;
                else if (strNewValue == L"NextPage")
                    eOp = tCQCKit::EScrNavOps::NextPage;
                else if (strNewValue == L"Prev")
                    eOp = tCQCKit::EScrNavOps::Prev;
                else if (strNewValue == L"PrevPage")
                    eOp = tCQCKit::EScrNavOps::PrevPage;
                else if (strNewValue == L"Right")
                    eOp = tCQCKit::EScrNavOps::Right;
                else if (strNewValue == L"Up")
                    eOp = tCQCKit::EScrNavOps::Up;
                else
                {
                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::BadValue;
                }
                m_orbcProxy->Navigation(eOp);
            }
        }
         else if (c4FldId == m_c4FldId_LoadOverlay)
        {
            //
            //  We get a value like "MyOver,ToLoad" and need to parse them
            //  out.
            //
            TString strOverlay(strNewValue);
            TString strTmpl;
            strOverlay.bSplit(strTmpl, kCIDLib::chComma);
            strOverlay.StripWhitespace();
            strTmpl.StripWhitespace();

            m_orbcProxy->LoadOverlay(strOverlay, strTmpl);
        }
         else if (c4FldId == m_c4FldId_LoadTemplate)
        {
            // We just get the template name
            m_orbcProxy->LoadTemplate(strNewValue);
        }
         else if (c4FldId == m_c4FldId_PlayWAV)
        {
            //
            //  The value is just the path to a file on the target IV's file
            //  system to have it play.
            //
            m_orbcProxy->PlayWAV(strNewValue);
        }
         else if (c4FldId == m_c4FldId_SetVariable)
        {
            //
            //  The value is a variable name, then a value to set, separated
            //  by a comma.
            //
            TString strVarName(strNewValue);
            TString strValue;
            if (!strVarName.bSplit(strValue, kCIDLib::chComma))
                return tCQCKit::ECommResults::BadValue;

            strVarName.StripWhitespace();
            strValue.StripWhitespace();
            if (!strVarName.bStartsWith(kCQCKit::strActVarPref_GVar))
                return tCQCKit::ECommResults::BadValue;
            m_orbcProxy->SetGlobalVar(strVarName, strValue);
        }
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        m_orbcProxy->ResetProxy();
        IncFailedWriteCounter();
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TIntfVCtrlSDriver::ReleaseCommResource()
{
    try
    {
        // Close our client proxy if we have one
        m_orbcProxy->ResetProxy();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TIntfVCtrlSDriver::TerminateImpl()
{
    // Nothing to do right now
}


