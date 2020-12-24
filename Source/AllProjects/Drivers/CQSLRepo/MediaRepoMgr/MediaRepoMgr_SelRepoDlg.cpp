//
// FILE NAME: MediaRepoMgr_SelRepoDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/17/2006
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
//  This is the header for the MediaRepoMGr_SelRepoDlg.cpp file, which
//  implements a small dialog box that is used to allow the user to select a
//  media repository to manage.
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
#include    "MediaRepoMgr_SelRepoDlg.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMediaRepoSelRepoDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoSelRepoDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaRepoSelRepoDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaRepoSelRepoDlg::TMediaRepoSelRepoDlg() :

    m_pmdbToFill(0)
    , m_pwndCancel(0)
    , m_pwndReload(0)
    , m_pwndRepoList(0)
    , m_pwndSelect(0)
{
}

TMediaRepoSelRepoDlg::~TMediaRepoSelRepoDlg()
{
}


// ---------------------------------------------------------------------------
//  TMediaRepoSelRepoDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TMediaRepoSelRepoDlg::bRunDlg(  const   TWindow&                wndOwner
                                , const TString&                strLeaseId
                                ,       tCQCMedia::EMediaTypes& eInitType
                                ,       TMediaDB&               mdbToFill
                                ,       TString&                strRepoMoniker
                                ,       TString&                strRepoPath
                                ,       TString&                strDBSerialNum)
{
    //
    //  Store points to the more complex stuff we need to fill in on selection. On
    //  entry previous repo moniker is passed, so copy that then clear the parm.
    //
    m_pmdbToFill = &mdbToFill;
    m_strCurMoniker = strRepoMoniker;
    m_strLeaseId = strLeaseId;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_SelRepo
    );

    // If accepted, then copy some stuff back that didn't get filled in directly
    if (c4Res == kMediaRepoMgr::ridDlg_SelRepo_Select)
    {
        eInitType = m_eInitType;
        strDBSerialNum = m_strDBSerialNum;
        strRepoMoniker = m_strRepoMoniker;
        strRepoPath = m_strRepoPath;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TMediaRepoSelRepoDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaRepoSelRepoDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelRepo_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelRepo_Reload, m_pwndReload);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelRepo_List, m_pwndRepoList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelRepo_Select, m_pwndSelect);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TMediaRepoSelRepoDlg::eClickHandler);
    m_pwndReload->pnothRegisterHandler(this, &TMediaRepoSelRepoDlg::eClickHandler);
    m_pwndSelect->pnothRegisterHandler(this, &TMediaRepoSelRepoDlg::eClickHandler);

    // Find all the repository drivers and load the list
    LoadList();
    return bRet;
}


tCIDCtrls::EEvResponses TMediaRepoSelRepoDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelRepo_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_SelRepo_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelRepo_Reload)
    {
        LoadList();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelRepo_Select)
    {
        // Make sure there's an item selected the list
        const tCIDLib::TCard4 c4ListInd = m_pwndRepoList->c4CurItem();
        if (c4ListInd == kCIDLib::c4MaxCard)
            return tCIDCtrls::EEvResponses::Handled;

        TString strRepoSel;
        m_pwndRepoList->ValueAt(c4ListInd, strRepoSel);

        //
        //  We need to get some info from the repository driver and store
        //  it on the facility class.
        //
        try
        {
            // Get a client proxy for the repo's management interface
            tMediaRepoMgr::TRepoMgrProxyPtr orbcRepo = facMediaRepoMgr.orbcMakeProxy(strRepoSel);

            // Try to get the lease. If rejected, tell the user why
            if (!orbcRepo->bGetLease(m_strLeaseId, kCIDLib::False))
            {
                // Couldn't get it, so tell the user
                TYesNoBox msgbInUse
                (
                    facMediaRepoMgr.strMsg
                    (
                        kMRMgrMsgs::midStatus_RepoAlreadyLeased, strRepoSel
                    )
                );
                if (msgbInUse.bShowIt(*this))
                {
                    if (!orbcRepo->bGetLease(m_strLeaseId, kCIDLib::True))
                    {
                        // Couldn't get it, so tell the user
                        TErrBox msgbInUse
                        (
                            facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_LeaseOverFailed)
                        );
                        msgbInUse.ShowIt(*this);
                        return tCIDCtrls::EEvResponses::Handled;
                    }
                 }
                  else
                 {
                    return tCIDCtrls::EEvResponses::Handled;
                 }
            }

            //
            //  Try to drop the lease on the previous repository if they didn't pick
            //  the same one again.
            //
            try
            {
                if ((m_strCurMoniker != strRepoSel) && !m_strCurMoniker.bIsEmpty())
                {
                    tMediaRepoMgr::TRepoMgrProxyPtr orbcOldProxy
                    (
                        facMediaRepoMgr.orbcMakeProxy(m_strCurMoniker)
                    );
                    orbcOldProxy->DropLease(m_strLeaseId);
                }
            }

            catch(TError& errToCatch)
            {
                // Log it, but not much else we can do
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                if (facMediaRepoMgr.bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);
            }

            //
            //  Download the repos's media database and load it up into a local
            //  temp to see if it works. If we get no data, then we start with
            //  this empty database.
            //
            tCQCMedia::EMTFlags eMTFlags = tCQCMedia::EMTFlags::Music;
            try
            {
                const tCIDLib::TBoolean bNewData = facMediaRepoMgr.bDownloadDB
                (
                    orbcRepo, *m_pmdbToFill, eMTFlags, m_strDBSerialNum, m_strRepoPath
                );

                if (bNewData)
                {
                    // Based on supported media types, select an initial type
                    if (facCQCMedia().bTestMediaFlags(tCQCMedia::EMediaTypes::Music, eMTFlags))
                    {
                        m_eInitType = tCQCMedia::EMediaTypes::Music;
                    }
                     else if (facCQCMedia().bTestMediaFlags(tCQCMedia::EMediaTypes::Movie, eMTFlags))
                    {
                        m_eInitType = tCQCMedia::EMediaTypes::Movie;
                    }
                     else
                    {
                        // Indicate we can't take this one
                        TErrBox msgbNoMType
                        (
                            facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoMTypes)
                        );
                        msgbNoMType.ShowIt(*this);
                        return tCIDCtrls::EEvResponses::Handled;
                    }
                }

                // Give back the moniker
                m_strRepoMoniker = strRepoSel;
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // Reset and don't select this one
                m_eInitType = tCQCMedia::EMediaTypes::Count;
                m_pmdbToFill->Reset();
                return tCIDCtrls::EEvResponses::Handled;
            }

            // It worked, so exit with success
            EndDlg(kMediaRepoMgr::ridDlg_SelRepo_Select);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , facMediaRepoMgr.strMsg(kMRMgrErrs::errcComm_ConnectError)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TMediaRepoSelRepoDlg::LoadList()
{
    m_pwndRepoList->RemoveAll();

    //
    //  Normally we'd just call bFindRepoDrivers() but now that we have
    //  split into V1 and V2 drivers, we have to use the more general
    //  method so that we can find both V1 and V2 CQSL Repo drivers.
    //  We use a regular expression filter for the model to do this.
    //
    TVector<TString> colDrivers;
    tCQCKit::TDevCatSet btsCats;
    btsCats.bSetBitState(tCQCKit::EDevCats::MediaRepository, kCIDLib::True);

    if (facCQCKit().bFindDrivers(TString::strEmpty()
                                , L"CQSL"
                                , L"CQSLRepo|CQSLRepo2"
                                , btsCats
                                , colDrivers))
    {
        const tCIDLib::TCard4 c4Count = colDrivers.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                m_pwndRepoList->c4AddItem(colDrivers[c4Index]);
            m_pwndRepoList->SelectByIndex(0);
        }
    }
}

