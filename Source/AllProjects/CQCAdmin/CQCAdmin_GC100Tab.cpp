//
// FILE NAME: CQCAdmin_GC100Tab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/03/2016
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
//  This file implements a tab for editing GC-100 serial ports.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_GC100Tab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TGC100Tab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TGC100Tab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100Tab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TGC100Tab::TGC100Tab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"GC-100", kCIDLib::True)
    , m_pwndList(nullptr)
    , m_pwndAddNew(nullptr)
    , m_pwndDel(nullptr)
{
}

TGC100Tab::~TGC100Tab()
{
}


// ---------------------------------------------------------------------------
//  TGC100Tab: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TGC100Tab::bIPETestCanEdit( const   tCIDCtrls::TWndId
                            , const tCIDLib::TCard4     c4ColInd
                            ,       TAttrData&          adatFill
                            ,       tCIDLib::TBoolean&  bValueSet)
{
    // Get the column text
    TString strVal;
    m_pwndList->QueryColText(c4IPERow(), c4ColInd, strVal);

    // Set up the attribute
    switch(c4ColInd)
    {
        case 0 :
            adatFill.Set(L"Name", L"Name", tCIDMData::EAttrTypes::String);
            break;

        case 1 :
            adatFill.Set(L"Addr", L"Addr", tCIDMData::EAttrTypes::String);
            break;

        case 2 :
            adatFill.Set(L"Enable1", L"Enable1", tCIDMData::EAttrTypes::Bool);
            break;

        case 3 :
            adatFill.Set(L"Enable2", L"Enable2", tCIDMData::EAttrTypes::Bool);
            break;

        default :
            break;
    };

    // Set the current value as the starting point to edit from
    adatFill.SetValueFromText(strVal);

    // Indicate we set the value
    bValueSet = kCIDLib::True;

    return kCIDLib::True;
}


//
//
tCIDLib::TBoolean
TGC100Tab::bIPEValidate(const   TString&            strSrc
                        ,       TAttrData&          adatTar
                        , const TString&            strNewVal
                        ,       TString&            strErrMsg
                        ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    if (adatTar.strId() == L"Name")
    {
        // Make sure it's unique
        TString strVal;
        const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            // Skip the original row itself
            if (c4Index != c4IPERow())
            {
                m_pwndList->QueryColText(c4Index, 0, strVal);
                if (strVal.eCompareI(strNewVal) == tCIDLib::ESortComps::Equal)
                {
                    strErrMsg = L"This name is already in use by another GC-100";
                    return kCIDLib::False;
                }
            }
            c4Index++;
        }
    }
     else if (adatTar.strId() == L"Addr")
    {
        // Make sure we can resolve it
        try
        {
            TIPAddress ipaTest(strNewVal, tCIDSock::EAddrTypes::IPV4);
        }

        catch(TError& errToCatch)
        {
            strErrMsg = errToCatch.strErrText();
            return kCIDLib::False;
        }
    }
     else if ((adatTar.strId() == L"Enable1") || (adatTar.strId() == L"Enable2"))
    {

    }

    // It passed so set it on the attribute
    adatTar.SetValueFromText(strNewVal);

    return kCIDLib::True;
}


tCIDLib::TVoid
TGC100Tab::IPEValChanged(const  tCIDCtrls::TWndId   widSrc
                        , const TString&            strSrc
                        , const TAttrData&          adatNew
                        , const TAttrData&          adatOld
                        , const tCIDLib::TCard8     )
{
    // Nothing to do, we don't store until the user saves
}


// ---------------------------------------------------------------------------
//  TGC100Tab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TGC100Tab::CreateTab(       TTabbedWnd&     wndParent
                    , const TString&        strTabText
                    , const TGC100CfgList&  gcclEdit)
{
    //
    //  Though the main facility should not let anyone with less than power user log into
    //  this program, check it just in case.
    //
    CIDAssert
    (
        facCQCAdmin.eUserRole() >= tCQCKit::EUserRoles::PowerUser
        , L"This requires power user or higher privileges"
    )

    // Store away the info, and a second copy for comparisons later
    m_gcclEdit = gcclEdit;
    m_gcclOrg = gcclEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TGC100Tab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGC100Tab::AreaChanged( const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && m_pwndList && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TGC100Tab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls.
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_GC100Ports, dlgdChildren);

    // Create the children as children of the client
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    //
    //  Set us to use the standard window background. The dialog description we
    //  used to create the content was set to the main window theme, so all of the
    //  widgets should be fine already.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get pointers to our handful of controls and load those that show data
    CastChildWnd(*this, kCQCAdmin::ridTab_GC100Ports_AddNew, m_pwndAddNew);
    CastChildWnd(*this, kCQCAdmin::ridTab_GC100Ports_Del, m_pwndDel);
    CastChildWnd(*this, kCQCAdmin::ridTab_GC100Ports_DelAll, m_pwndDelAll);
    CastChildWnd(*this, kCQCAdmin::ridTab_GC100Ports_List, m_pwndList);

    // Set the columns on the list box
    tCIDLib::TStrList colCols(4);
    colCols.objAdd(L"Unit Name     ");
    colCols.objAdd(L"Unit Address ");
    colCols.objAdd(L"Enable Port 1");
    colCols.objAdd(L"Enable Port 2");
    m_pwndList->SetColumns(colCols);

    // Go through the list and load up the multi-column list box
    const tCIDLib::TCard4 c4Count = m_gcclEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TGC100Cfg& gccfgCur = m_gcclEdit.gccfgAt(c4Index);

       colCols[0] = gccfgCur.strName();
       colCols[1] = gccfgCur.strAddr();
       colCols[2] = (gccfgCur.c4Ports() & 0x1) ? L"True" : L"False";
       colCols[3] = (gccfgCur.c4Ports() & 0x2) ? L"True" : L"False";

       m_pwndList->c4AddItem(colCols, 0);
    }

    // Set our event handlers
    m_pwndAddNew->pnothRegisterHandler(this, &TGC100Tab::eClickHandler);
    m_pwndDel->pnothRegisterHandler(this, &TGC100Tab::eClickHandler);
    m_pwndDelAll->pnothRegisterHandler(this, &TGC100Tab::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TGC100Tab::eLBHandler);

    // Set us as the in place editor on the list box
    m_pwndList->SetAutoIPE(this);

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TGC100Tab::eDoSave(         TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    //
    //  Go through the list and store them into a temp list. When we have them successfully
    //  stored, then we can copy to the edit list.
    //
    tCIDLib::TCard4 c4Flags = 0;
    TString strName;
    TString strAddr;
    TString strEnable1;
    TString strEnable2;
    TGC100CfgList gcclTmp;
    TGC100Cfg gccfgTmp;
    const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_pwndList->QueryColText(c4Index, 0, strName);
        m_pwndList->QueryColText(c4Index, 1, strAddr);
        m_pwndList->QueryColText(c4Index, 2, strEnable1);
        m_pwndList->QueryColText(c4Index, 3, strEnable2);

        c4Flags = 0;
        if (strEnable1 == L"True")
            c4Flags |= 1;
        if (strEnable2 == L"True")
            c4Flags |= 2;
        gccfgTmp.Set(strName, strAddr, c4Flags);
        gcclTmp.c4AddNew(gccfgTmp);
    }
    m_gcclEdit = gcclTmp;

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_gcclEdit == m_gcclOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        tCIDLib::TCard4 c4SerNum = 0;
        if (!facGC100Ser().bSaveConfig(m_gcclEdit, c4SerNum, facCQCAdmin.sectUser()))
        {
            strErrMsg = facCQCAdmin.strMsg
            (
                kCQCAErrs::errcPortCfg_CantStore, this->strName()
            );
            return tCIDLib::ESaveRes::Errors;
        }

        //
        //  Try to update the port factory as well. This just updates our local
        //  one. But any use of newly configured ports would require a driver
        //  install and the driver wizard forces an update on the target CQCServer
        //  during its init, so they'd get forced into the remote factory at that
        //  time.
        //
        facGC100Ser().bUpdateFactory(m_gcclEdit);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // Store the changes to the org member
    m_gcclOrg = m_gcclEdit;
    bChangesSaved = kCIDLib::True;

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}

// ---------------------------------------------------------------------------
//  TGC100Tab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TGC100Tab::eClickHandler(TButtClickInfo& wnotClick)
{
    if (wnotClick.widSource() == kCQCAdmin::ridTab_GC100Ports_AddNew)
    {
        // Find a unique default name to add it with
        tCIDLib::TCard4 c4Index = 1;
        TString strName;
        while (kCIDLib::True)
        {
            strName = L"GC-100_";
            strName.AppendFormatted(c4Index);

            tCIDLib::TCard4 c4At = m_pwndList->c4TextToIndex(strName, 0);
            if (c4At == kCIDLib::c4MaxCard)
                break;
            c4Index++;
        }

        // Add a new item to the list with default, tell it to select this new one
        tCIDLib::TStrList colCols(4);
        colCols.objAdd(strName);
        colCols.objAdd(L"localhost");
        colCols.objAdd(L"False");
        colCols.objAdd(L"False");
        m_pwndList->c4AddItem(colCols, 0, kCIDLib::c4MaxCard, kCIDLib::True);
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_GC100Ports_Del)
    {
        // Get the current index and remove that one
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
            m_pwndList->RemoveAt(c4Index);
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_GC100Ports_DelAll)
    {
        TYesNoBox msgbDelAll
        (
            facCQCAdmin.strMsg(kCQCAMsgs::midQ_DeleteAll, TString(L"GC-100 Ports"))
        );
        if (msgbDelAll.bShowIt(*this))
            m_pwndList->RemoveAll();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TGC100Tab::eLBHandler(TListChangeInfo& wnotEvent)
{
    // Update any stuff that depends on the selected field
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Disable the buttons that do something on the current field
        m_pwndDel->SetEnable(kCIDLib::False);

        // Move the default button to the Add since we disabled the current one
        m_pwndAddNew->SetDefButton();
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Enable buttons that require we have something selected
        m_pwndDel->SetEnable(kCIDLib::True);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

