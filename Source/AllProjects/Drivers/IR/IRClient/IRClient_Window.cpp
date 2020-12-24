//
// FILE NAME: IRClient_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This file implements the main window of the IRClient client side driver
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "IRClient.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TIRClientWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TIRClientWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRClientWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRClientWnd::TIRClientWnd( const   TCQCDriverObjCfg&   cqcdcInit
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcInit, L"TIRClientWnd", tCQCKit::EActLevels::Medium, cuctxToUse
    )
    , m_bTrainingState(kCIDLib::False)
    , m_c4BlasterTabId(kCIDLib::c4MaxCard)
    , m_c4RecTabId(kCIDLib::c4MaxCard)
    , m_c4TrainingStateSer(0)
    , m_pwndBlasterTab(nullptr)
    , m_pwndRecTab(nullptr)
    , m_pwndTabbed(nullptr)
{
    // Extract the device info from the manifest
    m_eDevCaps = facCQCIR().eExtractDevInfo(cqcdcInit, m_strDevType, m_strDevDescr);
}

TIRClientWnd::~TIRClientWnd()
{
}


// ---------------------------------------------------------------------------
//  TVarDriverCWnd: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  In our case, we do any actual editing in a dialog box, and store any changes before
//  the dialog exits. So at this main driver window level we never have any changes to
//  report.
//
tCIDLib::TBoolean TIRClientWnd::bChanges() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean TIRClientWnd::bSaveChanges(TString&)
{
    // Shouldn't get called but make the compiler happy
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TIRClientWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIRClientWnd::bTrainingMode() const
{
    return m_bTrainingState;
}


tCQCIR::EIRDevCaps TIRClientWnd::eDevCaps() const
{
    return m_eDevCaps;
}


const TString& TIRClientWnd::strDevType() const
{
    return m_strDevType;
}


// ---------------------------------------------------------------------------
//  TIRClientWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TIRClientWnd::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have the tabbed window created, then resize it.
    if (bSizeChanged && m_pwndTabbed && (ePosState != tCIDCtrls::EPosStates::Minimized))
    {
        TArea areaTabs(areaClient());
        areaTabs.Deflate(4, 4);
        m_pwndTabbed->SetSize(areaTabs.szArea(), kCIDLib::True);
    }
}


tCIDLib::TBoolean TIRClientWnd::bCreated()
{
    TParent::bCreated();

    // Create a tabbed window inside our area
    TArea areaTabs(areaClient());
    areaTabs.Deflate(4, 4);
    m_pwndTabbed = new TTabbedWnd();
    m_pwndTabbed->CreateTabbedWnd
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaTabs
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::ETabWStyles::None
        , tCIDCtrls::EExWndStyles::ControlParent
    );


    // If this guy supports blaster functionality, load the blaster tab
    const tCIDLib::TBoolean bCanBlast
    (
        tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::Blast)
    );

    if (bCanBlast || tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::BlastTrain))
    {
        m_pwndBlasterTab = new TIRBlasterTab(this);
        m_pwndTabbed->c4CreateTab
        (
            m_pwndBlasterTab
            , facIRClient().strMsg
              (
                 bCanBlast ? kIRCMsgs::midBlast_TitleBlaster
                           : kIRCMsgs::midBlast_TitleLearner
              )
        );
    }

    // If this guy supports receiver functionality, load the tab for that
    if (tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::Receive))
    {
        m_pwndRecTab = new TIRReceiverTab(this);
        m_pwndTabbed->c4CreateTab
        (
            m_pwndRecTab, facIRClient().strMsg(kIRCMsgs::midRec_Title)
        );
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TIRClientWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    return orbcTarget->bReadBoolByName
    (
        m_c4TrainingStateSer
        , strMoniker()
        , TFacCQCIR::strFldName_TrainingState
        , m_bTrainingState
    );
}


tCIDLib::TBoolean
TIRClientWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    // Get the one time info
    tCIDLib::TCard4 c4Ver = 0;
    orbcTarget->bReadStringByName
    (
        c4Ver, strMoniker(), TFacCQCIR::strFldName_FirmwareVer, m_strFirmware
    );

    // Reset the training state field's serial number
    m_c4TrainingStateSer = 0;

    return kCIDLib::True;
}


tCIDLib::TVoid TIRClientWnd::Connected()
{
    // Tell the tabs that we've connected
    if (m_pwndBlasterTab)
        m_pwndBlasterTab->SetConnected(m_strFirmware);
    if (m_pwndRecTab)
        m_pwndRecTab->SetConnected(m_strFirmware);
}


tCIDLib::TVoid TIRClientWnd::DoCleanup()
{
}


tCIDLib::TVoid TIRClientWnd::LostConnection()
{
}


tCIDLib::TVoid TIRClientWnd::UpdateDisplayData()
{
}


