//
// FILE NAME: CQCAdmin_DrvMonTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/07/2016
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
//  This file implements a tab for monitoring a device driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_DrvMonTab.hpp"
#include    "CQCAdmin_EdFldTriggerDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TDrvMonTab, TContentTab)


// ---------------------------------------------------------------------------
//  Local constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCAdmin_DrvMonTab
    {
        // -----------------------------------------------------------------------
        //  The indices of our list window columns
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4Col_Name      = 0;
        constexpr tCIDLib::TCard4   c4Col_Type      = 1;
        constexpr tCIDLib::TCard4   c4Col_SemType   = 2;
        constexpr tCIDLib::TCard4   c4Col_Access    = 3;
        constexpr tCIDLib::TCard4   c4Col_Trigger   = 4;
        constexpr tCIDLib::TCard4   c4Col_SetValue  = 5;
        constexpr tCIDLib::TCard4   c4Col_Value     = 6;

        constexpr tCIDLib::TCard4   c4ColCount      = 7;

        constexpr tCIDLib::TCard4   c4ActiveMSs     = 3000;
        constexpr tCIDLib::TCard4   c4InactiveMSs   = 8000;
    }
};


// ---------------------------------------------------------------------------
//  CLASS: TDrvMonTabPFI
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDrvMonTabPFI: Constructors and destructor
// ---------------------------------------------------------------------------
TDrvMonTabPFI::TDrvMonTabPFI(const TCQCFldDef& flddThis) :

    m_flddThis(flddThis)
{
    m_tmFmt.strDefaultFormat(TTime::strCTime());
}

TDrvMonTabPFI::~TDrvMonTabPFI()
{
}


// ---------------------------------------------------------------------------
//  TDrvMonTabPFI: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called periodically to update our value/status. If we have a new value we store
//  it in m_strLastVal and return true. The caller can update the displayed field
//  value. If nothing has changed, we return false.
//
tCIDLib::TBoolean TDrvMonTabPFI::bUpdate(TCQCPollEngine& polleSrc)
{
    // If not readable, just return false now
    if (!m_flddThis.bIsReadable())
        return kCIDLib::False;

    if (m_cfpiThis.bUpdateValue(polleSrc))
    {
        // The value or state changed, so we need to store a new value
        const tCQCPollEng::EFldStates eCurState = m_cfpiThis.eState();
        if (eCurState == tCQCPollEng::EFldStates::Ready)
        {
            m_cfpiThis.fvCurrent().Format(m_strLastVal);

            // If a time type, then also format it out as a time stamp
            if (m_flddThis.eType() == tCQCKit::EFldTypes::Time)
            {
                tCIDLib::TCard8 c8Val;
                if (m_strLastVal.bToCard8(c8Val, tCIDLib::ERadices::Hex))
                {
                    m_tmFmt = c8Val;
                    m_strLastVal.Append(L" (");
                    m_tmFmt.FormatToStr(m_strLastVal, tCIDLib::EAppendOver::Append);
                    m_strLastVal.Append(L")");
                }
            }
        }
         else
        {
            m_strLastVal = L"????";
        }

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TVoid
TDrvMonTabPFI::Init(const TString& strMoniker, TCQCPollEngine& polleToUse)
{
    // If we are readable set us up for polling
    if (m_flddThis.bIsReadable())
        m_cfpiThis.SetField(strMoniker, m_flddThis.strName());
}





// ---------------------------------------------------------------------------
//  CLASS: TDrvMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDrvMonTab: Constructors and destructor
// ---------------------------------------------------------------------------

TDrvMonTab::TDrvMonTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Driver", kCIDLib::False)
    , m_c4FldListId(0)
    , m_c4DriverId(0)
    , m_c4DrvListId(0)
    , m_colFldList(tCIDLib::EAdoptOpts::Adopt)
    , m_eDrvState(tCQCKit::EDrvStates::NotLoaded)
    , m_eVerbosity(tCQCKit::EVerboseLvls::Off)
    , m_pwndClearTrigs(nullptr)
    , m_pwndCopyVal(nullptr)
    , m_pwndDriverInfo(nullptr)
    , m_pwndEditTrig(nullptr)
    , m_pwndFldList(nullptr)
    , m_pwndLimits(nullptr)
    , m_pwndMoniker(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndVerbosity(nullptr)
    , m_strDropDownVal(L"v")
    , m_strPopupVal(L"...")
    , m_strVerboseFldName(tCQCKit::strAltXlatEStdFields(tCQCKit::EStdFields::Verbosity))
    , m_tmidMon(kCIDCtrls::tmidInvalid)
{
    // Get the host and moniker out of the path
    m_strHost.CopyInSubStr(strPath, kCQCRemBrws::strPath_Devices.c4Length() + 1);
    m_strHost.bSplit(m_strMoniker, kCIDLib::chForwardSlash);
}

TDrvMonTab::~TDrvMonTab()
{
}


// ---------------------------------------------------------------------------
//  TDrvMonTab: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't attempt to set any of the field's limits on the attribute. THat would be
//  a lot of work. So we just set the attribute to the basic type and we use the actual
//  field limits to do the validation (beyond it being basically valid for the type
//  which our base class will handle.)
//
tCIDLib::TBoolean
TDrvMonTab::bIPEValidate(const  TString&            strSrc
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

    if (adatTar.strId() == L"FldVal")
    {
        const tCIDLib::TCard4 c4Index = m_pwndFldList->c4IndexToId(c4IPERow());
        const TCQCFldDef& flddCur = m_colFldList[c4Index]->m_flddThis;

        TCQCFldLimit* pfldlTest = TCQCFldLimit::pfldlMakeNew(flddCur);
        if (pfldlTest)
        {
            TJanitor<TCQCFldLimit> janLims(pfldlTest);
            if (!pfldlTest->bValidate(strNewVal))
            {
                strErrMsg = L"The value doesn't match the limits for this field";
                return kCIDLib::False;
            }
        }

        //
        //  Looks ok, so store it. We use the generic text based value setter here
        //  to avoid a lot of nasty typing.
        //
        adatTar.SetValueFromText(strNewVal);
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDrvMonTab::IPEValChanged(  const   tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    // Try to write the new value
    const tCIDLib::TCard4 c4Index = m_pwndFldList->c4IndexToId(c4IPERow());
    const TCQCFldDef& flddCur = m_colFldList[c4Index]->m_flddThis;

    TString strToWrite;
    adatNew.FormatToText(strToWrite);
    WriteFld(flddCur.strName(), strToWrite);

    // If write only, don't keep the entered text
    if (flddCur.eAccess() == tCQCKit::EFldAccess::Write)
        m_pwndFldList->SetColText(c4Index, CQCAdmin_DrvMonTab::c4Col_Value, TString::strEmpty());

    //
    //  On the assumption that it's going to stick, update the underlying column with
    //  the entered column so that it doesn't flash back to the previous value and then
    //  update on the next poll.
    //
    m_pwndFldList->SetColText(c4IPERow(), CQCAdmin_DrvMonTab::c4Col_Value, strToWrite);
}


// ---------------------------------------------------------------------------
//  TDrvMonTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TDrvMonTab::CreateTab(TTabbedWnd& wndParent, const TString& strTabText)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TDrvMonTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TDrvMonTab::AreaChanged(const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have created children, and aren't minimized
    if (bSizeChanged && m_pwndStatus && (ePosState != tCIDCtrls::EPosStates::Minimized))
    {
        AutoAdjustChildren(areaPrev, areaNew);
    }
}


tCIDLib::TBoolean TDrvMonTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_DrvMon, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized up to us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    //
    //  Set us to use the standard window background. The dialog description we
    //  used to create the content was set to the main window theme, so all of the
    //  widgets should be fine already.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get pointers to our controls we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_ClearTrigs, m_pwndClearTrigs);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_CopyVal, m_pwndCopyVal);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_DriverInfo, m_pwndDriverInfo);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_EditTrig, m_pwndEditTrig);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_FldList, m_pwndFldList);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_Limits, m_pwndLimits);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCAdmin::ridTab_DrvMon_Verbosity, m_pwndVerbosity);

    //
    //  Load the incoming info that we can do reliably now. The rest is left to the
    //  bgn thread to handle.
    //
    m_pwndMoniker->strWndText(m_strMoniker);

    // Set up the columns on the list box
    tCIDLib::TStrList colCols(CQCAdmin_DrvMonTab::c4ColCount);
    colCols.objAdd(L"Name");
    colCols.objAdd(L"Fld Type");
    colCols.objAdd(L"Sem Type");
    colCols.objAdd(L"R/W");
    colCols.objAdd(L"Trig");
    colCols.objAdd(L"...");
    colCols.objAdd(L"Value");
    m_pwndFldList->SetColumns(colCols);

    // Set up event handlers
    m_pwndClearTrigs->pnothRegisterHandler(this, &TDrvMonTab::eClickHandler);
    m_pwndCopyVal->pnothRegisterHandler(this, &TDrvMonTab::eClickHandler);
    m_pwndDriverInfo->pnothRegisterHandler(this, &TDrvMonTab::eClickHandler);
    m_pwndEditTrig->pnothRegisterHandler(this, &TDrvMonTab::eClickHandler);
    m_pwndFldList->pnothRegisterHandler(this, &TDrvMonTab::eLBHandler);
    m_pwndVerbosity->pnothRegisterHandler(this, &TDrvMonTab::eLBHandler);

    // Initialize the driver status to question marks until we get the first good status query.
    m_pwndStatus->strWndText(L"????");

    //
    //  Load the verbosity combo box and select the Off level as the initial value until we
    //  get info to the contrary.
    //
    tCQCKit::EVerboseLvls eVerbosity = tCQCKit::EVerboseLvls::Min;
    while (eVerbosity <= tCQCKit::EVerboseLvls::Max)
    {
        m_pwndVerbosity->c4AddItem(tCQCKit::strXlatEVerboseLvls(eVerbosity));
        eVerbosity++;
    }

    tCIDLib::TCard4 c4At;
    if (!m_pwndVerbosity->bSelectByText(tCQCKit::strXlatEVerboseLvls(m_eVerbosity), c4At))
    {
        CIDAssert2(L"The initial verbosity level could not be selected")
    }

    //
    //  To get us going faster, do an initial call to the connnect method. If it
    //  fails, we'll just keep trying via the timer after we start it up.
    //
    ConnToDriver();

    // Start up a timer to monitor the driver, assume we are initially the active tab
    m_tmidMon = tmidStartTimer(CQCAdmin_DrvMonTab::c4ActiveMSs);

    return kCIDLib::True;
}


tCIDLib::TVoid TDrvMonTab::Destroyed()
{
    // Clean up our timer if we started it
    if (m_tmidMon != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidMon);
        m_tmidMon = kCIDCtrls::tmidInvalid;
    }
    TParent::Destroyed();
}


//
//  We use this to monitor the polling engine and keep our content up to date.
//
tCIDLib::TVoid TDrvMonTab::Timer(const tCIDCtrls::TTimerId tmidSrc)
{
    // If not our timer, or we are not currently visible
    if (tmidSrc != m_tmidMon)
        return;

    if (m_eDrvState != tCQCKit::EDrvStates::Connected)
    {
        //
        //  Try to get our list of fields from the driver and register them with
        //  the polling engine.
        //
        ConnToDriver();
    }
     else
    {
        // Our regular polling cycling
        Poll(kCIDLib::False);
    }

}




// ---------------------------------------------------------------------------
//  TDrvMonTab: Private, non-virtaul methods
// ---------------------------------------------------------------------------

//
//  This is called by the polling timer to get us initially connected to the driver
//  which in this case means being able to query his fields and register them with
//  the polling engine, and get them loaded into our field list window.
//
tCIDLib::TVoid TDrvMonTab::ConnToDriver()
{
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        //
        //  Get a client proxy for our host. First see if the poll engine has it
        //  first and use that, else get one manually.
        //
        TCQCPollEngine& polleToUse = facCQCAdmin.polleToUse();
        tCQCKit::TCQCSrvProxy orbcSrv;
        if (!polleToUse.bGetSrvProxy(m_strHost, orbcSrv))
            orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

        // And query our list of fields and get our ids initialized
        tCQCKit::TFldDefList colFldDefs;
        orbcSrv->c4QueryFields
        (
            m_strMoniker
            , m_eDrvState
            , colFldDefs
            , m_c4FldListId
            , m_c4DriverId
            , m_c4DrvListId
        );

        //
        //  Get the driver config for this driver, which will tell us what fields
        //  have triggers.
        //
        TCQCDriverObjCfg cqcdcDrv;
        {
            TString strTmpHost(m_strHost);
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
            tCIDLib::TCard4 c4Ver = 0;
            if (!orbcIS->bDrvCfgQueryConfig(c4Ver, m_strMoniker, strTmpHost, cqcdcDrv, facCQCAdmin.sectUser()))
            {
                CIDAssert2(L"Could not get the config for this driver")
            }
        }

        // Load up a new set of per-field info objects for the fields we got above
        m_colFldList.RemoveAll();
        colFldDefs.bForEach
        (
            [this](const TCQCFldDef& flddCur)
            {
                m_colFldList.Add(new TDrvMonTabPFI(flddCur)); return kCIDLib::True;
            }
        );

        // Load up empty values to the columns list so we can just set values into them
        tCIDLib::TStrList colCols(CQCAdmin_DrvMonTab::c4ColCount);
        colCols.AddXCopies(TString::strEmpty(), CQCAdmin_DrvMonTab::c4ColCount);

        // Stop redrawing while we do this part
        TWndPaintJanitor janPaint(this);

        //
        //  And let's load them up. We remember if any of them had a non-generic
        //  semantic type. If so, we size that column to the content, else we size it
        //  to the header.
        //
        m_pwndFldList->RemoveAll();
        tCIDLib::TBoolean bHaveSemTypes = kCIDLib::False;
        const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TDrvMonTabPFI& pfiCur = *m_colFldList[c4Index];
            TCQCFldDef& flddCur = pfiCur.m_flddThis;

            if (flddCur.bIsReadable())
            {
                pfiCur.Init(m_strMoniker, polleToUse);

                //
                //  Do an initial value update pass though we won't use the value
                //  we move to the poll state.
                //
                pfiCur.bUpdate(polleToUse);
            }

            // Add this one to our list window
            colCols[CQCAdmin_DrvMonTab::c4Col_Name] = flddCur.strName();
            colCols[CQCAdmin_DrvMonTab::c4Col_Type] = tCQCKit::strXlatEFldTypes(flddCur.eType());

            const tCQCKit::EFldSTypes eSType = flddCur.eSemType();
            if (eSType == tCQCKit::EFldSTypes::Generic)
            {
                colCols[CQCAdmin_DrvMonTab::c4Col_SemType].Clear();
            }
             else
            {
                colCols[CQCAdmin_DrvMonTab::c4Col_SemType] = tCQCKit::strXlatEFldSTypes(eSType);
                bHaveSemTypes = kCIDLib::True;
            }

            // Set the field access
            if (flddCur.eAccess() == tCQCKit::EFldAccess::Read)
                 colCols[CQCAdmin_DrvMonTab::c4Col_Access] = L"R";
            else if (flddCur.eAccess() == tCQCKit::EFldAccess::Write)
                 colCols[CQCAdmin_DrvMonTab::c4Col_Access] = L"W";
            else if (flddCur.eAccess() == tCQCKit::EFldAccess::ReadWrite)
                 colCols[CQCAdmin_DrvMonTab::c4Col_Access] = L"R/W";
             else
                 colCols[CQCAdmin_DrvMonTab::c4Col_Access] = L"??";

            //
            //  See if this is one we do a drop down on, or a popup. If so, set the
            //  flag on this one.
            //
            tCIDLib::TBoolean bSelVal = kCIDLib::False;
            const TCQCFldLimit* pfldlTest = TCQCFldLimit::pfldlMakeNew(flddCur);
            TJanitor<const TCQCFldLimit> janLimit(pfldlTest);
            if ((flddCur.eType() == tCQCKit::EFldTypes::Boolean)
            ||  pfldlTest->bIsA(TCQCFldEnumLimit::clsThis()))
            {
                bSelVal = kCIDLib::True;
            }

            // If writeable, set the select column that they can use to set the value
            if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Write))
            {
                if (bSelVal)
                    colCols[CQCAdmin_DrvMonTab::c4Col_SetValue] = m_strDropDownVal;
                else
                    colCols[CQCAdmin_DrvMonTab::c4Col_SetValue] = m_strPopupVal;
            }
             else
            {
                colCols[CQCAdmin_DrvMonTab::c4Col_SetValue] = TString::strEmpty();
            }

            //
            //  See if this guy has a trigger associated with it. The list is sorted
            //  so we can do a binary search. We add some spaces to keep the values
            //  long enough to keep the header fully visible.
            //
            if (cqcdcDrv.pfetFind(flddCur.strName()))
                colCols[CQCAdmin_DrvMonTab::c4Col_Trigger] = L"Yes";
            else
                colCols[CQCAdmin_DrvMonTab::c4Col_Trigger].Clear();

            //
            //  Set the row id to the original field index. For now the value column
            //  is left empty. It'll get updated on the first poll. Set the flag on
            //  this row if it supports interactive value setting.
            //
            const tCIDLib::TCard4 c4At = m_pwndFldList->c4AddItem(colCols, c4Index);
            if (bSelVal)
                m_pwndFldList->SetFlagAt(c4At, bSelVal);
        }

        //
        //  Force the non-value columns to size to fit now that we are loaded. The last
        //  column is auto-sized. The name and type want to size to contents. The access,
        //  trigger, and set value columns we want them to use the header width. The
        //  semantic type we will size to the header if all of them are generic (which
        //  we don't display) else we size to the displayed sem types.
        //
        m_pwndFldList->AutoSizeCol(0, kCIDLib::False);
        m_pwndFldList->AutoSizeCol(1, kCIDLib::False);
        m_pwndFldList->AutoSizeCol(2, kCIDLib::True);
        m_pwndFldList->AutoSizeCol(3, !bHaveSemTypes);
        m_pwndFldList->AutoSizeCol(4, kCIDLib::True);
        m_pwndFldList->AutoSizeCol(5, kCIDLib::True);

        // If we got any fields, force an initial selection
        if (m_pwndFldList->c4ItemCount())
            m_pwndFldList->SelectByIndex(0, kCIDLib::True);

        // Init the current driver state display
        m_pwndStatus->strWndText(tCQCKit::strXlatEDrvStates(m_eDrvState));

        //
        //  Now do an initial poll pass to get things started and get the engine
        //  starting to poll our fields. So by the next time poll is called, the
        //  values should all be available.
        //
        //  Indicate it's the first poll, so current field values will be forced
        //  to be shown, even if they aren't changed from a non-existent previous
        //  time.
        //
        Poll(kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        if (facCQCAdmin.bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// Called when the user clicks on the value column of a writeable field
tCIDLib::TVoid
TDrvMonTab::DoIPEdit(const tCIDLib::TCard4 c4ListIndex, const tCIDLib::TCard4 c4FldIndex)
{
    const TDrvMonTabPFI& pfiCur = *m_colFldList[c4FldIndex];

    //
    //  Get the current value to pass in as the iniital value. IF the field is write
    //  only, then it'll be empty.
    //
    TString strVal;
    m_pwndFldList->QueryColText(c4ListIndex, CQCAdmin_DrvMonTab::c4Col_Value, strVal);

    // Convert the field type to an attribute type.
    tCIDMData::EAttrTypes eAttrType;
    switch(pfiCur.m_flddThis.eType())
    {
        case tCQCKit::EFldTypes::Boolean :
            eAttrType = tCIDMData::EAttrTypes::Bool;
            break;

        case tCQCKit::EFldTypes::Card :
            eAttrType = tCIDMData::EAttrTypes::Card;
            break;

        case tCQCKit::EFldTypes::Float :
            eAttrType = tCIDMData::EAttrTypes::Float;
            break;

        case tCQCKit::EFldTypes::Int :
            eAttrType = tCIDMData::EAttrTypes::Int;
            break;

        case tCQCKit::EFldTypes::String :
            eAttrType = tCIDMData::EAttrTypes::String;
            break;

        case tCQCKit::EFldTypes::Time :
            eAttrType = tCIDMData::EAttrTypes::Time ;
            break;

        default :
            // We don't allow it to be edited
            return;
    };

    //
    //  Set up the attribute. We don't worry about limits. We'll use the actual field
    //  limits themselves to validate the value in bIPEValidate(), so as to avoid a
    //  lot of extra work.
    //
    TAttrData adatEdit;
    adatEdit.Set(L"FldVal", L"FldVal", eAttrType);

    // Set the current value as the starting point to edit from
    adatEdit.SetValueFromText(strVal);

    // Get the area of the cell we care about
    TArea areaCell;
    m_pwndFldList->QueryColArea(c4ListIndex, CQCAdmin_DrvMonTab::c4Col_Value, areaCell);

    //
    //  Invoke the in place editor. Set the list index on the IPE interface. Dont' care
    //  about the return. If they committed and it validated, then we will have stored it
    //  in the IPEValChanged() callback.
    //
    c4IPERow(c4ListIndex);
    TInPlaceEdit ipeVal;
    ipeVal.bProcess(*m_pwndFldList, areaCell, adatEdit, *this, 0);
}


//
//  This is called if the user clicks on the ... column and it's for a writeable field
//  row.
//
tCIDLib::TVoid
TDrvMonTab::DoVisEdit(const tCIDLib::TCard4 c4ListIndex, const tCIDLib::TCard4 c4FldIndex)
{
    const TDrvMonTabPFI& pfiCur = *m_colFldList[c4FldIndex];
    const TCQCFldDef& flddCur = pfiCur.m_flddThis;

    // Get the cell area and convert to screen coordinates
    TArea areaCell;
    m_pwndFldList->QueryColArea(c4ListIndex, CQCAdmin_DrvMonTab::c4Col_SetValue, areaCell);
    TArea areaCellScr;
    m_pwndFldList->ToScreenCoordinates(areaCell, areaCellScr);

    //
    //  If it's a boolean or an enumerated field with a reasonable number of values, then
    //  we do a drop down menu. Else we do a popup. During load we set the per row flag
    //  on those that are drop down types.
    //
    if (m_pwndFldList->bFlagAt(c4ListIndex))
    {
        if (flddCur.eType() == tCQCKit::EFldTypes::Boolean)
        {
            TPopupMenu menuSelect(L"bool attr value");
            menuSelect.Create();
            menuSelect.AddActionItem(kCIDLib::c4MaxCard, L"False", L"False", 1000);
            menuSelect.AddActionItem(kCIDLib::c4MaxCard, L"True", L"True", 1001);
            const tCIDLib::TCard4 c4Sel = menuSelect.c4Process
            (
                *this
                , areaCellScr.pntLL()
                , tCIDLib::EVJustify::Top
                , tCIDLib::EHJustify::Left
            );

            if (c4Sel)
                WriteFld(flddCur.strName(), facCQCKit().strBoolVal(c4Sel == 1001));
        }
         else
        {
            const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
            (
                TCQCFldLimit::pfldlMakeNew(flddCur)
            );
            TJanitor<const TCQCFldEnumLimit> janLimit(pfldlEnum);

            TPopupMenu menuSelect(L"enum attr value");
            menuSelect.Create();
            TString strValName;
            const tCIDLib::TCard4 c4ValCnt = pfldlEnum->c4ValCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
            {
                strValName = L"Value";
                strValName.AppendFormatted(c4Index);
                menuSelect.AddActionItem
                (
                    kCIDLib::c4MaxCard, strValName, pfldlEnum->strValueAt(c4Index), 1000 + c4Index
                );
            }
            const tCIDLib::TCard4 c4Sel = menuSelect.c4Process
            (
                *this
                , areaCellScr.pntLL()
                , tCIDLib::EVJustify::Top
                , tCIDLib::EHJustify::Left
            );

            if (c4Sel)
                WriteFld(flddCur.strName(), pfldlEnum->strValueAt(c4Sel - 1000));
        }
    }
     else
    {
        //
        //  Not a special case so pass it to the generic field value getter dialog. Get
        //  the current value as the initial value.
        //
        TString strNewVal;
        TString strOrgVal;
        m_pwndFldList->QueryColText(c4ListIndex, CQCAdmin_DrvMonTab::c4Col_Value, strOrgVal);
        const tCIDLib::TBoolean bRes = facCQCGKit().bQueryFldValue
        (
            *this, flddCur, m_strMoniker, areaCellScr.pntUR(), strNewVal, strOrgVal
        );

        if (bRes)
            WriteFld(flddCur.strName(), strNewVal);
    }
}


tCIDCtrls::EEvResponses TDrvMonTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_ClearTrigs)
    {
        RemoveAllTrigs();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_CopyVal)
    {
        // Get the list window index and use that to get the value column text
        tCIDLib::TCard4 c4Index = m_pwndFldList->c4CurItem();
        TString strValue;
        m_pwndFldList->QueryColText(c4Index, CQCAdmin_DrvMonTab::c4Col_Value, strValue);

        // Now convert that to the original field list index and get the access
        c4Index = m_pwndFldList->c4IndexToId(c4Index);

        // If it's readable, then we can do it
        if (m_colFldList[c4Index]->m_flddThis.bIsReadable())
        {
            try
            {
                TGUIClipboard gclipCopy(*this);
                gclipCopy.Clear();
                gclipCopy.StoreText(strValue);
            }

            catch(TError& errToCatch)
            {
                facCQCGKit().ShowError
                (
                    *this
                    , L"An error occurred while trying to store the value to the clipbard"
                    , errToCatch
                );
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_DriverInfo)
    {
        facCQCGKit().ShowDriverInfo(*this, m_strMoniker, facCQCAdmin.sectUser());
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_EditTrig)
    {
        EditTrigger();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TDrvMonTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_Verbosity)
    {
        // Update the drivers verbosity level to match the one we got
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            try
            {
                // If the polling engine already has a proxy, use it, else create a temp one
                TCQCPollEngine& polleToUse = facCQCAdmin.polleToUse();
                tCQCKit::TCQCSrvProxy orbcSrv;
                if (!polleToUse.bGetSrvProxy(m_strHost, orbcSrv))
                    orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

                tCQCKit::EVerboseLvls eToSet = tCQCKit::EVerboseLvls(wnotEvent.c4Index());
                orbcSrv->SetVerboseLevel(m_strMoniker, eToSet);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TExceptDlg dlgErr
                (
                    *this
                    , strWndText()
                    , L"The verbosity level couldn't be set"
                    , errToCatch
                );
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_DrvMon_FldList)
    {
        // Update any stuff that depends on the selected field
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndLimits->ClearText();

            // Disable the buttons that do something on the current field
            m_pwndClearTrigs->SetEnable(kCIDLib::False);
            m_pwndCopyVal->SetEnable(kCIDLib::False);
            m_pwndEditTrig->SetEnable(kCIDLib::False);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            const tCIDLib::TCard4 c4Index = m_pwndFldList->c4IndexToId(wnotEvent.c4Index());
            const TCQCFldDef& flddCur = m_colFldList[c4Index]->m_flddThis;

            // Update the limits string
            m_pwndLimits->strWndText(flddCur.strLimits());

            //
            //  Enable/disable some buttons based on the field attributes. Trigger stuff
            //  is only doable on readable fields. Same for copying the value.
            //
            m_pwndClearTrigs->SetEnable(flddCur.bIsReadable());
            m_pwndEditTrig->SetEnable(flddCur.bIsReadable());
            m_pwndCopyVal->SetEnable(flddCur.bIsReadable());
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::ColClicked)
        {
            //
            //  If not on the already selected row, or on one of the columns we care
            //  about, then ignore.
            //
            const tCIDLib::TCard4 c4ListInd = wnotEvent.c4Index();
            const tCIDLib::TCard4 c4ColInd  = wnotEvent.c4ColIndex();
            if ((m_pwndFldList->c4CurItem() != c4ListInd)
            ||  ((c4ColInd != CQCAdmin_DrvMonTab::c4Col_SetValue)
            &&   (c4ColInd != CQCAdmin_DrvMonTab::c4Col_Value)))
            {
                return tCIDCtrls::EEvResponses::Handled;
            }

            // If the driver is not in connected state, then not
            tCQCKit::EDrvStates eCurState;
            if (!facCQCAdmin.polleToUse().bCheckDrvState(m_strMoniker, eCurState)
            ||  (eCurState != tCQCKit::EDrvStates::Connected))
            {
                return tCIDCtrls::EEvResponses::Handled;
            }

            // Get the original index and get the field def
            const tCIDLib::TCard4 c4FldInd = m_pwndFldList->c4IndexToId(c4ListInd);
            const TDrvMonTabPFI& pfiCur = *m_colFldList[c4FldInd];

            // If not writeable, then ignore it
            if (!tCIDLib::bAllBitsOn(pfiCur.m_flddThis.eAccess(), tCQCKit::EFldAccess::Write))
                return tCIDCtrls::EEvResponses::Handled;

            // OK, based on the column clicked, let's either visual edit or in place edit
            if (c4ColInd == CQCAdmin_DrvMonTab::c4Col_SetValue)
                DoVisEdit(c4ListInd, c4FldInd);
            else if (c4ColInd == CQCAdmin_DrvMonTab::c4Col_Value)
                DoIPEdit(c4ListInd, c4FldInd);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called if a user asks to edit the trigger on a field. This is only viable
//  on readable fields since the trigger happens in response to the field value
//  changing.
//
tCIDLib::TVoid TDrvMonTab::EditTrigger()
{
    // Get the selected field def
    const tCIDLib::TCard4 c4ListIndex = m_pwndFldList->c4CurItem();
    const tCIDLib::TCard4 c4FldIndex = m_pwndFldList->c4IndexToId(c4ListIndex);
    const TCQCFldDef& flddEdit = m_colFldList[c4FldIndex]->m_flddThis;
    const TString& strFldName = flddEdit.strName();

    //
    //  Now we have to query the trigger for this field so that we can edit it. So
    //  we get the driver object config since that's where they are.
    //
    TCQCFldEvTrigger fetEdit;
    TCQCDriverObjCfg cqcdcDrv;
    try
    {
        TString strHost;
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4Ver = 0;
        if (!orbcIS->bDrvCfgQueryConfig(c4Ver, m_strMoniker, strHost, cqcdcDrv, facCQCAdmin.sectUser()))
        {
            CIDAssert2(L"Could get the driver configuration")
        }

        //
        //  We need to get rid of any potentially invalidated field triggers. Field names
        //  could have changed, and we don't want invalid ones hanging around. The server
        //  will have cleaned his triggers up, so we query the live driver config as
        //  well and copy over his triggers.
        //
        //  If for some reason we can't get it, just go ahead. We'll try again next time.
        //
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);
        TCQCDriverObjCfg cqcdcLive;
        if (orbcSrv->bQueryDriverInfo(m_strMoniker, cqcdcLive))
            cqcdcDrv.SetTriggers(cqcdcLive.colTriggers());

        // If we already have a trigger for this field, start with that, else create one
        const TCQCFldEvTrigger* pfetCur = cqcdcDrv.pfetFind(strFldName);
        if (pfetCur)
            fetEdit = *pfetCur;
        else
            fetEdit.strFldName(strFldName);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_QueryTrigger)
            , errToCatch
        );
        return;
    }

    // OK, let's invoke the trigger editing dialog
    TEdFldTriggerDlg dlgEdit;
    if (dlgEdit.bRunDlg(*this, m_strMoniker, flddEdit, fetEdit))
    {
        // They saved the changes so we need to update
        try
        {
            // Set this trigger on the config we read in above
            cqcdcDrv.SetTrigger(fetEdit);

            // And let's write it back to the server
            TString strTmpHost(m_strHost);
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
            tCIDLib::TCard4 c4NewVersion;
            orbcIS->DrvCfgSetConfig
            (
                m_strMoniker
                , m_strHost
                , cqcdcDrv
                , kCIDLib::False
                , c4NewVersion
                , facCQCAdmin.sectUser()
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_SetTrigger)
                , errToCatch
            );
            return;
        }

        //
        //  Now we we have to tell the CQCServer hosting the driver to update the
        //  driver's triggers from the info we stored above.
        //
        try
        {
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);
            orbcSrv->UpdateTriggers(m_strMoniker, facCQCAdmin.sectUser());
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_UpdateTriggers)
                , errToCatch
            );
            return;
        }

        // Update the trigger column to reflect the result
        m_pwndFldList->SetColText
        (
            c4ListIndex
            , CQCAdmin_DrvMonTab::c4Col_Trigger
            , (cqcdcDrv.pfetFind(strFldName) != nullptr) ? L"Yes" : TString::strEmpty()
        );

        // Indicate it's done
        TOkBox msgbDone(facCQCAdmin.strMsg(kCQCAMsgs::midStatus_TrigsUpdated, m_strMoniker));
        msgbDone.ShowIt(*this);
    }
}


//
//  This is called periodically to poll our fields and see if any have changed value
//  or state, or if the driver itself has changed state. We update appropriately based
//  on any changes.
//
//  If this is the first time, we update the displayed field values even if there's
//  no change, since we need to get the initial values shown.
//
tCIDLib::TVoid TDrvMonTab::Poll(const tCIDLib::TBoolean bFirst)
{
    try
    {
        TCQCPollEngine& polleToUse = facCQCAdmin.polleToUse();

        //
        //  FIrst see if the driver has changed in some way that requires us to
        //  resync with him.
        //
        if (polleToUse.bCheckDrvChange(m_strMoniker, m_c4FldListId, m_c4DriverId, m_c4DrvListId))
        {
            // Clear the field list
            m_pwndFldList->RemoveAll();

            // Force us back to not connected state
            m_eDrvState = tCQCKit::EDrvStates::NotLoaded;
            m_pwndStatus->strWndText(tCQCKit::strXlatEDrvStates(m_eDrvState));
            return;
        }

        // We are in sync ok, so check the state of the driver
        tCQCKit::EDrvStates eNewState;
        if (!facCQCAdmin.polleToUse().bCheckDrvState(m_strMoniker, eNewState))
            eNewState = tCQCKit::EDrvStates::NotLoaded;

        // If the state changed, then update the driver status
        tCIDLib::TBoolean bChanged(eNewState != m_eDrvState);
        if (bChanged)
        {
            m_eDrvState = eNewState;
            m_pwndStatus->strWndText(tCQCKit::strXlatEDrvStates(m_eDrvState));
        }

        //
        //  If we are connected, then poll the fields. If not, and we changed to
        //  offline state, mark them all bad.
        //
        const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
        if (m_eDrvState == tCQCKit::EDrvStates::Connected)
        {
            TString strClipped;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TDrvMonTabPFI& pfiCur = *m_colFldList[c4Index];

                if (pfiCur.bUpdate(polleToUse) || bFirst)
                {
                    //
                    //  Or original index in the id in the field list rows, so convert
                    //  our index to the index in the list, given the current sort order.
                    //
                    //  In some cases the values may be long, such as for a stringlist field
                    //  so we will check that and aload a shorter version if it's overly
                    //  long.
                    //
                    const tCIDLib::TCard4 c4At = m_pwndFldList->c4IdToIndex(c4Index);

                    const TString* pstrLoad = &pfiCur.m_strLastVal;
                    if (pfiCur.m_strLastVal.c4Length() > 256)
                    {
                        strClipped = pfiCur.m_strLastVal;
                        strClipped.CapAt(256);
                        strClipped.Append(L"...");

                        pstrLoad = &strClipped;
                    }
                    m_pwndFldList->SetColText
                    (
                        c4At, CQCAdmin_DrvMonTab::c4Col_Value, *pstrLoad
                    );

                    // If it's the verbosity field, update our combo box
                    if (pfiCur.m_flddThis.strName() == m_strVerboseFldName)
                    {
                        tCIDLib::TCard4 c4Dummy;
                        m_pwndVerbosity->bSelectByText(pfiCur.m_strLastVal, c4Dummy);
                    }
                }
            }
        }
         else if (bChanged)
        {
            //
            //  We aren't connected but the driver state changed, so we must have
            //  transitioned to not connected. So set all of the writeable fields
            //  to show an 'unknown' value indicator.
            //
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const tCIDLib::TCard4 c4At = m_pwndFldList->c4IndexToId(c4Index);
                if (m_colFldList[c4At]->m_flddThis.bIsReadable())
                {
                    m_pwndFldList->SetColText
                    (
                        c4Index, CQCAdmin_DrvMonTab::c4Col_Value, L"????"
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCAdmin.bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  This is called when the the user asks to remove all triggers from this driver.
//  We get a client proxy and ask the driver to remove them, then we go through and
//  make sure the triggers column is cleared on all the rows.
//
tCIDLib::TVoid TDrvMonTab::RemoveAllTrigs()
{
    // Make sure they want to do it
    TYesNoBox msgbConfirm
    (
        strWndText()
        , L"Are you sur eyou want to remove all triggers from this driver?"
    );

    // If not, give up now
    if (!msgbConfirm.bShowIt(*this))
        return;

    TWndPaintJanitor janPaint(this);

    // Query the driver config for this driver and remove all the triggers
    TCQCDriverObjCfg cqcdcDrv;
    try
    {
        TString strTmpHost(m_strHost);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4Ver = 0;
        if (!orbcIS->bDrvCfgQueryConfig(c4Ver, m_strMoniker, strTmpHost, cqcdcDrv, facCQCAdmin.sectUser()))
        {
            CIDAssert2(L"Could get the driver configuration")
        }
        cqcdcDrv.RemoveAllTriggers();

        // Write it back now
        tCIDLib::TCard4 c4NewVersion;
        orbcIS->DrvCfgSetConfig
        (
            m_strMoniker
            , m_strHost
            , cqcdcDrv
            , kCIDLib::False
            , c4NewVersion
            , facCQCAdmin.sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_RemoveAllTrigs), errToCatch
        );
        return;
    }

    // Now we need to ask the driver to update the driver's triggers
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);
        orbcSrv->UpdateTriggers(m_strMoniker, facCQCAdmin.sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this, facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_UpdateTriggers), errToCatch
        );
        return;
    }

    // Clear the trigger column for all fields
    m_pwndFldList->SetColTo(CQCAdmin_DrvMonTab::c4Col_Trigger, TString::strEmpty());

    // Indicate it's done
    TOkBox msgbDone(facCQCAdmin.strMsg(kCQCAMsgs::midStatus_TrigsUpdated, m_strMoniker));
    msgbDone.ShowIt(*this);
}


// A helper to write a field value, which we have to do from a couple places
tCIDLib::TVoid
TDrvMonTab::WriteFld(const TString& strFldName, const TString& strValue)
{
    //
    //  Try to write the new value. The window list column has already been updated.
    //  So, if this fails, we need to make sure to put back the current value from
    //  the latest polled info. We won't get any change back from the driver since
    //  the value wasn't successfully written.
    //
    try
    {
        // Get a client proxy we can use to write the value
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker));

        orbcAdmin->WriteFieldByName
        (
            m_strMoniker
            , strFldName
            , strValue
            , facCQCAdmin.sectUser()
            , tCQCKit::EDrvCmdWaits::Wait
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr
        (
            facCQCAdmin.strMsg(kCQCAMsgs::midTab_DrvMon_Title)
            , L"The value couldn't be written to the field. See the logs for more info..."
        );
        msgbErr.ShowIt(*this);
    }
}
