//
// FILE NAME: IRClient_ReceiverTab.cpp
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
//  This file implements the IR Receiver panel.
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
RTTIDecls(TIRReceiverTab,TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TIRReceiverTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRReceiverTab: Constructors and destructor
// ---------------------------------------------------------------------------
TIRReceiverTab::TIRReceiverTab(TIRClientWnd* const pwndDriver) :

    TTabWindow(L"IRRecTab", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pwndDeleteAll(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndDriver(pwndDriver)
    , m_pwndEdit(nullptr)
    , m_pwndList(nullptr)
    , m_pwndNew(nullptr)
{
}

TIRReceiverTab::~TIRReceiverTab()
{
}


// ---------------------------------------------------------------------------
//  TIRReceiverTab: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRReceiverTab::Reset()
{
    // <TBD> Implement this again
}


tCIDLib::TVoid TIRReceiverTab::SetConnected(const TString&)
{
    // The client has connected, so load up new event data
    try
    {
        LoadEvents(TString::strEmpty());
    }

    catch(TError& errToCatch)
    {
        if (facIRClient().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TErrBox msgbNoKey
        (
            facIRClient().strMsg
            (
                kIRCMsgs::midStatus_ServerNotFound
                , TString(L"reciever")
                , m_pwndDriver->strMoniker()
            )
        );
        msgbNoKey.ShowIt(*this);
    }
}



// ---------------------------------------------------------------------------
//  TIRReceiverTab: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TIRReceiverTab::AreaChanged(const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have created our children, then adjust them
    if (bSizeChanged && m_pwndList && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TIRReceiverTab::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facIRClient().bCreateDlgDesc(kIRClient::ridTab_Receiver, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to some of our widgets
    CastChildWnd(*this, kIRClient::ridRec_DeleteAll, m_pwndDeleteAll);
    CastChildWnd(*this, kIRClient::ridRec_Delete, m_pwndDelete);
    CastChildWnd(*this, kIRClient::ridRec_Edit, m_pwndEdit);
    CastChildWnd(*this, kIRClient::ridRec_List, m_pwndList);
    CastChildWnd(*this, kIRClient::ridRec_New, m_pwndNew);

    // We don't use the titles but still have to set them
    tCIDLib::TStrList colTitles(1);
    colTitles.objAdd(TString::strEmpty());
    m_pwndList->SetColumns(colTitles);

    // Register a handler for the event list box
    m_pwndList->pnothRegisterHandler(this, &TIRReceiverTab::eLBHandler);

    // And register our buttons with the click handler
    m_pwndDeleteAll->pnothRegisterHandler(this, &TIRReceiverTab::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TIRReceiverTab::eClickHandler);
    m_pwndEdit->pnothRegisterHandler(this, &TIRReceiverTab::eClickHandler);
    m_pwndNew->pnothRegisterHandler(this, &TIRReceiverTab::eClickHandler);

    // Initially disable the buttons that operate on a selected item
    m_pwndDelete->SetEnable(kCIDLib::False);
    m_pwndEdit->SetEnable(kCIDLib::False);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TIRReceiverTab: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TIRReceiverTab::bGetIREvent(TString& strToFill, const TString& strRetrainKey)
{
    //
    //  Ok, put the driver into training mode. We use a try/catch here because
    //  we have to get it back out of training mode, or it will be stuck and
    //  won't work.
    //
    try
    {
        m_pwndDriver->orbcServer()->c4SendCmd
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strEnterRTraining
            , TString::strEmpty()
            , m_pwndDriver->sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (facIRClient().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        throw;
    }

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        //
        //  Now pop up the wait dialog. This guy just puts up a message that
        //  tells the user to press the buttons a few times. It will check
        //  the incoming events. When it has seen the same even a few times,
        //  it is sure that its the one desired. It will then come back here
        //  with the results. It provides them with a cancel button to allow
        //  them to bail out.
        //
        TRecTrainingDlg dlgTrain;
        bRet = dlgTrain.bRunDlg(*this, strToFill, strRetrainKey, m_pwndDriver);

        if (!bRet)
        {
            TErrBox msgbNoKey
            (
                facIRClient().strMsg(kIRCMsgs::midStatus_NoEventReceived)
            );
            msgbNoKey.ShowIt(*this);
        }

        m_pwndDriver->orbcServer()->c4SendCmd
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strExitRTraining
            , TString::strEmpty()
            , m_pwndDriver->sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        if (facIRClient().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        m_pwndDriver->orbcServer()->c4SendCmd
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strExitRTraining
            , TString::strEmpty()
            , m_pwndDriver->sectUser()
        );
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCIDCtrls::EEvResponses TIRReceiverTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    try
    {
        const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();

        if (wnotEvent.widSource() == kIRClient::ridRec_DeleteAll)
        {
            // Make sure they want to do this
            TYesNoBox msgbAsk(facIRClient().strMsg(kIRCMsgs::midQ_DeleteAll));
            if (msgbAsk.bShowIt(*this))
            {
                TString strType;
                m_pwndDriver->orbcServer()->c4SendCmd
                (
                    m_pwndDriver->strMoniker()
                    , kCQCIR::strDelAllEvents
                    , TString::strEmpty()
                    , m_pwndDriver->sectUser()
                );

                // It worked, so clear our list
                m_pwndList->RemoveAll();
            }
        }
         else if (wnotEvent.widSource() == kIRClient::ridRec_Delete)
        {
            // Get a ref to the selected event
            const tCIDLib::TCard4 c4EvInd = m_pwndList->c4IndexToId(c4ListInd);
            const TCQCStdKeyedCmdSrc& csrcTmp = m_colEvents[c4EvInd];

            // Make sure they really wanna
            TYesNoBox msgbAsk
            (
                facIRClient().strMsg(kIRCMsgs::midQ_DeleteItem, csrcTmp.strTitle())
            );

            if (msgbAsk.bShowIt(*this))
            {
                // Send the delete event and reload the events
                m_pwndDriver->orbcServer()->c4SendCmd
                (
                    m_pwndDriver->strMoniker()
                    , kCQCIR::strDeleteEvent
                    , csrcTmp.strId()
                    , m_pwndDriver->sectUser()
                );
                LoadEvents(TString::strEmpty());
            }
        }
         else if ((wnotEvent.widSource() == kIRClient::ridRec_New)
              ||  (wnotEvent.widSource() == kIRClient::ridRec_Edit))
        {
            NewOrEdit(wnotEvent.widSource() == kIRClient::ridRec_New);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

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


tCIDCtrls::EEvResponses TIRReceiverTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  If its empty, then disable the Delete/Edit buttons. If its not, then enable the
    //  buttons and update the description field.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        m_pwndDelete->SetEnable(kCIDLib::True);
        m_pwndEdit->SetEnable(kCIDLib::True);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        m_pwndDelete->SetEnable(kCIDLib::False);
        m_pwndEdit->SetEnable(kCIDLib::False);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        // If its an invoke operation, then edit this one
        NewOrEdit(kCIDLib::False);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TIRReceiverTab::LoadEvents(const TString& strSelect)
{
    // Flush any current contents
    m_pwndList->RemoveAll();

    // Get the driver proxy and query the mapped events
    try
    {
        tCQCKit::TCQCSrvProxy& orbcSrv = m_pwndDriver->orbcServer();
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufData(8192UL);
        orbcSrv->bQueryData
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strQueryMappedEvents
            , TString::strEmpty()
            , c4Bytes
            , mbufData
        );
        TBinMBufInStream strmData(&mbufData, c4Bytes);
        strmData >> m_colEvents;
    }

    catch(TError& errToCatch)
    {
        TExceptDlg dlgFail
        (
            *this
            , strWndText()
            , L"Could not download the action list"
            , errToCatch
        );
        return;
    }

    // Load up the contents of
    const tCIDLib::TCard4 c4Count = m_colEvents.c4ElemCount();
    if (c4Count)
    {
        //
        //  We want to do a bulk load, so we have to fill in a non-adopting
        //  ref collection of list box items, since the list will adopt the
        //  elements. We use the id field of the list items to store the
        //  linear order, since the list is going to sort them and we want to
        //  get back from the selected item to the index in the event vector.
        //
        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());
        for (tCIDLib::TCard4 c4CurIndex = 0; c4CurIndex < c4Count; c4CurIndex++)
        {
            const TCQCStdKeyedCmdSrc& csrcCur = m_colEvents[c4CurIndex];

            colVals[0] = csrcCur.strTitle();
            m_pwndList->c4AddItem(colVals, c4CurIndex);
        }

        tCIDLib::TCard4 c4At;
        if (strSelect.bIsEmpty()
        ||  !m_pwndList->bSelectByText(strSelect, c4At, kCIDLib::True))
        {
            m_pwndList->SelectByIndex(0, kCIDLib::True);
        }
    }
}


tCIDLib::TVoid TIRReceiverTab::NewOrEdit(const tCIDLib::TBoolean bNew)
{
    //
    //  We'll edit a temp. if it already exists, then get a copy of the
    //  current values to edit.
    //
    TCQCStdKeyedCmdSrc csrcTmp;
    if (bNew)
    {
        // Get a name for this new action
        TString strPrompt(kIRCMsgs::midDlg_GetCmdName_Title, facIRClient());
        TString strNewTitle;
        if (!facCIDWUtils().bGetText(*this, strPrompt, TString::strEmpty(), strNewTitle))
            return;

        // Store the title
        csrcTmp.strTitle(strNewTitle);
    }
     else
    {
        tCIDLib::TCard4 c4Index = m_pwndList->c4CurItemId();
        csrcTmp = m_colEvents[c4Index];
    }

    //
    //  We just support one event. We have to add this whether it's a new
    //  one or we are creating one. This is the list of available events, not
    //  part of the configured commands.
    //
    csrcTmp.AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPress)
        , kCQCKit::strEvId_OnPress
        , tCQCKit::EActCmdCtx::Standard
    );

    //
    //  Invoke the standard command editing dialog on this guy. We have
    //  no extra targets, just the standard ones are ok for us, so set up
    //  an empty extra targets list.
    //
    TRefVector<MCQCCmdTarIntf> colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);
    const tCIDLib::TBoolean bSaved = facCQCIGKit().bEditAction
    (
        *this, csrcTmp, kCIDLib::False, m_pwndDriver->cuctxToUse(), &colExtraTars
    );

    //
    //  Note also we don't care if they saved the action unless it's new. If a new one,
    //  and they cancel the action editing, then we do nothing. If it's not a new one, then
    //  it's fine to keep the existing action content but still retrain it below.
    //
    if (!bSaved && bNew)
        return;

    //
    //  We have to get the trigger data that they want to use for this. If this is an
    //  edit, we ask them if they want to change it or not, since we can just keep the
    //  existing trigger data and just write it back.
    //
    //  Note that if they update the trigger data in an edit, we have to delete the
    //  original one and re-insert the new one. Otherwise, we'll have two entries that
    //  do the same thing but with both the old and new keys.

    tCIDLib::TBoolean bUpdateKey = kCIDLib::True;
    TString strRetrainKey;
    if (!bNew)
    {
        TYesNoBox msgbAsk(facIRClient().strMsg(kIRCMsgs::midQ_UpdateKey));

        bUpdateKey = msgbAsk.bShowIt(*this);
        strRetrainKey = csrcTmp.strId();
    }

    // If we are getting a new key, then lets do that
    TString strKeyValue;
    if (bUpdateKey)
    {
        if (bGetIREvent(strKeyValue, strRetrainKey))
        {
            //
            //  Ok, we are good to go, so if its not a new one, delete the
            //  original before we go further.
            //
            if (!bNew)
            {
                m_pwndDriver->orbcServer()->c4SendCmd
                (
                    m_pwndDriver->strMoniker()
                    , kCQCIR::strDeleteEvent
                    , csrcTmp.strId()
                    , m_pwndDriver->sectUser()
                );
            }
        }
         else
        {
            // We got no IREvent or they bailed out
            return;
        }

        // Its either new or they want to update the key, so store it
        csrcTmp.strId(strKeyValue);
    }

    //
    //  Now update the object with the new key and store it. We don't
    //  care about the return from the store. It just indicates whether it
    //  was a new entry or an update.
    //
    tCIDLib::TCard4 c4Bytes = 0;
    THeapBuf mbufData(8192UL);
    {
        TBinMBufOutStream strmData(&mbufData);
        strmData << csrcTmp << kCIDLib::FlushIt;
        c4Bytes = strmData.c4CurSize();
    }

    // We don't use the data type string, so just pass an empty one
    TString strType;
    m_pwndDriver->orbcServer()->bSendData
    (
        m_pwndDriver->strMoniker()
        , kCQCIR::strStoreTrainData
        , strType
        , c4Bytes
        , mbufData
        , m_pwndDriver->sectUser()
    );

    // Then we will update our list to get the latest info loaded
    LoadEvents(csrcTmp.strTitle());
}

