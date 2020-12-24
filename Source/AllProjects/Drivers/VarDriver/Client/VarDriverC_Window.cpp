//
// FILE NAME: VarDriverC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This file implements the main window of the variable driver client.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VarDriverC.hpp"



// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TVarDriverCWnd,TCQCDriverClient)


// ---------------------------------------------------------------------------
//   CLASS: TVarDriverCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVarDriverCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TVarDriverCWnd::TVarDriverCWnd( const   TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis, L"TVarDriverCWnd", tCQCKit::EActLevels::Low, cuctxToUse
    )
    , m_c4FldSerialNum(0)
    , m_eFldType(tCQCKit::EFldTypes::Count)
    , m_pwndDataType(nullptr)
    , m_pwndDefButton(nullptr)
    , m_pwndDefValue(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndFldList(nullptr)
    , m_pwndLimits(nullptr)
    , m_pwndImpButton(nullptr)
    , m_pwndPersist(nullptr)
    , m_pwndSetButton(nullptr)
    , m_pwndValue(nullptr)
{
    // Set the format we want to use for time fields
    m_tmFmt.strDefaultFormat(TTime::strCTime());
}

TVarDriverCWnd::~TVarDriverCWnd()
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
tCIDLib::TBoolean TVarDriverCWnd::bChanges() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean TVarDriverCWnd::bSaveChanges(TString&)
{
    // Shouldn't get called but make the compiler happy
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TVarDriverCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TVarDriverCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  Try to get the current configuration from the server side driver. If
    //  it fails, we stay offline.
    //
    try
    {
        // Ask for a buffer with the config data in it
        THeapBuf mbufCfg;
        tCIDLib::TCard4 c4Size;
        orbcTarget->QueryConfig(strMoniker(), c4Size, mbufCfg, sectUser());

        // Create an input stream over the buffer and set the end of stream
        TBinMBufInStream strmSrc(&mbufCfg, c4Size);

        // Stream in our configuration
        strmSrc >> m_cfgData;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    // Indicate we connected ok
    return kCIDLib::True;
}


tCIDLib::TBoolean TVarDriverCWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facVarDriverC().bCreateDlgDesc(kVarDriverC::ridDlg_Main, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to some of our widgets we need to interact with
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_DataType, m_pwndDataType);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_MakeDef, m_pwndDefButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_DefValue, m_pwndDefValue);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_List, m_pwndFldList);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_Edit, m_pwndEditButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_Limits, m_pwndLimits);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_ImportXML, m_pwndImpButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_Persist, m_pwndPersist);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_SetValue, m_pwndSetButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_Main_Value, m_pwndValue);

    // Install event handlers
    m_pwndDefButton->pnothRegisterHandler(this, &TVarDriverCWnd::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TVarDriverCWnd::eClickHandler);
    m_pwndFldList->pnothRegisterHandler(this, &TVarDriverCWnd::eLBHandler);
    m_pwndImpButton->pnothRegisterHandler(this, &TVarDriverCWnd::eClickHandler);
    m_pwndSetButton->pnothRegisterHandler(this, &TVarDriverCWnd::eClickHandler);

    // Set the columns on the field list
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Defined Fields");
    m_pwndFldList->SetColumns(colCols);

    return kCIDLib::True;
}

tCIDLib::TBoolean TVarDriverCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    if (m_strToPoll.bIsEmpty())
    {
        //
        //  There's no field to poll currently, but let's at least check that we can
        //  talk to our server.
        //

        return kCIDLib::True;
    }

    //
    //  We use a backdoor command which will get us both the current value
    //  and the current default value.
    //
    tCIDLib::TCard4 c4OutBytes;
    const tCIDLib::TBoolean bRes = orbcServer()->bQueryData
    (
        strMoniker(), kVarDriverC::pszCfg_CurFldVals, m_strToPoll, c4OutBytes, m_mbufPoll
    );

    // If we got new data, then store it. Else we couldn't get the field value
    if (bRes)
    {
        // Pull the values out
        TBinMBufInStream strmIn(&m_mbufPoll, c4OutBytes);

        TLocker lockrSync(pmtxSync());

        strmIn >> m_strCurVal;
        strmIn.CheckForFrameMarker(CID_FILE, CID_LINE);
        strmIn >> m_strCurDefVal;
        strmIn.CheckForEndMarker(CID_FILE, CID_LINE);

        //
        //  If this is a time based field, then we want to append a second line
        //  to the current value, showing the value formatted out as a time
        //  string as well.
        //
        if (m_eFldType == tCQCKit::EFldTypes::Time)
        {
            m_strCurVal.Append(L"\n");

            tCIDLib::TCard8 c8Val;
            if (m_strCurVal.bToCard8(c8Val, tCIDLib::ERadices::Hex))
            {
                m_tmFmt = c8Val;
                m_tmFmt.FormatToStr(m_strCurVal, tCIDLib::EAppendOver::Append);
            }
             else
            {
                m_strCurVal.Append(L"????");
            }
        }

        // Look up the field in the config
        TCQCFldDef* pflddUpdate = m_cfgData.pflddFind(m_strToPoll);
        if (pflddUpdate)
            pflddUpdate->strExtraCfg(m_strCurDefVal);
    }
     else
    {
        TLocker lockrSync(pmtxSync());
        m_strCurVal = L"???";
        m_strCurDefVal = L"???";
    }

    return bRes;
}


//
//  The stupid multi-column list box won't reliably draw his border. So we draw one
//  ourself.
//
tCIDLib::TBoolean TVarDriverCWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    TParent::bPaint(gdevToUse, areaUpdate);

    TArea areaBorder = m_pwndFldList->areaWnd();
    areaBorder.Inflate(1, 1);
    gdevToUse.Stroke(areaBorder, facCIDGraphDev().rgbDarkGrey);

    return kCIDLib::True;
}


tCIDLib::TVoid TVarDriverCWnd::Connected()
{
    //
    //  Reload our field list just in case it might have somehow changed while we were
    //  not connected.
    //
    LoadList();
}


tCIDLib::TVoid TVarDriverCWnd::DoCleanup()
{
    // We don't really have any
}


tCIDLib::TVoid TVarDriverCWnd::LostConnection()
{
    // Make sure the current field value get's set to question marks
    m_pwndValue->SetText(L"????");
}


tCIDLib::TVoid TVarDriverCWnd::UpdateDisplayData()
{
    TLocker lockrSync(pmtxSync());

    m_pwndValue->strWndText(m_strCurVal);
    m_pwndDefValue->strWndText(m_strCurDefVal);
}


// ---------------------------------------------------------------------------
//  TVarDriverCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TVarDriverCWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    if (wnotEvent.widSource() == kVarDriverC::ridDlg_Main_Edit)
    {
        //
        //  Invoke the edit dialog with a copy of our configuration. We need
        //  to lock while we copy the configuration since the timer is also
        //  accessing it to keep the current field value updated.
        //
        TVarDrvCfg cfgTmp;
        {
            TLocker lockrSync(pmtxSync());
            cfgTmp = m_cfgData;
        }

        // Lock out our update timer during this
        TTimerSusJanitor janTimer(this);

        // Let them edit. If they save, try to upload it to the server side
        TEdFldListDlg dlgEdit;
        if (dlgEdit.bRun(*this, cfgTmp))
            UploadCfg(cfgTmp);
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_Main_ImportXML)
    {
        ImportXML();
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_Main_MakeDef)
    {
        //
        //  Ask the server to make the current value of the current field that field's
        //  default value.
        //
        const tCIDLib::TCard4 c4FldIndex = m_pwndFldList->c4CurItemId();
        if (c4FldIndex != kCIDLib::c4MaxCard)
        {
            // Lock and get the field name, then unlock
            TString strName;
            {
                TLocker lockrSync(pmtxSync());
                strName = m_cfgData.flddAt(c4FldIndex).strName();
            }

            // And send the command to the server
            try
            {
                orbcServer()->c4SendCmd
                (
                    strMoniker(), kVarDriverC::pszCfg_SetDefault, strName, sectUser()
                );

                // Tell them it worked
                TOkBox msgbDone
                (
                    facVarDriverC().strMsg(kVarCMsgs::midStatus_DefaultSet, strName)
                );
                msgbDone.ShowIt(*this);
            }

            catch(const TError& errToCatch)
            {
                // It failed
                TExceptDlg dlgErr
                (
                    *this
                    , strWndText()
                    , facVarDriverC().strMsg(kVarCMsgs::midStatus_DefaultFailed, strName)
                    , errToCatch
                );
            }
        }
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_Main_SetValue)
    {
        const tCIDLib::TCard4 c4FldIndex = m_pwndFldList->c4CurItemId();
        if (c4FldIndex != kCIDLib::c4MaxCard)
        {
            //
            //  Convert our button org to screen coordinates. We'll use this
            //  as the origin of the dialog.
            //
            TPoint pntAt;
            ToScreenCoordinates(m_pwndDefButton->pntOrg(), pntAt);

            // Let the user set the field value
            facCQCGKit().SetFldValue
            (
                *this, m_cfgData.flddAt(c4FldIndex), strMoniker(), pntAt, sectUser()
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TVarDriverCWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  If the list was emptied, then clean out the display values. Else,
    //  update them with the info for the newly selected field.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Reset the field polling serial number
        m_c4FldSerialNum = 0;

        // Get the selected field
        const tCIDLib::TCard4 c4FldIndex = m_pwndFldList->c4CurItemId();
        const TCQCFldDef& flddCur = m_cfgData.flddAt(c4FldIndex);

        // And display it's values
        m_eFldType = flddCur.eType();
        m_pwndDataType->strWndText(tCQCKit::strXlatEFldTypes(m_eFldType));
        m_pwndLimits->strWndText(flddCur.strLimits());
        m_pwndDefValue->strWndText(flddCur.strExtraCfg());

        if (flddCur.bPrivate())
            m_pwndPersist->strWndText(L"Yes");
        else
            m_pwndPersist->strWndText(L"No");

        //
        //  Lock and update the name of the field to poll and clear the value
        //  display until it gets updated by polling. If this is a write only
        //  field, then clear the name and just set the value field to
        //  indicate it's not applicable.
        //
        TLocker lockrSync(pmtxSync());
        if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
        {
            m_pwndValue->ClearText();
            m_strCurVal.Clear();
            m_strToPoll = flddCur.strName();
        }
         else
        {
            m_pwndValue->strWndText(L"[writeonly]");
            m_strCurVal.Clear();
            m_strToPoll.Clear();
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Reset the field polling serial number
        m_c4FldSerialNum = 0;

        m_eFldType = tCQCKit::EFldTypes::Count;
        m_pwndDataType->ClearText();
        m_pwndLimits->ClearText();
        m_pwndDefValue->ClearText();

        // Lock and clear the name of the field to poll and the current polled value
        TLocker lockrSync(pmtxSync());
        m_strCurVal.Clear();
        m_strToPoll.Clear();
        m_pwndValue->ClearText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Let's the user select an XML file that was spit out by the field extractor utility
//   It describes a set of fields and we replace our current configuration with that
//  configuration if we can successfully set it up on the server side.
//
tCIDLib::TVoid TVarDriverCWnd::ImportXML()
{
    static TPathStr pathLastPath
    (
        TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData)
    );

    // Get a file from them
    TPathStr pathSrcFile;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Field Dump", L"*.CQCFldDump"));

        tCIDLib::TStrList colSelList;
        const tCIDLib::TBoolean bSel = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strWndText()
            , pathLastPath
            , colSelList
            , colFileTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::FileMustExist, tCIDCtrls::EFOpenOpts::PathMustExist
              )
        );

        if (!bSel)
            return;

        // Make sure they want to overwrite
        TYesNoBox msgbOver(facVarDriverC().strMsg(kVarCMsgs::midQ_OverwriteConfig));
        if (!msgbOver.bShowIt(*this))
            return;

        pathSrcFile = colSelList[0];

        // Save a new last path
        pathLastPath = pathSrcFile;
        pathLastPath.bRemoveNameExt();
        pathLastPath.bRemoveTrailingSeparator();
    }

    //
    //  Create a temp driver config and ask it to parse itself out of the
    //  XML file.
    //
    TVarDrvCfg cfgTmp;
    try
    {
        cfgTmp.ParseFromXML(pathSrcFile);
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facVarDriverC().strMsg(kVarCErrs::errcImp_ImportFailed)
            , errToCatch
        );
    }

    // Ok, that worked, so let's try to upload it to the server
    UploadCfg(cfgTmp);
}


//
//  Loads up the current list of fields in our configuration member. It's sorted so we
//  store the original field index as the row id so we can map back to it later.
//
tCIDLib::TVoid TVarDriverCWnd::LoadList()
{
    TWndPaintJanitor janList(m_pwndFldList);

    m_pwndFldList->RemoveAll();

    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());
    const tCIDLib::TCard4 c4Count = m_cfgData.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colCols[0] = m_cfgData.flddAt(c4Index).strName();
        m_pwndFldList->c4AddItem(colCols, c4Index);
    }

    if (c4Count)
        m_pwndFldList->SelectByIndex(0, kCIDLib::True);
}


//
//  Tries to upload the passed configuration to the server side driver.
//  if that works, it stores the configuration in our configuration data
//  member as the new configuration and uploads the interface to reflect
//  the new info.
//
tCIDLib::TVoid
TVarDriverCWnd::UploadCfg(const TVarDrvCfg& cfgToUpload)
{
    //
    //  Ok, we need to upload this configuration to the server side
    //  driver. We have to flatten into a memory buffer since this
    //  is a generic method. We actually send a couple of different
    //  commands to him this way, so we stream out a string first
    //  that indicates this is a full config upload, the configuration
    //  data.
    //
    THeapBuf mbufCfg(8192UL);
    TBinMBufOutStream strmOut(&mbufCfg);
    strmOut << TString(kVarDriverC::pszCfg_Full)
            << cfgToUpload << kCIDLib::FlushIt;

    // Ok, lock so we control the proxy and try to send it
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        TLocker lockrSync(pmtxSync());
        orbcServer()->SetConfig
        (
            strMoniker()
            , strmOut.c4CurSize()
            , mbufCfg
            , cuctxToUse().sectUser()
            , tCQCKit::EDrvCmdWaits::Wait
        );

        //
        //  It worked, so save the config as our config. We have to
        //  do this while locked, because the timer thread is hitting
        //  the configuration data.
        //
        m_cfgData = cfgToUpload;

        // And reload our list
        LoadList();

        // Tell them it worked
        TOkBox msgbDone(facVarDriverC().strMsg(kVarCMsgs::midStatus_ConfigSet));
        msgbDone.ShowIt(*this);
    }

    catch(const TError& errToCatch)
    {
        // Tell them it failed
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facVarDriverC().strMsg(kVarCMsgs::midStatus_CantSetConfig)
            , errToCatch
        );
    }
}

