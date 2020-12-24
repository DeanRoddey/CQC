//
// FILE NAME: CQCAdmin_EdSysCfgTab2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/05/2016
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
//  This file implements the room data oriented tabs, to keep the main tab file from
//  getting too large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "DevClsHdr_Security.hpp"
#include    "CQCAdmin_EdSysCfgTab.hpp"
#include    "CQCAdmin_EdSysCfgTab_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCESCTabBase, TWindow)
RTTIDecls(TEdSCValErrDlg, TDlgBox)
RTTIDecls(TESCRoomsTab, TTabWindow)
RTTIDecls(TCQCESCTGen, TCQCESCTabBase)
RTTIDecls(TCQCESCTGActs, TCQCESCTabBase)
RTTIDecls(TCQCESCTHVAC, TCQCESCTabBase)
RTTIDecls(TCQCESCTLoads, TCQCESCTabBase)
RTTIDecls(TCQCESCTMedia, TCQCESCTabBase)
RTTIDecls(TCQCESCTRmModes, TCQCESCTabBase)
RTTIDecls(TCQCESCTSecure, TCQCESCTabBase)
RTTIDecls(TCQCESCTVoice, TCQCESCTabBase)
RTTIDecls(TCQCESCTWeather, TCQCESCTabBase)
RTTIDecls(TCQCESCTXOvers, TCQCESCTabBase)



// ---------------------------------------------------------------------------
//   CLASS: TEdSCValErrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCValErrDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCValErrDlg::TEdSCValErrDlg() :

    m_c4SelErr(kCIDLib::c4MaxCard)
    , m_pcolErrList(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndGoTo(nullptr)
    , m_pwndList(nullptr)
{
}

TEdSCValErrDlg::~TEdSCValErrDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCValErrDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCValErrDlg::bRunDlg(const   TWindow&                wndOwner
                        , const TValErrInfo::TErrList&  colErrs
                        ,       tCIDLib::TCard4&        c4SelErr)
{
    //
    //  Store the list away for later use, and assume no error selected
    //  until proven otherwise.
    //
    c4SelErr = kCIDLib::c4MaxCard;
    m_pcolErrList = &colErrs;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_ESCValErrs
    );

    // Just return true if they selected save, and the selected error
    c4SelErr = m_c4SelErr;
    return (c4Res == kCQCAdmin::ridDlg_ESCValErrs_GoTo);
}


// ---------------------------------------------------------------------------
//  TEdSCValErrDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCValErrDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_ESCValErrs_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ESCValErrs_GoTo, m_pwndGoTo);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ESCValErrs_List, m_pwndList);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCValErrDlg::eClickHandler);
    m_pwndGoTo->pnothRegisterHandler(this, &TEdSCValErrDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEdSCValErrDlg::eLBHandler);

    // Set the columns on the list. They are not displayed but have to be there
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());
    m_pwndList->SetColumns(colCols);

    //
    //  Load up the errors. We load them by main tab and within the room tab by
    //  room. So we watch for the name to change and put out a separator item for
    //  each one as a separator.
    //
    const tCIDLib::TCard4 c4Count = m_pcolErrList->c4ElemCount();
    if (c4Count)
    {
        // Start off the current room name with the 0th entry
        TString strCurName;
        TString strText;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TValErrInfo& verrCur = m_pcolErrList->objAt(c4Index);

            if (verrCur.m_strName != strCurName)
            {
                //
                //  Put out a separator item. Ask the error info object to
                //  format out a separator for us.
                //
                verrCur.FormatSepText(strText);
                m_pwndList->c4AddDivider(strText);

                // Save this as the new current name
                strCurName = verrCur.m_strName;
            }

            colCols[0] = verrCur.m_strErr;
            m_pwndList->c4AddItem(colCols, c4Index);
        }
    }

    // Find the first non-divider item, which should be the 2nd one
    if (m_pwndList->c4ItemCount() > 1)
        m_pwndList->SelectByIndex(1, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCValErrDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdSCValErrDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ESCValErrs_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_ESCValErrs_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ESCValErrs_GoTo)
    {
        // GoTo should not be enabled unless it's on a non-divider
        m_c4SelErr = m_pwndList->c4CurItemId();
        EndDlg(kCQCAdmin::ridDlg_ESCValErrs_GoTo);
    }

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdSCValErrDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ESCValErrs_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            //
            //  If it's not one of the room divider items, then store the index
            //  of the selected error and exit. Dividers have the user flag set.
            //
            const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
            if (!m_pwndList->bFlagAt(c4ListInd))
            {
                m_c4SelErr = m_pwndList->c4IndexToId(c4ListInd);
                EndDlg(kCQCAdmin::ridDlg_ESCValErrs_GoTo);
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // If on a divider, disable Go To, else enable it
            m_pwndGoTo->SetEnable(!m_pwndList->bFlagAt(wnotEvent.c4Index()));
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTabBase::TValErrInfo
//  PREFIX: verr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTabBase::TValErrInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TValErrInfo::FormatSepText(TString& strToFill) const
{
    if (m_eType == TValErrInfo::ETypes::Room)
        strToFill = L"Room: ";
    else
        strToFill.Clear();

    strToFill.Append(m_strName);
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTabBase
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTabBase: Destructor
// ---------------------------------------------------------------------------
TCQCESCTabBase::~TCQCESCTabBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTabBase: Public, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCESCTabBase::DisableCtrls()
{
    // Nothing to display now
    m_strRoomUID.Clear();
}


tCIDLib::TVoid TCQCESCTabBase::ShowInfo(const  TString& strUID)
{
    // We just store away the current room UID for later use
    m_strRoomUID = strUID;
}


// ---------------------------------------------------------------------------
//  TCQCESCTabBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get the room we are set up for.
//
TCQCSysCfgRmInfo& TCQCESCTabBase::scriRoom()
{
    if (m_strRoomUID.bIsEmpty())
    {
        facCQCAdmin.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCAErrs::errcSysCfg_RoomUIDNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strWndText()
        );
    }

    tCIDLib::TCard4 c4Index;
    return m_pscfgEdit->scriRoomByUID(m_strRoomUID, c4Index);
}

const TCQCSysCfgRmInfo& TCQCESCTabBase::scriRoom() const
{
    if (m_strRoomUID.bIsEmpty())
    {
        facCQCAdmin.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCAErrs::errcSysCfg_RoomUIDNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strWndText()
        );
    }

    tCIDLib::TCard4 c4Index;
    return m_pscfgEdit->scriRoomByUID(m_strRoomUID, c4Index);
}


// Provide access to the configuration data
TCQCSysCfg& TCQCESCTabBase::scfgEdit()
{
    CIDAssert(m_pscfgEdit != 0, L"The configuration data pointer is not set");
    return *m_pscfgEdit;
}


// ---------------------------------------------------------------------------
//  TCQCESCTabBase: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCESCTabBase::TCQCESCTabBase( TCQCSysCfg* const       pscfgEdit
                                , TCQCFldCache* const   pcfcDevs
                                , const TString&        strTabId) :

    TTabWindow(strTabId, TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pcfcDevs(pcfcDevs)
    , m_pscfgEdit(pscfgEdit)
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTabBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Some helpers to make putting errors into the validation error list
//  much more convenient.
//
tCIDLib::TVoid
TCQCESCTabBase::AddError(const  TCQCSysCfgRmInfo&       scriSrc
                        ,       TValErrInfo::TErrList&  colErrs
                        , const tCIDLib::TMsgId         midErr
                        , const tCIDCtrls::TWndId       widFocus
                        , const tCIDLib::TCard4         c4Index)
{
    colErrs.objAdd
    (
        TValErrInfo
        (
            scriSrc, c4TabId(), widFocus, facCQCAdmin.strMsg(midErr), c4Index
        )
    );
}

tCIDLib::TVoid
TCQCESCTabBase::AddError(const  TCQCSysCfgRmInfo&       scriSrc
                        ,       TValErrInfo::TErrList&  colErrs
                        , const tCIDLib::TMsgId         midErr
                        , const MFormattable&           fmtblToken1
                        , const tCIDCtrls::TWndId       widFocus
                        , const tCIDLib::TCard4         c4Index)
{
    colErrs.objAdd
    (
        TValErrInfo
        (
            scriSrc
            , c4TabId()
            , widFocus
            , facCQCAdmin.strMsg(midErr, fmtblToken1)
            , c4Index
        )
    );
}

tCIDLib::TVoid
TCQCESCTabBase::AddError(const  TCQCSysCfgRmInfo&       scriSrc
                        ,       TValErrInfo::TErrList&  colErrs
                        , const tCIDLib::TMsgId         midErr
                        , const MFormattable&           fmtblToken1
                        , const MFormattable&           fmtblToken2
                        , const tCIDCtrls::TWndId       widFocus
                        , const tCIDLib::TCard4         c4Index)
{
    colErrs.objAdd
    (
        TValErrInfo
        (
            scriSrc
            , c4TabId()
            , widFocus
            , facCQCAdmin.strMsg(midErr, fmtblToken1, fmtblToken2)
            , c4Index
        )
    );
}

//
//  A helper to check whether a given function has been selected for the currently
//  set room.
//
tCIDLib::TBoolean TCQCESCTabBase::bFuncEnabled(const tCQCSysCfg::ERmFuncs eFunc) const
{
    return !scriRoom().strFuncId(eFunc).bIsEmpty();
}


//
//  A helper to return the unique id for a given function for our current room.
//
const TString& TCQCESCTabBase::strFuncId(const tCQCSysCfg::ERmFuncs eFunc) const
{
    return scriRoom().strFuncId(eFunc);
}


//
//  A helper to return the title text for a given function for our current room. This
//  is to make it easy for the derived tabs to display the name of the configured
//  references back to the main config lists.
//
//  WE just get our current room reference and call a helper on the config to get it.
//
const TString& TCQCESCTabBase::strFuncTitle(const tCQCSysCfg::ERmFuncs eFunc) const
{
    return m_pscfgEdit->strRoomFuncTitle(eFunc, scriRoom());
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTGen
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTGen: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTGen::TCQCESCTGen(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"General")
    , m_pwndAllowRmSel(nullptr)
    , m_pwndEnabled(nullptr)
    , m_pwndVarList(nullptr)
{
}

TCQCESCTGen::~TCQCESCTGen()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTGen: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCESCTGen::bIPETestCanEdit(const  tCIDCtrls::TWndId
                            , const tCIDLib::TCard4     c4ColInd
                            ,       TAttrData&          adatFill
                            ,       tCIDLib::TBoolean&  bValueSet)
{
    // The first column is the number
    if (!c4ColInd)
        return kCIDLib::False;

    adatFill.Set(L"VarVal", L"VarVal", tCIDMData::EAttrTypes::String);
    return kCIDLib::True;
}


//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTGen::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndAllowRmSel->SetCheckedState(kCIDLib::False);
    m_pwndEnabled->SetCheckedState(kCIDLib::False);

    m_pwndAllowRmSel->SetEnable(kCIDLib::False);
    m_pwndEnabled->SetEnable(kCIDLib::False);

    // We don't empty the variables list, but clear the variable values column
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRoomVars; c4Index++)
        m_pwndVarList->SetColText(c4Index, 1, TString::strEmpty());
    m_pwndVarList->SetEnable(kCIDLib::False);
}


tCIDLib::TVoid
TCQCESCTGen::IPEValChanged( const   tCIDCtrls::TWndId
                            , const TString&
                            , const TAttrData&          adatNew
                            , const TAttrData&
                            , const tCIDLib::TCard8     c8UserId)
{
    //
    //  Just store the value. In the auto-IPE scenario, the MC list box sends us the row
    //  id as the user id. We stored the row id when we loaded the list,
    //
    scriRoom().SetRoomVar(tCIDLib::TCard4(c8UserId), adatNew.strValue());
}


tCIDLib::TVoid TCQCESCTGen::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTGen::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    m_pwndEnabled->SetEnable(kCIDLib::True);
    m_pwndEnabled->SetCheckedState(scriRm.bEnabled());
    m_pwndAllowRmSel->SetEnable(kCIDLib::True);
    m_pwndAllowRmSel->SetCheckedState(scriRm.bAllowRmSel());

    // Load up the variables
    m_pwndVarList->SetEnable(kCIDLib::True);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRoomVars; c4Index++)
        m_pwndVarList->SetColText(c4Index, 1, scriRm.strRoomVarAt(c4Index));
    m_pwndVarList->SelectByIndex(0);

    // Set us as the auto IPE handler for the list
    m_pwndVarList->SetAutoIPE(this);
}


tCIDLib::TVoid
TCQCESCTGen::Validate(  TCQCSysCfgRmInfo&           scriRoom
                        , TValErrInfo::TErrList&    colErrs)
{
    // There's nothing they can do wrong in this one
}


// ---------------------------------------------------------------------------
//  TCQCESCTGen: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTGen::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgGen, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGen_AllowRmSel, m_pwndAllowRmSel);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGen_Enabled, m_pwndEnabled);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGen_VarList, m_pwndVarList);

    // Set up the columns on the variables list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Num");
    colCols.objAdd(L"Variable Value");
    m_pwndVarList->SetColumns(colCols);

    //
    //  Go ahead and pre-load it. When we need to show our info all we need to do is
    //  set the value column. Set the index as the row id, for auto-IPE purposes.
    //
    colCols[1].Clear();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRoomVars; c4Index++)
    {
        colCols[0].SetFormatted(c4Index + 1);
        m_pwndVarList->c4AddItem(colCols, c4Index);
    }
    m_pwndVarList->AutoSizeCol(0, kCIDLib::True);
    m_pwndVarList->AutoSizeCol(1, kCIDLib::True);

    // Set event handlers where needed
    m_pwndAllowRmSel->pnothRegisterHandler(this, &TCQCESCTGen::eClickHandler);
    m_pwndEnabled->pnothRegisterHandler(this, &TCQCESCTGen::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTGen: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCESCTGen::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGen_AllowRmSel)
        scriRoom().bAllowRmSel(m_pwndAllowRmSel->bCheckedState());

    else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGen_Enabled)
        scriRoom().bEnabled(m_pwndEnabled->bCheckedState());

    return tCIDCtrls::EEvResponses::Handled;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTGActs
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTGActs: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTGActs::TCQCESCTGActs(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Global Acts")
    , m_pwndActList(nullptr)
    , m_pwndDelParm(nullptr)
    , m_pwndDownParm(nullptr)
    , m_pwndParmList(nullptr)
    , m_pwndPath(nullptr)
    , m_pwndSelPath(nullptr)
    , m_pwndUpParm(nullptr)
{
}

TCQCESCTGActs::~TCQCESCTGActs()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTGActs: Public, inherited methods
// ---------------------------------------------------------------------------

// All we care about is if it's column 1. Column 0 is the parameter number
tCIDLib::TBoolean
TCQCESCTGActs::bIPETestCanEdit( const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&
                                ,       tCIDLib::TBoolean&)
{
    return (c4ColInd == 1);
}


//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTGActs::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndActList->SetEnable(kCIDLib::False);
    m_pwndDelParm->SetEnable(kCIDLib::False);
    m_pwndDownParm->SetEnable(kCIDLib::False);
    m_pwndParmList->SetEnable(kCIDLib::False);
    m_pwndPath->SetEnable(kCIDLib::False);
    m_pwndSelPath->SetEnable(kCIDLib::False);
    m_pwndUpParm->SetEnable(kCIDLib::False);

    m_pwndActList->RemoveAll();
    m_pwndParmList->ClearText();
    m_pwndPath->ClearText();
}


// We just need to store away the changed value
tCIDLib::TVoid
TCQCESCTGActs::IPEValChanged(const  tCIDCtrls::TWndId
                            , const TString&
                            , const TAttrData&          adatNew
                            , const TAttrData&
                            , const tCIDLib::TCard8     )
{
    // Get the current action
    const tCIDLib::TCard4 c4Index = m_pwndActList->c4CurItem();
    if (c4Index != kCIDLib::c4MaxCard)
    {
        TCQCSysCfgRmInfo& scriRm = scriRoom();
        const tCQCSysCfg::EGlobActs eCur = tCQCSysCfg::EGlobActs(c4Index);

        TSCGlobActInfo& scliEdit = scriRm.scliGlobActAt(eCur);
        scliEdit.m_colParms[c4IPERow()] = adatNew.strValue();
    }
}


tCIDLib::TVoid TCQCESCTGActs::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTGActs::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    // Only enable the action list. The others are controlled by the selected action
    m_pwndActList->SetEnable(kCIDLib::True);

    // Load up the global actions list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    m_pwndActList->RemoveAll();
    tCQCSysCfg::EGlobActs eCur = tCQCSysCfg::EGlobActs::Min;
    while (eCur <= tCQCSysCfg::EGlobActs::Max)
    {
        const TSCGlobActInfo& scliCur = scriRm.scliGlobActAt(eCur);
        colCols[1] = tCQCSysCfg::strXlatEGlobActs(eCur);
        const tCIDLib::TCard4 c4At = m_pwndActList->c4AddItem
        (
            colCols, tCIDLib::c4EnumOrd(eCur)
        );
        m_pwndActList->SetCheckAt(c4At, scliCur.m_bEnabled);
        eCur++;
    }
    m_pwndActList->SelectByIndex(0, kCIDLib::True);
}


tCIDLib::TVoid
TCQCESCTGActs::Validate(TCQCSysCfgRmInfo&           scriRoom
                        , TValErrInfo::TErrList&    colErrs)
{
    // Go through the actions. For any that are enabled, make sure there's a path set.
    tCQCSysCfg::EGlobActs eCur = tCQCSysCfg::EGlobActs::Min;
    TString strFmt;
    while (eCur <= tCQCSysCfg::EGlobActs::Max)
    {
        const TSCGlobActInfo& scliCur = scriRoom.scliGlobActAt(eCur);
        if (scliCur.m_bEnabled)
        {
            if (scliCur.m_strPath.bIsEmpty())
            {
                AddError
                (
                    scriRoom
                    , colErrs
                    , kCQCAErrs::errcSysCfg_NoActPath
                    , tCQCSysCfg::strXlatEGlobActs(eCur)
                    , m_pwndActList->widThis()
                );
            }
        }
        eCur++;
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTGActs: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTGActs::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgGAct, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_ActList, m_pwndActList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_DelParm, m_pwndDelParm);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_DownParm, m_pwndDownParm);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_ParmList, m_pwndParmList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_Path, m_pwndPath);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_SelPath, m_pwndSelPath);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgGAct_UpParm, m_pwndUpParm);

    // Set the action list columns, and make the second column the search column
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(L"Defined Actions");
    m_pwndActList->SetColumns(colCols, 1);

    // Set up the the parameters list columns
    colCols[0] = L" # ";
    colCols[1] = L"Action Parameter";
    m_pwndParmList->SetColumns(colCols);

    // Preload the list with all the possible parameter slots, without values for now
    colCols[1].Clear();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxGActParms; c4Index++)
    {
        colCols[0].SetFormatted(c4Index + 1);
        m_pwndParmList->c4AddItem(colCols, c4Index);
    }
    m_pwndParmList->AutoSizeCol(0, kCIDLib::True);
    m_pwndParmList->SelectByIndex(0);

    // Set us a the auto-IPE object on the parameter list
    m_pwndParmList->SetAutoIPE(this);


    // Set up handlers
    m_pwndActList->pnothRegisterHandler(this, &TCQCESCTGActs::eLBHandler);
    m_pwndDelParm->pnothRegisterHandler(this,&TCQCESCTGActs::eClickHandler);
    m_pwndDownParm->pnothRegisterHandler(this, &TCQCESCTGActs::eClickHandler);
    m_pwndParmList->pnothRegisterHandler(this, &TCQCESCTGActs::eLBHandler);
    m_pwndSelPath->pnothRegisterHandler(this, &TCQCESCTGActs::eClickHandler);
    m_pwndUpParm->pnothRegisterHandler(this, &TCQCESCTGActs::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTGActs: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle button clicke vents
tCIDCtrls::EEvResponses TCQCESCTGActs::eClickHandler(TButtClickInfo& wnotEvent)
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();
    const tCQCSysCfg::EGlobActs eAct = tCQCSysCfg::EGlobActs(m_pwndActList->c4CurItem());
    TSCGlobActInfo& scliEdit = scriRm.scliGlobActAt(eAct);

    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGAct_DelParm)
    {
        // Clear out the selected parameter
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        if (c4ParmInd != kCIDLib::c4MaxCard)
        {
            scliEdit.m_colParms[c4ParmInd] = TString::strEmpty();
            m_pwndParmList->SetColText(c4ParmInd, 1, TString::strEmpty());
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGAct_DownParm)
    {
        //
        //  Move the selected parameter down if not already at the end. We can't
        //  just swap the list box items, because we put numerical prefixes before
        //  them. So just swap the parms and then reload.
        //
        TSCGlobActInfo scliSel = scriRm.scliGlobActAt(eAct);
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        if (c4ParmInd + 1 < kCQCSysCfg::c4MaxGActParms)
        {
            scliSel.m_colParms.ExchangeElems(c4ParmInd, c4ParmInd + 1);
            LoadParms(scliSel, c4ParmInd + 1);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGAct_SelPath)
    {
        TString strSelected = scliEdit.m_strPath;
        tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a Global Action"
            , tCQCRemBrws::EDTypes::GlobalAct
            , strSelected
            , facCQCAdmin.cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSelected
        );

        if (bRes)
        {
            scliEdit.m_strPath = strSelected;
            m_pwndPath->strWndText(strSelected);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGAct_UpParm)
    {
        //
        //  Move the selected parameter up if not already at the start . We can't
        //  just swap the list box items, because we put numerical prefixes before
        //  them. So just swap the parms and then reload.
        //
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        if (c4ParmInd && (c4ParmInd != kCIDLib::c4MaxCard))
        {
            scliEdit.m_colParms.ExchangeElems(c4ParmInd - 1, c4ParmInd);
            LoadParms(scliEdit, c4ParmInd - 1);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle list events
tCIDCtrls::EEvResponses TCQCESCTGActs::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGAct_ActList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            ShowActInfo(tCQCSysCfg::EGlobActs(wnotEvent.c4Index()));
        }
        else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            // Toggle the current action
            TCQCSysCfgRmInfo& scriRm = scriRoom();
            const tCQCSysCfg::EGlobActs eAct = tCQCSysCfg::EGlobActs(wnotEvent.c4Index());

            TSCGlobActInfo& scliEdit = scriRm.scliGlobActAt(eAct);
            scliEdit.m_bEnabled = !scliEdit.m_bEnabled;

            // Update the check mark on the list
            m_pwndActList->SetCheckAt(tCIDLib::c4EnumOrd(eAct), scliEdit.m_bEnabled);

            // If disabling, clear the data out
            if (!scliEdit.m_bEnabled)
                scliEdit.Reset();

            // For a reselect to update the displayed info
            m_pwndActList->SelectByIndex(tCIDLib::c4EnumOrd(eAct), kCIDLib::True);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Load up the parameter list, with numeric prefixes before the values
tCIDLib::TVoid
TCQCESCTGActs::LoadParms(const  TSCGlobActInfo& scliCur
                        , const tCIDLib::TCard4 c4SelInd)
{
    // If no specific selection, take the first one
    tCIDLib::TCard4 c4InitSel = c4SelInd;
    if (c4InitSel == kCIDLib::c4MaxCard)
        c4InitSel = 0;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxGActParms; c4Index++)
        m_pwndParmList->SetColText(c4Index, 1, scliCur.m_colParms[c4Index]);
    m_pwndParmList->SelectByIndex(c4InitSel);
}


//
//  Update the action config display controls with the indicated action's information.
//
tCIDLib::TVoid TCQCESCTGActs::ShowActInfo(const tCQCSysCfg::EGlobActs eAct)
{
    const TCQCSysCfgRmInfo& scriRm = scriRoom();
    const TSCGlobActInfo& scliCur = scriRm.scliGlobActAt(eAct);

    m_pwndPath->strWndText(scliCur.m_strPath);
    LoadParms(scliCur, 0);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTHVAC
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTHVAC: Constructor and Destructor
// ---------------------------------------------------------------------------

TCQCESCTHVAC::TCQCESCTHVAC(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"HVAC")
    , m_pwndClearAll(nullptr)
    , m_pwndDefHVAC(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSelAll(nullptr)
    , m_pwndSetDef(nullptr)
{
}

TCQCESCTHVAC::~TCQCESCTHVAC()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTHVAC: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTHVAC::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndClearAll->SetEnable(kCIDLib::False);
    m_pwndSelAll->SetEnable(kCIDLib::False);
    m_pwndSetDef->SetEnable(kCIDLib::False);

    m_pwndDefHVAC->ClearText();
    m_pwndList->SetEnable(kCIDLib::False);
    m_pwndList->RemoveAll();
}


tCIDLib::TVoid TCQCESCTHVAC::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTHVAC::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    m_pwndClearAll->SetEnable(kCIDLib::True);
    m_pwndList->SetEnable(kCIDLib::True);
    m_pwndSelAll->SetEnable(kCIDLib::True);
    m_pwndSetDef->SetEnable(kCIDLib::True);

    LoadList(scriRm);

    // Display the default load if one is set
    const TString& strDefHVAC = scriRoom().strFuncId(tCQCSysCfg::ERmFuncs::HVAC);
    if (!strDefHVAC.bIsEmpty())
    {
        tCIDLib::TCard4 c4Index;
        const TSCHVACInfo& scliDef = scfgEdit().scliHVACByUID(strDefHVAC, c4Index);
        m_pwndDefHVAC->strWndText(scliDef.m_strTitle);
    }
     else
    {
        m_pwndDefHVAC->ClearText();
    }
}


tCIDLib::TVoid
TCQCESCTHVAC::Validate( TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
    //
    //  If HVACs are defined but no default, then take the 0ths one. It's stored as
    //  the function id, so we can just an is enabled check.
    //
    if (scriRoom.c4HVACCount() && !scriRoom.bFuncEnabled(tCQCSysCfg::ERmFuncs::HVAC))
    {
        scriRoom.SetFuncId(tCQCSysCfg::ERmFuncs::HVAC, scriRoom.strHVACUIDAt(0));
        AddError(scriRoom, colErrs, kCQCAErrs::errcSysCfg_NoDefHVAC, m_pwndList->widThis());
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTHVAC: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTHVAC::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgHVAC, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgHVAC_ClearAll, m_pwndClearAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgHVAC_DefHVAC, m_pwndDefHVAC);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgHVAC_HVACList, m_pwndList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgHVAC_SelAll, m_pwndSelAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgHVAC_SetDef, m_pwndSetDef);

    // Set up the list columns. We aren't showing them, but need to set them
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"  ");
    colCols.objAdd(L"HVAC");
    m_pwndList->SetColumns(colCols, 1);

    // Set up handlers
    m_pwndClearAll->pnothRegisterHandler(this, &TCQCESCTHVAC::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCESCTHVAC::eLBHandler);
    m_pwndSelAll->pnothRegisterHandler(this, &TCQCESCTHVAC::eClickHandler);
    m_pwndSetDef->pnothRegisterHandler(this, &TCQCESCTHVAC::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTHVAC: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TCQCESCTHVAC::eClickHandler(TButtClickInfo& wnotEvent)
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgHVAC_ClearAll)
    {
        // Remove all loads from our room and clear all the marks
        tCIDLib::TStrList colEmpty;
        scriRm.SetHVACs(colEmpty);

        const tCIDLib::TCard4 c4ListCnt = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
            m_pwndList->SetCheckAt(c4Index, kCIDLib::False);

        // We also have to clear the default load
        scriRm.SetFuncId(tCQCSysCfg::ERmFuncs::HVAC, TString::strEmpty());
        m_pwndDefHVAC->ClearText();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgHVAC_SelAll)
    {
        // Mark all of the loads and set them into a list as we go
        tCIDLib::TStrList colSet;
        const tCIDLib::TCard4 c4ListCnt = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
        {
            m_pwndList->SetCheckAt(c4Index, kCIDLib::True);
            colSet.objAdd(m_pwndList->strUserDataAt(c4Index));
        }

        // Set them on the room
        scriRm.SetHVACs(colSet);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgHVAC_SetDef)
    {
        //
        //  If there's a selection, then take it. Set store the UID of the HVAC as
        //  the function id for lighting on the room.
        //
        const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            scriRm.SetFuncId(tCQCSysCfg::ERmFuncs::HVAC, m_pwndList->strUserDataAt(c4ListInd));

            tCIDLib::TCard4 c4Index;
            const TSCHVACInfo& scliDef = scfgEdit().scliHVACByUID
            (
                m_pwndList->strUserDataAt(c4ListInd), c4Index
            );
            m_pwndDefHVAC->strWndText(scliDef.m_strTitle);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCESCTHVAC::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgHVAC_HVACList)
    {
        // If they double click, we toggle the mark
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            TCQCSysCfgRmInfo& scriRm = scriRoom();
            const tCIDLib::TBoolean bCur(m_pwndList->bIsCheckedAt(wnotEvent.c4Index()));
            const tCQCSysCfg::ECfgLists eList = tCQCSysCfg::ECfgLists::HVAC;
            if (bCur)
                scriRm.RemoveFromList(eList, m_pwndList->strUserDataAt(wnotEvent.c4Index()));
            else
                scriRm.AddHVAC(m_pwndList->strUserDataAt(wnotEvent.c4Index()));

            m_pwndList->SetCheckAt(wnotEvent.c4Index(), !bCur);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We have to load up the list box with the available thermos, marking the ones
//  that are currently in our list.
//
tCIDLib::TVoid TCQCESCTHVAC::LoadList(TCQCSysCfgRmInfo& scriRm)
{
    //
    //  Create a hash set of the currently selected loads, so that we can
    //  quickly see if any given one is selected. We are storing the UIDs
    //  here.
    //
    tCIDLib::TCard4 c4Count = scriRm.c4HVACCount();
    tCIDLib::TStrHashSet colUIDs(109, TStringKeyOps());
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colUIDs.objAdd(scriRm.strHVACUIDAt(c4Index));

    //
    //  Now let's load up all of the configured loads, checking the ones
    //  that are currently in our list.
    //
    TCQCSysCfg& scfgLoad = scfgEdit();

    tCIDLib::TStrList colList(2);
    colList.objAdd(TString::strEmpty());
    colList.objAdd(TString::strEmpty());

    m_pwndList->RemoveAll();
    c4Count = scfgLoad.c4ListIdCount(tCQCSysCfg::ECfgLists::HVAC);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCHVACInfo& scliCur = scfgLoad.scliHVACAt(c4Index);
        colList[1] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colList, 0);

        // Store the UID for later mapping back
        m_pwndList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
        m_pwndList->SetFlagAt(c4At, colUIDs.bHasElement(scliCur.m_strUniqueId));
    }

    m_pwndList->SetEnable(kCIDLib::True);
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTLoads
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTLoads: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTLoads::TCQCESCTLoads(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Lighting")
    , m_pwndClearAll(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSelAll(nullptr)
    , m_pwndSetDef(nullptr)
{
}

TCQCESCTLoads::~TCQCESCTLoads()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTLoads: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTLoads::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndClearAll->SetEnable(kCIDLib::False);
    m_pwndSelAll->SetEnable(kCIDLib::False);
    m_pwndSetDef->SetEnable(kCIDLib::False);

    m_pwndDefLoad->ClearText();
    m_pwndList->SetEnable(kCIDLib::False);
    m_pwndList->RemoveAll();
}


tCIDLib::TVoid TCQCESCTLoads::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTLoads::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    m_pwndClearAll->SetEnable(kCIDLib::True);
    m_pwndList->SetEnable(kCIDLib::True);
    m_pwndSelAll->SetEnable(kCIDLib::True);
    m_pwndSetDef->SetEnable(kCIDLib::True);

    LoadList(scriRm);

    // Display the default load if one is set
    const TString& strDefLoad = scriRoom().strFuncId(tCQCSysCfg::ERmFuncs::Lighting);
    if (!strDefLoad.bIsEmpty())
    {
        tCIDLib::TCard4 c4Index;
        const TSCLoadInfo& scliDef = scfgEdit().scliLoadByUID(strDefLoad, c4Index);
        m_pwndDefLoad->strWndText(scliDef.m_strTitle);
    }
     else
    {
        m_pwndDefLoad->ClearText();
    }
}


tCIDLib::TVoid
TCQCESCTLoads::Validate(TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
    //
    //  If loads are defined but no default, then take the 0th one. It's stored as
    //  the function id, so we can just an is enabled check.
    //
    if (scriRoom.c4LoadCount() && !scriRoom.bFuncEnabled(tCQCSysCfg::ERmFuncs::Lighting))
    {
        scriRoom.SetFuncId(tCQCSysCfg::ERmFuncs::Lighting, scriRoom.strLoadUIDAt(0));
        AddError
        (
            scriRoom, colErrs, kCQCAErrs::errcSysCfg_NoDefLoad, m_pwndList->widThis()
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTLoads: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTLoads::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgLoads, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgLoads_ClearAll, m_pwndClearAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgLoads_DefLoad, m_pwndDefLoad);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgLoads_LoadList, m_pwndList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgLoads_SelAll, m_pwndSelAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgLoads_SetDef, m_pwndSetDef);

    // Set up the list columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(L"Defined Lights");
    m_pwndList->SetColumns(colCols, 1);

    // Set up handlers
    m_pwndClearAll->pnothRegisterHandler(this, &TCQCESCTLoads::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCESCTLoads::eLBHandler);
    m_pwndSelAll->pnothRegisterHandler(this, &TCQCESCTLoads::eClickHandler);
    m_pwndSetDef->pnothRegisterHandler(this, &TCQCESCTLoads::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTLoads: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TCQCESCTLoads::eClickHandler(TButtClickInfo& wnotEvent)
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgLoads_ClearAll)
    {
        // Remove all loads from our room and clear all the marks
        tCIDLib::TStrList colEmpty;
        scriRm.SetLoads(colEmpty);

        const tCIDLib::TCard4 c4ListCnt = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
            m_pwndList->SetCheckAt(c4Index, kCIDLib::False);

        // We also have to clear the default load
        scriRm.SetFuncId(tCQCSysCfg::ERmFuncs::Lighting, TString::strEmpty());
        m_pwndDefLoad->ClearText();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgLoads_SelAll)
    {
        // Mark all of the loads and set them into a list as we go
        tCIDLib::TStrList colSet;
        const tCIDLib::TCard4 c4ListCnt = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListCnt; c4Index++)
        {
            m_pwndList->SetCheckAt(c4Index, kCIDLib::True);
            colSet.objAdd(m_pwndList->strUserDataAt(c4Index));
        }
        scriRm.SetLoads(colSet);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgLoads_SetDef)
    {
        //
        //  If there's a selection, then take it. Set store the UID of the load as
        //  the function id for lighting on the room.
        //
        const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            const TString& strCurId = m_pwndList->strUserDataAt(c4ListInd);
            scriRm.SetFuncId(tCQCSysCfg::ERmFuncs::Lighting, strCurId);

            tCIDLib::TCard4 c4Index;
            const TSCLoadInfo& scliDef = scfgEdit().scliLoadByUID(strCurId, c4Index);
            m_pwndDefLoad->strWndText(scliDef.m_strTitle);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCESCTLoads::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgLoads_LoadList)
    {
        // If they double click, we toggle the mark
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            const tCQCSysCfg::ECfgLists eList = tCQCSysCfg::ECfgLists::Lighting;
            TCQCSysCfgRmInfo& scriRm = scriRoom();
            if (m_pwndList->bIsCheckedAt(wnotEvent.c4Index()))
            {
                // It's currently selected, so unselect it
                m_pwndList->SetCheckAt(wnotEvent.c4Index(), kCIDLib::False);
                scriRm.RemoveFromList(eList, m_pwndList->strUserDataAt(wnotEvent.c4Index()));
            }
             else
            {
                // It's currently not selected, so select it
                m_pwndList->SetCheckAt(wnotEvent.c4Index(), kCIDLib::True);
                scriRm.AddLoad(m_pwndList->strUserDataAt(wnotEvent.c4Index()));
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We have to load up the list box with the available loads, marking the ones
//  that are currently in our list.
//
tCIDLib::TVoid TCQCESCTLoads::LoadList(TCQCSysCfgRmInfo& scriRm)
{
    //
    //  Create a hash set of the currently defined loads, so that we can quickly see if
    //  any given one is selected. We are storing the UIDs here.
    //
    tCIDLib::TCard4 c4Count = scriRm.c4LoadCount();
    tCIDLib::TStrHashSet colUIDs(109, TStringKeyOps());
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colUIDs.objAdd(scriRm.strLoadUIDAt(c4Index));

    //
    //  Now let's load up all of the configured loads, checking the ones
    //  that are currently in our list.
    //
    TCQCSysCfg& scfgLoad = scfgEdit();

    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    m_pwndList->RemoveAll();
    c4Count = scfgLoad.c4ListIdCount(tCQCSysCfg::ECfgLists::Lighting);
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCLoadInfo& scliCur = scfgLoad.scliLoadAt(c4Index);
            colCols[1] = scliCur.m_strTitle;
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);
            m_pwndList->SetUserDataAt(c4At, scliCur.m_strUniqueId);

            if (colUIDs.bHasElement(scliCur.m_strUniqueId))
                m_pwndList->SetFlagAt(c4At, kCIDLib::True);
        }
        m_pwndList->SelectByIndex(0, kCIDLib::True);
    }
    m_pwndList->SetEnable(kCIDLib::True);

}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTMedia
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTMedia: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTMedia::TCQCESCTMedia(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Media")
    , m_pwndMovie(nullptr)
    , m_pwndMovieDisable(nullptr)
    , m_pwndMovieSel(nullptr)
    , m_pwndMusic(nullptr)
    , m_pwndMusicDisable(nullptr)
    , m_pwndMusicSel(nullptr)
{
}

TCQCESCTMedia::~TCQCESCTMedia()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTMedia: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTMedia::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndMovie->SetEnable(kCIDLib::False);
    m_pwndMovieSel->SetEnable(kCIDLib::False);
    m_pwndMovieDisable->SetEnable(kCIDLib::False);

    m_pwndMusic->SetEnable(kCIDLib::False);
    m_pwndMusicSel->SetEnable(kCIDLib::False);
    m_pwndMusicDisable->SetEnable(kCIDLib::False);
}


tCIDLib::TVoid TCQCESCTMedia::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTMedia::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    m_pwndMovie->SetEnable(kCIDLib::True);
    m_pwndMovieSel->SetEnable(kCIDLib::True);
    m_pwndMovieDisable->SetEnable(kCIDLib::True);

    m_pwndMusic->SetEnable(kCIDLib::True);
    m_pwndMusicSel->SetEnable(kCIDLib::True);
    m_pwndMusicDisable->SetEnable(kCIDLib::True);

    m_pwndMovie->strWndText(strFuncTitle(tCQCSysCfg::ERmFuncs::Movies));
    m_pwndMusic->strWndText(strFuncTitle(tCQCSysCfg::ERmFuncs::Music));
}


tCIDLib::TVoid
TCQCESCTMedia::Validate(TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
    // Nothing for us to check
}


// ---------------------------------------------------------------------------
//  TCQCESCTMedia: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCESCTMedia::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgMedia, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_Movie, m_pwndMovie);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_MovieDisable, m_pwndMovieDisable);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_MovieSel, m_pwndMovieSel);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_Music, m_pwndMusic);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_MusicDisable, m_pwndMusicDisable);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgMedia_MusicSel, m_pwndMusicSel);

    // Set up handlers
    m_pwndMovieDisable->pnothRegisterHandler(this, &TCQCESCTMedia::eClickHandler);
    m_pwndMovieSel->pnothRegisterHandler(this, &TCQCESCTMedia::eClickHandler);
    m_pwndMusicDisable->pnothRegisterHandler(this, &TCQCESCTMedia::eClickHandler);
    m_pwndMusicSel->pnothRegisterHandler(this, &TCQCESCTMedia::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTMedia: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCESCTMedia::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgMedia_MovieDisable)
    {
        scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Movies, TString::strEmpty());
        m_pwndMovie->ClearText();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgMedia_MusicDisable)
    {
        scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Music, TString::strEmpty());
        m_pwndMusic->ClearText();
    }
     else if ((wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgMedia_MovieSel)
          ||  (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgMedia_MusicSel))
    {
        // These are basically the same except the room function and target control
        tCQCSysCfg::ERmFuncs eFunc;
        TEntryField* pwndTar;
        if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgMedia_MovieSel)
        {
            eFunc = tCQCSysCfg::ERmFuncs::Movies;
            pwndTar = m_pwndMovie;
        }
         else
        {
            eFunc = tCQCSysCfg::ERmFuncs::Music;
            pwndTar = m_pwndMusic;
        }

        // Build up a list of configured media objects
        tCIDLib::TKVPList colList;
        const tCIDLib::TCard4 c4Count = scfgEdit().c4QueryCfgList
        (
            tCQCSysCfg::ECfgLists::Media, colList
        );

        if (c4Count)
        {
            TKeyValuePair kvalSel;
            const tCIDLib::TBoolean bRes = facCIDWUtils().bKVPListSelect
            (
                *this, L"Select a Media Configuration", colList, kvalSel, kCIDLib::False
            );

            if (bRes)
            {
                pwndTar->strWndText(kvalSel.strKey());
                scriRoom().SetFuncId(eFunc, kvalSel.strValue());
            }
        }
         else
        {
            TOkBox msgbErr(L"No media devices have been configured");
            msgbErr.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTRmModes
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTRmModes: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTRmModes::TCQCESCTRmModes(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Modes")
    , m_pwndAddMode(nullptr)
    , m_pwndModeList(nullptr)
    , m_pwndDelMode(nullptr)
    , m_pwndDelParm(nullptr)
    , m_pwndDownParm(nullptr)
    , m_pwndParmList(nullptr)
    , m_pwndPath(nullptr)
    , m_pwndSelPath(nullptr)
    , m_pwndUpParm(nullptr)
{
}

TCQCESCTRmModes::~TCQCESCTRmModes()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTRmModes: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the parameters, then we only allow column 1, 0 is the order number. For the
//  mode list, there is only a column 0
//
tCIDLib::TBoolean
TCQCESCTRmModes::bIPETestCanEdit( const   tCIDCtrls::TWndId widSrc
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatEdit
                                ,       tCIDLib::TBoolean&)
{
    // There has to be a selected room mode entry either way
    if (m_pwndModeList->c4CurItem() == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    if (widSrc == kCQCAdmin::ridTab_SysCfgRmMode_ParmList)
        return (c4ColInd == 1);

    return (c4ColInd == 0);
}


//
//  If's the mode list, make sure that it's not a dup of an existing one. We can find.
//  the index of the original, since the per-row user data is the unique id.
//
tCIDLib::TBoolean
TCQCESCTRmModes::bIPEValidate(  const   TString&            strSrc
                                ,       TAttrData&          adatTar
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    ) const
{
    // If it's the mode list, let's check it
    if (strSrc == L"ModeList")
    {
        const TCQCSysCfgRmInfo& scriRm = scriRoom();
        TString strCur;
        const tCIDLib::TCard4 c4OrgInd = scriRm.c4RoomModeById
        (
            m_pwndModeList->strUserDataAt(c4IPERow())
        );

        const tCIDLib::TCard4 c4Count = scriRm.c4RoomModeCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // Skip the original
            if (c4Index == c4OrgInd)
                continue;

            const TSCGlobActInfo& scrliCur = scriRm.scliRoomModeAt(c4Index);

            if (strCur.bCompareI(scrliCur.m_strTitle))
            {
                strErrMsg = L"That room mode name is already in use";
                return kCIDLib::False;
            }
        }
    }

    // It's OK so store it
    adatTar.SetString(strNewVal);

    return kCIDLib::True;
}


//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTRmModes::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndDelMode->SetEnable(kCIDLib::False);
    m_pwndDelParm->SetEnable(kCIDLib::False);
    m_pwndDownParm->SetEnable(kCIDLib::False);
    m_pwndParmList->SetEnable(kCIDLib::False);
    m_pwndPath->SetEnable(kCIDLib::False);
    m_pwndSelPath->SetEnable(kCIDLib::False);
    m_pwndUpParm->SetEnable(kCIDLib::False);

    m_pwndModeList->RemoveAll();
    m_pwndPath->ClearText();

    // Clear the parameter values
    for(tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRmModeParms; c4Index++)
        m_pwndParmList->SetColText(c4Index, 1, TString::strEmpty());
}


// We just need to store away the changed value
tCIDLib::TVoid
TCQCESCTRmModes::IPEValChanged(const  tCIDCtrls::TWndId widSrc
                            , const TString&
                            , const TAttrData&          adatNew
                            , const TAttrData&
                            , const tCIDLib::TCard8     )
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();
    if (widSrc == kCQCAdmin::ridTab_SysCfgRmMode_ModeList)
    {
        //
        //  The user data of the list window row is the unique id of this mode entry,
        //  so use that to find the original list index of it.
        //
        const tCIDLib::TCard4 c4Index = scriRm.c4RoomModeById
        (
            m_pwndModeList->strUserDataAt(c4IPERow())
        );

        if (c4Index != kCIDLib::c4MaxCard)
        {
            TSCGlobActInfo& scliEdit = scriRm.scliRoomModeAt(c4Index);
            scliEdit.m_strTitle = adatNew.strValue();
        }
    }
     else
    {
        // Get the selected mode. If there is one, then look up the inf for this mode
        const tCIDLib::TCard4 c4ListInd = m_pwndModeList->c4CurItem();
        const tCIDLib::TCard4 c4ModeIndex = scriRm.c4RoomModeById
        (
            m_pwndModeList->strUserDataAt(c4ListInd)
        );

        // If we found it, then let's update the parameter
        if (c4ModeIndex != kCIDLib::c4MaxCard)
        {
            TSCGlobActInfo& scliEdit = scriRm.scliRoomModeAt(c4ModeIndex);
            scliEdit.m_colParms[c4IPERow()] = adatNew.strValue();
        }
    }
}


tCIDLib::TVoid TCQCESCTRmModes::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTRmModes::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    // Only enable the action list. The others are controlled by the selected action
    m_pwndModeList->SetEnable(kCIDLib::True);

    // Load up the global actions list
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    m_pwndModeList->RemoveAll();

    const tCIDLib::TCard4 c4Count = scriRm.c4RoomModeCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCGlobActInfo& scliCur = scriRm.scliRoomModeAt(c4Index);
            colCols[0] = scliCur.m_strTitle;
            const tCIDLib::TCard4 c4At = m_pwndModeList->c4AddItem(colCols, 0);
            m_pwndModeList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
        }

        m_pwndModeList->SelectByIndex(0, kCIDLib::True);
    }
}


tCIDLib::TVoid
TCQCESCTRmModes::Validate(TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
    const tCIDLib::TCard4 c4Count = scriRoom.c4RoomModeCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCGlobActInfo& scliCur = scriRoom.scliRoomModeAt(c4Index);
        if (scliCur.m_strPath.bIsEmpty())
        {
            AddError
            (
                scriRoom
                , colErrs
                , kCQCAErrs::errcSysCfg_NoActPath
                , scliCur.m_strTitle
                , m_pwndModeList->widThis()
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTRmModes: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTRmModes::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgRmMode, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_AddMode, m_pwndAddMode);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_ModeList, m_pwndModeList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_DelParm, m_pwndDelParm);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_DelMode, m_pwndDelMode);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_DownParm, m_pwndDownParm);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_ParmList, m_pwndParmList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_Path, m_pwndPath);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_SelPath, m_pwndSelPath);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRmMode_UpParm, m_pwndUpParm);

    // Set names on the two list windows, for identification in IPE callbacks
    m_pwndModeList->strName(L"ModeList");
    m_pwndParmList->strName(L"ParmList");

    // We don't have any column title on the modes list, but we have to set a column
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    m_pwndModeList->SetColumns(colCols);

    // We have two on the params
    colCols[0] = L" # ";
    colCols.objAdd(L"Action Parameter");
    m_pwndParmList->SetColumns(colCols);

    // Preload the list with all the possible parameter slots, without values for now
    colCols[1].Clear();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRmModeParms; c4Index++)
    {
        colCols[0].SetFormatted(c4Index + 1);
        m_pwndParmList->c4AddItem(colCols, c4Index);
    }
    m_pwndParmList->AutoSizeCol(0, kCIDLib::True);
    m_pwndParmList->SelectByIndex(0);

    // Set us a the auto-IPE object on the two list windows
    m_pwndParmList->SetAutoIPE(this);
    m_pwndModeList->SetAutoIPE(this);

    // Set up handlers
    m_pwndAddMode->pnothRegisterHandler(this, &TCQCESCTRmModes::eClickHandler);
    m_pwndDelMode->pnothRegisterHandler(this, &TCQCESCTRmModes::eClickHandler);
    m_pwndModeList->pnothRegisterHandler(this, &TCQCESCTRmModes::eLBHandler);
    m_pwndDownParm->pnothRegisterHandler(this, &TCQCESCTRmModes::eClickHandler);
    m_pwndParmList->pnothRegisterHandler(this, &TCQCESCTRmModes::eLBHandler);
    m_pwndSelPath->pnothRegisterHandler(this, &TCQCESCTRmModes::eClickHandler);
    m_pwndUpParm->pnothRegisterHandler(this, &TCQCESCTRmModes::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTRmModes: Private, non-virtual methods
// ---------------------------------------------------------------------------

// When the list is cleared, we need to make sure all current mode info is cleared out
tCIDLib::TVoid TCQCESCTRmModes::ClearModeInfo()
{
    m_pwndPath->ClearText();

    // Empty the content column of the parameters
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRmModeParms; c4Index++)
        m_pwndParmList->SetColText(c4Index, 1, TString::strEmpty());
}


// Handle button click events
tCIDCtrls::EEvResponses TCQCESCTRmModes::eClickHandler(TButtClickInfo& wnotEvent)
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();

    //
    //  Handle add new separately since this can be done even if there's nothing
    //  selected.
    //
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_AddMode)
    {
        // Just find a unique name and add it. They can edit it after the fact
        const tCIDLib::TCard4 c4Count = scriRm.c4RoomModeCount();

        // If any existing ones, check them to make sure we have a unique name
        TString strName(L"NewMode1");
        if (c4Count)
        {
            const tCIDLib::TCard4 c4OrgLen = strName.c4Length();

            tCIDLib::TCard4 c4NmIndex = 2;
            while (kCIDLib::True)
            {
                tCIDLib::TCard4 c4SrcInd = 0;
                while (c4SrcInd < c4Count)
                {
                    const TSCGlobActInfo& scliCur = scriRm.scliRoomModeAt(c4SrcInd);
                    if (scliCur.m_strTitle.bCompareI(strName))
                        break;

                    c4SrcInd++;
                }

                // If we made it to the end, it's good
                if (c4SrcInd == c4Count)
                    break;

                // Else try again
                strName.CapAt(c4OrgLen);
                strName.AppendFormatted(c4NmIndex++);
            }
        }

        //
        //  Create a new one and set the title. We don't use the enabled flag but
        //  make sure it's set.
        //
        TSCGlobActInfo scliNew;
        scliNew.m_strTitle = strName;
        scliNew.m_bEnabled = kCIDLib::True;

        // Let's add it to the room
        scriRm.bAddOrUpdateRoomMode(scliNew);

        // Let's add it to the list window as well
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(strName);
        const tCIDLib::TCard4 c4At = m_pwndModeList->c4AddItem(colCols, 0);
        m_pwndModeList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

        return tCIDCtrls::EEvResponses::Handled;
    }


    // In these cases, there has to be something selected. If not do nothing
    const tCIDLib::TCard4 c4ListInd = m_pwndModeList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return tCIDCtrls::EEvResponses::Handled;

    // We are ok, so get the index within the room data of this one
    const tCIDLib::TCard4 c4Index = scriRm.c4RoomModeById
    (
        m_pwndModeList->strUserDataAt(c4ListInd)
    );

    // And get a ref we can edit
    TSCGlobActInfo& scliEdit = scriRm.scliRoomModeAt(c4Index);

    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_DelMode)
    {
        // Make sure they really want to do it
        TYesNoBox msgbDel
        (
            facCQCAdmin.strMsg
            (
                kCQCAMsgs::midQ_Delete, TString(L"room mode"), scliEdit.m_strTitle
            )
        );

        if (msgbDel.bShowIt(*this))
        {
            // Remove it from the room and the list
            scriRm.RemoveRoomModeAt(c4Index);
            m_pwndModeList->RemoveAt(c4ListInd);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_DelParm)
    {
        // Clear out the selected parameter
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        if (c4ParmInd != kCIDLib::c4MaxCard)
        {
            scliEdit.m_colParms[c4ParmInd].Clear();
            m_pwndParmList->SetColText(c4ParmInd, 1, TString::strEmpty());
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_DownParm)
    {
        //
        //  Move the selected parameter down if not already at the end. We can't
        //  just swap the list box items, because we put numerical prefixes before
        //  them. So just swap the parms and then reload.
        //
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        scliEdit.m_colParms.ExchangeElems(c4ParmInd, c4ParmInd + 1);

        LoadParms(scliEdit, c4ParmInd + 1);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_SelPath)
    {
        TString strSelected = scliEdit.m_strPath;
        tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a Global Action"
            , tCQCRemBrws::EDTypes::GlobalAct
            , strSelected
            , facCQCAdmin.cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSelected
        );

        if (bRes)
        {
            scliEdit.m_strPath = strSelected;
            m_pwndPath->strWndText(strSelected);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_UpParm)
    {
        //
        //  Move the selected parameter up if not already at the start . We can't
        //  just swap the list box items, because we put numerical prefixes before
        //  them. So just swap the parms and then reload.
        //
        const tCIDLib::TCard4 c4ParmInd = m_pwndParmList->c4CurItem();
        if (c4ParmInd && (c4ParmInd != kCIDLib::c4MaxCard))
        {
            scliEdit.m_colParms.ExchangeElems(c4ParmInd - 1, c4ParmInd);
            LoadParms(scliEdit, c4ParmInd - 1);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle list events
tCIDCtrls::EEvResponses TCQCESCTRmModes::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRmMode_ModeList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            ShowActInfo(wnotEvent.c4Index());
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            // Clear out the selected item content
            ClearModeInfo();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Load up the parameter list, with numeric prefixes before the values
tCIDLib::TVoid
TCQCESCTRmModes::LoadParms( const   TSCGlobActInfo& scliCur
                            , const tCIDLib::TCard4 c4SelInd)
{
    // If no specific selection, take the first one
    tCIDLib::TCard4 c4InitSel = c4SelInd;
    if (c4InitSel == kCIDLib::c4MaxCard)
        c4InitSel = 0;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRmModeParms; c4Index++)
        m_pwndParmList->SetColText(c4Index, 1, scliCur.m_colParms[c4Index]);
    m_pwndParmList->SelectByIndex(c4InitSel);
}


//
//  Update the action config display controls with the indicated action's information.
//
tCIDLib::TVoid TCQCESCTRmModes::ShowActInfo(const tCIDLib::TCard4 c4ListIndex)
{
    // Enable the stuff that is dependent on there being something to display
    m_pwndDelMode->SetEnable(kCIDLib::True);
    m_pwndDelParm->SetEnable(kCIDLib::True);
    m_pwndDownParm->SetEnable(kCIDLib::True);
    m_pwndParmList->SetEnable(kCIDLib::True);
    m_pwndPath->SetEnable(kCIDLib::True);
    m_pwndSelPath->SetEnable(kCIDLib::True);
    m_pwndUpParm->SetEnable(kCIDLib::True);


    const TCQCSysCfgRmInfo& scriRm = scriRoom();
    const tCIDLib::TCard4 c4Index = scriRm.c4RoomModeById
    (
        m_pwndModeList->strUserDataAt(c4ListIndex)
    );
    const TSCGlobActInfo& scliCur = scriRm.scliRoomModeAt(c4Index);

    m_pwndPath->strWndText(scliCur.m_strPath);
    LoadParms(scliCur, 0);
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTSecure
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTSecure: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTSecure::TCQCESCTSecure(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Security")
    , m_pwndDisable(nullptr)
    , m_pwndSelArea(nullptr)
    , m_pwndSelZones(nullptr)
    , m_pwndSecArea(nullptr)
    , m_pwndZones(nullptr)
{
}

TCQCESCTSecure::~TCQCESCTSecure()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTSecure: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  To support in place editing of the spoken name column of the selected zones list.
//  Don't have to check the source window id since we only set up IPE on the zones list
//  box. We'll not set bValueSet and just let it get the text from the column as the
//  starting value.
//
tCIDLib::TBoolean
TCQCESCTSecure::bIPETestCanEdit(const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // The first column is the selected zone name, so can't be edited.
    if (!c4ColInd)
        return kCIDLib::False;

    adatFill.Set(L"SpokenName", L"SpokenName", tCIDMData::EAttrTypes::String);
    return kCIDLib::True;
}


//
//  For the in place editing of the spoken name, we need to make sure it is unique within
//  the zones of this room, and it's not the same as the title, in which case it should
//  not be set. We only allow one column of one window to be edited, so we don't have to
//  check the source attribute name.
//
//  The user id value will be the row id, but we don't use that because the list can be
//  modified. We just get the row index and use that to get the zone name, which lets
//  us find the original entry in the room data.
//
tCIDLib::TBoolean
TCQCESCTSecure::bIPEValidate(const  TString&
                            ,       TAttrData&          adatTar
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    ) const
{
    const TCQCSysCfgRmInfo& scriRm = scriRoom();

    // Get the zone name of the row we are editing
    TString strEditName;
    m_pwndZones->QueryColText(c4IPERow(), 0, strEditName);

    const tCIDLib::TCard4 c4Count = scriRm.c4SecZoneCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCurZoneName = scriRm.strZoneNameAt(c4Index);

        // It can't equal any zone name, even its own
        if (strCurZoneName.bCompareI(strNewVal))
        {
            strErrMsg = L"The spoken name cannot be the same as any zone name";
            return kCIDLib::False;
        }

        // If it's not the zone being edited, check it against the alt name as well
        if (!strEditName.bCompareI(strCurZoneName))
        {
            if (scriRm.strZoneAltNameAt(c4Index).bCompareI(strNewVal))
            {
                strErrMsg = L"That spoken name is already taken by another zone";
                return kCIDLib::False;
            }
        }
    }

    // It's good, so store it as is and return success
    adatTar.SetString(strNewVal);
    return kCIDLib::True;
}


// We need to store way the new zone spoken name
tCIDLib::TVoid
TCQCESCTSecure::IPEValChanged(  const   tCIDCtrls::TWndId
                                , const TString&
                                , const TAttrData&          adatNew
                                , const TAttrData&          adatOld
                                , const tCIDLib::TCard8     )
{
    const TCQCSysCfgRmInfo& scriRm = scriRoom();

    //
    //  We can't trust the row ids since the list can be edited, so just get the
    //  edited row's zone name and find it that way.
    //
    TString strEditName;
    m_pwndZones->QueryColText(c4IPERow(), 0, strEditName);

    const tCIDLib::TCard4 c4Count = scriRm.c4SecZoneCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (scriRm.strZoneNameAt(c4Index).bCompareI(strEditName))
        {
            scriRoom().SetZoneAltNameAt(c4Index, adatNew.strValue());
            break;
        }
    }
}



//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTSecure::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndSecArea->ClearText();
    m_pwndZones->RemoveAll();

    m_pwndDisable->SetEnable(kCIDLib::False);
    m_pwndSecArea->SetEnable(kCIDLib::False);
    m_pwndSelArea->SetEnable(kCIDLib::False);
    m_pwndSelZones->SetEnable(kCIDLib::False);
    m_pwndZones->SetEnable(kCIDLib::False);
}


tCIDLib::TVoid TCQCESCTSecure::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTSecure::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    m_pwndDisable->SetEnable(kCIDLib::True);
    m_pwndSelArea->SetEnable(kCIDLib::True);
    m_pwndSecArea->SetEnable(kCIDLib::True);
    m_pwndSecArea->strWndText(strFuncTitle(tCQCSysCfg::ERmFuncs::Security));

    //
    //  Enable the zone stuff and load them up if a security area has been selected.
    //  Else leave it disabled for now.
    //
    m_pwndZones->RemoveAll();
    if (bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
    {
        m_pwndSelZones->SetEnable(kCIDLib::True);
        m_pwndZones->SetEnable(kCIDLib::True);

        const tCIDLib::TCard4 c4ZoneCnt = scriRm.c4SecZoneCount();
        if (c4ZoneCnt)
        {
            tCIDLib::TStrList colCols(2);
            colCols.objAdd(TString::strEmpty());
            colCols.objAdd(TString::strEmpty());

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ZoneCnt; c4Index++)
            {
                colCols[0] = scriRm.strZoneNameAt(c4Index);
                colCols[1] = scriRm.strZoneAltNameAt(c4Index);
                m_pwndZones->c4AddItem(colCols, 0);
            }
            m_pwndZones->SelectByIndex(0);
        }
    }
     else
    {
        m_pwndSelZones->SetEnable(kCIDLib::False);
        m_pwndZones->SetEnable(kCIDLib::False);
        m_pwndZones->RemoveAll();
    }
}


//
//  Note that we MUST use the passed room, not the one for which the tab is currently
//  set.
//
tCIDLib::TVoid
TCQCESCTSecure::Validate(TCQCSysCfgRmInfo&          scriRoom
                        , TValErrInfo::TErrList&    colErrs)
{
    // Make sure that some zones are selected if security is enabled are enabled
    if (!scriRoom.c4SecZoneCount()
    &&  !scriRoom.strFuncId(tCQCSysCfg::ERmFuncs::Security).bIsEmpty())
    {
        AddError
        (
            scriRoom
            , colErrs
            , kCQCAErrs::errcSysCfg_NoSecZones
            , m_pwndZones->widThis()
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTSecure: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTSecure::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgSec, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgSec_Disable, m_pwndDisable);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgSec_SecArea, m_pwndSecArea);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgSec_SelArea, m_pwndSelArea);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgSec_SelZones, m_pwndSelZones);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgSec_Zones, m_pwndZones);

    // Get pointers to the stuff we need to interact with
    m_pwndDisable->pnothRegisterHandler(this, &TCQCESCTSecure::eClickHandler);
    m_pwndSelArea->pnothRegisterHandler(this, &TCQCESCTSecure::eClickHandler);
    m_pwndSelZones->pnothRegisterHandler(this, &TCQCESCTSecure::eClickHandler);
    m_pwndZones->pnothRegisterHandler(this, &TCQCESCTSecure::eLBHandler);

    // Set the columns on the zone list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Zone Name");
    colCols.objAdd(L"Spoken Name");
    m_pwndZones->SetColumns(colCols);

    // Set ourselves on the zones list as the auto-IPE handler
    m_pwndZones->SetAutoIPE(this);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTSecure: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TCQCESCTSecure::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgSec_Disable)
    {
        // Clear our selected area and the zone list
        m_pwndSecArea->ClearText();
        m_pwndZones->RemoveAll();

        m_pwndSelZones->SetEnable(kCIDLib::False);
        m_pwndZones->SetEnable(kCIDLib::False);
        m_pwndZones->RemoveAll();

        // And update the room by setting the security area reference to empty
        scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Security, TString::strEmpty());
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgSec_SelArea)
    {
        // Build up a list of configured area names/ids
        tCIDLib::TKVPList colList;
        const tCIDLib::TCard4 c4Count = scfgEdit().c4QueryCfgList
        (
            tCQCSysCfg::ECfgLists::Security, colList
        );

        if (c4Count)
        {
            TKeyValuePair kvalSel;
            const tCIDLib::TBoolean bRes = facCIDWUtils().bKVPListSelect
            (
                *this, L"Select a Security Area", colList, kvalSel, kCIDLib::False
            );

            if (bRes)
            {
                m_pwndSecArea->strWndText(kvalSel.strKey());
                scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Security, kvalSel.strValue());

                // Enable the zone stuff if it wasn't already
                m_pwndSelZones->SetEnable(kCIDLib::True);
                m_pwndZones->SetEnable(kCIDLib::True);

                // And empty the zone list and any previously selected zones
                tCIDLib::TKVPList colEmpty;
                scriRoom().SetSecZones(colEmpty);
                m_pwndZones->RemoveAll();
            }
        }
         else
        {
            TOkBox msgbErr(L"No Security devices have been configured");
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgSec_SelZones)
    {
        //
        //  We use standard helper dialog to select from the zones that are defined
        //  for the selected area.
        //
        SelectSecZones();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCESCTSecure::eLBHandler(TListChangeInfo& wnotEvent)
{
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCESCTSecure::SelectSecZones()
{
    TCQCSysCfgRmInfo& scriRm = scriRoom();

    //
    //  Let's get a list of zone names for the configured security area. We use a
    //  backdoor query to get a quoted comma list of zone names.
    //
    tCIDLib::TStrList colZones;
    try
    {
        // Get a ref to the security info our room is configured for
        tCIDLib::TCard4 c4Ind;
        const TSCSecInfo& scliCur = scfgEdit().scliSecByUID
        (
            strFuncId(tCQCSysCfg::ERmFuncs::Security), c4Ind
        );

        // Get a client proxy for the CQCServer that hosts the configured moniker
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(scliCur.m_strMoniker);

        // And use that to query the zones for our area name
        TString strVal;
        tCIDLib::TBoolean bRes = orbcSrv->bQueryTextVal
        (
            scliCur.m_strMoniker
            , kDevCls_Security::pszSecQN_AreaZones
            , scliCur.m_strAreaName
            , strVal
        );

        if (bRes)
        {
            // Parse out the list values
            tCIDLib::TCard4 c4ErrInd;
            bRes = TStringTokenizer::bParseQuotedCommaList(strVal, colZones, c4ErrInd);

            if (!bRes)
            {
                TErrBox msgbErr
                (
                    L"The driver returned invalid security zone list info"
                );
                msgbErr.ShowIt(*this);
                return;
            }
        }
         else
        {
            // We couldn't get zones for the passed area
            TErrBox msgbErr(L"Could not query the zones for this room's security area");
            msgbErr.ShowIt(*this);
            return;
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        return;
    }

    // Get a list of any currently selected zones for this room
    tCIDLib::TStrList colUpdate;
    tCIDLib::TCard4 c4ZoneCnt = scriRm.c4SecZoneCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ZoneCnt; c4Index++)
        colUpdate.objAdd(scriRm.strZoneNameAt(c4Index));

    // We have the list, so invoke the selection dialog
    const TString strTitle(L"Select Room Zones");
    if (facCIDWUtils().bListMultiSelect(*this, strTitle, colZones, colUpdate))
    {
        //
        //  Go through all fo the selected one. See if they are in our current room
        //  zone list. If so, get the spoken name value back out so that we don't lose
        //  that info.
        //
        const tCIDLib::TCard4 c4UpdateCnt = colUpdate.c4ElemCount();
        tCIDLib::TKVPList colToSet(c4UpdateCnt);
        TKeyValuePair kvalAdd;
        for (tCIDLib::TCard4 c4UpdateInd = 0; c4UpdateInd < c4UpdateCnt; c4UpdateInd++)
        {
            const TString& strCurZone = colUpdate[c4UpdateInd];
            kvalAdd.Set(strCurZone, TString::strEmpty());

            for (tCIDLib::TCard4 c4ZIndex = 0; c4ZIndex < c4ZoneCnt; c4ZIndex++)
            {
                if (scriRm.strZoneNameAt(c4ZIndex).bCompareI(strCurZone))
                {
                    kvalAdd.strValue(scriRm.strZoneAltNameAt(c4ZIndex));
                    break;
                }
            }
            colToSet.objAdd(kvalAdd);
        }

        // Update the room data
        scriRm.SetSecZones(colToSet);

        // Reload the list with the selected ones
        tCIDLib::TStrList colCols(2);
        colCols.AddXCopies(TString::strEmpty(), 2);

        m_pwndZones->RemoveAll();
        c4ZoneCnt = colUpdate.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ZoneCnt; c4Index++)
        {
            const TKeyValuePair& kvalCur = colToSet[c4Index];
            colCols[0] = kvalCur.strKey();
            colCols[1] = kvalCur.strValue();
            m_pwndZones->c4AddItem(colCols, 0);
        }

        if (c4ZoneCnt)
            m_pwndZones->SelectByIndex(0);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTVoice
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTVoice::TCQCESCTVoice(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Voice")
    , m_pwndEnable(nullptr)
    , m_pwndKeyWord(nullptr)
    , m_pwndList(nullptr)
{
}

TCQCESCTVoice::~TCQCESCTVoice()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTVoice::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();


}


tCIDLib::TVoid TCQCESCTVoice::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTVoice::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();
    const TCQCSysCfgVoice& scvcCur = scriRm.scvcVoice();

    m_pwndEnable->SetEnable(kCIDLib::True);

    // These are only enabled if the enable check box is ticked
    m_pwndKeyWord->SetEnable(scvcCur.bEnable());
    m_pwndList->SetEnable(scvcCur.bEnable());

    m_pwndEnable->SetCheckedState(scvcCur.bEnable());
    m_pwndKeyWord->strWndText(scvcCur.strKeyWord());

    //
    //  Select our host by the previously stored name. If not found, force it into
    //  the list if not empty. If empty, we leave it unselected so that there's an
    //  error during validation.
    //
    if (!scvcCur.strHost().bIsEmpty())
    {
        tCIDLib::TCard4 c4At = m_pwndList->c4FindText
        (
            scvcCur.strHost(), 0, 0, tCIDLib::EMatchTypes::Equals, kCIDLib::False
        );

        if (c4At == kCIDLib::c4MaxCard)
        {
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scvcCur.strHost());
            c4At = m_pwndList->c4AddItem(colCols, 0);
        }

        m_pwndList->SelectByIndex(c4At);
    }
}


tCIDLib::TVoid
TCQCESCTVoice::Validate(TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
    // Get the enable flag
    const tCIDLib::TBoolean bEnabled = m_pwndEnable->bCheckedState();

    // If enabled, check the other stuff
    if (bEnabled)
    {
        // Make sure there's a host selected
        const tCIDLib::TCard4 c4HostInd = m_pwndList->c4CurItem();
        if (c4HostInd == kCIDLib::c4MaxCard)
        {
            AddError
            (
                scriRoom
                , colErrs
                , kCQCAErrs::errcSysCfg_NoVoice
                , m_pwndList->widThis()
            );
        }

        // They have to provide a keyword
        const TString& strKeyWord = m_pwndKeyWord->strWndText();
        if (strKeyWord.bIsEmpty())
        {
            AddError
            (
                scriRoom
                , colErrs
                , kCQCAErrs::errcSysCfg_NoKeyWord
                , m_pwndList->widThis()
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCESCTVoice::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgVoice, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgVoice_Enable, m_pwndEnable);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgVoice_KeyWord, m_pwndKeyWord);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgVoice_List, m_pwndList);

    // Get pointers to the stuff we need to interact with
    m_pwndEnable->pnothRegisterHandler(this, &TCQCESCTVoice::eClickHandler);
    m_pwndKeyWord->pnothRegisterHandler(this, &TCQCESCTVoice::eEFHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCESCTVoice::eLBHandler);

    //
    //  Load up the available CQC Voice instances. We have a static method that will
    //  get the list, updating it only so often, so that we don't continually bang
    //  on the name server if there are lots of rooms defined.
    //
    LoadCQCVoiceInstances();

    // Set the columns
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCAdmin.strMsg(kCQCAMsgs::midTab_SysCfgVoice_Hosts));
    m_pwndList->SetColumns(colCols);

    const tCIDLib::TCard4 c4HostCnt = s_colVoiceHosts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4HostCnt; c4Index++)
    {
        colCols[0] = s_colVoiceHosts[c4Index];
        m_pwndList->c4AddItem(colCols, c4Index);
    }

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Private, static data
// ---------------------------------------------------------------------------
tCIDLib::TEncodedTime   TCQCESCTVoice::s_enctNextCheck = 0;
tCIDLib::TStrList       TCQCESCTVoice::s_colVoiceHosts;


// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Private, static methods
// ---------------------------------------------------------------------------

//
//  We get a name server proxy and iterate the scope where the CQC Voice programs register
//  themselves. The node names are the host names, which is what we need. So we load any
//  found to the hosts list.
//
//  We only do this every so many seconds, so as to avoid redundant traffic to the
//  name server.
//
tCIDLib::TVoid TCQCESCTVoice::LoadCQCVoiceInstances()
{
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();

    // If not time yet, nothing to do
    if (enctNow < s_enctNextCheck)
        return;

    try
    {
        // Reset the next time first, to 30 seconds from now
        s_enctNextCheck = enctNow + (kCIDLib::enctOneSecond * 30);

        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        TVector<TNameServerInfo> colNSItems;
        if (orbcNS->c4EnumObjects(  kCQCKit::pszCQCVoiceSrvScope, colNSItems, kCIDLib::False))
        {
            s_colVoiceHosts.RemoveAll();
            const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TNameServerInfo& nsiCur = colNSItems[c4Index];
                s_colVoiceHosts.objAdd(nsiCur.strNodeName());
            }
        }
         else
        {
            // There aren't any so clear the list
            s_colVoiceHosts.RemoveAll();
        }
    }

    catch(TError& errToCatch)
    {
        // Can't update it
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return;
    }
}



// ---------------------------------------------------------------------------
//  TCQCESCTVoice: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCESCTVoice::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If the enable check box, then enable or disable other stuff
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgVoice_Enable)
    {
        TCQCSysCfgRmInfo& scriRm = scriRoom();
        TCQCSysCfgVoice& scvcCur = scriRm.scvcVoice();

        // Enable/disable other controls based on check box state
        m_pwndKeyWord->SetEnable(wnotEvent.bState());
        m_pwndList->SetEnable(wnotEvent.bState());

        if (wnotEvent.bState())
        {
            // Just set the enable state, the user has to enter valid info
            scvcCur.bEnable(kCIDLib::True);
        }
         else
        {
            // Go back to the default keyword and remove any list selection
            m_pwndKeyWord->strWndText(L"Zira");
            m_pwndList->ClearSelection();

            // Reset this one back to defaults
            scvcCur.Reset();
        }

    }
    return tCIDCtrls::EEvResponses::Handled;
}

tCIDCtrls::EEvResponses TCQCESCTVoice::eEFHandler(TEFChangeInfo& wnotEvent)
{
    // If voice is enabled, update our keyword to track the entry field changes
    TCQCSysCfgRmInfo& scriRm = scriRoom();
    TCQCSysCfgVoice& scvcCur = scriRm.scvcVoice();

    if (scvcCur.bEnable())
        scvcCur.strKeyWord(m_pwndKeyWord->strWndText());

    return tCIDCtrls::EEvResponses::Handled;
}

tCIDCtrls::EEvResponses TCQCESCTVoice::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Clear the Voice host
        scriRoom().scvcVoice().strHost(TString::strEmpty());
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        TCQCSysCfgRmInfo& scriRm = scriRoom();
        TCQCSysCfgVoice& scvcCur = scriRm.scvcVoice();

        // Update our data based on the new selection
        if (wnotEvent.c4Index() == kCIDLib::c4MaxCard)
        {
            // No selection, so clear the host
            scvcCur.strHost(TString::strEmpty());
        }
         else
        {
            // Store the newly selected host
            TString strHost;
            m_pwndList->QueryColText(wnotEvent.c4Index(), 0, strHost);
            scvcCur.strHost(strHost);
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTWeather
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTWeather: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTWeather::TCQCESCTWeather(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Weather")
    , m_pwndMoniker(nullptr)
    , m_pwndDisable(nullptr)
    , m_pwndSel(nullptr)
{
}

TCQCESCTWeather::~TCQCESCTWeather()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTWeather: Public, inherited methods
// ---------------------------------------------------------------------------

// Enable/disable various controls if there's nothing selected for us to display.
tCIDLib::TVoid TCQCESCTWeather::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndMoniker->SetEnable(kCIDLib::False);
    m_pwndSel->SetEnable(kCIDLib::False);
    m_pwndDisable->SetEnable(kCIDLib::False);
}


tCIDLib::TVoid TCQCESCTWeather::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTWeather::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    m_pwndMoniker->SetEnable(kCIDLib::True);
    m_pwndSel->SetEnable(kCIDLib::True);
    m_pwndDisable->SetEnable(kCIDLib::True);

    m_pwndMoniker->strWndText(strFuncTitle(tCQCSysCfg::ERmFuncs::Weather));
}


tCIDLib::TVoid
TCQCESCTWeather::Validate(  TCQCSysCfgRmInfo&           scriRoom
                            , TValErrInfo::TErrList&    colErrs)
{
    // Nothing for us to check
}


// ---------------------------------------------------------------------------
//  TCQCESCTWeather: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTWeather::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgWeath, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgWeath_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgWeath_Disable, m_pwndDisable);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgWeath_Sel, m_pwndSel);

    // Set up handlers
    m_pwndDisable->pnothRegisterHandler(this, &TCQCESCTWeather::eClickHandler);
    m_pwndSel->pnothRegisterHandler(this, &TCQCESCTWeather::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTWeather: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCESCTWeather::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgWeath_Disable)
    {
        scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Weather, TString::strEmpty());
        m_pwndMoniker->ClearText();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgWeath_Sel)
    {
        // Build up a list of configured weather objects
        tCIDLib::TKVPList colList;
        const tCIDLib::TCard4 c4Count = scfgEdit().c4QueryCfgList
        (
            tCQCSysCfg::ECfgLists::Weather, colList
        );

        if (c4Count)
        {
            TKeyValuePair kvalSel;
            const tCIDLib::TBoolean bRes = facCIDWUtils().bKVPListSelect
            (
                *this, L"Select a Weather Source", colList, kvalSel, kCIDLib::False
            );

            if (bRes)
            {
                m_pwndMoniker->strWndText(kvalSel.strKey());
                scriRoom().SetFuncId(tCQCSysCfg::ERmFuncs::Weather, kvalSel.strValue());
            }
        }
         else
        {
            TOkBox msgbErr(L"No weather sources have been configured");
            msgbErr.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCESCTXOver
//  PREFIX: esct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCESCTXOver: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCESCTXOvers::TCQCESCTXOvers(TCQCSysCfg* const pscfgEdit, TCQCFldCache* const pcfcDevs) :

    TCQCESCTabBase(pscfgEdit, pcfcDevs, L"Extra Overlays")
    , m_pwndDeselAll(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSelAll(nullptr)
{
}

TCQCESCTXOvers::~TCQCESCTXOvers()
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTXOver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Enable/disable various controls if there's nothing selected for us to
//  display.
//
tCIDLib::TVoid TCQCESCTXOvers::DisableCtrls()
{
    // Pass it on to our parent FIRST
    TParent::DisableCtrls();

    m_pwndDeselAll->SetEnable(kCIDLib::False);
    m_pwndList->RemoveAll();
    m_pwndSelAll->SetEnable(kCIDLib::False);
}


tCIDLib::TVoid TCQCESCTXOvers::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a focus widget, give it the focus
    if (verrToSel.m_widFocus)
        pwndChildById(verrToSel.m_widFocus)->TakeFocus();
}


// Display the passed info
tCIDLib::TVoid TCQCESCTXOvers::ShowInfo(const TString& strUID)
{
    // Pass it on first to our parent
    TParent::ShowInfo(strUID);

    TCQCSysCfgRmInfo& scriRm = scriRoom();

    m_pwndDeselAll->SetEnable(kCIDLib::True);
    m_pwndSelAll->SetEnable(kCIDLib::True);
    m_pwndList->SetEnable(kCIDLib::True);

    //
    //  Load up the configured overs and check the ones we are configured for.
    //  If there aren't any, then we can skip all this and just clear the list.
    //
    m_pwndList->RemoveAll();
    TCQCSysCfg& scfgLoad = scfgEdit();
    const tCIDLib::TCard4 c4Count = scfgLoad.c4ListIdCount(tCQCSysCfg::ECfgLists::XOvers);
    if (c4Count)
    {
        //
        //  Create a hash set of the currently selected overlays, so that we can
        //  quickly see if any given one is selected. We are storing the UIDs here.
        //
        tCIDLib::TCard4 c4RmCount = scriRm.c4XOverCount();
        tCIDLib::TStrHashSet colUIDs(109, TStringKeyOps());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RmCount; c4Index++)
            colUIDs.objAdd(scriRm.strXOverUIDAt(c4Index));

        //
        //  Now let's load up all of the configured overs, checking the ones that are
        //  currently in our list.
        //
        tCIDLib::TStrList colCols(2);
        colCols.objAdd(TString::strEmpty());
        colCols.objAdd(TString::strEmpty());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCXOverInfo& scliCur = scfgLoad.scliXOverAt(c4Index);

            colCols[1] = scliCur.m_strTitle;
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);
            m_pwndList->SetUserDataAt(c4At, scliCur.m_strUniqueId);

            if (colUIDs.bHasElement(scliCur.m_strUniqueId))
                m_pwndList->SetCheckAt(c4At, kCIDLib::True);
        }
        m_pwndList->SelectByIndex(0, kCIDLib::True);
    }
}


//
//  Note that we MUST use the passed room, not the one for which the tab is currently
//  set.
//
tCIDLib::TVoid
TCQCESCTXOvers::Validate(TCQCSysCfgRmInfo& scriRoom, TValErrInfo::TErrList& colErrs)
{
}


// ---------------------------------------------------------------------------
//  TCQCESCTXOvers: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCESCTXOvers::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgXOs, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up the widgets we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgXOs_DeselAll, m_pwndDeselAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgXOs_List, m_pwndList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgXOs_SelAll, m_pwndSelAll);

    // Set the list columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L" ");
    colCols.objAdd(L"Overlays");
    m_pwndList->SetColumns(colCols, 1);

    // Get pointers to the stuff we need to interact with
    m_pwndDeselAll->pnothRegisterHandler(this, &TCQCESCTXOvers::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCESCTXOvers::eLBHandler);
    m_pwndSelAll->pnothRegisterHandler(this, &TCQCESCTXOvers::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCESCTXOver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCESCTXOvers::eClickHandler(TButtClickInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgXOs_DeselAll)
    ||  (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgXOs_SelAll))
    {
        const tCIDLib::TBoolean bSel
        (
            wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgXOs_SelAll
        );

        //
        //  Check/uncheck all of the list window items. If selecting, then add all
        //  of them to a list to set on the room at the end.
        //
        tCIDLib::TStrList colNew;
        const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_pwndList->SetCheckAt(c4Index, bSel);

            // If selecting, add to the list to set below
            if (bSel)
                colNew.objAdd(m_pwndList->strUserDataAt(c4Index));
        }

        // And clear or all all of the overs to our room
        scriRoom().SetXOvers(colNew);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCESCTXOvers::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgXOs_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            const tCQCSysCfg::ECfgLists eList = tCQCSysCfg::ECfgLists::XOvers;
            if (m_pwndList->bIsCheckedAt(wnotEvent.c4Index()))
            {
                m_pwndList->SetCheckAt(wnotEvent.c4Index(), kCIDLib::False);
                scriRoom().RemoveFromList(eList, m_pwndList->strUserDataAt(wnotEvent.c4Index()));
            }
             else
            {
                m_pwndList->SetCheckAt(wnotEvent.c4Index(), kCIDLib::True);
                scriRoom().AddXOver(m_pwndList->strUserDataAt(wnotEvent.c4Index()));
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TESCRoomsTab
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TESCRoomsTab: Public, static data
// ---------------------------------------------------------------------------
const tCIDLib::TCard4 TESCRoomsTab::c4CmdId_RefreshTabs  = 1;


// ---------------------------------------------------------------------------
//  TESCRoomsTab: Constructors and Destructor
// ---------------------------------------------------------------------------
TESCRoomsTab::TESCRoomsTab( TCQCSysCfg* const       pscfgEdit
                            , TCQCFldCache* const   pcfcDevs) :

    TTabWindow(L"/SystemCfg/Rooms", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pcfcDevs(pcfcDevs)
    , m_pscfgEdit(pscfgEdit)
    , m_pwndAddRoom(nullptr)
    , m_pwndCopyFrom(nullptr)
    , m_pwndDelRoom(nullptr)
    , m_pwndList(nullptr)
    , m_pwndRoomTabs(nullptr)
{
}

TESCRoomsTab::~TESCRoomsTab()
{
}


// ---------------------------------------------------------------------------
//  TESCRoomsTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The main dialog calls us here if the user selects a room oriented error. He
//  passes laong the error info. We distribute it to the correct room tab.
//
tCIDLib::TVoid TESCRoomsTab::SelectErrTarget(const TValErrInfo& verrToSel)
{
    if ((verrToSel.m_eType == TValErrInfo::ETypes::Room)
    &&  (verrToSel.m_c4TabId != kCIDLib::c4MaxCard))
    {
        //
        //  Find the room list window item that has the reported room UID. Then we will
        //  select this item, forcing an event just in case, so that the room data gets
        //  refreshed.
        //
        const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_pwndList->strUserDataAt(c4Index) == verrToSel.m_strUniqueId)
                m_pwndList->SelectByIndex(c4Index, kCIDLib::True);
        }

        // Activate the target error tab
        const tCIDLib::TCard4 c4TabInd = m_pwndRoomTabs->c4ActivateById(verrToSel.m_c4TabId);

        // And let it respond to the error
        TCQCESCTabBase* pwndCurTab = static_cast<TCQCESCTabBase*>
        (
            m_pwndRoomTabs->pwndTabAt(c4TabInd)
        );
        pwndCurTab->SelectErrTarget(verrToSel);
    }
     else if (verrToSel.m_eType == TValErrInfo::ETypes::RoomCfg)
    {
        // For now, do nothing, since it's not room specific
    }
}


//
//  Load up all our content for the current config data. This is called on us if
//  the user hits the Revert button to go back to previous data.
//
//  In our case, we just load the room list, and the list box handlers will rect
//  to the selection of a room and update the room tabs.
//
tCIDLib::TVoid TESCRoomsTab::Reload()
{
    LoadRooms(TString::strEmpty());
}


//
//  We go through all the data and see if it looks reasonable. If we find
//  any errors, we return them in a list to be displayed to the user.
//
tCIDLib::TVoid TESCRoomsTab::Validate(TValErrInfo::TErrList& colList)
{
    const tCIDLib::TCard4 c4RmCount = m_pwndList->c4ItemCount();

    //
    //  We have to have ave least one room, or there's no point in going on to do
    //  the room stuff.
    //
    if (!c4RmCount)
    {
        colList.objAdd
        (
            TValErrInfo
            (
                c4TabId()
                , facCQCAdmin.strMsg(kCQCAErrs::errcSysCfg_NoRooms)
                , 0
                , TValErrInfo::ETypes::RoomCfg
            )
        );

        // Just return, nothing else to check
        return;
    }

    // Let each tab pass judgement on each room
    tCIDLib::TStrList colUsedDrvs;
    for (tCIDLib::TCard4 c4RIndex = 0; c4RIndex < c4RmCount; c4RIndex++)
    {
        TCQCSysCfgRmInfo& scrmiRoom = m_pscfgEdit->scriRoomAt(c4RIndex);

        // If not enabled, skip it
        if (!scrmiRoom.bEnabled())
            continue;

        // Let each tab pass judgement on this room
        const tCIDLib::TCard4 c4TabCnt = m_pwndRoomTabs->c4TabCount();
        for (tCIDLib::TCard4 c4TIndex = 0; c4TIndex < c4TabCnt; c4TIndex++)
        {
            TCQCESCTabBase* pwndCurTab = static_cast<TCQCESCTabBase*>
            (
                m_pwndRoomTabs->pwndTabAt(c4TIndex)
            );
            pwndCurTab->Validate(scrmiRoom, colList);
        }
    }
}


// ---------------------------------------------------------------------------
//  TESCRoomsTab: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just need to keep our controls arranged to fit
tCIDLib::TVoid
TESCRoomsTab::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && m_pwndRoomTabs && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TESCRoomsTab::bCreated()
{
    TParent::bCreated();

    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgRoom, dlgdChildren);

    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRoom_AddRoom, m_pwndAddRoom);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRoom_CopyFrom, m_pwndCopyFrom);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRoom_DelRoom, m_pwndDelRoom);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRoom_List, m_pwndList);
    CastChildWnd(*this, kCQCAdmin::ridTab_SysCfgRoom_Tab, m_pwndRoomTabs);

    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Rooms");
    m_pwndList->SetColumns(colCols);

    // Load up all of the tabs
    TCQCESCTGen* pesctGenInfo = new TCQCESCTGen(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctGenInfo, L"General");

    TCQCESCTLoads* pesctLoads = new TCQCESCTLoads(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctLoads, L"Lighting");

    TCQCESCTSecure* pesctSecure = new TCQCESCTSecure(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctSecure, L"Security");

    TCQCESCTHVAC* pesctHVAC = new TCQCESCTHVAC(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctHVAC, L"HVAC");

    TCQCESCTMedia* pesctMedia = new TCQCESCTMedia(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctMedia, L"Media");

    TCQCESCTWeather* pesctWeather = new TCQCESCTWeather(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctWeather, L"Weather");

    TCQCESCTXOvers* pesctXOvers = new TCQCESCTXOvers(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctXOvers, L"Extra Overlays");

    TCQCESCTGActs* pesctGlobActs = new TCQCESCTGActs(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctGlobActs, L"Global Actions");

    TCQCESCTRmModes* pesctRoomModes = new TCQCESCTRmModes(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctRoomModes, L"Modes");

    TCQCESCTVoice* pesctVoice = new TCQCESCTVoice(m_pscfgEdit, m_pcfcDevs);
    m_pwndRoomTabs->c4CreateTab(pesctVoice, L"Voice");

    // Set up handlers
    m_pwndAddRoom->pnothRegisterHandler(this, &TESCRoomsTab::eClickHandler);
    m_pwndCopyFrom->pnothRegisterHandler(this, &TESCRoomsTab::eClickHandler);
    m_pwndDelRoom->pnothRegisterHandler(this, &TESCRoomsTab::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TESCRoomsTab::eLBHandler);

    // Load up the rooms, with no initial selection
    LoadRooms(TString::strEmpty());

    return kCIDLib::True;
}


//
//  If we are becoming the active tab, then force refresh of the display room
//  contents in case the global lists were changed while we were away.
//
tCIDLib::TVoid TESCRoomsTab::TabActivationChange(const tCIDLib::TBoolean bGetting)
{
    if (bGetting && (m_pwndList->c4CurItem() != kCIDLib::c4MaxCard))
        m_pwndList->SelectByIndex(m_pwndList->c4CurItem(), kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TESCRoomsTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called to add a new room or template (depending on which we are displaying
//  at the time) by either creating a new one from scratch or copying from
//  the selected one and just changing the name.
//
tCIDLib::TVoid TESCRoomsTab::AddNew(const tCIDLib::TBoolean bCopying)
{
    tCIDLib::TCard4 c4SrcIndex(kCIDLib::c4MaxCard);

    // If doing copy from, make sure there's something selected to copy
    TString strUID;
    if (bCopying)
    {
        if (!m_pwndList->c4ItemCount())
        {
            TErrBox msgbCopy
            (
                facCQCAdmin.strMsg(kCQCAErrs::errcSysCfg_NothingToCopy)
            );
            msgbCopy.ShowIt(*this);
            return;
        }

        // Store the source UID we are copying from for use below
        strUID = m_pwndList->strUserDataAt(m_pwndList->c4CurItem());
    }

    // We have to get a name and it has to be unique
    const TString       strTitle(kCQCAMsgs::midPrompt_EnterRoomName, facCQCAdmin);
    TString             strEntered;

    if (facCIDWUtils().bGetText(*this, strTitle, TString::strEmpty(), strEntered))
    {
        // Make sure that the name doesn't already exist
        if (!m_pscfgEdit->bIsUniqueName(strEntered))
        {
            TErrBox msgbErr
            (
                facCQCSysCfg().strMsg(kCQCSCfgErrs::errcSysCfg_UniqueName, strEntered)
            );
            msgbErr.ShowIt(*this);
            return;
        };

        //
        //  If copying, get the current room, update it and add, else just add a new
        //  one with default values.
        //
        if (bCopying)
        {
            tCIDLib::TCard4 c4RmIndex;

            //
            //  Do an explicit room copy operation, since we have to make it create
            //  a new unique id.
            //
            TCQCSysCfgRmInfo scrmiNew;
            scrmiNew.CopyFrom(m_pscfgEdit->scriRoomByUID(strUID, c4RmIndex));
            scrmiNew.strName(strEntered);

            // Now store the new UID for use below
            strUID = scrmiNew.strUniqueId();

            m_pscfgEdit->bAddOrUpdateRoom(scrmiNew, c4RmIndex);
        }
         else
        {
            const tCIDLib::TCard4 c4Ind = m_pscfgEdit->c4AddRoom(strEntered);
            TCQCSysCfgRmInfo& scriNew = m_pscfgEdit->scriRoomAt(c4Ind);
            strUID = scriNew.strUniqueId();

            //
            //  To be helpful, we'll try to set a couple things. If there's only
            //  a single weather location or security area, which will be fairly
            //  common, select them by default.
            //
            if (m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Weather) == 1)
            {
                scriNew.SetFuncId
                (
                    tCQCSysCfg::ERmFuncs::Weather
                    , m_pscfgEdit->scliWeatherAt(0).m_strUniqueId
                );
            }

            if (m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Security) == 1)
            {
                scriNew.SetFuncId
                (
                    tCQCSysCfg::ERmFuncs::Security
                    , m_pscfgEdit->scliSecAt(0).m_strUniqueId
                );
            }
        }

        //
        //  We have to get this guy into the list, but we have the issues
        //  of sorting and that we store indicates in the list window
        //  items and we are going to change those probably. So we just
        //  reload the list and select the new one as the initial one.
        //
        LoadRooms(strUID);
    }
}


tCIDLib::TVoid TESCRoomsTab::DeleteCur()
{
    // Get the current selection. If none, do nothing
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    // Get a copy of the UID and name before we trash it
    const TString strUID = m_pwndList->strUserDataAt(c4ListInd);
    TString strName;
    m_pwndList->QueryColText(c4ListInd, 0, strName);

    TYesNoBox msgbDel(facCQCAdmin.strMsg(kCQCAMsgs::midQ_DelRoom, strName));
    if (!msgbDel.bShowIt(*this))
        return;

    // Remove from the config and the list
    m_pscfgEdit->DeleteRoom(strUID);
    m_pwndList->RemoveAt(c4ListInd);
}


//
//  Handle button and check box clicks.
//
tCIDCtrls::EEvResponses TESCRoomsTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRoom_AddRoom)
    ||  (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRoom_CopyFrom))
    {
        AddNew(wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRoom_CopyFrom);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRoom_DelRoom)
    {
        DeleteCur();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle changes in our room/template list
tCIDCtrls::EEvResponses TESCRoomsTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgRoom_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            const tCIDLib::TCard4 c4TabCnt = m_pwndRoomTabs->c4TabCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
            {
                TCQCESCTabBase* pwndCurTab = static_cast<TCQCESCTabBase*>
                (
                    m_pwndRoomTabs->pwndTabAt(c4Index)
                );
                pwndCurTab->DisableCtrls();
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Load the newly selected room
            SelectListItem(wnotEvent.c4Index());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Load the list with the available rooms. If there are none, then ask all the
//  tabs to disable the relevant stuff. We can get the UID of an initial one to
//  select, but it can be an empty string in which case we'll just select the
//  0th one.
//
tCIDLib::TVoid TESCRoomsTab::LoadRooms(const TString& strUIDInit)
{
    m_pwndList->RemoveAll();
    const tCIDLib::TCard4 c4Count = m_pscfgEdit->c4RoomCount();
    if (c4Count)
    {
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(TString::strEmpty());

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCSysCfgRmInfo& scriCur = m_pscfgEdit->scriRoomAt(c4Index);

            colCols[0] = scriCur.strName();
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);
            m_pwndList->SetUserDataAt(c4At, scriCur.strUniqueId());
        }

        // Find the one with the indicated UID, if it's not empty
        tCIDLib::TCard4 c4InitSel = 0;
        if (!strUIDInit.bIsEmpty())
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (m_pwndList->strUserDataAt(c4Index) == strUIDInit)
                {
                    c4InitSel = c4Index;
                    break;
                }
            }
        }
        m_pwndList->SelectByIndex(c4InitSel, kCIDLib::True);
    }
     else
    {
        const tCIDLib::TCard4 c4TabCnt = m_pwndRoomTabs->c4TabCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
        {
            TCQCESCTabBase* pwndCurTab = static_cast<TCQCESCTabBase*>
            (
                m_pwndRoomTabs->pwndTabAt(c4Index)
            );
            pwndCurTab->DisableCtrls();
        }
    }
}


//
//  Called to select a new room/template when the user clicks in the list.
//  We get the index of the one to select. We tell all of the tab windows
//  about the new selection so that they can update.
//
//  Note that the passed index is the index of the list window item. We have
//  get that out and get the index from it into the actual list.
//
tCIDLib::TVoid TESCRoomsTab::SelectListItem(const tCIDLib::TCard4 c4Index)
{
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    CIDAssert(c4ListInd == c4Index, L"No/wrong room selected in list");

    // Uppdate all the tabs with the new info
    const TString& strCurUID = m_pwndList->strUserDataAt(c4ListInd);
    const tCIDLib::TCard4 c4TabCnt = m_pwndRoomTabs->c4TabCount();
    for (tCIDLib::TCard4 c4TIndex = 0; c4TIndex < c4TabCnt; c4TIndex++)
    {
        TCQCESCTabBase* pwndCurTab = static_cast<TCQCESCTabBase*>
        (
            m_pwndRoomTabs->pwndTabAt(c4TIndex)
        );
        pwndCurTab->ShowInfo(strCurUID);
    }
}

