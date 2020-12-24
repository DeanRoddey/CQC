//
// FILE NAME: MediaRepoMgr_ScanDrvsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/20/2006
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
#include    "MediaRepoMgr_ScanDrvsDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TScanDrvsDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace MediaRepoMgr_ScanDrvsDlg
{
    // -----------------------------------------------------------------------
    //  We remember the previous selected volume and select it again if it is
    //  in the list next time.
    // -----------------------------------------------------------------------
    TString strLastVol;
}



// ---------------------------------------------------------------------------
//   CLASS: TScanDrvsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TScanDrvsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TScanDrvsDlg::TScanDrvsDlg() :

    m_pwndCancel(0)
    , m_pwndList(0)
    , m_pwndProcess(0)
    , m_strBDType(L"Bluray Media")
    , m_strCDType(L"CD Media")
    , m_strDVDType(L"DVD Media")
    , m_strUnknown(L"Unknown Type")
{
}

TScanDrvsDlg::~TScanDrvsDlg()
{
    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TScanDrvsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TScanDrvsDlg::bRunDlg(  const   TWindow&                    wndOwner
                        ,       TString&                    strToFill
                        ,       TKrnlRemMedia::EMediaTypes& eMType
                        ,       tCIDLib::TCard4&            c4ItemCnt)
{
    //
    //  Scan for all available drives with media in thema and load up our
    //  lists.
    //
    try
    {
        TKrnlLList<TKrnlString> kllstDrvs;
        if (!TKrnlRemMedia::bEnumDrvs(kllstDrvs))
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrMsgs::midStatus_EnumDrvs
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        if (kllstDrvs.bIsEmpty())
        {
            TErrBox msgbNoDevs(L"No drives of the appropriate sort were found on this system");
            msgbNoDevs.ShowIt(wndOwner);
            return kCIDLib::False;
        }

        //
        //  Create an input text stream over it, and break them out into
        //  separate strings.
        //
        TKrnlRemMediaDrv    rmmdCur;
        TString             strTmp;
        TKrnlString*        pkstrCur;
        TKrnlRemMedia::TCDTOCInfo TOCData;
        while (kllstDrvs.bNext(pkstrCur))
        {
            // Get the next volume and try to open it up
            if (!rmmdCur.bSetPath(pkstrCur->pszValue()) || !rmmdCur.bOpen())
            {
                rmmdCur.Close();
                continue;
            }

            //
            //  If not ready, or we can't get the media type, or it's not
            //  a valid type, then skip it.
            //
            TKrnlRemMedia::EMediaTypes eType;
            if (!rmmdCur.bCheckIsReady()
            ||  !rmmdCur.bMediaType(eType)
            ||  !TKrnlRemMedia::bIsValidType(eType))
            {
                rmmdCur.Close();
                continue;
            }

            //
            //  If we can't read the TOC then skip it. Else read it and add the
            //  number of tracks to our item count list.
            //
            if (TKrnlRemMedia::bIsCDType(eType))
            {
                if (!rmmdCur.bQueryCDTOC(TOCData))
                {
                    rmmdCur.Close();
                    continue;
                }
                m_fcolItemCnts.c4AddElement((TOCData.c1LastTrack - TOCData.c1FirstTrack) + 1);
            }
             else
            {
                m_fcolItemCnts.c4AddElement(0);
            }

            m_colVolumes.objAdd(pkstrCur->pszValue());
            m_fcolTypes.c4AddElement(eType);
            rmmdCur.Close();
        }
    }

    catch( TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            wndOwner
            , strWndText()
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_ScanDrvs)
            , errToCatch
        );
        return kCIDLib::False;
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_ScanDrvs
    );

    if (c4Res == kMediaRepoMgr::ridDlg_ScanDrvs_Process)
    {
        // Give the user back the selected volume/type
        c4ItemCnt = m_fcolItemCnts[m_c4Selected];
        strToFill = m_colVolumes[m_c4Selected];
        eMType = m_fcolTypes[m_c4Selected];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TScanDrvsDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TScanDrvsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_ScanDrvs_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_ScanDrvs_List, m_pwndList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_ScanDrvs_Process, m_pwndProcess);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TScanDrvsDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TScanDrvsDlg::eComboHandler);
    m_pwndProcess->pnothRegisterHandler(this, &TScanDrvsDlg::eClickHandler);

    // Load up the drives
    const tCIDLib::TCard4 c4Count = m_colVolumes.c4ElemCount();
	if (c4Count)
    {
        tCIDLib::TCard4 c4InitSel = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strCur = m_colVolumes[c4Index];
            m_pwndList->c4AddItem(strCur);
            if (strCur == MediaRepoMgr_ScanDrvsDlg::strLastVol)
                c4InitSel = c4Index;
        }
        m_pwndList->SelectByIndex(c4InitSel);
    }
    return bRet;
}


tCIDCtrls::EEvResponses TScanDrvsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_ScanDrvs_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_ScanDrvs_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_ScanDrvs_Process)
    {
        m_c4Selected = m_pwndList->c4CurItem();
        if (m_c4Selected != kCIDLib::c4MaxCard)
        {
            // Store this one as new last selected volume
            MediaRepoMgr_ScanDrvsDlg::strLastVol = m_colVolumes[m_c4Selected];
            EndDlg(kMediaRepoMgr::ridDlg_ScanDrvs_Process);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TScanDrvsDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Update the type display for the newly selected drive
        const tCIDLib::TCard4 c4Index = wnotEvent.c4Index();

        TKrnlRemMedia::EMediaTypes eMType = m_fcolTypes[c4Index];
        if (TKrnlRemMedia::bIsCDType(eMType))
            strChildText(kMediaRepoMgr::ridDlg_ScanDrvs_MediaInfo, m_strCDType);
        else if (TKrnlRemMedia::bIsDVDType(eMType))
            strChildText(kMediaRepoMgr::ridDlg_ScanDrvs_MediaInfo, m_strDVDType);
        else if (TKrnlRemMedia::bIsBDType(eMType))
            strChildText(kMediaRepoMgr::ridDlg_ScanDrvs_MediaInfo, m_strBDType);
        else
            strChildText(kMediaRepoMgr::ridDlg_ScanDrvs_MediaInfo, m_strUnknown);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

