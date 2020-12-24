//
// FILE NAME: CppDrvTest_SrvDrvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/27/2018
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
//  This file implements the tab window that exposes the server side driver.
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
#include "CppDrvTest.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TSrvDrvTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TSrvDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
TSrvDrvTab::TSrvDrvTab() :

    TTabWindow(L"Server Driver", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_bLastRunState(kCIDLib::False)
    , m_bLastSessState(kCIDLib::False)
    , m_c4FieldListId(0)
    , m_eLastState(tCQCKit::EDrvStates::Count)
    , m_pwndClient(nullptr)
    , m_pwndFields(nullptr)
    , m_pwndLimits(nullptr)
    , m_pwndState(nullptr)
    , m_strEllipsis(L"...")
    , m_strError(L"???")
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TSrvDrvTab::~TSrvDrvTab()
{
}

// ---------------------------------------------------------------------------
//  TSrvDrvTab: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't attempt to set any of the field's limits on the attribute. That would be
//  a lot of work. So we just set the attribute to the basic type and we use the actual
//  field limits to do the validation (beyond it being basically valid for the type
//  which our base class will handle.)
//
tCIDLib::TBoolean
TSrvDrvTab::bIPEValidate(const  TString&            strSrc
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
        const tCIDLib::TCard4 c4Index = m_pwndFields->c4IndexToId(c4IPERow());
        const TCQCFldDef& flddCur = m_colFields[c4Index];

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
TSrvDrvTab::IPEValChanged(  const   tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    // Try to write the new value
    const tCIDLib::TCard4 c4Index = m_pwndFields->c4IndexToId(c4IPERow());
    const TCQCFldDef& flddCur = m_colFields[c4Index];

    // Get the attribute to format it out to text for us
    TString strToWrite;
    adatNew.FormatToText(strToWrite);

    if (facCppDrvTest.bWriteField(m_c4FieldListId, flddCur.c4Id(), strToWrite))
    {
        // If write only, don't keep the entered text
        if (flddCur.eAccess() == tCQCKit::EFldAccess::Write)
            m_pwndFields->SetColText(c4Index, c4Col_Value, TString::strEmpty());

        //
        //  On the assumption that it's going to stick, update the underlying column with
        //  the entered column so that it doesn't flash back to the previous value and then
        //  update on the next poll.
        //
        m_pwndFields->SetColText(c4IPERow(), c4Col_Value, strToWrite);
    }
}


// ---------------------------------------------------------------------------
//  TSrvDrvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

// The main window calls us here when it starts or stops the driver
tCIDLib::TVoid TSrvDrvTab::DriverStartStop(const tCIDLib::TBoolean bNewState)
{
    if (bNewState != m_bLastRunState)
    {
        m_bLastRunState = bNewState;

        if (bNewState)
        {
            // Get the driver's fields and load them up
            LoadFields();

            // Start our update timer
            m_tmidUpdate = tmidStartTimer(500);
        }
         else
        {
            // Stop our update timer if it is running
            if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidUpdate);
                m_tmidUpdate = kCIDCtrls::tmidInvalid;
            }

            // Clear the field list
            m_pwndFields->RemoveAll();

            // Update the driver state
            m_pwndState->ClearText();

            // Reset any per-run data
            m_eLastState = tCQCKit::EDrvStates::Count;
        }
    }
}


// The main window lets us know when the session starts or stops
tCIDLib::TVoid TSrvDrvTab::SessionStartStop(const tCIDLib::TBoolean bNewState)
{
    if (bNewState != m_bLastSessState)
    {
        m_bLastSessState = bNewState;

        //
        //  Session stop also implies driver stop. That should have already been done
        //  but just in case.
        //
        if (!bNewState)
            DriverStartStop(bNewState);
    }
}


// ---------------------------------------------------------------------------
//  TSrvDrvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TSrvDrvTab::AreaChanged(const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged
    (
        areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged
    );

    // Keep our content sized to fit
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndClient)
        m_pwndClient->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TSrvDrvTab::bCreated()
{
    TParent::bCreated();

    // Load our dialog description that defines our content
    TDlgDesc dlgdChildren;
    facCppDrvTest.bCreateDlgDesc(kCppDrvTest::ridTab_SrvDrv, dlgdChildren);

    //
    //  Create our generic window which will be sized to fit the tab. But initially
    //  set it to the size of the dialog content. So, when we create the controls below
    //  they will have the correct initial relationship to this window.
    //
    m_pwndClient = new TGenericWnd();
    m_pwndClient->CreateGenWnd
    (
        *this
        , dlgdChildren.areaPos()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    //
    //  Now create the children. No need to do an initial sizing since we sized our
    //  own window initially to fit. Tell it to limit minimum size to the original
    //  dialog content size.
    //
    tCIDLib::TCard4 c4InitFocus;
    m_pwndClient->PopulateFromDlg
    (
        dlgdChildren, c4InitFocus, kCIDLib::False, kCIDLib::True
    );

    // Look up the children we need to talk to directly
    CastChildWnd(*m_pwndClient, kCppDrvTest::ridTab_SrvDrv_Fields, m_pwndFields);
    CastChildWnd(*m_pwndClient, kCppDrvTest::ridTab_SrvDrv_Limits, m_pwndLimits);
    CastChildWnd(*m_pwndClient, kCppDrvTest::ridTab_SrvDrv_State, m_pwndState);

    // Set up event handlers
    m_pwndFields->pnothRegisterHandler(this, &TSrvDrvTab::eLBHandler);

    //
    //  Now do an initial size of the client to fit our tab area, which will get
    //  the child widgets sized up to fit correctly for the initial display.
    //
    m_pwndClient->SetSize(areaClient().szArea(), kCIDLib::False);

    // Set up the columns on the list box
    tCIDLib::TStrList colCols(c4Col_Count);
    colCols.objAdd(L"Name");
    colCols.objAdd(L"Fld Type");
    colCols.objAdd(L"Sem Type");
    colCols.objAdd(L"R/W");
    colCols.objAdd(L"...");
    colCols.objAdd(L"Value");
    m_pwndFields->SetColumns(colCols);

    return kCIDLib::True;
}


//
//  We need to clean up our update timer, and stop anything else that we might have
//  going. Since the driver is passive to us, i.e. we just poll it, we don't have
//  any concerns about the driver state.
//
tCIDLib::TVoid TSrvDrvTab::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    // Call our parent last
    TParent::Destroyed();
}


// We need to handle our update timer and keep our driver info up to date
tCIDLib::TVoid TSrvDrvTab::Timer(const tCIDCtrls::TTimerId  tmidSrc)
{
    // If not ours, pass it on and return
    if (tmidSrc != m_tmidUpdate)
    {
        TParent::Timer(tmidSrc);
        return;
    }

    //
    //  Get the driver and make sure it's ther. It should be if the timer is
    //  running but be sure.
    //
    TCQCServerBase* psdrvSrc = facCppDrvTest.psdrvTest();
    if (!psdrvSrc)
        return;

    // Let's do a poll on the driver and update if any changes
    try
    {
        //
        //  Check the driver state for changes first. If it's not in connected state,
        //  no need to poll fields.
        //
        const tCQCKit::EDrvStates eCurState = psdrvSrc->eState();
        if (eCurState != m_eLastState)
        {
            m_eLastState = eCurState;

            // Update our state display
            m_pwndState->strWndText(tCQCKit::strXlatEDrvStates(eCurState));
        }

        if (m_eLastState == tCQCKit::EDrvStates::Connected)
        {
            tCIDLib::TBoolean bResync = kCIDLib::False;
            const tCIDLib::TBoolean bChanges = psdrvSrc->bQueryChangedFlds
            (
                m_c4FieldListId
                , m_fcolSerialNums
                , m_fcolChanged
                , m_fcolErrors
                , m_colValues
                , bResync
            );


            if (bChanges && bResync)
            {
                //
                //  We need to reload our fields. We'll pick up values again on the
                //  next time through.
                //
                LoadFields();
            }
             else if (bChanges)
            {
                for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4FieldCount; c4Index++)
                {
                    // If no change on this one, then skip it
                    if (!m_fcolChanged[c4Index])
                        continue;

                    //
                    //  Find the row for this guy. The row id is the original list
                    //  index. So we map the original index (our loop index) to the
                    //  index within the list.
                    //
                    const tCIDLib::TCard4 c4LIndex = m_pwndFields->c4IdToIndex(c4Index);

                    if (m_fcolErrors[c4Index])
                        m_pwndFields->SetColText(c4LIndex, c4Col_Value, m_strError);
                    else
                        m_pwndFields->SetColText(c4LIndex, c4Col_Value, m_colValues[c4Index]);
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  Stop the timer while we are in here or it will just continue causing the
        //  same error over and over, stacking up popups.
        //
        TTimerJanitor janUpdate(this, m_tmidUpdate);
        TExceptDlg dlgErr
        (
            *this
            , facCppDrvTest.strMsg(kCppDTErrs::errcGen_PollFailure)
            , errToCatch
        );
    }
}


// ---------------------------------------------------------------------------
//  TSrvDrvTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Called when the user clicks on the value column of a writeable field
tCIDLib::TVoid
TSrvDrvTab::DoIPEdit(const tCIDLib::TCard4 c4ListIndex, const tCIDLib::TCard4 c4FldIndex)
{
    // Get the field def for the target field
    const TCQCFldDef& flddEdit = m_colFields[c4FldIndex];

    //
    //  Get the current value to pass in as the iniital value. If the field is write
    //  only, then it'll be empty.
    //
    TString strVal;
    m_pwndFields->QueryColText(c4ListIndex, c4Col_Value, strVal);

    // Convert the field type to an attribute type.
    tCIDMData::EAttrTypes eAttrType;
    switch(flddEdit.eType())
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
    m_pwndFields->QueryColArea(c4ListIndex, c4Col_Value, areaCell);

    //
    //  Invoke the in place editor. Set the list index on the IPE interface. Dont' care
    //  about the return. If they committed and it validated, then we will have stored it
    //  in the IPEValChanged() callback.
    //
    c4IPERow(c4ListIndex);
    TInPlaceEdit ipeVal;
    ipeVal.bProcess(*m_pwndFields, areaCell, adatEdit, *this, 0);
}


tCIDCtrls::EEvResponses TSrvDrvTab::eClickHandler(TButtClickInfo& wnotEvent)
{

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TSrvDrvTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCppDrvTest::ridTab_SrvDrv_Fields)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndLimits->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::ColClicked)
        {
            //
            //  A column was clicked. If it's one of the field setting ones, we need
            //  to check it out.
            //
            const tCIDLib::TCard4 c4ListInd = wnotEvent.c4Index();
            const tCIDLib::TCard4 c4ColInd  = wnotEvent.c4ColIndex();

            // Get the original field index and see if this guy is writeable
            const tCIDLib::TCard4 c4FldIndex = m_pwndFields->c4IndexToId(c4ListInd);
            const TCQCFldDef& flddTar = m_colFields[c4FldIndex];

            if (tCIDLib::bAllBitsOn(flddTar.eAccess(), tCQCKit::EFldAccess::Write))
            {
                // If one of our columns, then it could be something to deal with
                if (c4ColInd == c4Col_Value)
                {
                    // Let's do an in-place edit
                    DoIPEdit(c4ListInd, c4FldIndex);
                }
                 else if (c4ColInd == c4Col_SetValue)
                {
                    // If its' a free form text, let them edit it in a popup
                    if ((flddTar.eType() == tCQCKit::EFldTypes::String)
                    &&  flddTar.strLimits().bIsEmpty())
                    {
                        // <TBD>
                    }
                }
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Update the limits text with this field's limits
            const tCIDLib::TCard4 c4ListInd = wnotEvent.c4Index();
            const tCIDLib::TCard4 c4ColInd  = wnotEvent.c4ColIndex();

            // Get the original field index and see if this guy is writeable
            const tCIDLib::TCard4 c4FldIndex = m_pwndFields->c4IndexToId(c4ListInd);
            const TCQCFldDef& flddTar = m_colFields[c4FldIndex];

            m_pwndLimits->strWndText(flddTar.strLimits());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called when we get a driver start call. We need to query all of the
//  driver's fields and it's ids related to the field list and such, so that we
//  can load up the field list and so we can watch for changes in the field list
//  that might happen during this run (some drivers will re-register fields upon
//  some request from the user or possibly after getting something from the device.)
//
tCIDLib::TVoid TSrvDrvTab::LoadFields()
{
    // Prevent the list from redrawing while we do this to avoid blinkies
    TWndPaintJanitor janUpdate(this);

    // Remove any existing list items
    m_pwndFields->RemoveAll();

    // Shouldn't happen but make sure we do have a driver
    TCQCServerBase* psdrvSrc = facCppDrvTest.psdrvTest();
    if (!psdrvSrc)
        return;

    m_c4FieldCount = psdrvSrc->c4QueryFields(m_colFields, m_c4FieldListId);

    // If we got any fields, we need to set up our polling info
    if (m_c4FieldCount)
    {
        m_colValues.Reallocate(m_c4FieldCount, TString::strEmpty());
        m_fcolSerialNums.Reallocate(m_c4FieldCount, 0);
        m_fcolChanged.Reallocate(m_c4FieldCount, kCIDLib::False);
        m_fcolErrors.Reallocate(m_c4FieldCount, kCIDLib::True);
    }

    //
    //  Let's now load up the rows of the list window. We need a string list with
    //  a slot for each column. We won't actually set the last one (the value column)
    //  here. That will come with the first poll.
    //
    //  For fields that are strings and have no limit, i.e. free form text, we provide
    //  an ellipsis column to get to a popup in case they need to do multi-line text.
    //
    tCIDLib::TStrList colCols;
    colCols.AddXCopies(TString::strEmpty(), c4Col_Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4FieldCount; c4Index++)
    {
        const TCQCFldDef& flddCur = m_colFields[c4Index];

        colCols[c4Col_Name] = flddCur.strName();
        colCols[c4Col_Type] = tCQCKit::strXlatEFldTypes(flddCur.eType());
        colCols[c4Col_SemType] = tCQCKit::strXlatEFldSTypes(flddCur.eSemType());
        colCols[c4Col_Access] = tCQCKit::strXlatEFldAccess(flddCur.eAccess());

        if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Write))
        {
            if ((flddCur.eType() == tCQCKit::EFldTypes::String)
            &&  flddCur.strLimits().bIsEmpty())
            {
                colCols[c4Col_SetValue] = m_strEllipsis;
            }
        }
         else
        {
            colCols[c4Col_SetValue] = TString::strEmpty();
        }

        // If readable, start off with error state display until proven otherwise
        if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
            colCols[c4Col_Value] = m_strError;

        // Set the row id to the index into the field list for a back map
        m_pwndFields->c4AddItem(colCols, c4Index);
    }
}
