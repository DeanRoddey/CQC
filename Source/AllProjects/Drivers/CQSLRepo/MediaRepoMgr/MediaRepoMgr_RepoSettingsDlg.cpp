//
// FILE NAME: MediaRepoMgr_RepoSettings.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2006
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
//  This is the header for the MediaRepoMgr_RepoSettingsDlg.cpp file, which
//  implements a dialog box that drives the CD ripping process and displays
//  status information.
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
#include    "MediaRepoMgr.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRepoSettingsDlg, TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TRepoSettingsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRepoSettingsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TRepoSettingsDlg::TRepoSettingsDlg() :

    m_pwndCancel(nullptr)
    , m_pwndCodecList(nullptr)
    , m_pwndFormatList(nullptr)
    , m_pwndJitter(nullptr)
    , m_pwndMMName(nullptr)
    , m_pwndMMPassword(nullptr)
    , m_pwndRipEject(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndShowAdult(nullptr)
    , m_pwndShowEnglish(nullptr)
    , m_pwndUseMM(nullptr)
{
}

TRepoSettingsDlg::~TRepoSettingsDlg()
{
}


// ---------------------------------------------------------------------------
//  TRepoSettingsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TRepoSettingsDlg::bRunDlg(const TWindow& wndOwner, TMediaRepoSettings& mrsToEdit)
{
    // Copy the settings to edit
    m_mrsToEdit = mrsToEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_RepoSet
    );
    if (c4Res == kMediaRepoMgr::ridDlg_RepoSet_Save)
    {
        // Copy the changes back to the caller
        mrsToEdit = m_mrsToEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TRepoSettingsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRepoSettingsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_CodecList, m_pwndCodecList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_FormatList, m_pwndFormatList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_JitterCor, m_pwndJitter);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_MMName, m_pwndMMName);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_MMPassword, m_pwndMMPassword);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_RipEject, m_pwndRipEject);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_Save, m_pwndSave);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_ShowAdult, m_pwndShowAdult);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_ShowEnglish, m_pwndShowEnglish);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RepoSet_UseMM, m_pwndUseMM);

    //
    //  Get the available encoders and their formats from the DAE facility.
    //  There aren't many so we'll go ahead and just load them all up front
    //  into a vector of vectors and use that to drive the codec and format
    //  list boxes. Any codecs that come back empty we'll just leave out.
    //
    //  Note that for now we just create a WMA encoder, but later we'll
    //  also have a selection of encoders.
    //
    TCIDDAEWMAEnc* pdaeeInfo = new TCIDDAEWMAEnc;
    TJanitor<TCIDDAEWMAEnc> janEnc(pdaeeInfo);

    //
    //  First get the list of encodings. Then for each codec, get a list of
    //  the available formats. Any that come up empty, discard them.
    //
    tCIDLib::TCard4 c4CodecCnt = pdaeeInfo->c4QueryCodecNames(m_colCodecNames);
    tCIDLib::TCard4 c4CodecInd = 0;
    TNameList       colCurFmts;
    while(c4CodecInd < c4CodecCnt)
    {
        colCurFmts.RemoveAll();
        const tCIDLib::TCard4 c4FCnt = pdaeeInfo->c4QueryCodecFmtNames
        (
            m_colCodecNames[c4CodecInd], colCurFmts
        );

        if (c4FCnt)
        {
            c4CodecInd++;
            m_colCodecFmts.objAdd(colCurFmts);
        }
         else
        {
            c4CodecCnt--;
            m_colCodecNames.RemoveAt(c4CodecInd);
        }
    }

    //
    //  OK, load up the lists. We do them before we install the handlers
    //  so that we can re-select any incoming current values.
    //
    if (c4CodecCnt)
    {
        tCIDLib::TCard4 c4InitSel = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CodecCnt; c4Index++)
        {
            const TString& strCur = m_colCodecNames[c4Index];
            m_pwndCodecList->c4AddItem(strCur);
            if (strCur == m_mrsToEdit.strCodec())
                c4InitSel = c4Index;
        }
        m_pwndCodecList->SelectByIndex(c4InitSel);

        const tCIDLib::TStrList& colFormats = m_colCodecFmts[c4InitSel];
        const tCIDLib::TCard4 c4FmtCnt = colFormats.c4ElemCount();
        if (c4FmtCnt)
        {
            c4InitSel = 0;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FmtCnt; c4Index++)
            {
                const TString& strCur = colFormats[c4Index];
                m_pwndFormatList->c4AddItem(strCur);
                if (strCur == m_mrsToEdit.strCodecFmt())
                    c4InitSel = c4Index;
            }
            m_pwndFormatList->SelectByIndex(c4InitSel);
        }
    }

    // Install handlers
    m_pwndCancel->pnothRegisterHandler(this, &TRepoSettingsDlg::eClickHandler);
    m_pwndCodecList->pnothRegisterHandler(this, &TRepoSettingsDlg::eComboHandler);
    m_pwndSave->pnothRegisterHandler(this, &TRepoSettingsDlg::eClickHandler);

    // Set the ripping options
    m_pwndJitter->SetCheckedState(m_mrsToEdit.bJitterCorrection());
    m_pwndRipEject->SetCheckedState(m_mrsToEdit.bRipEject());

    // Set the My Movies service options
    m_pwndUseMM->SetCheckedState(m_mrsToEdit.bUseMyMovies());
    m_pwndMMName->strWndText(m_mrsToEdit.strMMName());
    m_pwndMMPassword->strWndText(m_mrsToEdit.strMMPassword());
    m_pwndShowAdult->SetCheckedState(m_mrsToEdit.bShowAdult());
    m_pwndShowEnglish->SetCheckedState(m_mrsToEdit.bShowEnglish());

    return bRet;
}


// ---------------------------------------------------------------------------
//  TRepoSettingsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TRepoSettingsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_RepoSet_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_RepoSet_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_RepoSet_Save)
    {
        // Get the values out and store them
        try
        {
            // Make sure we have a codec and format
            const tCIDLib::TCard4 c4CodecInd = m_pwndCodecList->c4CurItem();
            const tCIDLib::TCard4 c4FmtInd = m_pwndFormatList->c4CurItem();

            if ((c4CodecInd == kCIDLib::c4MaxCard)
            ||  (c4FmtInd == kCIDLib::c4MaxCard))
            {
                TOkBox msgbErr
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoEncodeVals)
                );
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }

            m_mrsToEdit.bJitterCorrection(m_pwndJitter->bCheckedState());
            m_mrsToEdit.bRipEject(m_pwndRipEject->bCheckedState());

            m_mrsToEdit.strCodec(m_colCodecNames[c4CodecInd]);
            const TVector<TString>& colFormats = m_colCodecFmts[c4CodecInd];
            m_mrsToEdit.strCodecFmt(colFormats[c4FmtInd]);

            m_mrsToEdit.bUseMyMovies(m_pwndUseMM->bCheckedState());
            m_mrsToEdit.strMMName(m_pwndMMName->strWndText());
            m_mrsToEdit.strMMPassword(m_pwndMMPassword->strWndText());
            m_mrsToEdit.bShowAdult(m_pwndShowAdult->bCheckedState());
            m_mrsToEdit.bShowEnglish(m_pwndShowEnglish->bCheckedState());

            //
            //  If they enabled My Movies, test the connection by querying how
            //  many queries they have left.
            //
            tCIDLib::TBoolean bOK = kCIDLib::True;
            if (m_mrsToEdit.bUseMyMovies())
            {
                tCIDLib::TCard4 c4Left;
                TString strErr;

                // Create a my movies source and test the connection
                TMyMoviesMetaSrc rmsTest(m_mrsToEdit);
                bOK = rmsTest.bTestConnection(c4Left, strErr);

                //
                //  If it worked, tell the user the count. If it fails, either it
                //  throws and we show it below, or it's an error response and the
                //  method above showed it.
                //
                if (bOK)
                {
                    TOkBox msgbOK
                    (
                        facMediaRepoMgr.strMsg
                        (
                            kMRMgrMsgs::midStatus_MMConnOK, TCardinal(c4Left)
                        )
                    );
                    msgbOK.ShowIt(*this);
                }
                 else
                {
                    TErrBox msgbErr(strErr);
                    msgbErr.ShowIt(*this);
                }
            }

            // OK, it all worked, so let's exit with success
            if (bOK)
                EndDlg(kMediaRepoMgr::ridDlg_RepoSet_Save);
        }

        catch(const TError& errToCatch)
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_UpdSettingsFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// Handle combo box events for our codec list combo
tCIDCtrls::EEvResponses TRepoSettingsDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Load up the format list for this selected coded
        const tCIDLib::TStrList& colFormats = m_colCodecFmts[wnotEvent.c4Index()];

        m_pwndFormatList->RemoveAll();
        const tCIDLib::TCard4 c4FCnt = colFormats.c4ElemCount();
        if (c4FCnt)
        {
            for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FCnt; c4FInd++)
                m_pwndFormatList->c4AddItem(colFormats[c4FInd]);
            m_pwndFormatList->SelectByIndex(0);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

