//
// FILE NAME: LutronHWC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This file implements the main window of the Lutron Radio HW client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LutronHWC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TLutronHWCWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TLutronHWCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLutronHWCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TLutronHWCWnd::TLutronHWCWnd(const  TCQCDriverObjCfg&   cqcdcThis
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient(cqcdcThis, L"TLutronHWCWnd", tCQCKit::EActLevels::Low, cuctxToUse)
    , m_pwndAdd(nullptr)
    , m_pwndDel(nullptr)
    , m_pwndDelAll(nullptr)
    , m_pwndList(nullptr)
    , m_pwndStatus(nullptr)
{
}

TLutronHWCWnd::~TLutronHWCWnd()
{
}


// ---------------------------------------------------------------------------
//  TLutronHWCWnd: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TLutronHWCWnd::bIPETestCanEdit( const   tCIDCtrls::TWndId   widSrc
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    const tLutronHWC::EListCols eCol = tLutronHWC::EListCols(c4ColInd);

    // We don't allow the type to be changed
    if (eCol == tLutronHWC::EListCols::Type)
        return kCIDLib::False;

    //
    //  Some are type specific so get the type column and translate. Any of those are
    //  beyond the name column.
    //
    if (eCol == tLutronHWC::EListCols::Number)
    {
        TString strType;
        m_pwndList->QueryColText
        (
            c4IPERow(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Type), strType
        );
        tLutronHWC::EItemTypes eType = tLutronHWC::eXlatEItemTypes(strType);

        if ((eType != tLutronHWC::EItemTypes::Button)
        &&  (eType != tLutronHWC::EItemTypes::LED))
        {
            return kCIDLib::False;
        }
    }

    //
    //  We will set the attribute value ourself since we want to deal with some of them
    //  as other than string type. Otherwise he'll just get the text and set that, which
    //  will fail if the attribute isn't string type.
    //
    bValueSet = kCIDLib::True;

    // Get the column text for use below
    TString strColText;
    m_pwndList->QueryColText(c4IPERow(), tCIDLib::c4EnumOrd(eCol), strColText);

    // If we got here, it's editable, so set up the attribute appropriately
    if (eCol == tLutronHWC::EListCols::Address)
    {
        // A required string type
        adatFill.Set
        (
            tLutronHWC::strXlatEListCols(eCol)
            , kLutronHWC::strAttr_Addr
            , kCIDMData::strAttrLim_Required
            , tCIDMData::EAttrTypes::String
        );
        adatFill.SetString(strColText);
    }
     else if (eCol == tLutronHWC::EListCols::Name)
    {
        // It's required so can't be empty
        adatFill.Set
        (
            tLutronHWC::strXlatEListCols(eCol)
            , kLutronHWC::strAttr_Name
            , kCIDMData::strAttrLim_Required
            , tCIDMData::EAttrTypes::String
        );
        adatFill.SetString(strColText);
    }
     else if (eCol == tLutronHWC::EListCols::Number)
    {
        tCIDLib::TCard4 c4Num;
        if (!strColText.bToCard4(c4Num, tCIDLib::ERadices::Dec))
            c4Num = 1;

        // It's required if editable so can't be empty
        adatFill.Set
        (
            tLutronHWC::strXlatEListCols(eCol)
            , kLutronHWC::strAttr_Number
            , L"Range: 1, 255"
            , tCIDMData::EAttrTypes::Card
        );
        adatFill.SetCard(c4Num);
    }
     else
    {
        TErrBox msgbBad(L"Got an unknown list box colum index in IPE");
        msgbBad.ShowIt(*this);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TLutronHWCWnd::bChanges() const
{
    //
    //  Save our content from the list window to a temp list. If that fails, say we
    //  have changes since obviously something has been edited.
    //
    TString strErrMsg;
    TItemList colNew;
    tCIDLib::TCard4 c4ErrInd;
    if (!bStoreItems(colNew, strErrMsg, c4ErrInd))
    {
        m_pwndList->SelectByIndex(c4ErrInd);
        return kCIDLib::True;
    }

    // Compare the two lists to see if we have changes
    return (colNew != m_colItems);
}


tCIDLib::TBoolean
TLutronHWCWnd::bIPEValidate(const   TString&            strSrc
                            ,       TAttrData&          adatVal
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatVal, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    // Figure out the type for this one
    TString strType;
    m_pwndList->QueryColText
    (
        c4IPERow(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Type), strType
    );
    tLutronHWC::EItemTypes eType = tLutronHWC::eXlatEItemTypes(strType);

    //
    //  Zero the user id. We'll update it if we change the combo address, so that it
    //  will get passed to the value changed callback who can update the window
    //  row user data./
    //
    c8UserId = 0;

    if (adatVal.strId() == kLutronHWC::strAttr_Addr)
    {
        //
        //  We need to both test the entered address for syntactical validity, but also
        //  test for a duplicate addr/number combo. If accepted, it will update the
        //  user data for this row with the new combination value.
        //
        tCIDLib::TCard8 c8Addr;
        tCIDLib::TCard4 c4Bytes;
        TString strNum;
        m_pwndList->QueryColText
        (
            c4IPERow(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Number), strNum
        );
        tCIDLib::TCard1 c1Num;
        if (!bTestDupAddr(eType, strNewVal, strNum, c8Addr, c4Bytes, c1Num, c4IPERow(), strErrMsg))
            return kCIDLib::False;

        // Update the user id parm to the combined address
        c8UserId = THWCItem::c8MakeComboAddr(eType, c8Addr, c1Num);

        //
        //  It's syntactically correct and not a dup so format it back out to insure
        //  consistent format, and store that back on the attribute.
        //
        TString strFmt;
        THWCItem::FormatAddr(c8Addr, c4Bytes, strFmt);
        adatVal.SetString(strFmt);
    }
     else if (adatVal.strId() == kLutronHWC::strAttr_Name)
    {
        // Make sure the name is a valid field name
        if (!facCQCKit().bIsValidFldName(strNewVal, kCIDLib::True))
        {
            strErrMsg = facCQCKit().strMsg(kKitErrs::errcFld_BadNameChars);
            return kCIDLib::False;
        }

        // Make sure it's not a dup
        tCIDLib::TCard4 c4At = m_pwndList->c4FindText
        (
            strNewVal
            , tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Name)
            , 0
            , tCIDLib::EMatchTypes::Equals
            , kCIDLib::False
        );

        //
        //  If it was found and not at the row being edited, then it's a dup if they are
        //  of the same type. Different types can have the same name.
        //
        if ((c4At != kCIDLib::c4MaxCard) && (c4At != c4IPERow()))
        {
            TString strOrgType, strNewType;
            m_pwndList->QueryColText
            (
                c4IPERow(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Type), strOrgType
            );
            m_pwndList->QueryColText
            (
                c4At, tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Type), strNewType
            );

            if (strOrgType.bCompareI(strNewType))
            {
                strErrMsg = L"There is already a unit with that name";
                return kCIDLib::False;
            }
        }
    }
     else if (adatVal.strId() == kLutronHWC::strAttr_Number)
    {
        //
        //  The basic validation will be handled fine by our parent class. But we need
        //  to check for a duplicate in combination with the address.
        //
        TString strAddr;
        m_pwndList->QueryColText
        (
            c4IPERow(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Address), strAddr
        );
        tCIDLib::TCard8 c8Addr;
        tCIDLib::TCard4 c4Bytes;
        tCIDLib::TCard1 c1Num;
        if (!bTestDupAddr(eType, strAddr, strNewVal, c8Addr, c4Bytes, c1Num, c4IPERow(), strErrMsg))
            return kCIDLib::False;

        // Update the user id parm to the combined address
        c8UserId = THWCItem::c8MakeComboAddr(eType, c8Addr, c1Num);
    }
    return kCIDLib::True;
}


// Called when the user asked to save changes If we can we do, else we return an error
tCIDLib::TBoolean TLutronHWCWnd::bSaveChanges(TString& strErrMsg)
{
    //
    //  Save our content from the list window to a temp list. It will be sorted in the
    //  same order as the original. If that fails, we can't save.
    //
    TItemList colNew;
    tCIDLib::TCard4 c4ErrInd;
    if (!bStoreItems(colNew, strErrMsg, c4ErrInd))
    {
        m_pwndList->SelectByIndex(c4ErrInd);
        return kCIDLib::False;
    }

    // If no changes, then nothing to do
    if (colNew == m_colItems)
        return kCIDLib::True;

    //
    //  Let's upload the configuration to the server side driver. If that works, it will
    //  save the passed values as the current values.
    //
    try
    {
        if (!bSendChanges(colNew, strErrMsg))
            return kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        if (facLutronHWC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_UploadFailed);
        kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  If the validation code above changes the address/number combination, it will update
//  the user id value. When we get called here, we'll update the list window row. We
//  can't do in the validation because it's constant. If he doesn't it will be zero.
//
tCIDLib::TVoid
TLutronHWCWnd::IPEValChanged(const  tCIDCtrls::TWndId
                            , const TString&
                            , const TAttrData&
                            , const TAttrData&
                            , const tCIDLib::TCard8     c8UserId)
{
    if (c8UserId)
        m_pwndList->SetUserDataAt(c4IPERow(), c8UserId);
}


// ---------------------------------------------------------------------------
//  TLutronHWCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TLutronHWCWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facLutronHWC().bCreateDlgDesc(kLutronHWC::ridIntf_Main, dlgdMain);

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
    strWndText(facLutronHWC().strMsg(kLHWCMsgs::midMain_Title));

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kLutronHWC::ridMain_Add, m_pwndAdd);
    CastChildWnd(*this, kLutronHWC::ridMain_Del, m_pwndDel);
    CastChildWnd(*this, kLutronHWC::ridMain_DelAll, m_pwndDelAll);
    CastChildWnd(*this, kLutronHWC::ridMain_UnitList, m_pwndList);
    CastChildWnd(*this, kLutronHWC::ridIntf_Main_DrvState, m_pwndStatus);

    // Register our event handlers
    m_pwndAdd->pnothRegisterHandler(this, &TLutronHWCWnd::eClickHandler);
    m_pwndDel->pnothRegisterHandler(this, &TLutronHWCWnd::eClickHandler);
    m_pwndDelAll->pnothRegisterHandler(this, &TLutronHWCWnd::eClickHandler);

    // Set us as the IPE interface on the list window
    m_pwndList->SetAutoIPE(this);

    // Set the column titles based on the list columns enumeration
    tCIDLib::TStrList colCols(tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));
    tCIDLib::ForEachE<tLutronHWC::EListCols>
    (
        [&colCols](const tLutronHWC::EListCols eCol)
        {
            colCols.objAdd(tLutronHWC::strXlatEListCols(eCol));
        }
    );
    m_pwndList->SetColumns(colCols);

    // Set up the column widths to reasonable defaults per column
    TFontMetrics fmtrDef;
    {
        TGraphWndDev gdevTmp(*this);
        gdevTmp.QueryFontMetrics(fmtrDef);
    }
    tCIDLib::ForEachE<tLutronHWC::EListCols>
    (
        [&colCols, this, &fmtrDef](const tLutronHWC::EListCols eCol)
        {
            tCIDLib::TCard4 c4Width;
            switch(eCol)
            {
                case tLutronHWC::EListCols::Type :
                    c4Width = 12;
                    break;

                case tLutronHWC::EListCols::Name :
                    c4Width = 18;
                    break;

                case tLutronHWC::EListCols::Address :
                    c4Width = 10;
                    break;

                case tLutronHWC::EListCols::Number:
                    c4Width = 8;
                    break;

                default :
                    c4Width = 0;
                    break;
            };

            if (c4Width)
            {
                m_pwndList->SetColOpts
                (
                    tCIDLib::c4EnumOrd(eCol), c4Width * fmtrDef.c4AverageWidth()
                );
            }
        }
    );

    // Set the status initially to waiting to connect
    m_pwndStatus->strWndText(L"Waiting to connect");

    return kCIDLib::True;
}


//
//  We get the initial configuration from the server side driver and store away for later
//  loading into the list window. This also is our original values that we can use to
//  check for changes.
//
//  No matter what happens, we return success. If we fail, we just start with no configured
//  items.
//
tCIDLib::TBoolean
TLutronHWCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    try
    {
        // Remove any current items
        m_colItems.RemoveAll();
        m_pwndList->RemoveAll();

        // Set up a scratch list of strings to use for column values
        tCIDLib::TStrList colTmp(tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));
        colTmp.AddXCopies(TString::strEmpty(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));

        tCIDLib::TCard4 c4ErrLine;
        TString strErr;
        if (bLoadConfig(orbcServer(), m_colItems, strErr, c4ErrLine))
        {
            // Let's load the items to the list
            THWCItem itemCur(TString::strEmpty(), tLutronHWC::EItemTypes::Count);
            const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                m_colItems[c4Index].AddToList(*m_pwndList, colTmp, kCIDLib::False);

            // If anything got loaded, select the 0th one
            if (m_pwndList->c4ItemCount())
                m_pwndList->SelectByIndex(0);
        }
         else
        {
            TString strMsg(L"An error occurred parsing the configuration.\r\n  Error=");
            strMsg.Append(strErr);
            strMsg.Append(L"\r\n  Line=");
            strMsg.AppendFormatted(c4ErrLine);

            TErrBox msgbErr(strMsg);
            msgbErr.ShowIt(*this);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgbFail
        (
            *this, L"An exception occurred while loading the configuration.", errToCatch
        );
        m_colItems.RemoveAll();
    }

    catch(...)
    {
        TErrBox msgbErr(L"An exception occurred while loading the configuration.");
        msgbErr.ShowIt(*this);

        m_colItems.RemoveAll();
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TLutronHWCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We don't show any field data, so there's nothing to do here, but want to make sure
    //  it's still alive.
    //
    return kCIDLib::True;
}


tCIDLib::TVoid TLutronHWCWnd::Connected()
{
    // Indicate we are now connected
    m_pwndStatus->strWndText(L"Connected");
}


tCIDLib::TVoid TLutronHWCWnd::DoCleanup()
{
}

tCIDLib::TVoid TLutronHWCWnd::LostConnection()
{
    // Indicate we lost connection
    m_pwndStatus->strWndText(L"Waiting to Connect");
}


tCIDLib::TVoid TLutronHWCWnd::UpdateDisplayData()
{
    // We don't show any field data, so there's nothing to do here
}


// ---------------------------------------------------------------------------
//  TLutronHWCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the user wants to add a new item. We get the type from the user
//  then insert a new item with default info into the list for them to further edit.
//
tCIDLib::TVoid TLutronHWCWnd::AddNew()
{
    // First we drop down the type selection menu and get a selection
    TPopupMenu menuType(L"new type selection");
    menuType.Create(facLutronHWC(), kLutronHWC::ridMenu_Add);

    // Pop it up to the right of the button
    TPoint pntAt(m_pwndAdd->areaWnd().pntUR());
    pntAt.Adjust(4, 0);
    TWindow::wndDesktop().XlatCoordinates(pntAt, *this);

    // And invoke it
    const tCIDLib::TCard4 c4Cmd = menuType.c4Process
    (
        *this, pntAt, tCIDLib::EVJustify::Top, tCIDLib::EHJustify::Left
    );
    if (!c4Cmd)
        return;

    tLutronHWC::EItemTypes eType = tLutronHWC::EItemTypes::Count;
    switch(c4Cmd)
    {
        case kLutronHWC::ridMenu_Add_Button :
            eType = tLutronHWC::EItemTypes::Button;
            break;

        case kLutronHWC::ridMenu_Add_Dimmer :
            eType = tLutronHWC::EItemTypes::Dimmer;
            break;

        case kLutronHWC::ridMenu_Add_LED :
            eType = tLutronHWC::EItemTypes::LED;
            break;

        default :
            TErrBox msgbErr(L"Unknown item type to add");
            msgbErr.ShowIt(*this);
            return;
    };

    // Find a unique name to give to this one
    tCIDLib::TCard4 c4Index = 1;
    TString strName(L"NewItem_");
    for (; c4Index < 2048; c4Index++)
    {
        strName.CapAt(8);
        strName.AppendFormatted(c4Index);

        const tCIDLib::TCard4 c4At = m_pwndList->c4FindText
        (
            strName, 0, 0, tCIDLib::EMatchTypes::Equals, kCIDLib::False
        );

        // If not found, we can use it
        if (c4At == kCIDLib::c4MaxCard)
            break;
    }

    if (c4Index == 2048)
    {
        TErrBox msgbErr(L"A unique initial item name could not be generated");
        msgbErr.ShowIt(*this);
        return;
    }

    // OK, looks reasonable so let's add one. Tell it to select the new one
    THWCItem itemNew(strName, eType);
    itemNew.AddToList(*m_pwndList, kCIDLib::True);
}


//
//  Pulls out the next line of the file, removing any leading/trailing white space and
//  skipping any empty lines.
//
//  If there is an expected prefix, the line must start with that prefix, and it will be
//  removed from the returned value.
//
tCIDLib::TBoolean
TLutronHWCWnd::bGetCfgLine(         TTextInStream&      strmCfg
                            ,       TString&            strToFill
                            , const TString&            strExpPref
                            ,       tCIDLib::TCard4&    c4LineNum)
{
    while (!strmCfg.bEndOfStream())
    {
        c4LineNum++;
        strmCfg >> strToFill;
        strToFill.StripWhitespace();

        // If we get something, break out
        if (!strToFill.bIsEmpty())
            break;
    }

    if (!strExpPref.bIsEmpty())
    {
        if (!strToFill.bStartsWithI(strExpPref))
        {
            facLutronHWC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kLHWCErrs::errcCfg_ExpectedLn
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strExpPref
                , TCardinal(c4LineNum)
            );
        }

        strToFill.Cut(0, strExpPref.c4Length());
        return kCIDLib::True;
    }

    // Never got another non-empty line
    return kCIDLib::False;
}


tCIDCtrls::EEvResponses TLutronHWCWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    try
    {
        // According to the button id, we do the right thing
        const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();

        if (widSrc == kLutronHWC::ridMain_Add)
            AddNew();
        else if (widSrc == kLutronHWC::ridMain_Del)
            DeleteCur();
        else if (widSrc == kLutronHWC::ridMain_DelAll)
            DeleteAll();
    }

    catch(TError& errToCatch)
    {
        if (facLutronHWC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail(*this, L"An exception occurred", errToCatch);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



//
//  Given a CQCServer proxy for the server hosting our driver, we ask it for the config
//  string, and parse it out to fill in the passed list. It will be sorted by type and then
//  name.
//
tCIDLib::TBoolean
TLutronHWCWnd::bLoadConfig( tCQCKit::TCQCSrvProxy&  orbcSrv
                            , TItemList&            colToFill
                            , TString&              strErr
                            , tCIDLib::TCard4&      c4ErrLine)
{
    c4ErrLine = 0;
    try
    {
        tCIDLib::TCard4 c4DroppedItemCnt = 0;
        tCIDLib::TCard4 c4FixedAddrs = 0;
        colToFill.RemoveAll();

        //
        //  Ask the driver for the config data. It takes a 'command id'
        //  but we only use this method for one thing, so we just pass
        //  L"Driver". It's ignored on the other side.
        //
        TString strConfig(8UL);
        if (!orbcSrv->bQueryTextVal(strMoniker(), L"Driver", L"Config", strConfig))
        {
            strErr = L"Could not get configuration data from the driver";
            return kCIDLib::False;
        }

        // Make an input stream over the data so we can parse it in
        TTextStringInStream strmIn(&strConfig);
        TString strCurLn;

        // Just pull in the first line directly which should be the version
        bGetCfgLine(strmIn, strCurLn, L"Version=", c4ErrLine);

        // Figure out the version, which has to be the rest of the line
        tCIDLib::TCard4 c4Version = 1;
        if (!strCurLn.bToCard4(c4Version, tCIDLib::ERadices::Dec))
        {
            strErr = L"The config data must start with Version=x";
            return kCIDLib::False;
        }

        //
        //  Now loop until done. We go through each item type and for each one, we first
        //  get the count and then the lines.
        //
        tLutronHWC::EItemTypes eType = tLutronHWC::EItemTypes::Min;
        TString strExpPref;
        THWCItem itemCur(TString::strEmpty(), tLutronHWC::EItemTypes::Count);
        tCIDLib::TStrList colTokens;
        const TString strTokenSeps(L",");
        TString strToken;
        while (eType < tLutronHWC::EItemTypes::Count)
        {
            // We should see a count for the current type
            strExpPref = tLutronHWC::strXlatEItemTypes(eType);
            strExpPref.Append(L"Cnt=");
            if (!bGetCfgLine(strmIn, strCurLn, strExpPref, c4ErrLine))
            {
                strErr = L"Expected ";
                strErr.Append(strExpPref);
                return kCIDLib::False;
            }

            // The remainder has to be the count. If not, that's fatal
            tCIDLib::TCard4 c4ThisCount;
            if (!strCurLn.bToCard4(c4ThisCount, tCIDLib::ERadices::Dec))
            {
                strErr = L"Expected a number of items to follow";
                return kCIDLib::False;
            }

            // Now we should see that many of this type of object
            strExpPref = tLutronHWC::strXlatEItemTypes(eType);
            strExpPref.Append(kCIDLib::chEquals);

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ThisCount; c4Index++)
            {
                if (!bGetCfgLine(strmIn, strCurLn, strExpPref, c4ErrLine))
                {
                    strErr = L"Expected ";
                    strErr.Append(strExpPref);
                    return kCIDLib::False;
                }

                //
                //  Tokenize it. We always put the type in first so that we have the
                //  same tokens as when we parse out of the list box when going the
                //  other direction.
                //
                colTokens.RemoveAll();
                colTokens.objAdd(tLutronHWC::strXlatEItemTypes(eType));
                TStringTokenizer stokCfg(&strCurLn, strTokenSeps);

                while (stokCfg.bGetNextToken(strToken))
                    colTokens.objAdd(strToken);

                // Has to be at least two (name and address)
                if (colTokens.c4ElemCount() < 2)
                {
                    strErr = L"An item configuration line must have at least two values";
                    return kCIDLib::False;
                }

                // And pass those to our temp item to set it up
                tCIDLib::TBoolean bFixAddrs = kCIDLib::True;
                if (itemCur.bParseFromData(colTokens, strErr, bFixAddrs))
                    colToFill.objAdd(itemCur);
                else
                    c4DroppedItemCnt++;

                if (bFixAddrs)
                    c4FixedAddrs++;
            }

            //
            //  Depending on file version, we may not have them all. Version 1 went up to the
            //  LEDs.
            //
            if ((c4Version == 1) && (eType == tLutronHWC::EItemTypes::LED))
                break;

            // There may be more so keep going
            eType++;
        }

        if (c4DroppedItemCnt || c4FixedAddrs)
        {
            TString strMsg;
            if (c4DroppedItemCnt)
                strMsg.Append(L"Some items were dropped due to invalid configuration");
            if (c4FixedAddrs)
                strMsg.Append(L". Some items had invalid Lutron addresses fixed.");
            TErrBox msgbDropped(strMsg);
            msgbDropped.ShowIt(*this);
        }
    }

    catch(TError& errToCatch)
    {
        if (facLutronHWC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this, facLutronHWC().strMsg(kLHWCErrs::errcCfg_CantParseCfg), errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We format out the configuration data, then get a proxy for the CQCServer that has
//  our server side driver, then send him the changes. If successful, we will copy the
//  passed list to our working list. It's assumed that the passed list has been set up
//  by reading the data in from the list window, and that we know that there are changes
//  that need to be sent.
//
tCIDLib::TBoolean
TLutronHWCWnd::bSendChanges(const TItemList& colToSend, TString& strErrMsg)
{
    // We'll format the data out into a string which we send to the server
    TString strCfg(8192UL);

    {
        TTextStringOutStream strmOut(&strCfg);

        // The version goes first
        strmOut << L"Version=" << kLutronHWC::c4ConfigVer << kCIDLib::NewLn;

        //
        //  Now we loop though the list. We have to output at least a count line for
        //  each type, so we just loop through the types. For each type, we output any
        //  items we have of that type. The list is sorted by type, but we don't really
        //  depend on that. We just loop through the whole list and find any of the
        //  current type each time.
        //
        TRefVector<const THWCItem> colCurType(tCIDLib::EAdoptOpts::NoAdopt);
        tLutronHWC::EItemTypes eType = tLutronHWC::EItemTypes::Min;
        const tCIDLib::TCard4 c4Count = colToSend.c4ElemCount();
        while (eType < tLutronHWC::EItemTypes::Count)
        {
            //
            //  Make one run through and find the ones of this type. Remembering them
            //  in a by reference list.
            //
            colCurType.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const THWCItem& itemCur = colToSend[c4Index];
                if (itemCur.eType() == eType)
                    colCurType.Add(&itemCur);
            }

            // Output the count line
            strmOut << tLutronHWC::strXlatEItemTypes(eType);
            strmOut << L"Cnt=" << colCurType.c4ElemCount() << kCIDLib::NewLn;

            // Now output them all
            const tCIDLib::TCard4 c4CurCnt = colCurType.c4ElemCount();
            for (tCIDLib::TCard4 c4CurInd = 0; c4CurInd < c4CurCnt; c4CurInd++)
                strmOut << *colCurType[c4CurInd] << kCIDLib::NewLn;

            eType++;
        }

        strmOut.Flush();
    }

    // Try to send it to the server side driver
    try
    {
        if (orbcServer()->c4SendCmd(strMoniker(), L"TakeChanges", strCfg, sectUser()) != 0)
        {
            strErrMsg = L"The configuration was not successfully stored";
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        strErrMsg = L"An error occurred: ";
        strErrMsg.Append(errToCatch.strErrText());
        return kCIDLib::False;
    }

    // It worked so make this our active content
    m_colItems = colToSend;
    return kCIDLib::True;
}


//
//  Iterates the list window and stores all of the items to the caller's list. They
//  are then sorted by type and then name. Mostly the data has to be at least semi-
//  legal since we enforce stuff during in-place editing. But, we do have to default
//  values for newly added ones, so there's some possibility of issues there because
//  they never modified the defaults. But there can't be any duplicate names here or
//  non-default invalid or duplicate addresses or numbers. We catch the defaulted address
//  and number values by the fact that they are zero, so we don't have to worry about
//  them as duplicates.
//
tCIDLib::TBoolean
TLutronHWCWnd::bStoreItems( TItemList&          colToFill
                            , TString&          strErrMsg
                            , tCIDLib::TCard4&  c4ErrInd) const
{
    colToFill.RemoveAll();

    tCIDLib::TStrList colTokens(tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));
    THWCItem itemNew(TString::strEmpty(), tLutronHWC::EItemTypes::Count);
    TString strColText;

    //
    //  Use this to watch for duplicate addresses. For each one we build his combined
    //  address, which is the number in the number in the top byte, and the regular
    //  address bytes in the rest. We have to check this combination since they make
    //  up the whole address.
    //
    const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    TFundVector<tCIDLib::TCard8> fcolAddrs(c4Count);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colTokens.RemoveAll();
        tLutronHWC::EListCols eCol = tLutronHWC::EListCols::Min;
        while (eCol < tLutronHWC::EListCols::Count)
        {
            m_pwndList->QueryColText(c4Index, tCIDLib::c4EnumOrd(eCol), strColText);
            colTokens.objAdd(strColText);
            eCol++;
        }

        // Indicate we don't want it to fix up bad addresses here. We want to report them
        tCIDLib::TBoolean bFixAddr = kCIDLib::False;
        if (!itemNew.bParseFromData(colTokens, strErrMsg, bFixAddr))
        {
            c4ErrInd = c4Index;
            return kCIDLib::False;
        }

        tCIDLib::TCard8 c8CombAddr = THWCItem::c8MakeComboAddr
        (
            itemNew.eType(), itemNew.c8Addr(), itemNew.c1Number()
        );
        if (fcolAddrs.bElementPresent(c8CombAddr))
        {
            THWCItem::FormatAddr(itemNew.c8Addr(), itemNew.c4AddrBytes(), strColText);
            if (THWCItem::bTypeNeedsNumber(itemNew.eType()))
            {
                strColText.Append(L". Number=");
                strColText.AppendFormatted(itemNew.c1Number());
            }

            strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_DupAddr, strColText);
            c4ErrInd = c4Index;
            return kCIDLib::False;
        }
        fcolAddrs.c4AddElement(c8CombAddr);

        // Looks ok, so store it
        colToFill.objAdd(itemNew);
    }

    colToFill.Sort(THWCItem::eComp);
    return kCIDLib::True;
}


//
//  For us, a duplicate address involves two list columns, so we have to check both
//  if either is edited. So this code is broken out separately to avoid replication.
//  The combined type/address/number is stored as user data on each row, so we can build
//  a combo address and search for a dup. If we accept the change, we update the user
//  data with the new values.
//
tCIDLib::TBoolean
TLutronHWCWnd::bTestDupAddr(const   tLutronHWC::EItemTypes  eType
                            , const TString&                strAddr
                            , const TString&                strNum
                            ,       tCIDLib::TCard8&        c8Addr
                            ,       tCIDLib::TCard4&        c4AddrBytes
                            ,       tCIDLib::TCard1&        c1Num
                            , const tCIDLib::TCard4         c4ListIndex
                            ,       TString&                strErrMsg) const
{
    // Parse the address first
    if (!THWCItem::bXlatAddr(strAddr, eType, c8Addr, c4AddrBytes, strErrMsg))
        return kCIDLib::False;

    // Test the number if needed, else it's just zero
    if (THWCItem::bTypeNeedsNumber(eType))
    {
        tCIDLib::TCard4 c4Num;
        if (!strNum.bToCard4(c4Num, tCIDLib::ERadices::Dec)
        || (c4Num == 0)
        || (c4Num > 255))
        {
            strErrMsg = L"The number must be from 1 to 255";
            return kCIDLib::False;
        }
        c1Num = tCIDLib::TCard1(c4Num);
    }
     else
    {
        c1Num = 0;
    }

    // Create the combo address and see if it is a dup
    tCIDLib::TCard8 c8ComboAddr = THWCItem::c8MakeComboAddr(eType, c8Addr, c1Num);
    tCIDLib::TCard4 c4Match = m_pwndList->c4FindByUserData(c8ComboAddr, 0);

    //
    //  If we found the original one, which we shouldn't because this should only get
    //  called if something actually changed, search again to see if we find another
    //  one.
    //
    if (c4Match == c4ListIndex)
        c4Match = m_pwndList->c4FindByUserData(c8ComboAddr, c4Match + 1);

    //  If found and not the original item, if so, then it's a dup.
    if ((c4Match != kCIDLib::c4MaxCard) && (c4Match != c4ListIndex))
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_DupAddr);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  Called when the user wants to delete all current items.
//
tCIDLib::TVoid TLutronHWCWnd::DeleteAll()
{
    // If the list is already empty, nothing to do
    if (!m_pwndList->c4ItemCount())
        return;

    TString strMsg(kLHWCMsgs::midQ_DelAllItems, facLutronHWC());
    TYesNoBox msgbDel(strMsg);
    if (!msgbDel.bShowIt(*this))
        return;

    // They do, so remove all items
    m_pwndList->RemoveAll();
}


//
//  Called when the user wants to delete the current item.
//
tCIDLib::TVoid TLutronHWCWnd::DeleteCur()
{
    // See if there is a selected item. If not, nothing to do
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return;

    // Get the name column text and make sure they want to do this
    TString strName;
    m_pwndList->QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Name), strName);
    TString strMsg(kLHWCMsgs::midQ_DelItem, facLutronHWC(), strName);
    TYesNoBox msgbDel(strMsg);
    if (!msgbDel.bShowIt(*this))
        return;

    // They do, so remove this guy
    m_pwndList->RemoveAt(c4Index);
}


