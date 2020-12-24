//
// FILE NAME: IRClient_BlasterTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2003
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
//  This file implements the IR blaster panel.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRBlasterTab,TTabWindow)


// ---------------------------------------------------------------------------
//  CLASS: TIRBlasterTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRBlasterTab: Constructors and destructor
// ---------------------------------------------------------------------------
TIRBlasterTab::TIRBlasterTab(TIRClientWnd* const  pwndDriver) :

    TTabWindow(L"IRBlasterTab", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pathLastExport(TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData))
    , m_pwndBlasterInfo(nullptr)
    , m_pwndCmds(nullptr)
    , m_pwndCreateButton(nullptr)
    , m_pwndDevInfo(nullptr)
    , m_pwndDriver(pwndDriver)
    , m_pwndEditButton(nullptr)
    , m_pwndExportButton(nullptr)
    , m_pwndImportButton(nullptr)
    , m_pwndLoadButton(nullptr)
    , m_pwndModels(nullptr)
    , m_pwndUnloadButton(nullptr)
    , m_pwndZone(nullptr)
{
}

TIRBlasterTab::~TIRBlasterTab()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

// The driver has connected, so we need to update our config data
tCIDLib::TVoid TIRBlasterTab::SetConnected(const TString& strFirmware)
{
    // Load up our current configuration data
    try
    {
        LoadConfigData();
    }

    catch(const TError& errToCatch)
    {
        if (facIRClient().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TErrBox msgbNoKey
        (
            facIRClient().strMsg
            (
                kIRCMsgs::midStatus_ServerNotFound
                , TString(L"blaster")
                , m_pwndDriver->strMoniker()
            )
        );
        msgbNoKey.ShowIt(*this);
    }

    // Just load it into the display area
    m_pwndBlasterInfo->strWndText(strFirmware);
}



// ---------------------------------------------------------------------------
//  TIRBlasterTab: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TIRBlasterTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have created our children, then adjust them
    if (bSizeChanged && m_pwndCmds && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TIRBlasterTab::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facIRClient().bCreateDlgDesc(kIRClient::ridTab_Blaster, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get some typed pointers for our own use
    CastChildWnd(*this, kIRClient::ridBlast_Info, m_pwndBlasterInfo);
    CastChildWnd(*this, kIRClient::ridBlast_Cmds, m_pwndCmds);
    CastChildWnd(*this, kIRClient::ridBlast_CreateModel, m_pwndCreateButton);
    CastChildWnd(*this, kIRClient::ridBlast_DevInfo, m_pwndDevInfo);
    CastChildWnd(*this, kIRClient::ridBlast_Models, m_pwndModels);
    CastChildWnd(*this, kIRClient::ridBlast_EditModel, m_pwndEditButton);
    CastChildWnd(*this, kIRClient::ridBlast_Export, m_pwndExportButton);
    CastChildWnd(*this, kIRClient::ridBlast_Import, m_pwndImportButton);
    CastChildWnd(*this, kIRClient::ridBlast_LoadModel, m_pwndLoadButton);
    CastChildWnd(*this, kIRClient::ridBlast_UnloadModel, m_pwndUnloadButton);
    CastChildWnd(*this, kIRClient::ridBlast_Zone, m_pwndZone);

    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Defined Commands for this Model");
    m_pwndCmds->SetColumns(colCols);

    // Install handlers for the buttons and the device combo box
    m_pwndModels->pnothRegisterHandler(this, &TIRBlasterTab::eComboHandler);
    m_pwndCmds->pnothRegisterHandler(this, &TIRBlasterTab::eLBHandler);

    m_pwndCreateButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);
    m_pwndExportButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);
    m_pwndImportButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);
    m_pwndLoadButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);
    m_pwndUnloadButton->pnothRegisterHandler(this, &TIRBlasterTab::eClickHandler);

    //
    //  If the device doesn't support blasting, only learning, then we need to
    //  disable the zone selector.
    //
    if (!tCIDLib::bAllBitsOn(m_pwndDriver->eDevCaps(), tCQCIR::EIRDevCaps::Blast))
        m_pwndZone->SetEnable(kCIDLib::False);

    //
    //  Power users can load existing protocol files, but cannot create,
    //  edit or unload any. Below power users only the invoke command button
    //  is available.
    //
    if (m_pwndDriver->eUserRole() < tCQCKit::EUserRoles::PowerUser)
    {
        m_pwndEditButton->SetEnable(kCIDLib::False);
        m_pwndLoadButton->SetEnable(kCIDLib::False);
    }
    if (m_pwndDriver->eUserRole() < tCQCKit::EUserRoles::SystemAdmin)
    {
        m_pwndCreateButton->SetEnable(kCIDLib::False);
        m_pwndUnloadButton->SetEnable(kCIDLib::False);
    }

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TIRBlasterTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called to get a file name for import/export.
//
tCIDLib::TBoolean
TIRBlasterTab::bGetInpExpFile(          TPathStr&               pathToFill
                                , const TString&                strTitle
                                , const tCIDLib::TBoolean       bImport)
{
    // Set up a file types collection to set
    tCIDLib::TKVPList colFileTypes(1);
    colFileTypes.objAdd(TKeyValuePair(L"CQC IR Export", L"*.IRExport"));

    tCIDLib::TStrList colSel;

    tCIDLib::TBoolean bSelected;
    if (bImport)
    {
        pathToFill = m_pathLastExport;
        bSelected = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strTitle
            , pathToFill
            , colSel
            , colFileTypes
            , tCIDCtrls::EFOpenOpts::FileMustExist
        );

        // Store the one selection if there was one
        if (bSelected)
            pathToFill = colSel[0];
    }
     else
    {
        // Get the name of the current model as the default file name
        TString strCurModel;
        m_pwndModels->bQueryCurVal(strCurModel);

        bSelected = facCIDCtrls().bSaveFileDlg
        (
            *this
            , strTitle
            , m_pathLastExport
            , strCurModel
            , pathToFill
            , colFileTypes
            , tCIDCtrls::EFSaveOpts::FilesOnly
        );
    }

    // If they bailed out on the file selection, then give up
    if (!bSelected)
        return kCIDLib::False;


    // If this isn't a new file, and we are exporting, ask if they want to overwrite.
    if (!bImport && TFileSys::bExists(pathToFill))
    {
        TYesNoBox msgbAsk
        (
            strTitle, facIRClient().strMsg(kIRCMsgs::midQ_OverwriteExport, pathToFill)
        );
        if (!msgbAsk.bShowIt(*this))
            return kCIDLib::False;
    }

    // Store the path of the new file as the new last path
    pathToFill.bQueryPath(m_pathLastExport);
    m_pathLastExport.bRemoveTrailingSeparator();

    // If it has no extension, then add one
    if (!pathToFill.bHasExt())
        pathToFill.AppendExt(L"IRExport");

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TIRBlasterTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    try
    {
        if (wnotEvent.widSource() == kIRClient::ridBlast_LoadModel)
        {
            LoadModel();
        }
         else if (wnotEvent.widSource() == kIRClient::ridBlast_UnloadModel)
        {
            UnloadModel();
        }
         else if (wnotEvent.widSource() == kIRClient::ridBlast_Export)
        {
            Export();
        }
         else if (wnotEvent.widSource() == kIRClient::ridBlast_Import)
        {
            Import();
        }
         else if ((wnotEvent.widSource() == kIRClient::ridBlast_EditModel)
              ||  (wnotEvent.widSource() == kIRClient::ridBlast_CreateModel))
        {
            EditOrCreate(wnotEvent.widSource() == kIRClient::ridBlast_EditModel);
        }
    }

    catch(TError& errToCatch)
    {
        if (facIRClient().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Oh well, didn't work. Display the error to the user
        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facIRClient().strMsg(kIRCMsgs::midStatus_SendFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TIRBlasterTab::eComboHandler(TListChangeInfo& wnotEvent)
{
    //
    //  If the combo box is being emptied, then we then we need to disable
    //  the Unload and Edit buttons, then we are done. But if one of those
    //  buttons is the default, move it back to the Load button. And we
    //  need to disable the export menu item.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        m_pwndCmds->RemoveAll();

        m_pwndLoadButton->TakeFocus();
        m_pwndLoadButton->SetDefButton();

        m_pwndEditButton->SetEnable(kCIDLib::False);
        m_pwndUnloadButton->SetEnable(kCIDLib::False);
        m_pwndExportButton->SetEnable(kCIDLib::False);
        pwndChildById(kIRClient::ridBlast_DevInfo)->ClearText();
        return tCIDCtrls::EEvResponses::Handled;
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        //
        //  Enable the unload and edit buttons since we have devices. But only
        //  if the current user role is Admin. Same for the import and export
        //  commands.
        //
        if (m_pwndDriver->eUserRole() == tCQCKit::EUserRoles::SystemAdmin)
        {
            m_pwndEditButton->SetEnable(kCIDLib::True);;
            m_pwndExportButton->SetEnable(kCIDLib::True);;
            m_pwndUnloadButton->SetEnable(kCIDLib::True);;
        }

        //
        //  Get the currently selected combo box item. This tells us the device
        //  name. We'll use this to look up the device in the config data.
        //
        TString strModName;
        m_pwndModels->ValueAt(wnotEvent.c4Index(), strModName);
        const TIRBlasterDevModel& irbdmCur = m_irbcData.irbdmFromName(strModName);

        // Load the commands for the new device
        m_pwndCmds->RemoveAll();
        LoadCmds(irbdmCur);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TIRBlasterTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        if (m_pwndEditButton->bHasFocus() || m_pwndExportButton->bHasFocus())
            m_pwndLoadButton->TakeFocus();

        m_pwndLoadButton->SetDefButton();
        m_pwndEditButton->SetEnable(kCIDLib::False);
        m_pwndExportButton->SetEnable(kCIDLib::False);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Make sure the command specific buttons are enabled
        m_pwndEditButton->SetEnable(kCIDLib::True);
        m_pwndExportButton->SetEnable(kCIDLib::True);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        //
        //  If this is an invoke event, then call InvokeCmd(), but only
        //  if the device implements blasting.
        //
        try
        {
            if (tCIDLib::bAllBitsOn(m_pwndDriver->eDevCaps(), tCQCIR::EIRDevCaps::Blast))
            {
                InvokeCmd();
            }
             else
            {
                TOkBox msgbNoBlast(L"This device does not support blasting");
                msgbNoBlast.ShowIt(*this);
            }
        }

        catch(TError& errToCatch)
        {
            if (facIRClient().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // Oh well, didn't work. Display the error to the user
            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , L"An error occurred while trying to invoke the command"
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TIRBlasterTab::EditOrCreate(const tCIDLib::TBoolean bEdit)
{

    //
    //  Invoke the edit/create dialog. For the create command we pass in an
    //  empty new device. For the edit, we copy the current contents into the
    //  temp.
    //
    TIRBlasterDevModel irbdmTmp;
    TString strModName;
    if (bEdit)
    {
        m_pwndModels->ValueAt(m_pwndModels->c4CurItem(), strModName);
        irbdmTmp = m_irbcData.irbdmFromName(strModName);
    }
     else
    {
        //
        //  This is going to be a new one, so we need to get a device model
        //  name which will be given to the new one. Since it might be taken,
        //  we have to do a loop until they get an unused one or give up.
        //
        const TString strInstruct(kIRCMsgs::midDlg_GetModelName_Instruct, facIRClient());
        const TString strTitle(kIRCMsgs::midDlg_GetModelName_Title, facIRClient());
        while (kCIDLib::True)
        {
            const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
            (
                *this, strTitle, strInstruct, TString::strEmpty(), strModName, L".+"
            );

            if (!bRes)
                return;

            try
            {
                // Check for a dup
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                tCQCIR::TRepoSrvProxy orbcIRRepo = facCQCIR().orbcRepoSrvProxy();

                if (orbcIRRepo->bModelExists(strModName, m_pwndDriver->strDevType()))
                {
                    TErrBox msgbWarn
                    (
                        facIRClient().strMsg(kIRCMsgs::midStatus_AlreadyExists, strModName)
                    );
                    msgbWarn.ShowIt(*this);
                }
                 else
                {
                    irbdmTmp.strModel(strModName);
                    break;
                }
            }

            catch(TError& errToCatch)
            {
                if (facIRClient().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // Oh well, didn't work. Display the error to the user
                TExceptDlg dlgbFail
                (
                    *this
                    , strWndText()
                    , L"Could not test that the model name is unique"
                    , errToCatch
                );
            }
        }
    }

    // And now lets edit the device model
    TEditModelDlg dlgEditModel;
    if (dlgEditModel.bRunDlg(*this, irbdmTmp, m_pwndDriver, m_pwndZone->c4ItemCount()))
    {
        // Add or update this device in our config data
        tCIDLib::TBoolean bAdded;
        TIRBlasterDevModel& irbdmLoad = m_irbcData.irbdmAddOrUpdate(irbdmTmp, bAdded);

        // If added, then we need to add an entry to the devices combo
        if (bAdded)
        {
            m_pwndModels->c4AddItem(irbdmLoad.strModel());

            // If it's the first one, then select it
            if (m_pwndModels->c4ItemCount() == 1)
                m_pwndModels->SelectByIndex(0, kCIDLib::True);
        }

        //
        //  If we are editing, then we have to refresh the command list
        //  box. Otherwise, it won't be the selected model, unless its the
        //  first, in which case the combo box handler will do it for us
        //  when this first one becomes the selected item.
        //
        if (bEdit)
            LoadCmds(irbdmLoad);
    }
}


tCIDLib::TVoid TIRBlasterTab::Export()
{
    //
    //  Get the target directory that they want to export the file into. The
    //  name is automatically generated from the model name.
    //
    TString strModel;
    m_pwndModels->ValueAt(m_pwndModels->c4CurItem(), strModel);
    const TIRBlasterDevModel& irbdmCur = m_irbcData.irbdmFromName(strModel);

    // Load the title text
    const TString strTitle(kIRCMsgs::midBlast_SelExportFile, facIRClient());

    //
    //  Get a file from the user. Use the single create flag, which lets him
    //  either create a file or select an existing one.
    //
    TPathStr pathSelected;
    if (!bGetInpExpFile(pathSelected, strTitle, kCIDLib::False))
        return;

    // And let's try to open it and stream out the content
    try
    {
        TTextFileOutStream strmExp
        (
            pathSelected
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_OutStream
            , new TUTF8Converter
        );

        // Write out the header stuff
        TString strCat = tCQCKit::strXlatEDevCats(irbdmCur.eCategory());
        const TTextOutStream::Spaces Spacer(18);
        strmExp << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << L"<CQCIR:ExportFile "
                   L"CQCIR:Blaster='" << m_pwndDriver->strDevType() << L"'\n" << Spacer
                << L"CQCIR:Make='" << irbdmCur.strMake() << L"'\n" << Spacer
                << L"CQCIR:Model='" << irbdmCur.strModel() << L"'\n" << Spacer
                << L"CQCIR:RepeatCount='" << irbdmCur.c4RepeatCount() << L"'\n" << Spacer
                << L"CQCIR:Description='" << irbdmCur.strDescription() << L"'\n" << Spacer
                << L"CQCIR:Category='" << strCat << L"'\n" << Spacer
                << L"xmlns:CQCIR='http://www.charmedquark.com/IRExport'>\n"
                   L"  <CQCIR:Commands>\n";

        TIRBlasterDevModel::TCmdCursor cursCmds = irbdmCur.cursCmds();
        if (cursCmds.bReset())
        {
            TString strData;
            do
            {
                const TIRBlasterCmd& irbcCur = cursCmds.objRCur();
                irbcCur.FormatData(strData);

                strmExp << L"      <CQCIR:Cmd CQCIR:CmdName='"
                        << irbcCur.strName()
                        << L"'\n                 CQCIR:Data='"
                        << strData<< L"'/>\n";
            }   while(cursCmds.bNext());
        }

        strmExp << L"  </CQCIR:Commands>\n</CQCIR:ExportFile>\n"
                << kCIDLib::FlushIt;
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strTitle
            , facIRClient().strMsg(kIRCMsgs::midStatus_ExportFailed)
            , errToCatch
        );
    }

    catch(...)
    {
    }
}


tCIDLib::TVoid TIRBlasterTab::Import()
{
    // Load the title text
    const TString strTitle(kIRCMsgs::midBlast_SelImportFile, facIRClient());

    // Get a file from the user
    TPathStr pathSelected;
    if (!bGetInpExpFile(pathSelected, strTitle, kCIDLib::True))
        return;

    //
    //  Create a temp device model and ask it to load itself from the
    //  passed file.
    //
    TIRBlasterDevModel irbdmNew;
    try
    {
        irbdmNew.LoadFromFile
        (
            pathSelected, m_pwndDriver->strDevType(), kCIDLib::False
        );
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strTitle
            , facIRClient().strMsg(kIRCMsgs::midStatus_ImportFailed)
            , errToCatch
        );
        return;
    }

    catch(...)
    {
        return;
    }

    // And let them edit the model, and save it if they want to
    TEditModelDlg dlgEditModel;
    if (dlgEditModel.bRunDlg(*this
                            , irbdmNew
                            , m_pwndDriver
                            , m_pwndZone->c4ItemCount()
                            , kCIDLib::True))
    {
        //
        //  If we already have this model loaded, then just update it and
        //  reload the command list. Else, load the model to the blaster
        //  and reload.
        //
        tCIDLib::TBoolean bAdded;
        m_irbcData.irbdmAddOrUpdate(irbdmNew, bAdded);

        if (bAdded)
        {
            m_pwndModels->c4AddItem(irbdmNew.strModel(), 0);
        }
         else
        {
            // Find this one in the list
            const tCIDLib::TCard4 c4Ind = m_pwndModels->c4FindByText(irbdmNew.strModel());

            //
            //  If it's the selected one, then update the commands, else
            //  select it, which will make it reload the commands.
            //
            if (c4Ind == m_pwndModels->c4CurItem())
                LoadCmds(irbdmNew);
            else
                m_pwndModels->c4SelectById(c4Ind, kCIDLib::True);
        }
    }
}

tCIDLib::TVoid TIRBlasterTab::InvokeCmd()
{
    // We need the name of the device and command
    TString strDev;
    m_pwndModels->ValueAt(m_pwndModels->c4CurItem(), strDev);
    const TIRBlasterDevModel& irbdmCur = m_irbcData.irbdmFromName(strDev);

    TString strCmd;
    m_pwndCmds->QueryColText(m_pwndCmds->c4CurItem(), 0, strCmd);
    const TIRBlasterCmd& irbcCur = irbdmCur.irbcFromName(strCmd);

    //
    //  Get the zone. They are zero based as sent to the device, so the raw index is
    //  fine. If it is max card, there are no zones defined, so do nothing.
    //
    const tCIDLib::TCard4 c4Zone = m_pwndZone->c4CurItem();
    if (c4Zone == kCIDLib::c4MaxCard)
    {
        TErrBox msgbNoZones(L"The driver reported no blaster zones available");
        msgbNoZones.ShowIt(*this);
        return;
    }

    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCKit::TCQCSrvProxy& orbcSrv = m_pwndDriver->orbcServer();

    // We have to flatten the info into a memory buffer to send it
    tCIDLib::TCard4 c4Bytes = 0;
    THeapBuf mbufData(1024 * 16);
    {
        TBinMBufOutStream strmData(&mbufData);
        strmData << irbdmCur.strModel() << irbcCur.strName() << c4Zone
                 << kCIDLib::FlushIt;
        c4Bytes = strmData.c4CurSize();
    }

    // We don't use the data type string here, so just send an empty one
    TString strType;
    orbcSrv->bSendData
    (
        m_pwndDriver->strMoniker()
        , kCQCIR::strInvokeBlastCmd
        , strType
        , c4Bytes
        , mbufData
        , m_pwndDriver->sectUser()
    );
}


tCIDLib::TVoid TIRBlasterTab::LoadCmds(const TIRBlasterDevModel& irbdmSrc)
{
    TIRBlasterDevModel::TCmdCursor cursCmds = irbdmSrc.cursCmds();
    if (cursCmds.bReset())
    {
        TWndPaintJanitor janCmds(m_pwndCmds);
        m_pwndCmds->RemoveAll();

        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());
        do
        {
            colVals[0] = cursCmds.objRCur().strName();
            m_pwndCmds->c4AddItem(colVals, 0);
        }   while (cursCmds.bNext());
        m_pwndCmds->SelectByIndex(0, kCIDLib::True);
    }

    // Update the device info display
    m_pwndDevInfo->strWndText(irbdmSrc.strDescription());
}


tCIDLib::TVoid TIRBlasterTab::LoadConfigData()
{
    tCQCIR::TRepoSrvProxy orbcIRRepo = facCQCIR().orbcRepoSrvProxy();
    tCQCKit::TCQCSrvProxy& orbcSrv = m_pwndDriver->orbcServer();

    // Flush the current contents
    m_pwndCmds->RemoveAll();
    m_pwndModels->RemoveAll();
    m_irbcData.Reset();

    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

    // Ask the driver what devices it has loaded
    TFacCQCIR::TModelInfoList colModels;
    tCIDLib::TCard4 c4ZoneCount = facCQCIR().c4QueryDevIRModels
    (
        m_pwndDriver->orbcServer(), m_pwndDriver->strMoniker(), colModels
    );
    const tCIDLib::TCard4 c4Count = colModels.c4ElemCount();

    //
    //  Update the zone combo to the number of blaster zones the device tells us it has.
    //  It could in theory be zero.
    //
    if (c4ZoneCount)
    {
        m_pwndZone->LoadNumRange(1, c4ZoneCount);
        m_pwndZone->SelectByIndex(0);
    }

    // If no devices loaded, then we are done
    if (!c4Count)
        return;

    // Load up the devices list
    tCIDLib::TBoolean   bAdded;
    TIRBlasterDevModel  irbdmNew;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TIRBlasterDevModelInfo& irbdmiCur = colModels[c4Index];

        // Ask the repository for the file data
        orbcIRRepo->QueryDevModel
        (
            irbdmiCur.strModel(), m_pwndDriver->strDevType(), irbdmNew
        );

        // Add this new one to our list
        m_irbcData.irbdmAddOrUpdate(irbdmNew, bAdded);

        //
        //  These should all be unique and be added, not updated. If added,
        //  then add a list window item to the collection. Else, log a msg
        //  and leave it out.
        //
        if (bAdded)
        {
            m_pwndModels->c4AddItem(irbdmNew.strModel(), 0);
        }
         else
        {
            facIRClient().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIRCErrs::errcCfg_DupKeyInDev
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , irbdmiCur.strModel()
            );
        }
    }

    if (m_pwndModels->c4ItemCount())
        m_pwndModels->SelectByIndex(0, kCIDLib::True);
}


tCIDLib::TVoid TIRBlasterTab::LoadModel()
{
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    try
    {
        // And pass this list to the device selection dialog
        TString         strSelected;
        tCIDLib::TBoolean bLoad;
        {
            TSelModelDlg dlgSel;
            bLoad = dlgSel.bRunDlg(*this, strSelected, m_pwndDriver->strDevType());
        }

        if (bLoad)
        {
            // Ask the driver to load this selected model
            m_pwndDriver->orbcServer()->c4SendCmd
            (
                m_pwndDriver->strMoniker()
                , kCQCIR::strLoadModel
                , strSelected
                , m_pwndDriver->sectUser()
            );

            // And reload our config data
            LoadConfigData();
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facIRClient().strMsg(kIRCMsgs::midStatus_LoadFailed)
            , errToCatch
        );
    }
}


tCIDLib::TVoid TIRBlasterTab::Reset()
{
    m_pwndDriver->orbcServer()->c4SendCmd
    (
        m_pwndDriver->strMoniker()
        , kCQCIR::strExitBTraining
        , TString::strEmpty()
        , m_pwndDriver->sectUser()
    );
}


tCIDLib::TVoid TIRBlasterTab::UnloadModel()
{
    // Ask the server driver to unload the current model
    TString strModel;
    m_pwndModels->ValueAt(m_pwndModels->c4CurItem(), strModel);

    // Make sure that they want to do this
    TYesNoBox msgbConfirm(facIRClient().strMsg(kIRCMsgs::midQ_Unload, strModel));

    // If so, then go for it
    if (msgbConfirm.bShowIt(*this))
    {
        try
        {
            m_pwndDriver->orbcServer()->c4SendCmd
            (
                m_pwndDriver->strMoniker()
                , kCQCIR::strUnloadModel
                , strModel
                , m_pwndDriver->sectUser()
            );
            m_irbcData.bRemoveModel(strModel);

            // And now remove the item from drop down
            m_pwndModels->RemoveCurrent();

            // Ask if they want to delete it from the IR repository
            TYesNoBox msgbDelConfirm
            (
                facIRClient().strMsg(kIRCMsgs::midQ_DelFromRepo, strModel)
            );

            if (msgbDelConfirm.bShowIt(*this))
            {
                facCQCIR().orbcRepoSrvProxy()->DeleteDevModel
                (
                    strModel, m_pwndDriver->strDevType()
                );

                // If we have more left, select a new one
                if (m_pwndModels->c4ItemCount())
                    m_pwndModels->SelectByIndex(0, kCIDLib::True);
            }
        }

        catch(const TError& errToCatch)
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , facIRClient().strMsg(kIRCMsgs::midStatus_UnloadFailed)
                , errToCatch
            );
            return;
        }
    }
}


