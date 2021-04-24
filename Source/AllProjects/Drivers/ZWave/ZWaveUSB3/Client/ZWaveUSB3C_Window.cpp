//
// FILE NAME: ZWaveUSB3C_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2018
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
//  This file implements the main window of the ZWave USB3 client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUSB3CWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//  CLASS: TZW3UnitAttrWnd
// PREFIX: wnd
//
//  We just need a simple derivative of the attribute editor, so that we can provide some
//  custom visual editing. The main driver window implements the IPE interface and sets
//  himself on us, so he can provide validation and storing new values.
// ---------------------------------------------------------------------------
class TZW3UnitAttrWnd : public TAttrEditWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZW3UnitAttrWnd();

        TZW3UnitAttrWnd(const TZW3UnitAttrWnd&) = delete;

        ~TZW3UnitAttrWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZW3UnitAttrWnd& operator=(const TZW3UnitAttrWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetEditInfo
        (
                    TZWaveUSB3CWnd* const   pwndDriver
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bVisEdit
        (
                    TAttrData&              adatEdit
            , const TArea&                  areaCellScr
            , const tCIDLib::TCard8         c8UserId
        )   override;

        tCIDLib::TVoid CellClicked
        (
            const   tCIDLib::TCard4         c4Row
            , const tCIDLib::TCard4         c4Column
            , const tCIDLib::TBoolean       bLeftButton
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndDriver
        //      We are given a pointer to the driver window which we need in order to
        //      get to the unit and device index stuff, and we also disallow editing
        //      if the server side driver is doing a scan.
        // -------------------------------------------------------------------
        TZWaveUSB3CWnd* m_pwndDriver;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZW3UnitAttrWnd, TAttrEditWnd);
};
AdvRTTIDecls(TZW3UnitAttrWnd, TAttrEditWnd);


// ---------------------------------------------------------------------------
//  TZW3UnitAttrWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZW3UnitAttrWnd::TZW3UnitAttrWnd() :

    m_pwndDriver(nullptr)
{
}

TZW3UnitAttrWnd::~TZW3UnitAttrWnd()
{
}


// ---------------------------------------------------------------------------
//  TZW3UnitAttrWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The main driver window gives us a pointer to himself. He also implements the IPE
//  interface for the attribute editor, and set him as our IPE handler.
//
tCIDLib::TVoid TZW3UnitAttrWnd::SetEditInfo(TZWaveUSB3CWnd* const pwndDriver)
{
    m_pwndDriver = pwndDriver;
    SetIPEHandler(pwndDriver);
}


// ---------------------------------------------------------------------------
//  TZW3UnitAttrWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZW3UnitAttrWnd::bVisEdit(          TAttrData&      adatEdit
                            , const TArea&          areaCellScr
                            , const tCIDLib::TCard8 c8UserId)
{
    // Let the base class check it first
    if (TAttrEditWnd::bVisEdit(adatEdit, areaCellScr, c8UserId))
        return kCIDLib::True;


    return kCIDLib::False;
}


tCIDLib::TVoid
TZW3UnitAttrWnd::CellClicked(const  tCIDLib::TCard4     c4Row
                            , const tCIDLib::TCard4     c4Column
                            , const tCIDLib::TBoolean   bLeftButton)
{
    //
    //  If the driver is busy, we prevent editing. Else we just let it go on to
    //  the underlying class.
    //
    if (bLeftButton && !m_pwndDriver->bCanEdit())
    {
        TErrBox msgbBusy
        (
            L"The driver is either not in a state that allows editing, or there are "
            L"changes pending on the server side that need to be downloaded first."
        );
        msgbBusy.ShowIt(*this);
        return;
    }

    TParent::CellClicked(c4Row, c4Column, bLeftButton);
}




// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSB3CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUSB3CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUSB3CWnd::TZWaveUSB3CWnd( const   TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient(cqcdcThis, L"TZWaveUSB3CWnd", tCQCKit::EActLevels::Low, cuctxToUse)
    , m_bDelayNewCfg(kCIDLib::False)
    , m_bGotConfig(kCIDLib::False)
    , m_bInNetworkCur(kCIDLib::False)
    , m_bInNetworkNew(kCIDLib::False)
    , m_menuOpts(L"Options")
    , m_pwndAttrs(nullptr)
    , m_pwndDrvState(nullptr)
    , m_pwndInNetwork(nullptr)
    , m_pwndList(nullptr)
    , m_pwndOptions(nullptr)
    , m_pwndUnitInst(nullptr)
{
    // We have to set these upon reconnnect as well so we split this off
    InitStateData();
}

TZWaveUSB3CWnd::~TZWaveUSB3CWnd()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3CWnd: Public, inherited methods
// ---------------------------------------------------------------------------

// If our edit info is different from our original info, then we've changed something
tCIDLib::TBoolean TZWaveUSB3CWnd::bChanges() const
{
    // If we haven't gotten our config even once yet, say none
    if (!m_bGotConfig)
        return kCIDLib::False;

    // Otherwise compare our working config to the previously gotten/saved
    return m_dcfgEdit != m_dcfgOrg;
}


tCIDLib::TBoolean
TZWaveUSB3CWnd::bIPEValidate(const  TString&            strSrc
                            ,       TAttrData&          adatVal
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Get the unit (the user id is the unit id), and make sure that it's state
    //  is still good. If it went south while we were editing, then something is
    //  wrong.
    //
    const TZWUnitInfo* punitiTar = m_dcfgEdit.punitiFindById(tCIDLib::TCard1(c8UserId));
    if (!punitiTar || (punitiTar->eState() < tZWaveUSB3Sh::EUnitStates::GetInitVals))
    {
        strErrMsg = L"The driver configuration has changed while you were editing, in "
                    L"such a way that this change cannot be stored.";
        return kCIDLib::False;
    }

    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatVal, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;


    if (adatVal.strId() == kZWaveUSB3Sh::strUAttr_Name)
    {
        // Make sure it's not a dup and it's a legal base field name
        const TZWUnitInfo* punitiDup = m_dcfgEdit.punitiFindByName(strNewVal);

        if (punitiDup)
        {
            strErrMsg = L"This name is already in use by another unit";
            return kCIDLib::False;
        }

        if (!facCQCKit().bIsValidBaseFldName(strNewVal))
        {
            strErrMsg = L"The name must start with a character and be only characters, "
                        L"digits, underscrore or hyphen";
            return kCIDLib::False;
        }
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TZWaveUSB3CWnd::bSaveChanges(TString& strErrMsg)
{
    // If we never managed to get any, then do nothing
    if (!m_bGotConfig)
        return kCIDLib::True;

    // Do validation on our edit config. If that fails, give up
    if (!m_dcfgEdit.bValidate(strErrMsg))
        return kCIDLib::False;

    // If we are not connected to our server side driver, nothing we can do
    if(eConnState() == tCQCGKit::EConnStates::SrvOffline)
    {
        strErrMsg = L"The server side driver cannot be contacted, so the changes cannot be saved";
        return kCIDLib::False;
    }

    // Let's try to send our current edit config to the driver
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        // Flatten the current edit config out to the transfer object
        m_zwdxdGUI.Reset();
        m_zwdxdGUI.m_strCmd = kZWaveUSB3Sh::strSendData_SetConfig;
        {
            TBinMBufOutStream strmTar(&m_zwdxdGUI.m_mbufData);
            strmTar << m_dcfgEdit << kCIDLib::FlushIt;
            m_zwdxdGUI.m_c4BufSz = strmTar.c4CurSize();
        }

        if (bSendGUICmd(orbcServer()))
        {
            // It worked, so copy to the original now
            m_dcfgOrg = m_dcfgEdit;
        }
         else
        {
            strErrMsg = m_zwdxdGUI.m_strMsg;
            bRet = kCIDLib::False;
        }
   }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        bRet = kCIDLib::False;
    }
    return bRet;
}


//
//  If an edited value validates OK, this is called and we store the data back to the
//  unit.
//
tCIDLib::TVoid
TZWaveUSB3CWnd::IPEValChanged(  const   tCIDCtrls::TWndId   widSrc
                                , const TString&            strSrc
                                , const TAttrData&          adatNew
                                , const TAttrData&          adatOld
                                , const tCIDLib::TCard8     c8UserId)
{
    //
    //  Get the unit (the user id is the unit id), and make sure that it's state
    //  is still good. If it went south while we were editing, then something is
    //  wrong.
    //
    TZWUnitInfo* punitiTar = m_dcfgEdit.punitiFindById(tCIDLib::TCard1(c8UserId));
    if (!punitiTar || (punitiTar->eState() < tZWaveUSB3Sh::EUnitStates::GetInitVals))
    {
        TErrBox msgbErr
        (
            L"The driver configuration has changed while you were editing, in "
            L"such a way that this change cannot be stored."
        );
        msgbErr.ShowIt(*this);
        return;
    }

    //
    //  Some of these we generate ourself so we have to handle them ourself. The
    //  others just become option values on the unit, so we take the key and the
    //  formatted text of the value and set it as an option value.
    //
    if (adatNew.strId() == kZWaveUSB3Sh::strUAttr_Name)
    {
        // Update the unit and for this initial change the main list
        punitiTar->strName(adatNew.strValue());

        tCIDLib::TCard4 c4ListInd = m_pwndList->c4IdToIndex(tCIDLib::TCard4(c8UserId));
        m_pwndList->SetColText
        (
            c4ListInd, tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Name), adatNew.strValue()
        );
    }
     else
    {
        // The attribute id is the option key
        TString strNewVal;
        adatNew.FormatToText(strNewVal);
        punitiTar->SetOption(adatNew.strId(), strNewVal);
    }
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3CWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is for the attribute editor window, so he can prevent edits while the driver
//  is doing things that we know is about to invalidate any change that would be made.
//  So if he is doing a replication scan or he has configuration changes.
//
tCIDLib::TBoolean TZWaveUSB3CWnd::bCanEdit() const
{
    // Use the 'new' versions so that we have the most recent info
    TLocker lockrSync(pmtxSync());
    return
    (
        (m_c4CfgSerNumNew == m_c4CfgSerNumCur)
        && (m_eDrvStateNew >= tZWaveUSB3Sh::EDrvStates::Ready)
    );
}


//
//  A helper to send a msg to the server and get the response back. Other things
//  besides this window need to do this. Dialog boxes need to definitely. The caller
//  provides us with some temp values to use, to avoid constant creation and
//  destruction of the resources required.
//
tCIDLib::TBoolean
TZWaveUSB3CWnd::bSendSrvMsg(const   TWindow&                wndOwner
                            ,       TZWDriverXData&         zwdxdComm
                            ,       THeapBuf&               mbufTmp)
{
    tCIDLib::TCard4 c4Cnt;
    {
        TBinMBufOutStream strmTar(&mbufTmp);
        strmTar << zwdxdComm << kCIDLib::FlushIt;
        c4Cnt = strmTar.c4CurSize();
    }

    try
    {
        // We currently don't use the data name
        TString strDataName;
        const tCIDLib::TBoolean bRes = orbcServer()->bSendData
        (
            strMoniker()
            , kZWaveUSB3Sh::strSendData_Type
            , strDataName
            , c4Cnt
            , mbufTmp
            , sectUser()
        );

        if (!bRes)
        {
            // We didn't get anything back, fake an error back into the exchange object
            zwdxdComm.StatusReset(kCIDLib::False, L"The driver didn't understand the msg");
            return kCIDLib::False;
        }

        // STream in the response
        TBinMBufInStream strmSrc(&mbufTmp, c4Cnt);
        strmSrc >> zwdxdComm;
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        zwdxdComm.StatusReset(kCIDLib::False, errToCatch.strErrText());
        return kCIDLib::False;
    }

    catch(...)
    {
        TString strMsg = TString::strConcat(L"A system exception occured sending command: ", zwdxdComm.m_strCmd);
        TErrBox msgbErr(strMsg);
        msgbErr.ShowIt(*this);
        zwdxdComm.StatusReset(kCIDLib::False, strMsg);
        return kCIDLib::False;
    }

    // Return the status reply of the driver
    return zwdxdComm.m_bStatus;
}



// ---------------------------------------------------------------------------
//  TZWaveUSB3CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveUSB3CWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facZWaveUSB3C().bCreateDlgDesc(kZWaveUSB3C::ridIntf_Main, dlgdMain);

    // Find the attribute editor and replace the class with our own
    dlgdMain.SetCppType(kZWaveUSB3C::ridMain_Attrs, L"TZW3UnitAttrWnd");

    // And now create the controls
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    //
    //  Now do an initial auto-size to fit our client area. This will cause the
    //  the pane window to be resized, and it in turn will resize the panes, which
    //  will resize their contents.
    //
    AutoAdjustChildren(dlgdMain.areaPos(), areaClient());

    // It won't be shown but set our window text so it'll get picked up by any popups
    strWndText(facZWaveUSB3C().strMsg(kZWU3CMsgs::midMain_Title));

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kZWaveUSB3C::ridMain_Attrs, m_pwndAttrs);
    CastChildWnd(*this, kZWaveUSB3C::ridMain_DrvState, m_pwndDrvState);
    CastChildWnd(*this, kZWaveUSB3C::ridMain_InNetwork, m_pwndInNetwork);
    CastChildWnd(*this, kZWaveUSB3C::ridMain_UnitList, m_pwndList);
    CastChildWnd(*this, kZWaveUSB3C::ridMain_Options, m_pwndOptions);
    CastChildWnd(*this, kZWaveUSB3C::ridMain_UnitInstruct, m_pwndUnitInst);

    //
    //  Tell the attribute editor about us. We couldn't do it until now because it's
    //  create via dialog description. He will also set us up as the in place editor
    //  for himself.
    //
    m_pwndAttrs->SetEditInfo(this);

    // Register our event handlers
    m_pwndOptions->pnothRegisterHandler(this, &TZWaveUSB3CWnd::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TZWaveUSB3CWnd::eLBHandler);
    m_pwndAttrs->pnothRegisterHandler(this, &TZWaveUSB3CWnd::eAttrEditHandler);

    // Set up the column titles
    tCIDLib::TStrList colCols(tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Count));
    tCIDLib::ForEachE<tZWaveUSB3C::EListCols>
    (
        [&colCols](const tZWaveUSB3C::EListCols eCol)
        {
            colCols.objAdd(tZWaveUSB3C::strAltXlatEListCols(eCol));
        }
    );
    m_pwndList->SetColumns(colCols);


    // Set up the column widths to reasonable defaults per column
    TFontMetrics fmtrDef;
    {
        TGraphWndDev gdevTmp(*this);
        gdevTmp.QueryFontMetrics(fmtrDef);
    }

    tCIDLib::ForEachE<tZWaveUSB3C::EListCols>
    (
        [&colCols, this, &fmtrDef](const tZWaveUSB3C::EListCols eCol)
        {
            tCIDLib::TCard4 c4Width = 0;
            tCIDLib::EHJustify eHJustify = tCIDLib::EHJustify::Left;
            switch(eCol)
            {
                case tZWaveUSB3C::EListCols::Menu :
                    c4Width = 4;
                    eHJustify = tCIDLib::EHJustify::Center;
                    break;

                case tZWaveUSB3C::EListCols::Id :
                    c4Width = 6;
                    eHJustify = tCIDLib::EHJustify::Right;
                    break;

                case tZWaveUSB3C::EListCols::Name :
                    c4Width = 32;
                    break;

                case tZWaveUSB3C::EListCols::State:
                    c4Width = 12;
                    break;

                case tZWaveUSB3C::EListCols::Listener :
                    c4Width = 10;
                    break;

                case tZWaveUSB3C::EListCols::Make :
                case tZWaveUSB3C::EListCols::Model :
                    c4Width = 16;
                    break;

                default :
                    // Shoudln't happen, but just so we see it if it does
                    c4Width = 8;
                    break;
            };

            if (c4Width)
            {
                m_pwndList->SetColOpts
                (
                    tCIDLib::c4EnumOrd(eCol)
                    , c4Width * fmtrDef.c4AverageWidth()
                    , eHJustify
                    , 2 * fmtrDef.c4AverageWidth()
                );
            }
        }
    );

    //
    //  Set our in network text to match the default value of our current value
    //  flag so that it will be correct if we happen to be in that state when we
    //  start up (hence it would not be updated by our polling.)
    //
    m_pwndInNetwork->strWndText(facCQCKit().strBoolYesNo(m_bInNetworkCur));

    // Pre-create our options popup
    m_menuOpts.Create(facZWaveUSB3C(), kZWaveUSB3C::ridMenu_Options);

    //
    //  Check the off trace level since that's what we set as our default initial
    //  value. If the server is the same, we won't ever update, so we need to get
    //  this initial value set.
    //
    m_menuOpts.SetItemCheck(kZWaveUSB3C::ridMenu_Opt_Trace_Off, kCIDLib::True);

    // Make sure the got config flag is false initially upon creation
    m_bGotConfig = kCIDLib::False;

    return kCIDLib::True;
}


//
//  This is for getting 'once on connect' type data from the server side driver. It
//  is called from the background thread. We reset our state data, and force a call
//  to our own poll method, since it does what we need.
//
tCIDLib::TBoolean TZWaveUSB3CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    TLocker lockrSync(pmtxSync());

    // Initialize our per-connection state data
    InitStateData();

    // Do a poll and return whether we succeeded. If so, Connected() will be called
    return bDoPoll(orbcTarget);
}


//
//  This is called from the background polling thread, to give us a chance to ask
//  the server for the latest data.
//
//  We first ask for basic data, which let's us keep our status displays up to date.
//  And, if this indicates there is new configuration, we download that. We just
//  leave the data for the GUI thread to pick up.
//
//  We have to gather all the data into locals first, then lock, then store.
//
tCIDLib::TBoolean TZWaveUSB3CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    const tCIDLib::TCard4       c4PrevSerialNum = m_c4CfgSerNumNew;
    tCIDLib::TBoolean           bInNetwork;
    tCIDLib::TCard4             c4CfgSerNum;
    tZWaveUSB3Sh::EDrvStates    eDrvState;
    tCQCKit::EVerboseLvls       eTraceLevel;
    try
    {
        // As an initial test, get the general status info
        m_zwdxdPoll.Reset();
        m_zwdxdPoll.m_strCmd = kZWaveUSB3Sh::strSendData_QueryStatusInfo;
        if (!bSendPollCmd(orbcTarget))
            return kCIDLib::False;

        //
        //  It worked so stream the values out.
        {
            TBinMBufInStream strmSrc(&m_zwdxdPoll.m_mbufData, m_zwdxdPoll.m_c4BufSz);
            strmSrc >> c4CfgSerNum
                    >> bInNetwork
                    >> eDrvState
                    >> eTraceLevel;
        }

        //
        //  If not locked out, and the config serial number changed, we need to get
        //  the config.
        //
        if ((c4CfgSerNum != c4PrevSerialNum) && !m_bDelayNewCfg)
        {
            m_zwdxdPoll.Reset();
            m_zwdxdPoll.m_strCmd = kZWaveUSB3Sh::strSendData_QueryConfig;
            if (!bSendPollCmd(orbcTarget))
                return kCIDLib::False;

            TBinMBufInStream strmSrc(&m_zwdxdPoll.m_mbufData, m_zwdxdPoll.m_c4BufSz);
            strmSrc >> m_dcfgTmp;
        }

        // OK, let's lock and store the new data
        TLocker lockrSync(pmtxSync());

        m_bInNetworkNew = bInNetwork;
        m_eDrvStateNew = eDrvState;
        m_eTraceLevelNew = eTraceLevel;

        // If not locked out, update the config stuff
        if (!m_bDelayNewCfg)
        {
            m_c4CfgSerNumNew = c4CfgSerNum;
            if (c4CfgSerNum != c4PrevSerialNum)
                m_dcfgNew = m_dcfgTmp;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TZWaveUSB3CWnd::Connected()
{
    //
    //  Do an initial update of our data. Indicate this is the initial update after
    //  connection. That makes him take the new config as is. He locks so we don't
    //  have to do that.
    //
    UpdateData(kCIDLib::True);

    //
    //  Do an initial load of the unit list. We are just dealing with our own GUI
    //  thread data here, so don't need to lock.
    //
    LoadUnits();
}


tCIDLib::TVoid TZWaveUSB3CWnd::DoCleanup()
{
}

tCIDLib::TVoid TZWaveUSB3CWnd::LostConnection()
{
    // Clear the unit list
    m_pwndList->RemoveAll();
}


//
//  This is called if our poll callback indicates it left new data. In our case it
//  always does and we just check for changes ourself and update what needs to be.
//
tCIDLib::TVoid TZWaveUSB3CWnd::UpdateDisplayData()
{
    //
    //  We have a helper for this since we also need to do a variation of it from
    //  the initial connect. Pick up any new data left for us by the poll thread
    //  and update display as required. Indicate this is not the initial update.
    //  He locks while updating.
    //
    UpdateData(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

// When the user selects the new unit approval option, this is called
tCIDLib::TVoid TZWaveUSB3CWnd::ApproveNewUnits()
{
    // If there are outstanding changes, warn that they will be lost
    if (m_dcfgEdit != m_dcfgOrg)
    {
        const tCIDLib::TCard4 c4Choice = facCIDWUtils().c4TriChoiceDlg
        (
            *this
            , L"Lose Changes?"
            , facZWaveUSB3Sh().strMsg(kZWU3CMsgs::midQ_CouldLoseChanges)
            , L"Save First"
            , L"Just Approve"
            , L"Cancel"
        );

        if (c4Choice == 3)
            return;

        if (c4Choice == 1)
        {
            TString strErr;
            if (!bSaveChanges(strErr))
            {
                TErrBox msgbErr(L"Save Failed", strErr);
                msgbErr.ShowIt(*this);
                return;
            }

            // Wait for a little bit for the server to settle
            TThread::Sleep(2000);
        }
    }

    // We can do it, so send the command to the server
    m_zwdxdGUI.Reset();
    m_zwdxdGUI.m_strCmd = kZWaveUSB3Sh::strSendData_ApproveNewUnits;
    if (bSendGUICmd(orbcServer()))
    {
        TOkBox msgbDone(facZWaveUSB3C().strMsg(kZWU3CMsgs::midStatus_UnitsApproved));
        msgbDone.ShowIt(*this);
    }
     else
    {
        TErrBox msgbErr(m_zwdxdGUI.m_strMsg);
        msgbErr.ShowIt(*this);
    }

    //
    //  Just in case, get back in sync with the server now. We just call our own
    //  poll method, who will do what is required.
    //
    bDoPoll(orbcServer());
}


//
//  A helper to handle sending commands and getting the reply. We just call the
//  public one that lets everyone get access to this functionality, providing the
//  values it needs. WE have one for the GUI and one for the poll thread, since
//  we don't want to have to lock while sending msgs, and therefore they need to
//  use different members.
//
tCIDLib::TBoolean TZWaveUSB3CWnd::bSendGUICmd(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    if (!bSendSrvMsg(*this, m_zwdxdGUI, m_mbufGUI))
    {
        TErrBox msgbErr(m_zwdxdGUI.m_strMsg);
        msgbErr.ShowIt(*this);
    }
    return m_zwdxdGUI.m_bStatus;
}

// This one cannot do the error box since this is background stuff
tCIDLib::TBoolean TZWaveUSB3CWnd::bSendPollCmd(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    if (!bSendSrvMsg(*this, m_zwdxdPoll, m_mbufPoll))
        return kCIDLib::False;

    // We got something back so return the status
    return m_zwdxdPoll.m_bStatus;
}


//
//  This is called to load a specific unit up into the list, or to update it if it is
//  already there. We set the unit id as the row id, so that we can easily get from
//  id to raw regardless of sort. The caller sends us a temp list of of strings to use
//  to load up the column values, so it must be one string per column we have.
//
tCIDLib::TCard4
TZWaveUSB3CWnd::c4LoadUnitToList(const TZWUnitInfo& unitiCur, tCIDLib::TStrList& colCols)
{
    // Set up the new values
    colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Menu)] = L"v";
    if (unitiCur.strMake().bIsEmpty() || unitiCur.strModel().bIsEmpty())
    {
        colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Make)] = TString::strEmpty();
        colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Model)] = TString::strEmpty();
    }
     else
    {
        colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Make)] = unitiCur.strMake();
        colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Model)] = unitiCur.strModel();
    }

    colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Listener)] = facCQCKit().strBoolYesNo(unitiCur.bAlwaysOn());
    colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Id)].SetFormatted(unitiCur.c1Id());
    colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::State)] = tZWaveUSB3Sh::strAltXlatEUnitStates(unitiCur.eState());
    colCols[tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Name)] = unitiCur.strName();

    // Either add or update
    tCIDLib::TCard4 c4ListInd = m_pwndList->c4IdToIndex(unitiCur.c1Id(), kCIDLib::False);
    if (c4ListInd == kCIDLib::c4MaxCard)
        m_pwndList->c4AddItem(colCols, unitiCur.c1Id());
    else
        m_pwndList->UpdateRowAt(colCols, c4ListInd, unitiCur.c1Id(), kCIDLib::True);

    return c4ListInd;
}


//
//  Called when the user selects the disable option for a unit. We just send the
//  command. If it actually happens, we'll see it as a change coming back and will
//  update the unit's state display accordingly.
//
tCIDLib::TVoid
TZWaveUSB3CWnd::DisableUnit(TZWUnitInfo& unitiTar, const tCIDLib::TCard4 c4ListInd)
{
    m_zwdxdGUI.Reset();
    m_zwdxdGUI.m_strCmd = kZWaveUSB3Sh::strSendData_DisableUnit;
    m_zwdxdGUI.m_c1UnitId = unitiTar.c1Id();
    if (bSendGUICmd(orbcServer()))
    {
        TOkBox msgbDone(facZWaveUSB3C().strMsg(kZWU3CMsgs::midWarn_DisableDone));
        msgbDone.ShowIt(*this);

        //
        //  Go ahead and update our local display on the assumption is going to happen
        //  anyway given that we didn't get an error. This will make it happen faster.
        //
        unitiTar.eState(tZWaveUSB3Sh::EUnitStates::Disabled);
        m_pwndList->SetColText
        (
            c4ListInd
            , tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::State)
            , tZWaveUSB3Sh::strAltXlatEUnitStates(unitiTar.eState())
        );
    }
}


//
//  When the user selects to start an include/exclude operation from the options menu.
//  We tell the server side driver to start that process.
//
//  We block the poll thread from storing configuration during this process because
//  we need to force a full update of config after this is done and it would sneak
//  in and see the change before we could deal with it otherwise. If we end up failing
//  the process, the normal poll will subsequently catch up if any changes happened
//  while we were doing this.
//
tCIDLib::TVoid TZWaveUSB3CWnd::DoIncEx()
{
    // If they have any changes, tell them to save that first
    if (m_dcfgEdit != m_dcfgOrg)
    {
        TYesNoBox msgbWarn(facZWaveUSB3C().strMsg(kZWU3CMsgs::midQ_RepWithChanges));
        if (!msgbWarn.bShowIt(*this))
            return;
    }

    TBoolJanitor janInc(&m_bDelayNewCfg, kCIDLib::True);
    TZWUSB3IncludeDlg dlgInclude;
    const tCIDLib::TBoolean bRes = dlgInclude.bRunDlg(*this);

    // We need to query config now
    m_zwdxdGUI.Reset();
    m_zwdxdGUI.m_strCmd = kZWaveUSB3Sh::strSendData_QueryConfig;
    if (bSendGUICmd(orbcServer()))
    {
        TBinMBufInStream strmSrc(&m_zwdxdGUI.m_mbufData, m_zwdxdGUI.m_c4BufSz);
        strmSrc >> m_dcfgNew;

        //
        //  Update the new serial number so that UpdateData() will see new info.
        //  We can update this becasue we have the poll thread locked out from
        //  updating config info. Otherwise he'd just ovewrite this.
        //
        m_c4CfgSerNumNew = m_dcfgNew.c4CalcSNHash();

        //
        //  Do an initial mode data update. That will make him just take the new
        //  config as is, not try to integrate with existing stuff.
        //
        UpdateData(kCIDLib::True);
    }
     else
    {
        //
        //  Tell the user there was an error. Once we clear the include dialog
        //  flag, the polling will pick up any changes that might have occurred
        //  while we were in the dialog and update normally.
        //
        TErrBox msgbErr(m_zwdxdGUI.m_strMsg);
        msgbErr.ShowIt(*this);
    }
}


//
//  When the user drops down the per-unit menu and makes a selection, this is called.
//  We get the unit id and the list index.
//
tCIDLib::TVoid
TZWaveUSB3CWnd::DoUnitOpt(const tCIDLib::TCard1 c1Id, const tCIDLib::TCard4 c4ListInd)
{
    //
    //  Lock out new config being downloaded during this since it would invalidate our
    //  target unit object. It's an issue because there are non-blocking operations
    //  here that will allow the poll method to start happening again.
    //
    TBoolJanitor janInc(&m_bDelayNewCfg, kCIDLib::True);

    // Get the unit so we can update the menu based on the unit
    TZWUnitInfo& unitiTar = m_dcfgEdit.unitiFindById(c1Id);

    // Create a popup menu
    TPopupMenu menuOpts(L"UnitMenu");
    menuOpts.Create(facZWaveUSB3C(), kZWaveUSB3C::ridMenu_UnitOpts);

    // If the selected unit doesn't support associations disable those options
    if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Association))
    {
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_AssocToDrv, kCIDLib::False);
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_MngAssoc, kCIDLib::False);
    }

    // If it doesn't support configuration, disable the configuration related ones
    if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Config))
    {
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_MngCfgParam, kCIDLib::False);
    }

    //
    //  If neither association nor configuration, then clearly auto-config is useless.
    //  Or if the unit doesn't yet have device info, since we can't have any auto-cfg
    //  data until then.
    //
    if ((!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Association)
    &&  !unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Config))
    ||  !unitiTar.bHasDevInfo())
    {
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_AutoConfig, kCIDLib::False);
    }

    //  Set the enable/disable item to the opposite of whatever it is now
    if (unitiTar.eState() == tZWaveUSB3Sh::EUnitStates::Disabled)
        menuOpts.SetItemText(kZWaveUSB3C::ridMenu_UnitOpt_Disable, L"Enable");
    else
        menuOpts.SetItemText(kZWaveUSB3C::ridMenu_UnitOpt_Disable, L"Disable");

    //
    //  If the unit already has device info, then disable setting of the unit type.
    //  If it doesn't have it, then disable show notes.
    //
    if (unitiTar.bHasDevInfo())
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_SetType, kCIDLib::False);
    else
        menuOpts.SetItemEnable(kZWaveUSB3C::ridMenu_UnitOpt_ShowNotes, kCIDLib::False);

    // Get the area of the cell and convert to screen coordinates
    TArea areaCell;
    m_pwndList->QueryColArea
    (
        c4ListInd, tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Menu), areaCell
    );
    m_pwndList->ToScreenCoordinates(areaCell, areaCell);

    // Pop it up at the bottom left of the cell, top aligned
    const tCIDLib::TCard4 c4SelInd = menuOpts.c4Process
    (
        *this, areaCell.pntLL(), tCIDLib::EVJustify::Top, tCIDLib::EHJustify::Left
    );

    switch(c4SelInd)
    {
        case 0 :
            // No selection
            break;

        case kZWaveUSB3C::ridMenu_UnitOpt_AssocToDrv :
        {
            TZWUSB3AssocToDrvDlg dlgAssoc;
            dlgAssoc.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_AutoConfig :
        {
            TZWUSB3AutoCfgDlg dlgAutoCfg;
            dlgAutoCfg.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_Disable :
        {
            //
            //  Ask the server to enable or disable the node. If he does we'll see
            //  that as a change coming back to us.
            //
            DisableUnit(unitiTar, c4ListInd);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_GenReport:
        {
            TZWUSB3GenReportDlg dlgReport;
            dlgReport.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_MngAssoc :
        {
            TZWUSB3AssocDlg dlgAssoc;
            dlgAssoc.RunDlg(*this, m_dcfgEdit, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_MngCfgParam :
        {
            TZWUSB3CfgParamDlg dlgCfgParam;
            dlgCfgParam.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_ShowInfo :
        {
            //
            //  Pass this guy to a simple dialog that dipslays the info and allows
            //  the user to save it to a text file.
            //
            TZWUSB3UnitInfoDlg dlgInfo;
            dlgInfo.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_ShowNotes :
        {
            TString strNotes;
            if (facZWaveUSB3Sh().bQueryModelNotes(unitiTar.c8ManIds(), strNotes))
            {
                TString strTitle = TString::strConcat(L"Notes for unit: ", unitiTar.strName());

                TOkBox msgbNotes(strTitle, strNotes);
                msgbNotes.ShowIt(*this);
            }
             else
            {
                TOkBox msgbNone(L"This unit doesn't have any associated notes");
                msgbNone.ShowIt(*this);
            }
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_Rescan :
        {
            TZWUSB3ResetUnitDlg dlgReset;
            dlgReset.RunDlg(*this, unitiTar);
            break;
        }

        case kZWaveUSB3C::ridMenu_UnitOpt_SetType :
        {
            SetUnitType(unitiTar, c4ListInd);
            break;
        }

        default :
            break;
    };
}


//
//  For our attribute editor, if the user does visual editing, we don't get any IPE
//  calls since that is not related to in place editing. Instead we get notified this
//  way. We will validate any that need validating in our bVisEdit() override in our
//  attribute editor derived class above. But we need this to store the value.
//
tCIDCtrls::EEvResponses TZWaveUSB3CWnd::eAttrEditHandler(TAttrEditInfo& wnotInfo)
{
    //
    //  The unit id is the user data. Just look it up and pass it the new attribute
    //  data.
    //
    tCIDLib::TCard1 c1Id = tCIDLib::TCard1(wnotInfo.c8UserId());
    TZWUnitInfo& unitiLoad = m_dcfgEdit.unitiFindById(c1Id);
    unitiLoad.StoreUnitAttr(wnotInfo.adatNew());

    return tCIDCtrls::EEvResponses::Handled;
}


// Handle clicks from our buttons
tCIDCtrls::EEvResponses TZWaveUSB3CWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    if (wnotEvent.widSource() == kZWaveUSB3C::ridMain_Options)
    {
        //
        //  Search the current config. If no units are in wait for approval, disable
        //  the approve new units options, else enable it.
        //
        m_menuOpts.SetItemEnable
        (
            kZWaveUSB3C::ridMenu_Opt_ApproveNew, m_dcfgEdit.bUnitsToApprove()
        );

        //
        //  Get the options button area and translate to screen coordinates, so that
        //  we can pop it up relative to where the button is.
        //
        TArea areaButt = m_pwndOptions->areaWnd();
        ToScreenCoordinates(areaButt, areaButt);
        const tCIDLib::TCard4 c4SelId = m_menuOpts.c4Process
        (
            *this, areaButt.pntUR(), tCIDLib::EVJustify::Bottom, tCIDLib::EHJustify::Right
        );

        switch(c4SelId)
        {
            case 0 :
                // No selection was made
                break;

            case kZWaveUSB3C::ridMenu_Opt_ApproveNew :
                ApproveNewUnits();
                break;

            case kZWaveUSB3C::ridMenu_Opt_Trace_Off :
            case kZWaveUSB3C::ridMenu_Opt_Trace_Low :
            case kZWaveUSB3C::ridMenu_Opt_Trace_Medium :
            case kZWaveUSB3C::ridMenu_Opt_Trace_High :
            case kZWaveUSB3C::ridMenu_Opt_Trace_Flush :
            case kZWaveUSB3C::ridMenu_Opt_Trace_Reset :
                SetTraceOption(c4SelId);
                break;

            case kZWaveUSB3C::ridMenu_Opt_IncEx :
                DoIncEx();
                break;

            case kZWaveUSB3C::ridMenu_Opt_SaveCfg :
                SaveCfgToFile();
                break;

            default :
                TErrBox msgbErr(L"Unknown option");
                msgbErr.ShowIt(*this);
                break;
        };
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle events from our main list window
tCIDCtrls::EEvResponses TZWaveUSB3CWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridMain_UnitList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            // Clear the attribute window and the status description text
            m_pwndAttrs->RemoveAll();
            m_pwndUnitInst->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            //
            //  Load up the attributes for this guy. It also displays some helper
            //  text for the newly selected unit, based on its state.
            //
            LoadUnitAttrs(tCIDLib::TCard1(wnotEvent.c4Id()));
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::ColClicked)
        {
            //
            //  We handle this in order to provide a little drop down menu for
            //  the units, so that the user can invoke some operations on them.
            //
            //  So, if they click that cell, we position a menu just below the
            //  menu cell.
            //
            if (wnotEvent.c4ColIndex() == tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Menu))
                DoUnitOpt(tCIDLib::TCard1(wnotEvent.c4Id()), wnotEvent.c4Index());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Called from the ctor and upon (re)establishing a connection, to get our polling
//  oriented flags set up.
//
tCIDLib::TVoid TZWaveUSB3CWnd::InitStateData()
{
    m_bDelayNewCfg = kCIDLib::False;

    m_bInNetworkCur = kCIDLib::False;
    m_bInNetworkNew = kCIDLib::False;

    m_c4CfgSerNumCur = 0;
    m_c4CfgSerNumNew = 0;

    m_eDrvStateCur = tZWaveUSB3Sh::EDrvStates::Startup;
    m_eDrvStateNew = tZWaveUSB3Sh::EDrvStates::Startup;

    m_eTraceLevelCur = tCQCKit::EVerboseLvls::Off;
    m_eTraceLevelNew = tCQCKit::EVerboseLvls::Off;
}


//
//  This will load up the attribute editor window with the editable attributes of
//  the passed unit.
//
tCIDLib::TVoid TZWaveUSB3CWnd::LoadUnitAttrs(const tCIDLib::TCard1 c1Id)
{
    m_pwndAttrs->RemoveAll();
    try
    {
        TZWUnitInfo& unitiLoad = m_dcfgEdit.unitiFindById(c1Id);

        //
        //  Based on the unit state, we display some helper text in a static text
        //  control under the list.
        //
        m_pwndUnitInst->strWndText
        (
            tZWaveUSB3Sh::strXlatEUnitStates(unitiLoad.eState())
        );

        // See if this guy has device info. If not, nothing more to do.
        if (!unitiLoad.bHasDevInfo())
            return;

        //
        //  Let it try to create its unit object. Even if it already has one, we let
        //  let it create a new one, in case something has changed that would affect
        //  which one it would create, or the options it would present us. Tell it
        //  this is being done on the client side.
        //
        //  This will give him a chance to parse out any extra info from the device
        //  info file, which might affect options.
        //
        unitiLoad.MakeUnitObject(kCIDLib::True);

        // If by some chance he failed to create one, we can't do anything
        if (!unitiLoad.bHasUnitObject())
        {
            TErrBox msgbErr(L"The unit failed to create a unit handler object");
            msgbErr.ShowIt(*this);
            return;
        }

        //
        //  And now ask him for his editable attributes. Some of these are option
        //  values provided by the unit handler and CC impls. Some are just direct
        //  edits of members of the unit info object.
        //
        tCIDMData::TAttrList colAttrs;
        unitiLoad.QueryUnitAttrs(colAttrs);

        //
        //  And now we can load them to the attribute editor. So that we can get
        //  back to this unit when we get IPE callbacks, we need to pass along the
        //  unit id.
        //
        m_pwndAttrs->LoadValues(colAttrs, unitiLoad.c1Id());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// Does the grunt work to load up the unit list
tCIDLib::TVoid TZWaveUSB3CWnd::LoadUnits()
{
    // Prevent painting while we do this to avoid blinkies
    TWndPaintJanitor janPaint(m_pwndList);

    // Remove existing units
    m_pwndList->RemoveAll();

    // If our config indicates we are in network, load the info
    if (m_dcfgEdit.bInNetwork())
    {
        tCIDLib::TStrList colCols(tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Count));
        colCols.AddXCopies(TString::strEmpty(), tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Count));

        const tZWaveUSB3Sh::TUnitInfoList& colLoad = m_dcfgEdit.colUnits();
        const tCIDLib::TCard4 c4Count = colLoad.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            c4LoadUnitToList(colLoad[c4Index], colCols);
    }

    // If the list is not empty, select the first one
    if (m_pwndList->c4ItemCount())
        m_pwndList->SelectByIndex(0);
}


//
//  Called when the user selects Save Config from the manu option menu. We let him choose
//  a target file and write out our config to that.
//
tCIDLib::TVoid TZWaveUSB3CWnd::SaveCfgToFile()
{
    // Let the user select an output file
    tCIDLib::TBoolean bRes = kCIDLib::False;
    TString strSelected;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Z-Wave Cfg Dump", L"*.CQCZWCfg"));

        bRes = facCIDCtrls().bSaveFileDlg
        (
            *this
            , facZWaveUSB3C().strMsg(kZWU3CMsgs::midMsg_SelectTargetFl)
            , TString::strEmpty()
            , TString::strEmpty()
            , strSelected
            , colFileTypes
            , tCIDCtrls::EFSaveOpts::FilesOnly
        );
    }

    // If they made a selection, then let's do it
    if (bRes)
    {
        try
        {
            TBinFileOutStream strmOut
            (
                strSelected
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmOut << m_dcfgNew << kCIDLib::FlushIt;
        }

        catch(TError& errToCatch)
        {
            TExceptDlg dlgErr
            (
                *this
                , L"An error occurred while saving the configuration file"
                , errToCatch
            );
        }
    }
}


// Called when the user select one of the trace file related menu options
tCIDLib::TVoid TZWaveUSB3CWnd::SetTraceOption(const tCIDLib::TCard4 c4SelId)
{
    tCIDLib::TBoolean bSetCheck = kCIDLib::False;
    tCIDLib::TCard4 c4TraceOpt = kCIDLib::c4MaxCard;
    tCIDLib::TCard4 c4MenuId = 0;
    switch(c4SelId)
    {
        case kZWaveUSB3C::ridMenu_Opt_Trace_Off :
            bSetCheck = kCIDLib::True;
            c4TraceOpt = 0;
            break;

        case kZWaveUSB3C::ridMenu_Opt_Trace_Low :
            bSetCheck = kCIDLib::True;
            c4TraceOpt = 1;
            break;

        case kZWaveUSB3C::ridMenu_Opt_Trace_Medium :
            bSetCheck = kCIDLib::True;
            c4TraceOpt = 2;
            break;

        case kZWaveUSB3C::ridMenu_Opt_Trace_High :
            bSetCheck = kCIDLib::True;
            c4TraceOpt = 3;
            break;

        case kZWaveUSB3C::ridMenu_Opt_Trace_Flush :
            c4TraceOpt = 5;
            break;

        case kZWaveUSB3C::ridMenu_Opt_Trace_Reset :
            c4TraceOpt = 6;
            break;

        default :
            break;
    };

    if (c4TraceOpt == kCIDLib::c4MaxCard)
    {
        TErrBox msgbErr(L"Invalid trace menu option received");
        msgbErr.ShowIt(*this);
        return;
    }

    m_zwdxdGUI.Reset();
    m_zwdxdGUI.m_strCmd = kZWaveUSB3Sh::strSendData_SetTrace;
    m_zwdxdGUI.m_c4Val1 = c4TraceOpt;
    if (bSendGUICmd(orbcServer()))
    {
        if (bSetCheck)
            m_menuOpts.SetItemCheck(c4SelId, kCIDLib::True);
    }
}



tCIDLib::TVoid
TZWaveUSB3CWnd::SetUnitType(TZWUnitInfo& unitiTar, const tCIDLib::TCard4 c4ListInd)
{
    //
    //  Pass in any existing manids value. It'll try to select that one if it is
    //  present.
    //
    tCIDLib::TCard8 c8ManIds = unitiTar.c8ManIds();
    const tCIDLib::TCard8 c8OrgIds = c8ManIds;

    // Let them select. If no selection just return
    TZWUSB3SelUTypeDlg dlgSel;
    if (!dlgSel.bRunDlg(*this, c8ManIds, unitiTar.strName()))
        return;

    // OK, let's let this unit update himself with the selected device info
    TString strErr;
    if (unitiTar.bForceManIds(c8ManIds, strErr))
    {
        //
        //  Update the list row for this guy, and reselect it, forcing an event to
        //  be sent so that the attribute editor will update.
        //
        tCIDLib::TStrList colCols;
        colCols.AddXCopies
        (
            TString::strEmpty(), tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Count)
        );
        c4LoadUnitToList(unitiTar, colCols);
        m_pwndList->SelectByIndex(c4ListInd, kCIDLib::True);

        //
        //  If the man id is a generic one, make sure they know they have to do any
        //  associations and configuration parameters. Else just let them know they
        //  have to save before the change will take effect.
        //
        TString strMsg
        (
            L"The type has been set, but will not take effect until you save changes."
        );
        if (facZWaveUSB3Sh().bIsGenericManId(c8ManIds))
        {
            strMsg.Append
            (
                L"\n\nAlso note that, for generic handlers, you must set up any configuration "
                L"parameters and associations required. The driver has no idea what is "
                L"appropriate."
            );
        }
        TOkBox msgbDone(strMsg);
        msgbDone.ShowIt(*this);
    }
     else
    {
        TErrBox msgbErr(strErr);
        msgbErr.ShowIt(*this);
    }
}

//
//  Update our data. This is called from the GUI thread. We see if the poll thread has
//  gotten any new info. If so we update as needed.
//
//  WE HAVE A special case of the initial one, where we just want to take what is in
//  new as is. This is when we first get config from the driver, or after a replication
//  completes.
//
//  We have to lock during this since we are accessing the 'new' versions of the data
//  members. We make those our working values, so we need to lock out the poll thread
//  during this or race conditions galore.
//
tCIDLib::TVoid TZWaveUSB3CWnd::UpdateData(const tCIDLib::TBoolean bInitial)
{
    TLocker lockrSync(pmtxSync());

    //
    //  If any states have changed update our displays of them. We need to lock while
    //  doing this to insure consistency since the poll thread will be updating them.
    //
    if (m_bInNetworkNew != m_bInNetworkCur)
    {
        m_bInNetworkCur = m_bInNetworkNew;
        m_pwndInNetwork->strWndText(facCQCKit().strBoolYesNo(m_bInNetworkCur));

        // If we aren't in the network anymore, then clear the unit list
        if (!m_bInNetworkCur)
            m_pwndList->RemoveAll();
    }

    if(m_eDrvStateNew != m_eDrvStateCur)
    {
        m_eDrvStateCur = m_eDrvStateNew;
        m_pwndDrvState->strWndText(tZWaveUSB3Sh::strLoadEDrvStates(m_eDrvStateCur));
    }

    // Update the menu if the trace level changes
    if (m_eTraceLevelCur != m_eTraceLevelNew)
    {
        m_eTraceLevelCur = m_eTraceLevelNew;

        tCIDLib::TResId ridNew = 0;
        if (m_eTraceLevelCur == tCQCKit::EVerboseLvls::Off)
            ridNew = kZWaveUSB3C::ridMenu_Opt_Trace_Off;
        else if (m_eTraceLevelCur == tCQCKit::EVerboseLvls::Low)
            ridNew = kZWaveUSB3C::ridMenu_Opt_Trace_Low;
        else if (m_eTraceLevelCur == tCQCKit::EVerboseLvls::Medium)
            ridNew = kZWaveUSB3C::ridMenu_Opt_Trace_Medium;
        else if (m_eTraceLevelCur == tCQCKit::EVerboseLvls::High)
            ridNew = kZWaveUSB3C::ridMenu_Opt_Trace_High;

        // Check new one and uncheck any previous
        if (ridNew)
            m_menuOpts.SetItemCheck(ridNew, kCIDLib::True, kCIDLib::True);
    }

    //
    //  If we see the configuration change here, then it was not caused by us, it
    //  was by the driver. The only thing that can be is that we have updated the
    //  state of a unit or it woke up and we were able to get info about it. Anything
    //  else can only change because we forced a replication and in that case the
    //  new config is explicitly requested and stored up front.
    //
    if(m_c4CfgSerNumNew != m_c4CfgSerNumCur)
    {
        m_c4CfgSerNumCur = m_c4CfgSerNumNew;

        tCIDLib::TBoolean bCurChanged = kCIDLib::False;
        tCIDLib::TCard4 c4ListSelInd = kCIDLib::c4MaxCard;
        if (bInitial)
        {
            // Remember we have at least gotten valid config once
            m_bGotConfig = kCIDLib::True;

            // Copy new stuff to edit, then load units
            m_dcfgEdit = m_dcfgNew;
            LoadUnits();
        }
         else
        {
            //
            //  Not an initial load, so we start with current edits, and we will
            //  weave in any changes.
            //
            const tCIDLib::TCard4 c4Count = m_dcfgEdit.c4UnitCount();
            c4ListSelInd = m_pwndList->c4CurItem();
            tCIDLib::TStrList colCols;
            colCols.AddXCopies
            (
                TString::strEmpty(), tCIDLib::c4EnumOrd(tZWaveUSB3C::EListCols::Count)
            );
            const tZWaveUSB3Sh::TUnitInfoList& colSrc = m_dcfgNew.colUnits();
            for(tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                tCIDLib::TCard4 c4At;
                if (m_dcfgEdit.bUpdateUnit(colSrc[c4Index], c4At))
                {
                    m_dcfgEdit.bUpdateUnit(colSrc[c4Index], c4At);
                    const tCIDLib::TCard4 c4CurListInd = c4LoadUnitToList
                    (
                        m_dcfgEdit.unitiAt(c4At), colCols
                    );

                    if (c4ListSelInd == c4CurListInd)
                        bCurChanged = kCIDLib::True;
                }
            }
        }

        // Make new edit content the new baseline
        m_dcfgOrg = m_dcfgEdit;

        //
        //  If there weren't any before, select one if we can. Else force a re-selection
        //  on the current item if we changed it, so that the attribute editor will
        //  update.
        //
        if ((c4ListSelInd == kCIDLib::c4MaxCard) && m_pwndList->c4ItemCount())
            m_pwndList->SelectByIndex(0);
        else if (bCurChanged)
            m_pwndList->SelectByIndex(c4ListSelInd, kCIDLib::True);
    }
}
