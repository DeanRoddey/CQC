//
// FILE NAME: LutronRA2C_Window.cpp
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
//  This file implements the main window of the Lutron Radio RA2 client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LutronRA2C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TLutronRA2CWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TLutronRA2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLutronRA2CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TLutronRA2CWnd::TLutronRA2CWnd( const   TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis
        , L"TLutronRA2CWnd"
        , tCQCKit::EActLevels::Low
        , cuctxToUse
    )
    , m_pwndAdd(nullptr)
    , m_pwndDel(nullptr)
    , m_pwndDelAll(nullptr)
    , m_pwndList(nullptr)
{
}

TLutronRA2CWnd::~TLutronRA2CWnd()
{
}


// ---------------------------------------------------------------------------
//  TLutronRA2CWnd: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TLutronRA2CWnd::bIPETestCanEdit(const   tCIDCtrls::TWndId   widSrc
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // We don't allow the type to be changed
    if (c4ColInd == 0)
        return kCIDLib::False;

    // Some are type specific
    const tLutronRA2C::EListCols eColInd = tLutronRA2C::EListCols(c4ColInd);
    if (eColInd > tLutronRA2C::EListCols::Name)
    {
        TString strType;
        m_pwndList->QueryColText
        (
            c4IPERow(), tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Type), strType
        );
        tLutronRA2C::EItemTypes eType = tLutronRA2C::eXlatEItemTypes(strType);

        if (eColInd == tLutronRA2C::EListCols::Trigger)
        {
            if ((eType != tLutronRA2C::EItemTypes::Button)
            &&  (eType != tLutronRA2C::EItemTypes::OccSensor)
            &&  (eType != tLutronRA2C::EItemTypes::Switch))
            {
                return kCIDLib::False;
            }
        }
         else if (eColInd == tLutronRA2C::EListCols::Extra)
        {
            if (eType != tLutronRA2C::EItemTypes::Thermo)
                return kCIDLib::False;
        }
    }


    // If we got here, it's editable, so set up the attribute appropriately
    if (eColInd == tLutronRA2C::EListCols::Addr)
    {
        // A required string type
        adatFill.Set
        (
            L"Addr"
            , kLutronRA2C::strAttr_Addr
            , kCIDMData::strAttrLim_Required
            , tCIDMData::EAttrTypes::String
        );
    }
     else if (eColInd == tLutronRA2C::EListCols::Extra)
    {
        // Currently only used on thermos, where it's C or F
        adatFill.Set
        (
            L"Option"
            , kLutronRA2C::strAttr_Name
            , L"Enum: Celcius, Fahrenheit"
            , tCIDMData::EAttrTypes::String
        );
    }
     else if (eColInd == tLutronRA2C::EListCols::Name)
    {
        // It's required so can't be empty
        adatFill.Set
        (
            L"Name"
            , kLutronRA2C::strAttr_Name
            , kCIDMData::strAttrLim_Required
            , tCIDMData::EAttrTypes::String
        );
    }
     else if (eColInd == tLutronRA2C::EListCols::Trigger)
    {
        adatFill.Set
        (
            L"Triggers"
            , kLutronRA2C::strAttr_Trigger
            , L"Enum: No, Yes"
            , tCIDMData::EAttrTypes::String
        );
    }

    // We didn't set the value on the attribute, so he will just grab the current one
    bValueSet = kCIDLib::False;
    return kCIDLib::True;
}


tCIDLib::TBoolean TLutronRA2CWnd::bChanges() const
{
    // Save our content from the list window to a temp list
    TItemList colNew;
    StoreItems(colNew);

    // And return if they are not equal
    return (colNew != m_colItems);
}


tCIDLib::TBoolean
TLutronRA2CWnd::bIPEValidate(const  TString&            strSrc
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
        c4IPERow(), tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Type), strType
    );
    tLutronRA2C::EItemTypes eType = tLutronRA2C::eXlatEItemTypes(strType);

    if (adatVal.strId() == kLutronRA2C::strAttr_Addr)
    {
        tCIDLib::TCard4 c4CompID = 0;
        tCIDLib::TCard4 c4IntID = 0;
        if (!TRA2CItem::bParseAddr(strNewVal, c4IntID, c4CompID, eType, strErrMsg))
            return kCIDLib::False;

        //
        //  Update the attribute with the leading zero version of the addresses we display
        //  since this is what will be used to update the column text.
        //
        TString strAddr;
        TRA2CItem::FormatAddr(c4IntID, c4CompID, strAddr);
        adatVal.SetString(strAddr);

        return kCIDLib::True;
    }
     else if (adatVal.strId() == kLutronRA2C::strAttr_Extra)
    {
        // This one is an enumerated type so the basic checks above should catch any issues
    }
     else if (adatVal.strId() == kLutronRA2C::strAttr_Name)
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
            , tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Name)
            , 0
            , tCIDLib::EMatchTypes::Equals
            , kCIDLib::False
        );

        // If it was found and not at the row being edited, then it's a dup
        if ((c4At != kCIDLib::c4MaxCard) && (c4At != c4IPERow()))
        {
            strErrMsg = L"There is already a unit with that name";
            return kCIDLib::False;
        }
    }
     else if (adatVal.strId() == kLutronRA2C::strAttr_Trigger)
    {
        // This is an enumerated type so the basic checks above should catch any errors
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TLutronRA2CWnd::bSaveChanges(TString& strErrMsg)
{
    // Save our content from the list window to a temp list
    TItemList colNew;
    StoreItems(colNew);

    // We could have an invalid address if they added a new item and never edited it
    const tCIDLib::TCard4 c4Count = colNew.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!colNew[c4Index].bIsValid(strErrMsg))
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
        if (facLutronRA2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = facLutronRA2C().strMsg(kRA2CErrs::errcCfg_UploadFailed);
        kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TLutronRA2CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TLutronRA2CWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facLutronRA2C().bCreateDlgDesc(kLutronRA2C::ridIntf_Main, dlgdMain);

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
    strWndText(facLutronRA2C().strMsg(kRA2CMsgs::midMain_Title));

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kLutronRA2C::ridMain_Add, m_pwndAdd);
    CastChildWnd(*this, kLutronRA2C::ridMain_Del, m_pwndDel);
    CastChildWnd(*this, kLutronRA2C::ridMain_DelAll, m_pwndDelAll);
    CastChildWnd(*this, kLutronRA2C::ridMain_UnitList, m_pwndList);

    // Register our event handlers
    m_pwndAdd->pnothRegisterHandler(this, &TLutronRA2CWnd::eClickHandler);
    m_pwndDel->pnothRegisterHandler(this, &TLutronRA2CWnd::eClickHandler);
    m_pwndDelAll->pnothRegisterHandler(this, &TLutronRA2CWnd::eClickHandler);

    // Set us as the IPE interface on the list window
    m_pwndList->SetAutoIPE(this);

    // Set the column titles based on the list columsn enumeration
    tLutronRA2C::EListCols eCol = tLutronRA2C::EListCols::Min;
    tLutronRA2C::TColStrs colCols(tLutronRA2C::EListCols::Count);
    while (eCol < tLutronRA2C::EListCols::Count)
        colCols.objAdd(tLutronRA2C::strXlatEListCols(eCol++));
    m_pwndList->SetColumns(colCols);

    // Set up the column widths to reasonable defaults per column
    TFontMetrics fmtrDef;
    {
        TGraphWndDev gdevTmp(*this);
        gdevTmp.QueryFontMetrics(fmtrDef);
    }
    eCol = tLutronRA2C::EListCols::Min;
    tCIDLib::TCard4 c4Width;
    while (eCol <= tLutronRA2C::EListCols::Max)
    {
        switch(eCol)
        {
            case tLutronRA2C::EListCols::Type :
                c4Width = 12;
                break;

            case tLutronRA2C::EListCols::Name :
                c4Width = 18;
                break;

            case tLutronRA2C::EListCols::Addr :
                c4Width = 10;
                break;

            case tLutronRA2C::EListCols::Trigger :
                c4Width = 9;
                break;

            default :
                // We don't do Extra it just gets the rest of the available space
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

        eCol++;
    }

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
TLutronRA2CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    try
    {
        tLutronRA2C::EListCols eCol;

        // Remove any current items
        m_colItems.RemoveAll();
        m_pwndList->RemoveAll();

        // Set up a scratch list of strings to use for column values
        tLutronRA2C::TColStrs colTmp(tLutronRA2C::EListCols::Count);
        eCol = tLutronRA2C::EListCols::Min;
        while (eCol <= tLutronRA2C::EListCols::Max)
        {
            colTmp.objAdd(TString::strEmpty());
            eCol++;
        }

        tCIDLib::TCard4 c4ErrLine;
        TString strErr;
        if (bLoadConfig(orbcServer(), m_colItems, strErr, c4ErrLine))
        {
            // Let's load the items to the list
            TRA2CItem itemCur;
            const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                m_colItems[c4Index].AddToList(*m_pwndList, colTmp);

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


tCIDLib::TBoolean TLutronRA2CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We don't show any field data, so there's nothing to do here, but want to make sure
    //  it's still alive.
    //
    return kCIDLib::True;
}


tCIDLib::TVoid TLutronRA2CWnd::Connected()
{
    // Load up our list box data now
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();

}


tCIDLib::TVoid TLutronRA2CWnd::DoCleanup()
{
}

tCIDLib::TVoid TLutronRA2CWnd::LostConnection()
{

}


tCIDLib::TVoid TLutronRA2CWnd::UpdateDisplayData()
{
    // We don't show any field data, so there's nothing to do here
}


// ---------------------------------------------------------------------------
//  TLutronRA2CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the user wants to add a new item. We get the type from the user
//  then insert a new item with default info into the list for them to further edit.
//
tCIDLib::TVoid TLutronRA2CWnd::AddNew()
{
    // First we drop down the type selection menu and get a selection
    TPopupMenu menuType(L"new type selection");
    menuType.Create(facLutronRA2C(), kLutronRA2C::ridMenu_Add);

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

    tLutronRA2C::EItemTypes eType = tLutronRA2C::EItemTypes::Count;
    switch(c4Cmd)
    {
        case kLutronRA2C::ridMenu_Add_Button :
            eType = tLutronRA2C::EItemTypes::Button;
            break;

        case kLutronRA2C::ridMenu_Add_Contact :
            eType = tLutronRA2C::EItemTypes::ContactCl;
            break;

        case kLutronRA2C::ridMenu_Add_Dimmer :
            eType = tLutronRA2C::EItemTypes::Dimmer;
            break;

        case kLutronRA2C::ridMenu_Add_LED :
            eType = tLutronRA2C::EItemTypes::LED;
            break;

        case kLutronRA2C::ridMenu_Add_OccSensor :
            eType = tLutronRA2C::EItemTypes::OccSensor;
            break;

        case kLutronRA2C::ridMenu_Add_Switch:
            eType = tLutronRA2C::EItemTypes::Switch;
            break;

        case kLutronRA2C::ridMenu_Add_Thermo :
            eType = tLutronRA2C::EItemTypes::Thermo;
            break;

        case kLutronRA2C::ridMenu_Add_Fan :
            eType = tLutronRA2C::EItemTypes::Fan;
            break;

        case kLutronRA2C::ridMenu_Add_Shade :
            eType = tLutronRA2C::EItemTypes::Shade;
            break;

        case kLutronRA2C::ridMenu_Add_ShadeGroup :
            eType = tLutronRA2C::EItemTypes::ShadeGroup;
            break;

        default :
            TErrBox msgbErr(L"Unknown item type to add");
            msgbErr.ShowIt(*this);
            return;
    };

    // Find a unique name to give to this one
    tCIDLib::TCard4 c4Index = 1;
    TString strName(L"NewItem_");
    for (; c4Index < 256; c4Index++)
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

    if (c4Index == 256)
    {
        TErrBox msgbErr(L"A unique initial item name could not be generated");
        msgbErr.ShowIt(*this);
        return;
    }

    // OK, looks reasonable so let's add one
    TRA2CItem itemNew(strName, eType);
    itemNew.AddToList(*m_pwndList);
}


//
//  Pulls out the next line of the file, removing any leading/trailing white space and
//  skipping any empty lines.
//
//  If there is an expected prefix, the line must start with that prefix, and it will be
//  removed from the returned value.
//
tCIDLib::TBoolean
TLutronRA2CWnd::bGetCfgLine(        TTextInStream&      strmCfg
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
            facLutronRA2C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kRA2CErrs::errcCfg_ExpectedLn
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


tCIDCtrls::EEvResponses TLutronRA2CWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    try
    {
        // According to the button id, we do the right thing
        const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();

        if (widSrc == kLutronRA2C::ridMain_Add)
            AddNew();
        else if (widSrc == kLutronRA2C::ridMain_Del)
            DeleteCur();
        else if (widSrc == kLutronRA2C::ridMain_DelAll)
            DeleteAll();
    }

    catch(TError& errToCatch)
    {
        if (facLutronRA2C().bShouldLog(errToCatch))
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
TLutronRA2CWnd::bLoadConfig(tCQCKit::TCQCSrvProxy&  orbcSrv
                            , TItemList&            colToFill
                            , TString&              strErr
                            , tCIDLib::TCard4&      c4ErrLine)
{
    c4ErrLine = 0;
    try
    {
        colToFill.RemoveAll();

        //
        //  Ask the driver for the config data. It takes a 'command id'
        //  but we only use this method for one thing, so we just pass
        //  L"Driver". It's ignored on the other side.
        //
        TString strConfig(8UL);
        orbcSrv->bQueryTextVal(strMoniker(), L"Driver", L"Config", strConfig);

        // Make an input stream over the data so we can parse it in
        TTextStringInStream strmIn(&strConfig);
        TString strCurLn;

        bGetCfgLine(strmIn, strCurLn, L"Version=", c4ErrLine);

        // Figure out the version, which has to be the rest of the line
        tCIDLib::TCard4 c4Version = 1;
        if (!strCurLn.bToCard4(c4Version, tCIDLib::ERadices::Dec))
        {
            strErr = L"The file must start with Version=x";
            return kCIDLib::False;
        }

        //
        //  Now loop until done. We go through each item type and for each one, we first
        //  get the count and then the lines.
        //
        tLutronRA2C::EItemTypes eType = tLutronRA2C::EItemTypes::Min;
        TString strExpPref;
        TRA2CItem itemCur;
        while (!strmIn.bEndOfStream())
        {
            // We should see a count for the current type
            strExpPref = tLutronRA2C::strXlatEItemTypes(eType);
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
            strExpPref = tLutronRA2C::strXlatEItemTypes(eType);
            strExpPref.Append(kCIDLib::chEquals);

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ThisCount; c4Index++)
            {
                if (!bGetCfgLine(strmIn, strCurLn, strExpPref, c4ErrLine))
                {
                    strErr = L"Expected ";
                    strErr.Append(strExpPref);
                    return kCIDLib::False;
                }

                if (!itemCur.bParseFromCfgLine(eType, strCurLn, strErr))
                    return kCIDLib::False;

                colToFill.objAdd(itemCur);
            }

            // There may be more so keep going
            eType++;
        }
    }

    catch(const TError& errToCatch)
    {
        if (facLutronRA2C().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , facLutronRA2C().strMsg(kRA2CErrs::errcCfg_CantParseCfg)
            , errToCatch
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
TLutronRA2CWnd::bSendChanges(const TItemList& colToSend, TString& strErrMsg)
{
    // We'll format the data out into a string which we send to the server
    TString strCfg(8192UL);

    {
        TTextStringOutStream strmOut(&strCfg);

        // The version goes first
        strmOut << L"Version=" << kLutronRA2C::c4ConfigVer << kCIDLib::NewLn;

        //
        //  Now we loop though the list. We have to output at least a count line for
        //  each type, so we just loop through the types. For each type, we output any
        //  items we have of that type. The list is sorted by type, but we don't really
        //  depend on that. We just loop through the whole list and find any of the
        //  current type each time.
        //
        TRefVector<const TRA2CItem> colCurType(tCIDLib::EAdoptOpts::NoAdopt);
        tLutronRA2C::EItemTypes eType = tLutronRA2C::EItemTypes::Min;
        const tCIDLib::TCard4 c4Count = colToSend.c4ElemCount();
        while (eType < tLutronRA2C::EItemTypes::Count)
        {
            //
            //  Make one run through and find the ones of this type. Remembering them
            //  in a non-adopting by reference list.
            //
            colCurType.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TRA2CItem& itemCur = colToSend[c4Index];
                if (itemCur.eType() == eType)
                    colCurType.Add(&itemCur);
            }

            // Output the count line
            strmOut << tLutronRA2C::strXlatEItemTypes(eType);
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
        if (orbcServer()->c4SendCmd(strMoniker(), L"TakeChanges", strCfg, sectUser()) != 1)
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
//  Called when the user wants to delete all current items.
//
tCIDLib::TVoid TLutronRA2CWnd::DeleteAll()
{
    // If the list is already empty, nothing to do
    if (!m_pwndList->c4ItemCount())
        return;

    TString strMsg(kRA2CMsgs::midQ_DelAllItems, facLutronRA2C());
    TYesNoBox msgbDel(strMsg);
    if (!msgbDel.bShowIt(*this))
        return;

    // They do, so remove all items
    m_pwndList->RemoveAll();
}



//
//  Called when the user wants to delete the current item.
//
tCIDLib::TVoid TLutronRA2CWnd::DeleteCur()
{
    // See if there is a selected item. If not, nothing to do
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return;

    // Get the name column text and make sure they want to do this
    TString strName;
    m_pwndList->QueryColText
    (
        c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Name), strName
    );
    TString strMsg(kRA2CMsgs::midQ_DelItem, facLutronRA2C(), strName);
    TYesNoBox msgbDel(strMsg);
    if (!msgbDel.bShowIt(*this))
        return;

    // They do, so remove this guy
    m_pwndList->RemoveAt(c4Index);
}


//
//  Iterates the list window and stores all of the items to the caller's list. They
//  are then sorted by type and then name.
//
tCIDLib::TVoid TLutronRA2CWnd::StoreItems(TItemList& colToFill) const
{
    colToFill.RemoveAll();

    TRA2CItem itemCur;
    const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TRA2CItem& itemNew = colToFill.objAdd(itemCur);
        itemNew.ParseFromLB(*m_pwndList, c4Index);
    }

    colToFill.Sort(&TRA2CItem::eComp);
}


