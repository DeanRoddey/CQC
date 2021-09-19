//
// FILE NAME: IRClient_EditModelDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/24/2002
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
//  This file implements the dialog that allows the user to edit device
//  protocol files, or create new ones, and store them on the data server
//  in the IR protocol repository.
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
#include    "IRClient.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditModelDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEditModelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditModelDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditModelDlg::TEditModelDlg() :

    m_bChanges(kCIDLib::False)
    , m_bEnableSave(kCIDLib::True)
    , m_c4ZoneCount(0)
    , m_pirbdmTarget(nullptr)
    , m_pwndCategories(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndList(nullptr)
    , m_pwndModel(nullptr)
    , m_pwndMake(nullptr)
    , m_pwndNewButton(nullptr)
    , m_pwndRepeatCount(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndTestButton(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndZone(nullptr)
{
}

TEditModelDlg::~TEditModelDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditModelDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEditModelDlg::bRunDlg( const   TWindow&            wndOwner
                        ,       TIRBlasterDevModel& irbdmTarget
                        ,       TIRClientWnd* const pwndDriver
                        , const tCIDLib::TCard4     c4ZoneCount
                        , const tCIDLib::TBoolean   bEnableSave)
{
    //
    //  Store pointers to some of the parms so that they can be used from
    //  event handlers below.
    //
    m_bEnableSave = bEnableSave;
    m_c4ZoneCount = c4ZoneCount;
    m_pirbdmTarget = &irbdmTarget;
    m_pwndDriver = pwndDriver;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facIRClient(), kIRClient::ridDlg_EditDev
    );

    return (c4Res == kIRClient::ridDlg_EditDev_Save);
}


// ---------------------------------------------------------------------------
//  TEditModelDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditModelDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Category, m_pwndCategories);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Delete, m_pwndDelButton);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Descr, m_pwndDescr);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_List, m_pwndList);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Make, m_pwndMake);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Model, m_pwndModel);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_New, m_pwndNewButton);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Repeat, m_pwndRepeatCount);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Save, m_pwndSaveButton);
    CastChildWnd(*this, kIRClient::ridDlg_EditDev_Edit, m_pwndEditButton);

    // Set the columns on the list window
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Defined Model Commands");
    m_pwndList->SetColumns(colCols);

    // Load up the model, make, and description
    m_pwndMake->strWndText(m_pirbdmTarget->strMake());
    m_pwndModel->strWndText(m_pirbdmTarget->strModel());
    m_pwndDescr->strWndText(m_pirbdmTarget->strDescription());

    // Load the categories list and select the current category
    tCQCKit::EDevCats eCat;
    tCIDLib::TCard4   c4CurIndex = 0;
    for (eCat = tCQCKit::EDevCats::Min; eCat <= tCQCKit::EDevCats::Max; eCat++)
    {
        if (eCat == m_pirbdmTarget->eCategory())
            c4CurIndex = tCIDLib::TCard4(eCat);

        m_pwndCategories->c4AddItem(tCQCKit::strXlatEDevCats(eCat), tCIDLib::c4EnumOrd(eCat));
    }
    m_pwndCategories->c4SelectById(c4CurIndex);

    //
    //  Load up the repeat count combo and set it's initial value. The count is one based
    //  so sub one for a list index.
    //
    m_pwndRepeatCount->LoadNumRange(1, 8);
    m_pwndRepeatCount->SelectByIndex(m_pirbdmTarget->c4RepeatCount() - 1);

    // And register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);
    m_pwndCategories->pnothRegisterHandler(this, &TEditModelDlg::eLBHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEditModelDlg::eLBHandler);
    m_pwndNewButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);
    m_pwndRepeatCount->pnothRegisterHandler(this, &TEditModelDlg::eLBHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);

    //
    //  If it's only a learner, i.e. no blasting capabilities, then we destroy the test
    //  related controls and move the bottom of the list down to where the bottom of
    //  the test button was.
    //
    if (tCIDLib::bAllBitsOn(m_pwndDriver->eDevCaps(), tCQCIR::EIRDevCaps::Blast))
    {
        CastChildWnd(*this, kIRClient::ridDlg_EditDev_Test, m_pwndTestButton);
        CastChildWnd(*this, kIRClient::ridDlg_EditDev_Zone, m_pwndZone);

        m_pwndTestButton->pnothRegisterHandler(this, &TEditModelDlg::eClickHandler);
        if (m_c4ZoneCount)
        {
            m_pwndZone->LoadNumRange(1, m_c4ZoneCount);
            m_pwndZone->SelectByIndex(0);
        }
    }
     else
    {
        // Save the test button first
        TArea areaTest(pwndChildById(kIRClient::ridDlg_EditDev_Test)->areaWnd());

        DestroyChild(kIRClient::ridDlg_EditDev_Test);
        DestroyChild(kIRClient::ridDlg_EditDev_TestInstruct);
        DestroyChild(kIRClient::ridDlg_EditDev_ZonePref);
        DestroyChild(kIRClient::ridDlg_EditDev_Zone);

        // Now resize the list
        TArea areaList = m_pwndList->areaWnd();
        areaList.i4Bottom(areaTest.i4Bottom());
        m_pwndList->SetSize(areaList.szArea(), kCIDLib::True);
    }

    // Load the current commands, if any
    LoadCmds();

    // If they wanted an initially enabled save, then do that
    m_pwndSaveButton->SetEnable(m_bEnableSave);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditModelDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditModelDlg::bSaveData()
{
    // Make sure that there is a make and description
    if (m_pwndMake->strWndText().bIsEmpty()
    || (m_pwndDescr->strWndText().bIsEmpty()))
    {
        TErrBox msgbErr(facIRClient().strMsg(kIRCMsgs::midStatus_EnterMakeDescr));
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    // Set the name and description on the device
    m_pirbdmTarget->strMake(m_pwndMake->strWndText());
    m_pirbdmTarget->strModel(m_pwndModel->strWndText());
    m_pirbdmTarget->strDescription(m_pwndDescr->strWndText());

    //
    //  Get the category translated back to the enum. We stored the numerical
    //  value of the category enum in the row id, so just cast it back.
    //
    m_pirbdmTarget->eCategory
    (
        tCQCKit::EDevCats(m_pwndCategories->c4CurItemId())
    );

    // Get the repeat count out and stored on the model
    m_pirbdmTarget->c4RepeatCount(m_pwndRepeatCount->c4CurItem() + 1);

    // Get an IR repository proxy and write this data
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCIR::TRepoSrvProxy orbcIRRepo = facCQCIR().orbcRepoSrvProxy();
    orbcIRRepo->WriteDevModel
    (
        m_pirbdmTarget->strModel(), m_pwndDriver->strDevType(), *m_pirbdmTarget
    );

    // Ask the server to reload this file so it'll stay in sync
    m_pwndDriver->orbcServer()->c4SendCmd
    (
        m_pwndDriver->strMoniker()
        , kCQCIR::strLoadModel
        , m_pirbdmTarget->strModel()
        , m_pwndDriver->sectUser()
    );

    // We can clear the changes line now and disable the save button
    m_bChanges = kCIDLib::False;
    m_pwndSaveButton->SetEnable(kCIDLib::False);

    return kCIDLib::True;
}


tCIDLib::TVoid TEditModelDlg::DeleteCmd()
{
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    // Get the selected command name
    TString strCmd;
    m_pwndList->QueryColText(c4ListInd, 0, strCmd);

    // Make sure that they want to do this
    TYesNoBox msgbAsk(facIRClient().strMsg(kIRCMsgs::midQ_DeleteItem, strCmd));

    if (msgbAsk.bShowIt(*this))
    {
        //
        //  Set the changes flag, enable the save button, and remove the
        //  current list box item.
        //
        m_bChanges = kCIDLib::True;
        m_pwndSaveButton->SetEnable(kCIDLib::True);
        m_pwndList->RemoveCurrent();

        // Ask the device to remove this command
        m_pirbdmTarget->RemoveCmd(strCmd);
    }
}


tCIDCtrls::EEvResponses TEditModelDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    try
    {
        if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Cancel)
        {
            if (m_bChanges)
            {
                TYesNoBox msgbAsk(facIRClient().strMsg(kIRCMsgs::midQ_SaveChanges));
                if (msgbAsk.bShowIt(*this))
                {
                    // Try to save the change. If it doesn't work, don't exit
                    if (!bSaveData())
                        return tCIDCtrls::EEvResponses::Handled;
                }
            }
            EndDlg(kIRClient::ridDlg_EditDev_Cancel);
        }
         else if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Delete)
        {
            DeleteCmd();
        }
         else if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Save)
        {
            if (bSaveData())
                EndDlg(kIRClient::ridDlg_EditDev_Save);
        }
         else if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Test)
        {
            TAudio::AudioCue(tCIDLib::EAudioCues::Click);
            InvokeCmd();
        }
         else if ((wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Edit)
              ||  (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_New))
        {
            NewEditCmd(wnotEvent.widSource() == kIRClient::ridDlg_EditDev_New);
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , errToCatch.strErrText()
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEditModelDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Category)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Just remember we have changes
            if (!m_bChanges)
            {
                m_bChanges = kCIDLib::True;
                m_pwndSaveButton->SetEnable(kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            if (m_pwndTestButton)
                m_pwndTestButton->SetEnable(kCIDLib::False);

            m_pwndEditButton->SetEnable(kCIDLib::False);
            m_pwndDelButton->SetEnable(kCIDLib::False);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            m_pwndEditButton->SetEnable(kCIDLib::True);
            m_pwndDelButton->SetEnable(kCIDLib::True);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            // If its an invoke, then invoke this one if the device supports it
            if (tCIDLib::bAllBitsOn(m_pwndDriver->eDevCaps(), tCQCIR::EIRDevCaps::Blast))
                InvokeCmd();
        }
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_EditDev_Repeat)
    {
        // Just remember we have changes
        if (!m_bChanges)
        {
            m_bChanges = kCIDLib::True;
            m_pwndSaveButton->SetEnable(kCIDLib::True);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;

}


tCIDLib::TVoid TEditModelDlg::InvokeCmd()
{
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    // Get the selected command name
    TString strCmd;
    m_pwndList->QueryColText(c4ListInd, 0, strCmd);

    // We need the name of the device and command
    const TIRBlasterCmd& irbcCur = m_pirbdmTarget->irbcFromName(strCmd);

    // And get the selected zone. It's zero based, so the raw list index is fine
    const tCIDLib::TCard4 c4Zone = tCIDLib::TCard4(m_pwndZone->c4CurItem());

    // Get the repeat count which is one-based so add 1
    const tCIDLib::TCard4 c4Repeat = tCIDLib::TCard4(m_pwndRepeatCount->c4CurItem() + 1);

    //
    //  And ask the driver to invoke this guy. In this case, because this command isn't
    //  on the server side yet, we have to call the API that just sends raw data for us.
    //
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCIDLib::TCard4 c4Bytes = 0;
    THeapBuf mbufOut(4192UL);
    {
        TBinMBufOutStream strmOut(&mbufOut);
        strmOut << irbcCur.c4DataLen() << c4Zone << c4Repeat;
        strmOut.c4WriteBuffer(irbcCur.mbufData(), irbcCur.c4DataLen());
        strmOut.Flush();
        c4Bytes = strmOut.c4CurSize();
    }

    // We don't use the data type string, so just send an empty one
    TString strType;
    m_pwndDriver->orbcServer()->bSendData
    (
        m_pwndDriver->strMoniker()
        , kCQCIR::strSendData
        , strType
        , c4Bytes
        , mbufOut
        , m_pwndDriver->sectUser()
    );
}


tCIDLib::TVoid TEditModelDlg::LoadCmds()
{
    // Fill in the current commands into the list.
    TIRBlasterDevModel::TCmdCursor cursCmds = m_pirbdmTarget->cursCmds();
    if (cursCmds.bReset())
    {
        TWndPaintJanitor janCmds(m_pwndList);
        m_pwndList->RemoveAll();

        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());
        do
        {
            colVals[0] = cursCmds.objRCur().strName();
            m_pwndList->c4AddItem(colVals, 0);
        }   while (cursCmds.bNext());

        m_pwndList->SelectByIndex(0);
    }

    if (!m_pwndList->c4ItemCount())
    {
        m_pwndDelButton->SetEnable(kCIDLib::False);
        m_pwndEditButton->SetEnable(kCIDLib::False);

        if (m_pwndTestButton)
            m_pwndTestButton->SetEnable(kCIDLib::False);
    }
}


tCIDLib::TVoid TEditModelDlg::NewEditCmd(const tCIDLib::TBoolean bNew)
{
    // We need a blaster client proxy
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

    //
    //  See if we already think that the device is in training mode. If so,
    //  tell the user, and ask if they want to force it out.
    //
    const tCIDLib::TBoolean bTrainMode = m_pwndDriver->orbcServer()->bQueryVal
    (
        m_pwndDriver->strMoniker(), kCQCIR::strQueryBTrainMode
    );

    if (bTrainMode)
    {
        TYesNoBox msgbAsk(facIRClient().strMsg(kIRCMsgs::midQ_ForceOutOfTraining));
        if (!msgbAsk.bShowIt(*this))
            return;

        m_pwndDriver->orbcServer()->c4SendCmd
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strExitBTraining
            , TString::strEmpty()
            , m_pwndDriver->sectUser()
        );
    }

    tCIDLib::TBoolean bActualNew = bNew;

    //
    //  Invoke the training dialog. We have to give him pointers to our
    //  client proxy and our mutex, so that he can talk to the server, and
    //  lock when doing it.
    //
    tCIDLib::TCard4 c4Bytes;
    THeapBuf        mbufData;
    TString         strPrevData;

    // If its a new one, then get the command name
    TString strName;
    if (bNew)
    {
        const TString strInstruct(kIRCMsgs::midDlg_GetCmdName_Instruct, facIRClient());
        const TString strTitle(kIRCMsgs::midDlg_GetCmdName_Title, facIRClient());
        const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
        (
            *this, strTitle, strInstruct, TString::strEmpty(), strName, L".+"
        );

        // If this one already exists, tell them edit it instead
        if (m_pirbdmTarget->bCmdExists(strName))
        {
            TYesNoBox msgbAlready(facIRClient().strMsg(kIRCMsgs::midQ_KeyAlreadyExists, strName));
            if (!msgbAlready.bShowIt(*this))
                return;

            // It's not actually new
            bActualNew = kCIDLib::False;
        }
    }
     else
    {
        //
        //  Get the existing data so we can pass it in. We have to ask the server
        //  side driver to format the data for us.
        //
        TString strCurCmd;
        m_pwndList->QueryColText(m_pwndList->c4CurItem(), 0, strCurCmd);
        TIRBlasterCmd& irbcCur = m_pirbdmTarget->irbcFromName(strCurCmd);
        if (irbcCur.c4DataLen())
        {
            {
                TBinMBufOutStream strmTar(&mbufData);
                strmTar << irbcCur << kCIDLib::FlushIt;
                c4Bytes = strmTar.c4CurSize();
            }

            const tCIDLib::TBoolean bRes = m_pwndDriver->orbcServer()->bQueryData2
            (
                m_pwndDriver->strMoniker(), kCQCIR::strFmtBlastData, c4Bytes, mbufData
            );

            if (bRes)
            {
                TBinMBufInStream strmSrc(&mbufData, c4Bytes);
                strmSrc >> strPrevData;
            }
        }
    }

    TBlastTrainingDlg  dlgTrain;
    c4Bytes = 0;
    if (dlgTrain.bRunDlg(*this, mbufData, c4Bytes, m_pwndDriver, strPrevData))
    {
        //
        //  If its a new command, add it. Else find the existing one and update
        //  it's data.
        //
        if (bActualNew)
        {
            //
            //  Add this new command to the device object, and add a new
            //  item to the command list. Since we want to keep them sorted,
            //  we just reload it from scratch.
            //
            m_pirbdmTarget->irbcAdd(strName, mbufData, c4Bytes);
            LoadCmds();
        }
         else
        {
            TString strCurCmd;
            m_pwndList->QueryColText(m_pwndList->c4CurItem(), 0, strCurCmd);
            TIRBlasterCmd& irbcCur = m_pirbdmTarget->irbcFromName(strCurCmd);
            irbcCur.SetData(mbufData, c4Bytes);
        }

        // Set the changes flag and enable the save button
        if (!m_bChanges)
        {
            m_bChanges = kCIDLib::True;
            m_pwndSaveButton->SetEnable(kCIDLib::True);
        }
    }
}



