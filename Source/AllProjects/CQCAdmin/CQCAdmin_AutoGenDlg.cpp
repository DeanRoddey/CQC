//
// FILE NAME: CQCAdmin_AutoGenDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 5/15/2014
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
//  This file implements the auto-generation dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_AutoGenDlg.hpp"
#include    "CQCIntfEng_BooleanImg.hpp"
#include    "CQCIntfEng_DynamicText.hpp"
#include    "CQCIntfEng_PushButton.hpp"
#include    "CQCIntfEng_CmdButton.hpp"
#include    "CQCIntfEng_Slider.hpp"



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCAdmin_AutoGenDlg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The max size that we'll let our I/O streams grow. 32MB is plenty large enough
        //  for any reasonable image repo image.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MaxBuf(kCIDLib::c4Sz_32M);

        // -----------------------------------------------------------------------
        //  The max size of the contents of a custom overlay's tile template. We
        //  pull that stuff in and create a tile, and it has to be within the
        //  area that is not covered by the top/bottom bars of the tile and the
        //  left/right edge lines.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MaxUserTileHeight = 242;
        constexpr tCIDLib::TCard4   c4MaxUserTileWidth  = 390;
    }
}



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TAutoGenDlg, TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TAutoGenDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAutoGenDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TAutoGenDlg::TAutoGenDlg() :

    m_bSkipImages(kCIDLib::False)
    , m_bVerbose(kCIDLib::False)
    , m_c4PreTmplCnt(0)
    , m_c4SingleImgCnt(0)
    , m_colMsgQ(tCIDLib::EMTStates::Safe)
    , m_strmImg(CQCAdmin_AutoGenDlg::c4MaxBuf / 4, CQCAdmin_AutoGenDlg::c4MaxBuf)
    , m_strmThumb(CQCAdmin_AutoGenDlg::c4MaxBuf / 8, CQCAdmin_AutoGenDlg::c4MaxBuf / 4)
    , m_pwndCloseButton(nullptr)
    , m_pwndGenButton(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndVerbose(nullptr)
    , m_strSrcPath(L"/System/Themes/BlueGlass")
    , m_szSrcRes(1280, 800)
    , m_thrGenerate
      (
        TString(L"CQCAutoGenThread")
        , TMemberFunc<TAutoGenDlg>(this, &TAutoGenDlg::eGenThread)
      )
    , m_tmidToDo(kCIDCtrls::tmidInvalid)
{
}

TAutoGenDlg::~TAutoGenDlg()
{
}


// ---------------------------------------------------------------------------
//  TAutoGenDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAutoGenDlg::RunDlg(const TWindow& wndOwner)
{
    try
    {
        TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
        (
            TCQCSysCfgClientProxy::strBinding, 4000
        );
        TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);

        tCIDLib::TCard4 c4SerialNum = 0;
        orbcProxy->bQueryRoomCfg(c4SerialNum, m_scfgGen);

        // Make sure that we have at least one room
        if (!m_scfgGen.c4RoomCount())
        {
            TOkBox msgbNoRooms(facCQCAdmin.strMsg(kCQCAErrs::errcSysCfg_NoRooms));
            msgbNoRooms.ShowIt(wndOwner);
            return;
        }

        // We need an interface server to upload templates
        m_pdsclSrv = new TDataSrvClient();
    }

    catch(TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            wndOwner
            , strWndText()
            , L"Could not get an interface server proxy"
            , errToCatch
        );
        return;
    }

    // Just run the dialog, we don't have to pass anything in our get any results
    c4RunDlg(wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_AutoGen);

    // Make sure that the thread is all the way down
    try
    {
        if (m_thrGenerate.bIsRunning())
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            m_thrGenerate.ReqShutdownSync(12000);
            m_thrGenerate.eWaitForDeath(5000);
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            wndOwner
            , strWndText()
            , L"The solution generation thread could not be stopped"
            , errToCatch
        );
    }

    // And that the data server client is cleaned up
    if (m_pdsclSrv)
        delete m_pdsclSrv;
}



// ---------------------------------------------------------------------------
//  TAutoGenDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAutoGenDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_AutoGen_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCAdmin::ridDlg_AutoGen_Gen, m_pwndGenButton);
    CastChildWnd(*this, kCQCAdmin::ridDlg_AutoGen_SkipImgs, m_pwndSkipImgs);
    CastChildWnd(*this, kCQCAdmin::ridDlg_AutoGen_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCAdmin::ridDlg_AutoGen_Verbose, m_pwndVerbose);

    // And register our event handlers
    m_pwndCloseButton->pnothRegisterHandler(this, &TAutoGenDlg::eClickHandler);
    m_pwndGenButton->pnothRegisterHandler(this, &TAutoGenDlg::eClickHandler);
    m_pwndSkipImgs->pnothRegisterHandler(this, &TAutoGenDlg::eClickHandler);
    m_pwndVerbose->pnothRegisterHandler(this, &TAutoGenDlg::eClickHandler);

    return bRet;
}


tCIDLib::TVoid TAutoGenDlg::Destroyed()
{
    // Clean up our update timer if it got started and wasn't stopped normally
    if (m_tmidToDo != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidToDo);
        m_tmidToDo = kCIDCtrls::tmidInvalid;
    }

    TParent::Destroyed();
}


//
//  The background thread posts msgs to a queue, which we grab out and display
//  and/or react to. The msgs are in the form:
//
//  x - Msg
//
//  Where x is a msg type:
//
//      I - Info, just display it
//      F - Failure msg. Display it, set done flag, and indicate failure
//      S - Success. Display it, set done flag, and indicate success
//      C - Cancel. Display it, set done flag, and indicate cancelled
//
tCIDLib::TVoid TAutoGenDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    enum ERes
    {
        ERes_Fail
        , ERes_Success
        , ERes_Cancel

        , ERes_Unknown
    };

    if (tmidToDo == m_tmidToDo)
    {
        // While there are any msgs in the queue, then process them.
        tCIDLib::TBoolean   bDone = kCIDLib::False;
        tCIDLib::TCard4     c4Count = 0;
        ERes                eRes = ERes_Unknown;
        TString             strText;
        while (m_colMsgQ.bPopBottom(m_strQMsg))
        {
            // Shouldn't happen, but just in case
            if (m_strQMsg.c4Length() < 5)
                continue;

            const tCIDLib::TCh chStatus = m_strQMsg[0];

            // Cut the leading bit off and display the msg
            m_strQMsg.Cut(0, 4);
            strText = m_strQMsg;
            strText.Append(L"\n");
            c4Count++;

            if (chStatus == kCIDLib::chLatin_F)
            {
                bDone = kCIDLib::True;
                eRes = ERes_Fail;
            }
             else if (chStatus == kCIDLib::chLatin_S)
            {
                bDone = kCIDLib::True;
                eRes = ERes_Success;
            }
             else if (chStatus == kCIDLib::chLatin_C)
            {
                bDone = kCIDLib::True;
                eRes = ERes_Cancel;
            }

            m_pwndStatus->AddString(strText);
        }

        //
        //  If it's one of the ones that indicate the end, then re-enable controls
        //  appropriately.
        //
        if (bDone)
        {
            // We can stop the timer now
            if (m_tmidToDo != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidToDo);
                m_tmidToDo = kCIDCtrls::tmidInvalid;
            }

            // Re-enable the stuff now that a new one could be started
            m_pwndGenButton->SetEnable(kCIDLib::True);
            m_pwndSkipImgs->SetEnable(kCIDLib::True);

            // Set the close button text back to close
            m_pwndCloseButton->strWndText(L"Close");

            // And show the status to the user
            TString strMsg;
            if (eRes == ERes_Success)
                strMsg = L"The generation completed successfully";
            else if (eRes == ERes_Cancel)
                strMsg = L"The generation was cancelled";
            else
                strMsg = L"The generation failed due to an error";

            TOkBox msgbDone(strMsg);
            msgbDone.ShowIt(*this);
        }
    }
}


// ---------------------------------------------------------------------------
//  TAutoGenDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to add the pre-fab templates to the template paths list. We return
//  the index at which it went.
//
tCIDLib::TCard4 TAutoGenDlg::c4AddPrefabTmplsToList(const tCIDLib::TCh* const pszRelPath)
{
    // Set up the source path
    TString strSrc(m_strSrcPath);
    strSrc.Append(kCIDLib::chForwardSlash);
    strSrc.Append(pszRelPath);

    //
    //  And the target path. We have to add a replacement token for the resolution
    //  and the room name.
    //
    TString strTar(m_scfgGen.strTarScope());
    strTar.Append(L"/%(r)/%(n)/");
    strTar.Append(pszRelPath);

    // The return value is the current count, which is the index of the new item
    const tCIDLib::TCard4 c4Ret = m_colTPaths.c4ElemCount();

    // And we can now add an entry to the template paths list
    m_colTPaths.objPlace(strSrc, strTar);

    return c4Ret;
}


//
//  Given a target resolution. Calculate the horz and vertical scaling factors
//  required, relative to our known source res.
//
tCIDLib::TVoid
TAutoGenDlg::CalcScaleFactors(  const   TSize&              szTarRes
                                ,       tCIDLib::TFloat8&   f8XScale
                                ,       tCIDLib::TFloat8&   f8YScale)
{
    f8XScale = tCIDLib::TFloat8(szTarRes.c4Width())
                / tCIDLib::TFloat8(m_szSrcRes.c4Width());

    f8YScale = tCIDLib::TFloat8(szTarRes.c4Height())
                / tCIDLib::TFloat8(m_szSrcRes.c4Height());
}


//
//  Widgets often need to be removed based on configuration. Since they can be part of
//  a group or not, and have to be removed differently if so, these methods are called
//  to do it.
//
tCIDLib::TVoid
TAutoGenDlg::DeleteWidget(TCQCIntfTemplate& iwdgParent, const TString& strName)
{
    // Look up the widget by name and call the other version
    TCQCIntfWidget* pwidgTar = iwdgParent.piwdgFindByName(strName, kCIDLib::False);
    DeleteWidget(iwdgParent, pwidgTar);
}

tCIDLib::TVoid
TAutoGenDlg::DeleteWidget(TCQCIntfTemplate& iwdgParent, TCQCIntfWidget* const piwdgToDelete)
{
    //
    //  If it's part of a group, orphan it from the group then delete it. Else, just
    //  do a normal remove.
    //
    if (piwdgToDelete->c4GroupId() == kCIDLib::c4MaxCard)
    {
         iwdgParent.RemoveWidget(piwdgToDelete);
    }
     else
    {
        tCIDLib::TBoolean bGrpDestroyed;
        tCIDLib::TCard4 c4GrpZ;
        iwdgParent.OrphanGrpMember(piwdgToDelete, c4GrpZ, bGrpDestroyed);

        // We are responsible for cleaning it up
        delete piwdgToDelete;
    }
}


// Handle button click events
tCIDCtrls::EEvResponses TAutoGenDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_AutoGen_Close)
    {
        EndDlg(kCQCAdmin::ridDlg_AutoGen_Close);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_AutoGen_Gen)
    {
        // Disable some stuff while we are in here
        m_pwndGenButton->SetEnable(kCIDLib::False);
        m_pwndSkipImgs->SetEnable(kCIDLib::False);

        // Make sure the status output window is clear
        m_pwndStatus->ClearText();

        // Set the close button text to Cancel while this is happening
        m_pwndCloseButton->strWndText(L"Cancel");

        // And make sure the msg queue is empty
        m_colMsgQ.RemoveAll();

        // Start the msg processing timer for a half second period
        m_tmidToDo = tmidStartTimer(500);

        // OK, let's start up the thread
        m_thrGenerate.Start();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_AutoGen_SkipImgs)
    {
        m_bSkipImages = m_pwndSkipImgs->bCheckedState();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_AutoGen_Verbose)
    {
        m_bVerbose = m_pwndVerbose->bCheckedState();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We point the background generation thread at this method. We do the work here
//  and post status messages to the GUI thread.
//
//  In order to prevent the engine from auto-generating names for unnamed widgets,
//  we put the engine into designer mode while we are generating the content.
//  We have to be sure to get it back out again when done.
//
tCIDLib::EExitCodes TAutoGenDlg::eGenThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the parent thread go
    thrThis.Sync();

    try
    {
        // Flush any lists that are per-round
        m_colScopeMap.RemoveAll();
        m_colImgMap.RemoveAll();
        m_colSrcTmpls.RemoveAll();
        m_colTPaths.RemoveAll();
        m_colTileTmpls.RemoveAll();

        // Add all of the prefab templates to the template paths list
        m_c4TmplInd_Main = c4AddPrefabTmplsToList(L"Main");
        c4AddPrefabTmplsToList(L"Overlay_Lights");
        c4AddPrefabTmplsToList(L"Overlay_Lights_Current");
        m_c4TmplInd_LightsList = c4AddPrefabTmplsToList(L"Overlay_Lights_List");
        c4AddPrefabTmplsToList(L"Overlay_Main");
        m_c4TmplInd_MainLeft = c4AddPrefabTmplsToList(L"Overlay_Main_Left");
        m_c4TmplInd_Tiles = c4AddPrefabTmplsToList(L"Overlay_Main_Tiles");
        c4AddPrefabTmplsToList(L"Overlay_MovieBrowse");
        m_c4TmplInd_MovieNP = c4AddPrefabTmplsToList(L"Overlay_MoviePlaying");
        c4AddPrefabTmplsToList(L"Overlay_MusicBrowse");
        m_c4TmplInd_MusicNP = c4AddPrefabTmplsToList(L"Overlay_MusicPlaying");
        c4AddPrefabTmplsToList(L"Overlay_Weather");
        m_c4TmplInd_WeatherFC = c4AddPrefabTmplsToList(L"Overlay_Weather_FCDays");

        c4AddPrefabTmplsToList(L"Overlay_HVAC");
        c4AddPrefabTmplsToList(L"Overlay_HVAC_Current");
        m_c4TmplInd_HVACList = c4AddPrefabTmplsToList(L"Overlay_HVAC_List");

        // Layout templates
        c4AddPrefabTmplsToList(L"Layouts/MovieCAB");
        c4AddPrefabTmplsToList(L"Layouts/MusicCAB");
        c4AddPrefabTmplsToList(L"Layouts/MusicItemBrowser");

        // Popups
        c4AddPrefabTmplsToList(L"Popup/Popup_AdjustVal");
        c4AddPrefabTmplsToList(L"Popup/Popup_Login");
        c4AddPrefabTmplsToList(L"Popup/Popup_MediaSearch");
        c4AddPrefabTmplsToList(L"Popup/Popup_OK");
        c4AddPrefabTmplsToList(L"Popup/Popup_PLMode");
        c4AddPrefabTmplsToList(L"Popup/Popup_SelCategory");
        c4AddPrefabTmplsToList(L"Popup/Popup_SelRoom");
        c4AddPrefabTmplsToList(L"Popup/Popup_SortMode");
        c4AddPrefabTmplsToList(L"Popup/Popup_VolControl");
        c4AddPrefabTmplsToList(L"Popup/Popup_YesNo");
        c4AddPrefabTmplsToList(L"Popup/Popup_YesNoCancel");


        // If there is a custom layout, then add it
        if (!m_scfgGen.strCustLayout().bIsEmpty())
        {
            TString strTar(m_scfgGen.strTarScope());
            strTar.Append(L"/%(r)/%(n)/Overlay_CustLayout");
            m_colTPaths.objPlace(m_scfgGen.strCustLayout(), strTar);
        }


        // Remember the pre-fab count of templates
        m_c4PreTmplCnt = m_colTPaths.c4ElemCount();

        //
        //  Add any extra overlays to the template source path list. These we just
        //  create arbitrary target file names for, so that we can put them all
        //  into a single sub-scope under the per-room scopes. We put in
        //  replacement tokens for both resolution and room. And we add the sub
        //  scope that we put all of these into. Then we just need to add a generated
        //  name for each one.
        //
        //  NOTE: We only put the content templates in here. The tiles are not
        //  actually processed themselves. We integrate them into the tiles
        //  screen. They will be handled separately.
        //
        const tCIDLib::TCard4 c4XOverCnt = m_scfgGen.c4ListIdCount
        (
            tCQCSysCfg::ECfgLists::XOvers
        );
        if (c4XOverCnt)
        {
            TString strTarBase(m_scfgGen.strTarScope());
            strTarBase.Append(L"/%(r)/%(n)/XOvers");

            TString strTar;
            tCIDLib::TCard4 c4NameNum = 1;
            for (tCIDLib::TCard4 c4XInd = 0; c4XInd < c4XOverCnt; c4XInd++)
            {
                const TSCXOverInfo& scliCur = m_scfgGen.scliXOverAt(c4XInd);

                //
                //  Check that both are non-empty. If there's no tile then the
                //  content overlay cannot ever be invoked.
                //
                if (!scliCur.m_strTileTmpl.bIsEmpty()
                &&  !scliCur.m_strContentTmpl.bIsEmpty())
                {
                    strTar = strTarBase;
                    strTar.Append(L"/XOver_");
                    strTar.AppendFormatted(c4NameNum++);
                    m_colTPaths.objPlace(scliCur.m_strContentTmpl, strTar);
                }
            }
        }

        //
        //  First thing is the most obvious scenario. See if the target scope exists. If
        //  so, we have to clean it out. If this can't be done for some reason, no point
        //  doing anything else. If it doesn't exist, we need to create it.
        //
        if (m_pdsclSrv->bScopeExists(m_scfgGen.strTarScope(), tCQCRemBrws::EDTypes::Template))
        {
            QUpMsg(L"I - Cleaning up target template scope...");
            m_pdsclSrv->DeleteScope
            (
                m_scfgGen.strTarScope()
                , tCQCRemBrws::EDTypes::Template
                , facCQCAdmin.cuctxToUse().sectUser()
            );
        }
         else
        {
            m_pdsclSrv->MakePath
            (
                m_scfgGen.strTarScope()
                , tCQCRemBrws::EDTypes::Template
                , facCQCAdmin.cuctxToUse().sectUser()
            );
        }

        if (!m_bSkipImages)
        {
            if (m_pdsclSrv->bScopeExists(m_scfgGen.strTarScope(), tCQCRemBrws::EDTypes::Image))
            {
                QUpMsg(L"I - Cleaning up target image scope...");
                m_pdsclSrv->DeleteScope
                (
                    m_scfgGen.strTarScope()
                    , tCQCRemBrws::EDTypes::Image
                    , facCQCAdmin.cuctxToUse().sectUser()
                );
            }
             else
            {
                m_pdsclSrv->MakePath
                (
                    m_scfgGen.strTarScope()
                    , tCQCRemBrws::EDTypes::Image
                    , facCQCAdmin.cuctxToUse().sectUser()
                );
            }
        }
        QUpMsg(L"I - Target scope cleanup complete\n");

        //
        //  Next step is to just go ahead and download the src templates so that we
        //  have them around. We'll need to make numerous passes through them, so
        //  best to just have them here locally.
        //
        //  We just put them into a list in the same order a the source templates
        //  list, so that the same index can be used for both.
        //
        QUpMsg(L"I - Querying templates...");

        tCIDLib::TCard4 c4Bytes;
        tCIDLib::TCard4 c4SerialNum;
        tCIDLib::TEncodedTime enctLastChange;
        THeapBuf mbufTmpl(64 * 1024UL);
        TString strCur(m_strSrcPath);
        tCIDLib::TKVPFList colMeta;
        tCIDLib::TCard4 c4TmplCnt = m_colTPaths.c4ElemCount();
        TCQCIntfTemplate iwdgCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
        {
            const TString& strSrcPath = m_colTPaths[c4Index].strKey();

            if (m_bVerbose)
                QUpMsg(L"I -     %(1)", strSrcPath);

            //
            //  We don't use the helper methods in the CQCIntfEng facility. We use
            //  the data server client directly and tell it not to cache these things.
            //
            {
                c4SerialNum = 0;
                m_pdsclSrv->bReadTemplate
                (
                    strSrcPath
                    , c4SerialNum
                    , enctLastChange
                    , mbufTmpl
                    , c4Bytes
                    , colMeta
                    , facCQCAdmin.cuctxToUse().sectUser()
                    , kCIDLib::True
                );

                TBinMBufInStream strmSrc(&mbufTmpl, c4Bytes);
                strmSrc >> iwdgCur;

                m_colSrcTmpls.objAdd(iwdgCur);
            }

            // Allow us to be shut down quickly, and avoid hitting the server too hard
            if (!thrThis.bSleep(150))
            {
                QUpMsg(L"C - \nOperation was cancelled\n");
                return tCIDLib::EExitCodes::Normal;
            }
        }

        //
        //  Do the same for any tile templates, but into a separate list. These originals
        //  will never be uploaded, so make things easier on us by putting the Xover
        //  UID into the template name. This will help later to make it obvious which
        //  XOver these templates represent.
        //
        for (tCIDLib::TCard4 c4XInd = 0; c4XInd < c4XOverCnt; c4XInd++)
        {
            const TSCXOverInfo& scliCur = m_scfgGen.scliXOverAt(c4XInd);
            if (scliCur.m_strTileTmpl.bIsEmpty())
                continue;

            if (m_bVerbose)
                QUpMsg(L"I -     %(1)", scliCur.m_strTileTmpl);

            {
                c4SerialNum = 0;
                m_pdsclSrv->bReadTemplate
                (
                    scliCur.m_strTileTmpl
                    , c4SerialNum
                    , enctLastChange
                    , mbufTmpl
                    , c4Bytes
                    , colMeta
                    , facCQCAdmin.cuctxToUse().sectUser()
                    , kCIDLib::True
                );

                TBinMBufInStream strmSrc(&mbufTmpl, c4Bytes);
                strmSrc >> iwdgCur;
            }

            // See block comments above
            iwdgCur.strTemplateName(scliCur.m_strUniqueId);
            m_colTileTmpls.objAdd(iwdgCur);

            if (!thrThis.bSleep(150))
            {
                QUpMsg(L"C - \nOperation was cancelled\n");
                return tCIDLib::EExitCodes::Normal;
            }
        }

        QUpMsg(L"I - Template downloads complete");


        //
        //  This will both build us a list of source/target image pairs for all of
        //  the referenced images, and do the scaling of the images for each res,
        //  leaving them in the target scopes.
        //
        //  The target scopes in the k/v pairs will have a replacement token for
        //  the resolution, so we can create each target resolution's image path
        //  below when we search and replace the image paths during the scaling
        //  and updating process, before we write them back out to the target
        //  scope.
        //
        QUpMsg(L"I - Processing images...");
        ScaleImages(thrThis);
        QUpMsg(L"I - Image processing complete\n");

        // Now update the pre-defined templates and upload them
        QUpMsg(L"I - Generating standard templates...");
        GenTemplates(thrThis);
        QUpMsg(L"I - Standard template generation complete\n");

        // Send a completion status to the dialog and we are done
        QUpMsg(L"S - Generation completed successfully\n");
    }

    catch(TError& errToCatch)
    {
        // Log the error
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Send an error status to the dialog and fall through
        QUpMsg(L"F - %(1)", errToCatch.strErrText());
    }

    catch(...)
    {
        // Send an error status to the dialog and fall through
        QUpMsg(L"F - An unknown exception occurred");
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  Look through our list of previously downloaded tile templates and find the
//  one that has the passed UID stored in its template name.
//
const TCQCIntfTemplate& TAutoGenDlg::iwdgFindTileTmpl(const TString& strUID) const
{
    const tCIDLib::TCard4 c4Count = m_colTileTmpls.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;

    while (c4Index < c4Count)
    {
        if (m_colTileTmpls[c4Index].strTemplateName() == strUID)
            break;
        c4Index++;
    }
    return m_colTileTmpls[c4Index];
}



//
//  Once the images are scaled, this is called to do the template generation. We
//  just iterate the rooms, and then the resolutions. We copy the templates for
//  each resolution and room, scale the templates for the current resolution, then
//  call a helper to do the other updating and uploading required for each round.
//
tCIDLib::TVoid TAutoGenDlg::GenTemplates(TThread& thrThis)
{
    const tCIDLib::TCard4 c4RoomCnt = m_scfgGen.c4RoomCount();

    //
    //  Make a first pass through the rooms and build up an enumerated limit string
    //  that will be set on a variable in the main template OnPreload and used to
    //  display the room list. Actually set set up two, one with the names and one
    //  with the ids.
    //
    m_strRmListIds = L"Enum: ";
    m_strRmListNames = L"Enum: ";
    for (tCIDLib::TCard4 c4RmInd = 0; c4RmInd < c4RoomCnt; c4RmInd++)
    {
        const TCQCSysCfgRmInfo& scriCur = m_scfgGen.scriRoomAt(c4RmInd);
        if (scriCur.bEnabled())
        {
            if (c4RmInd)
            {
                m_strRmListIds.Append(L", ");
                m_strRmListNames.Append(L", ");
            }

            m_strRmListNames.Append(scriCur.strName());
            m_strRmListIds.Append(scriCur.strUniqueId());
        }
    }

    //
    //  Create a field cache for use during this, without any filtering so that
    //  all drivers and fields are available, even though we will only use V2
    //  drivers.
    //
    TCQCFldCache cfcInfo;
    cfcInfo.Initialize(0);

    // Loop through all of the configured rooms
    TTmplList       colModTmpls;
    TGraphWndDev    gdevTmp(*this);
    TString         strTmp;
    TString         strMsg;
    TString         strResFmt;

    for (tCIDLib::TCard4 c4RmInd = 0; c4RmInd < c4RoomCnt; c4RmInd++)
    {
        // If asked to shutdown, then break out now
        if (thrThis.bCheckShutdownRequest())
            break;

        const TCQCSysCfgRmInfo& scriCur = m_scfgGen.scriRoomAt(c4RmInd);

        // If disabled, skip this one
        if (!scriCur.bEnabled())
        {
            if (m_bVerbose)
                QUpMsg(L"I -    Disabled room: %(1)", scriCur.strName());
            continue;
        }

        QUpMsg(L"I -    Processing room: %(1)", scriCur.strName());

        const tCIDLib::TCard4 c4ResCnt = m_scfgGen.c4ResCount();
        for (tCIDLib::TCard4 c4ResInd= 0; c4ResInd< c4ResCnt; c4ResInd++)
        {
            // If asked to shutdown, then break out now
            if (thrThis.bCheckShutdownRequest())
                break;

            const TSize& szCurRes = m_scfgGen.szResAt(c4ResInd);

            // Format out this resolution, which we'll use to create the target path
            szCurRes.FormatToText(strResFmt, tCIDLib::ERadices::Dec, kCIDLib::chLatin_x);
            QUpMsg(L"I -         Resolution: %(1)...", strResFmt);

            // Calc the scale factor for this resolution
            tCIDLib::TFloat8 f8XScale, f8YScale;
            CalcScaleFactors(szCurRes, f8XScale, f8YScale);

            //
            //  To make things simpler, we just pre-create the output scopes
            //  for the templates. There are only three (plus the per resolution
            //  top one.)
            //
            strTmp = m_scfgGen.strTarScope();
            strTmp.Append(kCIDLib::chForwardSlash);
            strTmp.Append(strResFmt);
            strTmp.Append(kCIDLib::chForwardSlash);
            strTmp.Append(scriCur.strName());

            m_pdsclSrv->MakePath
            (
                strTmp, tCQCRemBrws::EDTypes::Template, facCQCAdmin.cuctxToUse().sectUser()
            );

            m_pdsclSrv->MakeNewScope
            (
                strTmp, L"Popup", tCQCRemBrws::EDTypes::Template, facCQCAdmin.cuctxToUse().sectUser()
            );

            m_pdsclSrv->MakeNewScope
            (
                strTmp, L"Layouts", tCQCRemBrws::EDTypes::Template, facCQCAdmin.cuctxToUse().sectUser()
            );

            m_pdsclSrv->MakeNewScope
            (
                strTmp, L"XOvers", tCQCRemBrws::EDTypes::Template, facCQCAdmin.cuctxToUse().sectUser()
            );

            //
            //  For each round we have to copy the source templates, and we will
            //  scale them for this new resolution.
            //
            colModTmpls.RemoveAll();

            //
            //  Do the standard templates. We do all of them here, since we want
            //  get pre-fab and user defined Xovers scaled.
            //
            const tCIDLib::TCard4 c4Count = m_colSrcTmpls.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TCQCIntfTemplate& iwdgCur = colModTmpls.objAdd(m_colSrcTmpls[c4Index]);

                // Do any special processing
                if (c4Index == m_c4TmplInd_LightsList)
                    ProcessLightList(iwdgCur, scriCur);
                else if (c4Index == m_c4TmplInd_Main)
                    ProcessMain(iwdgCur, scriCur);
                else if (c4Index == m_c4TmplInd_MainLeft)
                    ProcessMainLeft(iwdgCur, scriCur);
                else if (c4Index == m_c4TmplInd_MovieNP)
                    ProcessNowPlaying(iwdgCur, scriCur, kCIDLib::False);
                else if (c4Index == m_c4TmplInd_MusicNP)
                    ProcessNowPlaying(iwdgCur, scriCur, kCIDLib::True);
                else if (c4Index == m_c4TmplInd_Tiles)
                    ProcessTiles(iwdgCur, scriCur, strResFmt);
                else if (c4Index == m_c4TmplInd_HVACList)
                    ProcessHVACList(iwdgCur, scriCur);
                else if (c4Index == m_c4TmplInd_WeatherFC)
                    ProcessWeatherFC(iwdgCur, scriCur);

                facCQCIntfEng().ScaleTemplate(gdevTmp, iwdgCur, f8XScale, f8YScale);
            }

            // And now let's process these for the current room
            GenTemplatesOneRes(thrThis, scriCur, colModTmpls, strResFmt);

            QUpMsg(L"I -         Completed: %(1)", szCurRes);
        }

        QUpMsg(L"I -     Completed room: %(1)", scriCur.strName());
    }
}


//
//  For each room and resolution combo, this is called. We get copies of the
//  original source templates which have been scaled for the current resolution.
//
//  We need to go through the templates and update them for the passed room's
//  settings, then upload it to the target.
//
//  This will include all of the user defined Xover content templates, though
//  we will skip some processing on those.
//
tCIDLib::TVoid
TAutoGenDlg::GenTemplatesOneRes(        TThread&            thrThis
                                , const TCQCSysCfgRmInfo&   scriCur
                                ,       TTmplList&          colTmpls
                                , const TString&            strCurRes)
{
    static const tCQCIntfEng::ERepFlags eFlags = tCIDLib::eOREnumBits
    (
        tCQCIntfEng::ERepFlags::Image
        , tCQCIntfEng::ERepFlags::ActionParms
        , tCQCIntfEng::ERepFlags::Children
    );

    const tCIDLib::TCard4 c4TmplCnt = colTmpls.c4ElemCount();

    //
    //  Search and replace all image references. We have to build up a regular
    //  expression for each image to do the search and replace, so we go through
    //  all of the images, and for each one we search and replace on each template,
    //  so that we only have to do image setup once. All templates in this resolution
    //  will share the same scaled images.
    //
    //  NOTE: We only have to do up to the count of single images. The rest are
    //  from scopes so we don't have to do them directly (unless they happen to
    //  be also referenced as single images, in which case they will be included
    //  in this pass as well.)
    //
    TString strRegEx;
    TString strTok;
    TString strTarPath;
    TStringTokenizer stokSrc;
    tCIDLib::TKVPFList colMeta;
    for (tCIDLib::TCard4 c4IInd = 0; c4IInd < m_c4SingleImgCnt; c4IInd++)
    {
        const TKeyValuePair& kvalCur = m_colImgMap[c4IInd];

        //
        //  Build up a regular expression that will match this source
        //  path in any single/double slash combination.
        //
        strRegEx.Clear();
        stokSrc.Reset(&kvalCur.strKey(), L"/");
        while (stokSrc.bGetNextToken(strTok))
        {
            strRegEx.Append(L"(/|//)");
            strRegEx.Append(strTok);
        };
        m_regxSaR.SetExpression(strRegEx);

        // Set up the target path, we have to replace the resolution token
        strTarPath = kvalCur.strValue();
        strTarPath.eReplaceToken(strCurRes, kCIDLib::chLatin_r);

        //
        //  And update all of the templates. Since we are using a reg ex the source
        //  value doesn't matter, but we have to pass in something, so we pass in
        //  the source path.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
        {
            colTmpls[c4Index].Replace
            (
                eFlags
                , kvalCur.strKey()
                , strTarPath
                , kCIDLib::True
                , kCIDLib::True
                , m_regxSaR
            );
        }
    }

    //
    //  Now do the same for scopes. Since we replaced the individual images
    //  above, we don't have to worry that we might replace part of an images
    //  path while we do the scopes.
    //
    const tCIDLib::TCard4 c4ScopeCount = m_colScopeMap.c4ElemCount();
    for (tCIDLib::TCard4 c4SInd = 0; c4SInd < c4ScopeCount; c4SInd++)
    {
        const TKeyValuePair& kvalCur = m_colScopeMap[c4SInd];

        strRegEx.Clear();
        stokSrc.Reset(&kvalCur.strKey(), L"/");
        while (stokSrc.bGetNextToken(strTok))
        {
            strRegEx.Append(L"(/|//)");
            strRegEx.Append(strTok);
        };

        //
        //  Allow it to optionally end with slashes. We removed the slash from
        //  the original if it was present.
        //
        strRegEx.Append(L"[/]*");
        m_regxSaR.SetExpression(strRegEx);

        // Set up the target path, we have to replace the resolution token
        strTarPath = kvalCur.strValue();
        strTarPath.eReplaceToken(strCurRes, kCIDLib::chLatin_r);

        //
        //  And update all of the templates. Since we are using a reg ex the Src
        //  value doesn't matter, but we have to pass in something, so we pass in
        //  the source path.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
        {
            colTmpls[c4Index].Replace
            (
                eFlags
                , kvalCur.strKey()
                , strTarPath
                , kCIDLib::True
                , kCIDLib::True
                , m_regxSaR
            );
        }
    }

    //
    //  Now let's loop through all of the templates and update them in terms of
    //  fields, monikers, zones, lights, and so forth, then upload them.
    //
    //  NOTE: We only need to call the update method for the standard templates,
    //  but we do some work on all of them.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        // Watch for a shutdown request and throttle a bit
        if (!thrThis.bSleep(50))
            break;

        TCQCIntfTemplate& iwdgCur = colTmpls[c4Index];

        // Set up the target path for this one
        strTarPath = m_colTPaths[c4Index].strValue();
        strTarPath.eReplaceToken(strCurRes, L'r');
        strTarPath.eReplaceToken(scriCur.strName(), L'n');

        // Set it on the template
        iwdgCur.strTemplateName(strTarPath);

        // Update pre-fab ones with the settings for the current room
        if (c4Index < m_c4PreTmplCnt)
            UpdateTmplForRoom(thrThis, c4Index, iwdgCur, scriCur, strCurRes);
    }


    //
    //  OK, it's all done. So let let's upload all of the templates now. We are
    //  doing them all this time.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        const TCQCIntfTemplate& iwdgCur = colTmpls[c4Index];
        try
        {
            // Use the image stream as a temp stream to write out the template
            m_strmImg.Reset();
            m_strmImg << iwdgCur << kCIDLib::FlushIt;

            //
            //  The path has been set on them, so we don't have to rebuild it. Tell the
            //  data server not to memory cache it if it is not already.
            //
            colMeta.RemoveAll();
            tCIDLib::TCard4 c4SerialNum;
            tCIDLib::TEncodedTime enctLastChange;
            m_pdsclSrv->WriteTemplate
            (
                iwdgCur.strTemplateName()
                , c4SerialNum
                , enctLastChange
                , m_strmImg.mbufData()
                , m_strmImg.c4CurSize()
                , iwdgCur.eMinRole()
                , iwdgCur.strNotes()
                , kCQCRemBrws::c4Flag_OverwriteOK | kCQCRemBrws::c4Flag_NoDataCache
                , colMeta
                , facCQCAdmin.cuctxToUse().sectUser()
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            QUpMsg
            (
                L"I - Failed to upload template '%(1)' for room '%(2)'"
                , iwdgCur.strTemplateName()
                , scriCur.strName()
            );
        }

        catch(...)
        {
            QUpMsg
            (
                L"I - System error uploading template '%(1)' for room '%(2)'"
                , iwdgCur.strTemplateName()
                , scriCur.strName()
            );
        }
    }
}


//
//  This one is called to process the HVAC list overlay. We have to replicate the
//  one thermo in the list, updating it for each configured thermo. If no thermos are
//  configured then this won't be called.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessHVACList(TCQCIntfTemplate& iwdgEdit, const TCQCSysCfgRmInfo& scriCur)
{
    const tCIDLib::TCard4 c4HVACCnt = scriCur.c4HVACCount();

    // Nothing to do if no HVACs are configured
    if (!c4HVACCnt)
        return;

    //
    //  We need to query the widgets in the Thermo1Grp group. This will be our working
    //  content. We get the area of the whole group and that is our baseline area,
    //  which will we move right for each new thermo.
    //
    tCIDLib::TCard4         c4FirstZ;
    tCQCIntfEng::TChildList colMembers(tCIDLib::EAdoptOpts::NoAdopt);
    iwdgEdit.bFindGroupMembers(L"Thermo1Grp", colMembers, c4FirstZ);


    //
    //  Go through them all and build up a containing area. Also, as we go, ungroup
    //  them so that when we copy them, we aren't also copying the grouping. This
    //  way we don't have to be sure to change the group name.
    //
    const tCIDLib::TCard4 c4WidgetCnt = colMembers.c4ElemCount();
    CIDAssert(c4WidgetCnt != 0, L"No widgets were in the thermo group");

    TArea areaBase = colMembers[0]->areaActual();
    colMembers[0]->Ungroup();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4WidgetCnt; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = colMembers[c4Index];
        areaBase |= piwdgCur->areaActual();
        piwdgCur->Ungroup();
    }

    // Size the template to hold our count of loads of this size
    TSize szTmpl(c4HVACCnt * areaBase.c4Width(), areaBase.c4Height());
    iwdgEdit.SetSize(szTmpl, kCIDLib::False);


    //
    //  Now for each thermo, we create copies of these widgets, adjust their positions,
    //  and adjust their field names and other things.
    //
    TArea areaNew;
    TArea areaCurHVAC = areaBase;
    TString strFldBase;
    TString strFldNameBase;
    TString strVal;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4HVACCnt; c4Index++)
    {
        tCIDLib::TCard4 c4HVACInd;
        const TSCHVACInfo& scliCur = m_scfgGen.scliHVACByUID
        (
            scriCur.strHVACUIDAt(c4Index), c4HVACInd
        );


        // Build up the base parts of this thermo's fields
        strFldBase = scliCur.m_strMoniker;
        strFldBase.Append(kCIDLib::chPeriod);

        strFldNameBase = L"THERM#";
        if (!scliCur.m_strThermoSub.bIsEmpty())
        {
            strFldNameBase.Append(scliCur.m_strThermoSub);
            strFldNameBase.Append(kCIDLib::chTilde);
        }

        strFldBase.Append(strFldNameBase);

        for (tCIDLib::TCard4 c4WInd = 0; c4WInd < c4WidgetCnt; c4WInd++)
        {
            TCQCIntfWidget* piwdgCur = static_cast<TCQCIntfWidget*>
            (
                colMembers[c4WInd]->pobjDuplicate()
            );

            // Adjust the position to be relative to the current load area
            areaNew = piwdgCur->areaActual();
            areaNew -= areaBase.pntOrg();
            areaNew += areaCurHVAC.pntOrg();
            piwdgCur->SetArea(areaNew, kCIDLib::False);

            // Based on which one, we will update various things
            if (piwdgCur->strWidgetId() == L"ThermoCmd")
            {
                // Get the list of actions for the OnClick
                TCQCIntfCmdButton* piwdgCmd = static_cast<TCQCIntfCmdButton*>(piwdgCur);
                MCQCCmdSrcIntf::TOpcodeBlock* pcolOnCl
                (
                    piwdgCmd->pcolOpsForEvent(kCQCKit::strEvId_OnClick)
                );

                const tCIDLib::TCard4 c4CmdCnt = pcolOnCl->c4ElemCount();
                for (tCIDLib::TCard4 c4OpInd = 0; c4OpInd < c4CmdCnt; c4OpInd++)
                {
                    TActOpcode& aocCur = pcolOnCl->objAt(c4OpInd);

                    // We only care about variable setting global var commands
                    if ((aocCur.m_eOpcode != tCQCKit::EActOps::Cmd)
                    ||  (aocCur.m_ccfgStep.strTargetId() != kCQCKit::strCTarId_GVars)
                    ||  (aocCur.m_ccfgStep.strCmdId() != kCQCKit::strCmdId_SetVariable))
                    {
                        continue;
                    }

                    strVal.Clear();
                    if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurThermoBase")
                    {
                        strVal = strFldBase;
                    }
                     else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurThermoName")
                    {
                        strVal = scliCur.m_strTitle;
                    }

                    aocCur.m_ccfgStep.SetParmAt(1, strVal);
                }
            }
             else if ((piwdgCur->strWidgetId() == L"CurTemp")
                  ||  (piwdgCur->strWidgetId() == L"FanMode")
                  ||  (piwdgCur->strWidgetId() == L"ThermoMode"))
            {
                strVal = strFldNameBase;
                if (piwdgCur->strWidgetId() == L"CurTemp")
                    strVal.Append(L"CurrentTemp");
                else if (piwdgCur->strWidgetId() == L"FanMode")
                    strVal.Append(L"FanOpMode");
                else if (piwdgCur->strWidgetId() == L"ThermoMode")
                    strVal.Append(L"OpMode");

                TCQCIntfFldText* piwdgText = static_cast<TCQCIntfFldText*>(piwdgCur);
                piwdgText->AssociateField(scliCur.m_strMoniker, strVal);
            }
             else if (piwdgCur->strWidgetId() == L"ThermoName")
            {
                // Set tht title text as the caption on this widget
                piwdgCur->strCaption(scliCur.m_strTitle);
            }

            //
            //  Remove the widget id now, since it's not needed and we avoid
            //  duplicate ids. And then add it to the template.
            //
            piwdgCur->strWidgetId(TString::strEmpty());

            // Add it to the
            iwdgEdit.AddChild(piwdgCur);
        }

        // Move the area to the right
        areaCurHVAC.Move(tCIDLib::EDirs::Right);
    }

    //
    //  Now go back and remove the originals from the template. We ungrouped them
    //  before we did it, so no special concerns.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WidgetCnt; c4Index++)
        iwdgEdit.RemoveWidget(colMembers[c4Index]);
}


//
//  This one is called to process the light list overlay. We have to replicate the
//  one light in the list, updating it for each configured light. If no lights are
//  configured then this won't be called.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessLightList(          TCQCIntfTemplate&   iwdgEdit
                                , const TCQCSysCfgRmInfo&   scriCur)
{
    const tCIDLib::TCard4 c4LoadCnt = scriCur.c4LoadCount();

    // Nothing to do if no lights are configured
    if (!c4LoadCnt)
        return;

    //
    //  We need to query the widgets in the Light1Grp group. This will be our working
    //  content. We get the area of the whole group and that is our baseline area,
    //  which will we move right for each new light.
    //
    tCIDLib::TCard4         c4FirstZ;
    tCQCIntfEng::TChildList colMembers(tCIDLib::EAdoptOpts::NoAdopt);
    iwdgEdit.bFindGroupMembers(L"Light1Grp", colMembers, c4FirstZ);

    //
    //  Go through them all and build up a containing area. Also, as we go, ungroup
    //  them so that when we copy them, we aren't also copying the grouping. This
    //  way we don't have to be sure to change the group name.
    //
    const tCIDLib::TCard4 c4WidgetCnt = colMembers.c4ElemCount();
    CIDAssert(c4WidgetCnt != 0, L"No widgets were in the light group");

    TArea areaBase = colMembers[0]->areaActual();
    colMembers[0]->Ungroup();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4WidgetCnt; c4Index++)
    {
        TCQCIntfWidget* piwdgCur = colMembers[c4Index];
        areaBase |= piwdgCur->areaActual();
        piwdgCur->Ungroup();
    }

    // Size the template to hold our count of loads of this size
    TSize szTmpl(c4LoadCnt * areaBase.c4Width(), areaBase.c4Height());
    iwdgEdit.SetSize(szTmpl, kCIDLib::False);

    //
    //  Now for each light, we create copies of these widgets, adjust their positions,
    //  and adjust their field names and other things.
    //
    TArea areaNew;
    TArea areaCurLoad = areaBase;
    TString strVal;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4LoadCnt; c4Index++)
    {
        tCIDLib::TCard4 c4LoadInd;
        const TSCLoadInfo& scliCur = m_scfgGen.scliLoadByUID
        (
            scriCur.strLoadUIDAt(c4Index), c4LoadInd
        );

        for (tCIDLib::TCard4 c4WInd = 0; c4WInd < c4WidgetCnt; c4WInd++)
        {
            TCQCIntfWidget* piwdgCur = static_cast<TCQCIntfWidget*>
            (
                colMembers[c4WInd]->pobjDuplicate()
            );

            // Adjust the position to be relative to the current load area
            areaNew = piwdgCur->areaActual();
            areaNew -= areaBase.pntOrg();
            areaNew += areaCurLoad.pntOrg();
            piwdgCur->SetArea(areaNew, kCIDLib::False);

            // Based on which one, we will update various things
            if (piwdgCur->strWidgetId() == L"LightCmd")
            {
                // Get the list of actions for the OnClick
                TCQCIntfCmdButton* piwdgCmd = static_cast<TCQCIntfCmdButton*>(piwdgCur);
                MCQCCmdSrcIntf::TOpcodeBlock* pcolOnCl
                (
                    piwdgCmd->pcolOpsForEvent(kCQCKit::strEvId_OnClick)
                );

                const tCIDLib::TCard4 c4CmdCnt = pcolOnCl->c4ElemCount();
                for (tCIDLib::TCard4 c4OpInd = 0; c4OpInd < c4CmdCnt; c4OpInd++)
                {
                    TActOpcode& aocCur = pcolOnCl->objAt(c4OpInd);

                    // We only care about variable setting global var commands
                    if ((aocCur.m_eOpcode != tCQCKit::EActOps::Cmd)
                    ||  (aocCur.m_ccfgStep.strTargetId() != kCQCKit::strCTarId_GVars)
                    ||  (aocCur.m_ccfgStep.strCmdId() != kCQCKit::strCmdId_SetVariable))
                    {
                        continue;
                    }

                    strVal.Clear();
                    if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightSwFld")
                    {
                        if (scliCur.bIsDimmer())
                        {
                            // We may or may not have a switch
                            if (!scliCur.m_strField2.bIsEmpty())
                                strVal = scliCur.m_strField2;
                        }
                         else
                        {
                            strVal = scliCur.m_strField;
                        }

                        if (!strVal.bIsEmpty())
                        {
                            strVal.Prepend(L".");
                            strVal.Prepend(scliCur.m_strMoniker);
                        }
                    }
                     else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightDimFld")
                    {
                        if (scliCur.bIsDimmer())
                        {
                            strVal = scliCur.m_strMoniker;
                            strVal.Append(kCIDLib::chPeriod);
                            strVal.Append(scliCur.m_strField);
                        }
                    }
                     else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightClrFld")
                    {
                        if (!scliCur.m_strClrField.bIsEmpty())
                        {
                            strVal = scliCur.m_strMoniker;
                            strVal.Append(kCIDLib::chPeriod);
                            strVal.Append(scliCur.m_strClrField);
                        }
                    }
                     else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightName")
                    {
                        strVal = scliCur.m_strTitle;
                    }

                    aocCur.m_ccfgStep.SetParmAt(1, strVal);
                }
            }
             else if (piwdgCur->strWidgetId() == L"LightBulb")
            {
                //
                //  Set up the field association. We have special considerations
                //  here. If it's a dimmer and has a switch, or it's a switch, then
                //  we just want to use the switch field.
                //
                //  If it's purely a dimmer, then we need to set up the light bulk
                //  to work on the dimmer field, which should be ok since the expression
                //  is just 'is true', and 0 or non-zero will work fine with that.
                //
                if (scliCur.bIsDimmer())
                {
                    if (!scliCur.m_strField2.bIsEmpty())
                        strVal = scliCur.m_strField2;
                    else
                        strVal = scliCur.m_strField;
                }
                 else
                {
                    strVal = scliCur.m_strField;
                }

                TCQCIntfFldBooleanImg* piwdgBulb = static_cast<TCQCIntfFldBooleanImg*>(piwdgCur);
                piwdgBulb->AssociateField(scliCur.m_strMoniker, strVal);
            }
             else if (piwdgCur->strWidgetId() == L"LightName")
            {
                // Set tht title text as the caption on this widget
                piwdgCur->strCaption(scliCur.m_strTitle);
            }

            //
            //  Remove the widget id now, since it's not needed and we avoid
            //  duplicate ids. And then add it to the template.
            //
            piwdgCur->strWidgetId(TString::strEmpty());

            // Add it to the
            iwdgEdit.AddChild(piwdgCur);
        }

        // Move the area to the right
        areaCurLoad.Move(tCIDLib::EDirs::Right);
    }

    //
    //  Now go back and remove the originals from the template. We ungrouped them, so
    //  so no special issues with removing them.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WidgetCnt; c4Index++)
        iwdgEdit.RemoveWidget(colMembers[c4Index]);
}



//
//  This is called to update some stuff in the OnPreload and possibly other events
//  that fire when this room's main template loads. We have to set up some information
//  in various commands that are driven by the room configuration.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessMain(           TCQCIntfTemplate&   iwdgEdit
                            , const TCQCSysCfgRmInfo&   scriCur)
{
    // Get the list of actions for the OnPreload
    MCQCCmdSrcIntf::TOpcodeBlock* pcolOnEv
    (
        iwdgEdit.pcolOpsForEvent(kCQCKit::strEvId_OnPreload)
    );

    //
    //  To make this more efficient, see if we have lighting or HVAC configured and a
    //  default set. If so, get a pointer to them.
    //
    const TSCLoadInfo* pscliLoad = 0;
    if (scriCur.c4LoadCount() && scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Lighting))
    {
        tCIDLib::TCard4 c4LoadInd;
        pscliLoad = &m_scfgGen.scliLoadByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Lighting), c4LoadInd
        );
    }

    const TSCHVACInfo* pscliHVAC = 0;
    if (scriCur.c4HVACCount() && scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::HVAC))
    {
        tCIDLib::TCard4 c4HVACInd;
        pscliHVAC = &m_scfgGen.scliHVACByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::HVAC), c4HVACInd
        );
    }

    //
    //  Look through them and look for variable creating and setting commands. We
    //  check which variable and based on that we set up the values.
    //
    TString strFld;
    const tCIDLib::TCard4 c4Count = pcolOnEv->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TActOpcode& aocCur = pcolOnEv->objAt(c4Index);

        // We only care about commands that target the global variables target
        if ((aocCur.m_eOpcode != tCQCKit::EActOps::Cmd)
        ||  (aocCur.m_ccfgStep.strTargetId() != kCQCKit::strCTarId_GVars))
        {
            continue;
        }

        if (aocCur.m_ccfgStep.strCmdId() == kCQCKit::strCmdId_SafeCreateVariable)
        {
            if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CfgRmListIds")
            {
                // We need to set up the enum and initial value
                aocCur.m_ccfgStep.SetParmAt(2, m_strRmListIds);
                aocCur.m_ccfgStep.SetParmAt(3, scriCur.strUniqueId());
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CfgRmListNames")
            {
                // We need to set up the enum and initial value
                aocCur.m_ccfgStep.SetParmAt(2, m_strRmListNames);
                aocCur.m_ccfgStep.SetParmAt(3, scriCur.strName());
            }
        }
         else if (aocCur.m_ccfgStep.strCmdId() == kCQCKit::strCmdId_SetVariable)
        {
            if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurRoomName")
            {
                aocCur.m_ccfgStep.SetParmAt(1, scriCur.strName());
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CfgRoomSecArea")
            {
                // If we are configured for security, set the value, else disable
                if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
                {
                    tCIDLib::TCard4 c4SecInd;
                    const TSCSecInfo& scliSec = m_scfgGen.scliSecByUID
                    (
                        scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Security), c4SecInd
                    );
                    aocCur.m_ccfgStep.SetParmAt(1, scliSec.m_strAreaName);
                }
                 else
                {
                    aocCur.m_bDisabled = kCIDLib::True;
                }
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightName")
            {
                if (pscliLoad)
                    aocCur.m_ccfgStep.SetParmAt(1, pscliLoad->m_strTitle);
                else
                    aocCur.m_ccfgStep.SetParmAt(1, TString::strEmpty());
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightSwFld")
            {
                if (pscliLoad)
                {
                    // If a dimmer, then the switch is field 2, else field 1
                    if (pscliLoad->bIsDimmer())
                        strFld = pscliLoad->m_strField2;
                    else
                        strFld = pscliLoad->m_strField;

                    if (!strFld.bIsEmpty())
                    {
                        strFld.Prepend(L".");
                        strFld.Prepend(pscliLoad->m_strMoniker);
                    }
                    aocCur.m_ccfgStep.SetParmAt(1, strFld);
                }
                 else
                {
                    aocCur.m_ccfgStep.SetParmAt(1, TString::strEmpty());
                }
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightDimFld")
            {
                if (pscliLoad && pscliLoad->bIsDimmer())
                {
                    strFld = pscliLoad->m_strMoniker;
                    strFld.Append(kCIDLib::chPeriod);
                    strFld.Append(pscliLoad->m_strField);
                    aocCur.m_ccfgStep.SetParmAt(1, strFld);
                }
                 else
                {
                    aocCur.m_ccfgStep.SetParmAt(1, TString::strEmpty());
                }
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurLightClrFld")
            {
                if (pscliLoad && !pscliLoad->m_strClrField.bIsEmpty())
                {
                    strFld = pscliLoad->m_strMoniker;
                    strFld.Append(kCIDLib::chPeriod);
                    strFld.Append(pscliLoad->m_strClrField);
                    aocCur.m_ccfgStep.SetParmAt(1, strFld);
                }
                 else
                {
                    aocCur.m_ccfgStep.SetParmAt(1, TString::strEmpty());
                }
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurThermoBase")
            {
                strFld.Clear();
                if (pscliHVAC)
                {
                    strFld = pscliHVAC->m_strMoniker;
                    strFld.Append(kCIDLib::chPeriod);
                    strFld.Append(L"THERM#");
                    if (!pscliHVAC->m_strThermoSub.bIsEmpty())
                    {
                        strFld.Append(pscliHVAC->m_strThermoSub);
                        strFld.Append(kCIDLib::chTilde);
                    }
                }
                aocCur.m_ccfgStep.SetParmAt(1, strFld);
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CurThermoName")
            {
                if (pscliHVAC)
                    aocCur.m_ccfgStep.SetParmAt(1, pscliHVAC->m_strTitle);
                else
                    aocCur.m_ccfgStep.SetParmAt(1, TString::strEmpty());
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CQCAG_Var1")
            {
                aocCur.m_ccfgStep.SetParmAt(1, scriCur.strRoomVarAt(0));
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CQCAG_Var2")
            {
                aocCur.m_ccfgStep.SetParmAt(1, scriCur.strRoomVarAt(1));
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CQCAG_Var3")
            {
                aocCur.m_ccfgStep.SetParmAt(1, scriCur.strRoomVarAt(2));
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:CQCAG_Var4")
            {
                aocCur.m_ccfgStep.SetParmAt(1, scriCur.strRoomVarAt(3));
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:AllowRoomSel")
            {
                // Default is Yes, so only have to set it if not allowing
                if (!scriCur.bAllowRmSel())
                    aocCur.m_ccfgStep.SetParmAt(1, L"No");
            }
             else if (aocCur.m_ccfgStep.piAt(0).m_strValue == L"GVar:DoThermos")
            {
                //
                //  Default is Yes, so only have to set it if not allowing. REmember if
                //  we are doing thermos here.
                //
                if (!pscliHVAC)
                    aocCur.m_ccfgStep.SetParmAt(1, L"No");
            }
        }
    }

    //
    //  If this room has an enabled OnRoomLoad global action, then we need to
    //  find the command in the OnLoad event, enable the action command, and
    //  update the path and parameters.
    //
    if (scriCur.bGlobalActEnabled(tCQCSysCfg::EGlobActs::OnRoomLoad))
    {
        UpdateGActCommand
        (
            iwdgEdit, tCQCSysCfg::EGlobActs::OnRoomLoad, scriCur, kCQCKit::strEvId_OnLoad
        );
    }
}


//
//  This is called to update some stuff in the left side template of the main
//  overlay.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessMainLeft(       TCQCIntfTemplate&   iwdgEdit
                            , const TCQCSysCfgRmInfo&   scriCur)
{
    //
    //  Update the action of the exit button to enable the OnIVExit command if the
    //  user configured it.
    //
    if (scriCur.bGlobalActEnabled(tCQCSysCfg::EGlobActs::OnIVExit))
    {
        TCQCIntfCmdButton* piwdgExit = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"ExitButton");
        UpdateGActCommand
        (
            *piwdgExit, tCQCSysCfg::EGlobActs::OnIVExit, scriCur, kCQCKit::strEvId_OnClick
        );
    }
}


//
//  This is called to update some stuff in the music/movie playing overlays.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessNowPlaying(         TCQCIntfTemplate&   iwdgEdit
                                , const TCQCSysCfgRmInfo&   scriCur
                                , const tCIDLib::TBoolean   bMusic)
{
    // If the function we are doing isn't enabled, then nothing to do
    tCQCSysCfg::ERmFuncs eFunc = bMusic ? tCQCSysCfg::ERmFuncs::Music
                                        : tCQCSysCfg::ERmFuncs::Movies;
    if (!scriCur.bFuncEnabled(eFunc))
        return;

    // Do the power on command
    tCQCSysCfg::EGlobActs eAct;
    TCQCIntfCmdButton* piwdgCmd;
    {
        if (bMusic)
            eAct = tCQCSysCfg::EGlobActs::MusicPowerOn;
        else
            eAct = tCQCSysCfg::EGlobActs::MoviePowerOn;

        if (scriCur.bGlobalActEnabled(eAct))
        {
            piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"PowerOn");
            UpdateGActCommand(*piwdgCmd, eAct, scriCur, kCQCKit::strEvId_OnClick);
        }
         else
        {
            // Remove the power on button and divider
            iwdgEdit.RemoveByName(L"PowerOn");
            iwdgEdit.RemoveByName(L"PowerOnDiv");
        }
    }

    // And the power off command
    {
        if (bMusic)
            eAct = tCQCSysCfg::EGlobActs::MusicPowerOff;
        else
            eAct = tCQCSysCfg::EGlobActs::MoviePowerOff;

        if (scriCur.bGlobalActEnabled(eAct))
        {
            piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"PowerOff");
            UpdateGActCommand(*piwdgCmd, eAct, scriCur, kCQCKit::strEvId_OnClick);
        }
         else
        {
            // Remove the power on button and divider
            iwdgEdit.RemoveByName(L"PowerOff");
            iwdgEdit.RemoveByName(L"PowerOffDiv");
        }
    }

    // Update the volume control stuff
    tCIDLib::TCard4 c4Index;
    const TSCMediaInfo& scliMedia = m_scfgGen.scliMediaByUID
    (
        scriCur.strFuncId(eFunc), c4Index
    );
    ReplaceAudioSrc(iwdgEdit, scliMedia, eFunc);
}


//
//  This is called for the tiles overlay. We have to do a good bit of work here
//  to get rid of any of the tiles that are not enabled, and to rerrange all
//  of the remaining tiles again.
//
//  We also have to add in any custom tiles as well.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessTiles(          TCQCIntfTemplate&   iwdgEdit
                            , const TCQCSysCfgRmInfo&   scriCur
                            , const TString&            strResFmt)
{
    //
    //  First thing we want to do is to figure out the spacing. So we get the first
    //  tile in the upper left, and the one to the right and the one below. This
    //  lets us know how to arrange the tiles we keep, column and row.
    //
    //  The top/left tile is the time/date one, which never has to be removed.
    //
    //  We use the weather and music tiles to calculate X/Y inter-tile distances from
    //  the first one. We may remove these below, so we need to get area info now before
    //  that happens. The weather one also has title and link button widgets that we
    //  need to use below for generating extra overlays, so we make copies of those
    //  before we possibly remove it.
    //
    TCQCIntfWidget* piwdgTime = iwdgEdit.piwdgFindByName(L"TimePanel", kCIDLib::False);

    TArea areaWeather;
    TCQCIntfWidget* piwdgTileLink = nullptr;
    TCQCIntfWidget* piwdgTileArrow = nullptr;
    const TArea areaBase = piwdgTime->areaActual();
    tCIDLib::TInt4 i4XOfs;
    tCIDLib::TInt4 i4YOfs;

    {
        TCQCIntfWidget* piwdgWeath = iwdgEdit.piwdgFindByName(L"WeatherPanel", kCIDLib::False);
        TCQCIntfWidget* piwdgMusic = iwdgEdit.piwdgFindByName(L"MusicPanel", kCIDLib::False);
        i4XOfs = piwdgWeath->areaActual().i4X() - areaBase.i4X();
        i4YOfs = piwdgMusic->areaActual().i4Y() - areaBase.i4Y();
        piwdgTileLink = static_cast<TCQCIntfWidget*>
        (
            iwdgEdit.piwdgFindByName(L"LinkButton", kCIDLib::True)->pobjDuplicate()
        );
        piwdgTileArrow = static_cast<TCQCIntfWidget*>
        (
            iwdgEdit.piwdgFindByName(L"LinkArrow", kCIDLib::True)->pobjDuplicate()
        );
        areaWeather = piwdgWeath->areaActual();
    }
    TJanitor<TCQCIntfWidget> janTileLink(piwdgTileLink);
    TJanitor<TCQCIntfWidget> janTileArrow(piwdgTileArrow);


    //
    //  To make this more reasonable, we create a 3x2 array that contains the group
    //  names of the tiles and then we go through and remove those that are not
    //  configured. As we remove them, we empty those slots. At the end, we have info
    //  on both those removed and those that remain, and we can move them down to
    //  keep them contiguous.
    //
    //  Later, if we have more standard tiles, we'll expand this out.
    //
    const tCIDLib::TCard4 c4SlotCnt = 7;
    TFundArray<tCQCSysCfg::ERmFuncs> fcolFuncs(c4SlotCnt);
    TObjArray<TString> objaSlots(c4SlotCnt);

    objaSlots[0] = L"TimeDate";
    objaSlots[1] = L"Weather";
    objaSlots[2] = L"Security";
    objaSlots[3] = L"Lights";
    objaSlots[4] = L"HVAC";
    objaSlots[5] = L"Music";
    objaSlots[6] = L"Movies";

    fcolFuncs[0] = tCQCSysCfg::ERmFuncs::Count;
    fcolFuncs[1] = tCQCSysCfg::ERmFuncs::Weather;
    fcolFuncs[2] = tCQCSysCfg::ERmFuncs::Security;
    fcolFuncs[3] = tCQCSysCfg::ERmFuncs::Lighting;
    fcolFuncs[4] = tCQCSysCfg::ERmFuncs::HVAC;
    fcolFuncs[5] = tCQCSysCfg::ERmFuncs::Music;
    fcolFuncs[6] = tCQCSysCfg::ERmFuncs::Movies;

    {
        tCIDLib::TCard4 c4SlotInd = 1;
        while (c4SlotInd < c4SlotCnt)
        {
            //
            //  Lightings and HVAC is enabled by the presence of configured entries.THe
            //  others are just enabled or not.
            //
            if (((fcolFuncs[c4SlotInd] == tCQCSysCfg::ERmFuncs::Lighting) && !scriCur.c4LoadCount())
            ||  ((fcolFuncs[c4SlotInd] == tCQCSysCfg::ERmFuncs::HVAC) && !scriCur.c4HVACCount())
            ||  !scriCur.bFuncEnabled(fcolFuncs[c4SlotInd]))
            {
                RemoveGroup(iwdgEdit, objaSlots[c4SlotInd]);
                objaSlots[c4SlotInd].Clear();
                fcolFuncs[c4SlotInd] = tCQCSysCfg::ERmFuncs::Count;
            }
            c4SlotInd++;
        }
    }

    //
    //  Move up to the first removed one. This will be our target index as we move
    //  forward.
    //
    tCIDLib::TCard4 c4TarInd = 1;
    while (c4TarInd < c4SlotCnt)
    {
        if (objaSlots[c4TarInd].bIsEmpty())
            break;
        c4TarInd++;
    }

    // If we found one, then we need to do some rearranging
    tCQCIntfEng::TChildList colMembers(tCIDLib::EAdoptOpts::NoAdopt);
    tCIDLib::TCard4     c4FirstZ;
    TPoint              pntNew;
    TPoint              pntSrc;
    TPoint              pntTar;
    TString             strRep;

    if (c4TarInd < c4SlotCnt)
    {
        tCIDLib::TCard4 c4SrcInd = c4TarInd;
        while ((c4SrcInd < c4SlotCnt) && (c4TarInd < c4SlotCnt))
        {
            //
            //  Find the next one after that that is available. This will be our
            //  source index.
            //
            while (c4SrcInd < c4SlotCnt)
            {
                if (!objaSlots[c4SrcInd].bIsEmpty())
                    break;
                c4SrcInd++;
            }

            // If we hit the end, then we are done
            if (c4SrcInd == c4SlotCnt)
                break;

            //
            //  Let's adjust the positions of the widgets of the source group to
            //  the correct position for the target. Calculate the origins for the
            //  source and target groups.
            //
            pntTar = areaBase.pntOrg();
            pntTar.Adjust
            (
                tCIDLib::TInt4(c4TarInd % 3) * i4XOfs
                , tCIDLib::TInt4(c4TarInd / 3) * i4YOfs
            );

            pntSrc = areaBase.pntOrg();
            pntSrc.Adjust
            (
                tCIDLib::TInt4(c4SrcInd % 3) * i4XOfs
                , tCIDLib::TInt4(c4SrcInd / 3) * i4YOfs
            );

            // Let's get all the members of the source group
            iwdgEdit.bFindGroupMembers(objaSlots[c4SrcInd], colMembers, c4FirstZ);

            //
            //  For each widget in the group, we get its area, subtract it's original
            //  group origin (to create a group relative position), then add the new
            //  target group origin. We set that back on the widget.
            //
            const tCIDLib::TCard4 c4GrpCnt = colMembers.c4ElemCount();
            for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
            {
                pntNew = colMembers[c4GrpInd]->areaActual().pntOrg();
                pntNew -= pntSrc;
                pntNew += pntTar;
                colMembers[c4GrpInd]->SetOrg(pntNew);
            }

            // Move down the group names and funcion enums values as well
            objaSlots[c4TarInd] = objaSlots[c4SrcInd];
            objaSlots[c4SrcInd].Clear();
            fcolFuncs[c4TarInd] = fcolFuncs[c4SrcInd];

            //  Move the target and source indices upwards
            c4TarInd++;
            c4SrcInd++;
        }
    }

    // If we kept the HVAC, then update it
    TCQCIntfCmdButton* piwdgCmd = 0;
    if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::HVAC))
    {
        tCIDLib::TCard4 c4HVACInd;
        const TSCHVACInfo& scliCur = m_scfgGen.scliHVACByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::HVAC), c4HVACInd
        );

        //
        //  For all of the widgets in this group, replace the base part of the field
        //  with the new base.
        //
        iwdgEdit.bFindGroupMembers(L"HVAC", colMembers, c4FirstZ);

        strRep = scliCur.m_strMoniker;
        strRep.Append(kCIDLib::chPeriod);
        strRep.Append(L"THERM#");
        if (!scliCur.m_strThermoSub.bIsEmpty())
        {
            strRep.Append(scliCur.m_strThermoSub);
            strRep.Append(kCIDLib::chTilde);
        }

        const tCIDLib::TCard4 c4GrpCnt = colMembers.c4ElemCount();
        for (tCIDLib::TCard4 c4GrpInd = 0; c4GrpInd < c4GrpCnt; c4GrpInd++)
        {
            colMembers[c4GrpInd]->Replace
            (
                tCIDLib::eOREnumBits
                (
                  tCQCIntfEng::ERepFlags::ActionParms
                  , tCQCIntfEng::ERepFlags::AssocField
                )
                , L"AG_Thermo.THERM#House~"
                , strRep
                , kCIDLib::False
                , kCIDLib::False
                , m_regxSaR
            );
        }

        //
        //  Also do the same replacement on the template itself to get the onpreload
        //  commands that set up the value adjustment variables.
        //
        iwdgEdit.Replace
        (
            tCQCIntfEng::ERepFlags::ActionParms
            , L"AG_Thermo.THERM#House~"
            , strRep
            , kCIDLib::False
            , kCIDLib::False
            , m_regxSaR
        );
    }
     else
    {
        //
        //  Delete on-preload commands related to the thermo setup. For now, that is
        //  all that's in the preload, so we just delete all of the preload commands.
        //
        iwdgEdit.ResetOps(kCQCKit::strEvId_OnPreload);
    }


    //
    //  I kept the lights, then update it. We also have to deal with the possibiltiy of
    //  a custom layout and their wanting to use it as the detail template for lighitng
    //  instead of our standard one.
    //
    piwdgCmd = 0;
    if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Lighting))
    {
        tCIDLib::TCard4 c4LoadInd;
        const TSCLoadInfo& scliCur = m_scfgGen.scliLoadByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Lighting), c4LoadInd
        );

        //
        //  Update the Off/On commands to use the appropriate field. If we have
        //  a switch, then use that. Else use the dimmer.
        //
        strRep = scliCur.m_strMoniker;
        strRep.Append(kCIDLib::chPeriod);
        if (scliCur.bIsDimmer())
        {
            if (scliCur.m_strField2.bIsEmpty())
                strRep.Append(scliCur.m_strField);
            else
                strRep.Append(scliCur.m_strField2);
        }
         else
        {
            strRep.Append(scliCur.m_strField);
        }

        piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"LightOff");
        piwdgCmd->Replace
        (
            tCQCIntfEng::ERepFlags::ActionParms
            , L"AG_Lighting.LGHT#Sw_KitchTbl"
            , strRep
            , kCIDLib::False
            , kCIDLib::False
            , m_regxSaR
        );

        piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"LightOn");
        piwdgCmd->Replace
        (
            tCQCIntfEng::ERepFlags::ActionParms
            , L"AG_Lighting.LGHT#Sw_KitchTbl"
            , strRep
            , kCIDLib::False
            , kCIDLib::False
            , m_regxSaR
        );

        //
        //  If we are a dimmer, then update the slider to use this field. Else,
        //  remove the slider since this room will not use it.
        //
        TCQCIntfFieldSlider* piwdgSlider
        (
            iwdgEdit.piwdgCastByName<TCQCIntfFieldSlider>(L"LightDimmer")
        );
        if (scliCur.bIsDimmer())
        {
            strRep = scliCur.m_strMoniker;
            strRep.Append(kCIDLib::chPeriod);
            strRep.Append(scliCur.m_strField);
            piwdgSlider->Replace
            (
                tCIDLib::eOREnumBits
                (
                  tCQCIntfEng::ERepFlags::ActionParms
                  , tCQCIntfEng::ERepFlags::AssocField
                )
                , L"AG_Lighting.LGHT#Dim_KitchTbl"
                , strRep
                , kCIDLib::False
                , kCIDLib::False
                , m_regxSaR
            );
        }
         else
        {
            DeleteWidget(iwdgEdit, piwdgSlider);
        }

        //
        //  Update the bulb. We use the switch if it's available, else we use
        //  the dimmer. The boolean expression is 'is true', so it'll work with
        //  either value as is.
        //
        if (!scliCur.m_strField2.bIsEmpty())
            strRep = scliCur.m_strField2;
        else
            strRep = scliCur.m_strField;
        TCQCIntfFldBooleanImg* piwdgBulb
        (
            iwdgEdit.piwdgCastByName<TCQCIntfFldBooleanImg>
            (
                L"StatusLight", kCIDLib::False
            )
        );
        piwdgBulb->AssociateField(scliCur.m_strMoniker, strRep);


        // If a dimmer only
        if (scliCur.bIsDimmer() && scliCur.m_strField2.bIsEmpty())
        {
            // And change the 1 value in the on command to 100 since it's a dimmer
            piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"LightOn");
            piwdgCmd->Replace
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , L"1"
                , L"100"
                , kCIDLib::False
                , kCIDLib::True
                , m_regxSaR
            );
        }

        if (!m_scfgGen.strCustLayout().bIsEmpty() && m_scfgGen.bUseCustLights())
        {
            // Find the button and update it's target to point to the custom layout
            strRep = strFindTarPath(m_scfgGen.strCustLayout());
            strRep.eReplaceToken(strResFmt, L'r');
            strRep.eReplaceToken(scriCur.strName(), L'n');

            // Replace the original link text with the new target
            TCQCIntfWidget* piwdgBut = iwdgEdit.piwdgFindByName(L"LightingButton", kCIDLib::False);
            piwdgBut->Replace
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , L"./Overlay_Lights"
                , strRep
                , kCIDLib::False
                , kCIDLib::False
                , m_regxSaR
            );

        }
    }

    //
    //  If we kept the movies or music tiles, update them. We need to enable the
    //  pre-music/movies actions if they are configured. And we have to do some
    //  special case search and replace to deal with the audio control moniker/field.
    //
    if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Movies))
    {
        if (scriCur.bGlobalActEnabled(tCQCSysCfg::EGlobActs::PreMovie))
        {
            piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"MovieButton");
            UpdateGActCommand
            (
                *piwdgCmd
                , tCQCSysCfg::EGlobActs::PreMovie
                , scriCur
                , kCQCKit::strEvId_OnClick
            );
        }

        // Get the movie volume control button and update the audio source
        tCIDLib::TCard4 c4Index;
        const TSCMediaInfo& scliMovies = m_scfgGen.scliMediaByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Movies), c4Index
        );
        TCQCIntfWidget* piwdgVol = iwdgEdit.piwdgFindByName(L"MovieVolCtrl", kCIDLib::False);
        ReplaceAudioSrc(*piwdgVol, scliMovies, tCQCSysCfg::ERmFuncs::Movies);
    }

    if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Music))
    {
        if (scriCur.bGlobalActEnabled(tCQCSysCfg::EGlobActs::PreMusic))
        {
            piwdgCmd = iwdgEdit.piwdgCastByName<TCQCIntfCmdButton>(L"MusicButton");
            UpdateGActCommand
            (
                *piwdgCmd
                , tCQCSysCfg::EGlobActs::PreMusic
                , scriCur
                , kCQCKit::strEvId_OnClick
            );
        }

        // Get the music volume control button and update the audio source
        tCIDLib::TCard4 c4Index;
        const TSCMediaInfo& scliMusic = m_scfgGen.scliMediaByUID
        (
            scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Music), c4Index
        );
        TCQCIntfWidget* piwdgVol = iwdgEdit.piwdgFindByName(L"MusicVolCtrl", kCIDLib::False);
        ReplaceAudioSrc(*piwdgVol, scliMusic, tCQCSysCfg::ERmFuncs::Music);
    }


    //
    //  If we kept the security tile, then we have to deal with whether there is a
    //  custom layout set. If so, we delete the regular title. If not, we delete the
    //  title bar button and arrow. And, if we keep the button, we have to update the
    //  target path that it loads.
    //
    if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
    {
        if (m_scfgGen.strCustLayout().bIsEmpty())
        {
            DeleteWidget(iwdgEdit, L"SecurityButton");
            DeleteWidget(iwdgEdit, L"SecurityArrow");
        }
         else
        {
            DeleteWidget(iwdgEdit, L"SecurityTitle");

            //
            //  Find our target security template path in the target path list and
            //  update it for the current room and resolution.
            //
            strRep = strFindTarPath(m_scfgGen.strCustLayout());
            strRep.eReplaceToken(strResFmt, L'r');
            strRep.eReplaceToken(scriCur.strName(), L'n');

            // Replace the original link text with the new target
            TCQCIntfWidget* piwdgBut = iwdgEdit.piwdgFindByName(L"SecurityButton", kCIDLib::False);
            piwdgBut->Replace
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , L"./Overlay_CustSecurity"
                , strRep
                , kCIDLib::False
                , kCIDLib::False
                , m_regxSaR
            );
        }
    }


    //
    //  And now if there are any extra overlays configured for this room, we need
    //  to process those.
    //
    const tCIDLib::TCard4 c4XOverCnt = scriCur.c4XOverCount();
    if (c4XOverCnt)
    {
        TCQCIntfWidget* piwdgTileBng(iwdgEdit.piwdgFindByName(L"TimePanel", kCIDLib::False));
        TCQCIntfWidget* piwdgTileTitle(iwdgEdit.piwdgFindByName(L"TimeTitle", kCIDLib::False));

        TArea areaTile;
        TArea areaTileCnt;
        TArea areaCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XOverCnt; c4Index++)
        {
            // Get the uid for the current xtra overlay in this room
            const TString& strXUID = scriCur.strXOverUIDAt(c4Index);

            //
            //  Now find that UID in the main config to get the actual Xover info. It
            //  returns also the index of this XOver in the main list which we don't
            //  need here.
            //
            tCIDLib::TCard4 c4At;
            const TSCXOverInfo& scliCur = m_scfgGen.scliXOverByUID(strXUID, c4At);
            if (scliCur.m_strTileTmpl.bIsEmpty())
                continue;

            //
            //  We have at least a tile overlay. Look up this one in the list of
            //  tile templates we previously downloaded.
            //
            const TCQCIntfTemplate& iwdgTile = iwdgFindTileTmpl(scliCur.m_strUniqueId);

            //
            //  Set up area of the tile into the next available tile slot. We take the
            //  base tile's bgn panel and adjust it to the next slot.
            //
            areaTile = piwdgTileBng->areaActual();
            areaTile.AdjustOrg
            (
                tCIDLib::TInt4(c4TarInd % 3) * i4XOfs
                , tCIDLib::TInt4(c4TarInd / 3) * i4YOfs
            );

            //
            //  Now get the size of the user's content and center it in the overall
            //  tile area. The difference between this org and the original org is now
            //  much we have to offset each widget to get it into correct relative pos.
            //
            //  We have to move it down to account for the title bar.
            //
            areaTileCnt = iwdgTile.areaActual();
            areaTileCnt.CenterIn(areaTile);
            areaTileCnt.i4Y
            (
                areaTile.i4Y() + piwdgTileLink->areaRelative().c4Height() + 3
            );

            // Duplicate the base tile bgn, update it, and put it into place
            TCQCIntfWidget* piwdgNew = static_cast<TCQCIntfWidget*>
            (
                piwdgTileBng->pobjDuplicate()
            );
            piwdgNew->strWidgetId(TString::strEmpty());
            piwdgNew->Ungroup();
            piwdgNew->SetArea(areaTile, kCIDLib::False);
            iwdgEdit.AddChild(piwdgNew);

            //
            //  If it has a content template we have to do a link, else we just
            //  do a static text title.
            //
            if (!scliCur.m_strContentTmpl.bIsEmpty())
            {
                //
                //  Look up the source template in the template paths list, to
                //  find the correct target
                //
                const TString& strTarPath = strFindXOverTarPath(scliCur.m_strContentTmpl);
                if (strTarPath.bIsEmpty())
                {
                    QUpMsg
                    (
                        L"I -         Could not find target for: %(1)"
                        , scliCur.m_strContentTmpl
                    );
                }
                 else
                {
                    piwdgNew = static_cast<TCQCIntfWidget*>(piwdgTileLink->pobjDuplicate());
                    piwdgNew->strWidgetId(TString::strEmpty());
                    piwdgNew->Ungroup();

                    //
                    //  The link we want to make in the form "./XOver/[name] so that it
                    //  is correctly relative. So we need to get the name part of the
                    //  target template we generated.
                    //
                    facCQCRemBrws().QueryNamePart(strTarPath, strRep);
                    strRep.Insert(L"./XOvers/", 0);

                    // Replace the original link text with the new target
                    piwdgNew->Replace
                    (
                        tCQCIntfEng::ERepFlags::ActionParms
                        , L"./Overlay_Weather"
                        , strRep
                        , kCIDLib::False
                        , kCIDLib::False
                        , m_regxSaR
                    );

                    // Remove it's original tile origin and add the new
                    areaCur = piwdgNew->areaActual();
                    areaCur -= areaWeather.pntOrg();
                    areaCur += areaTile.pntOrg();
                    piwdgNew->SetArea(areaCur, kCIDLib::False);
                    piwdgNew->strCaption(TString(L"    ") + scliCur.m_strTitle);
                    iwdgEdit.AddChild(piwdgNew);

                    // Add the arrow that indicates a link
                    piwdgNew = static_cast<TCQCIntfWidget*>(piwdgTileArrow->pobjDuplicate());
                    piwdgNew->strWidgetId(TString::strEmpty());
                    piwdgNew->Ungroup();

                    const TArea areaLink(areaCur);
                    areaCur = piwdgNew->areaActual();
                    areaCur.JustifyIn(areaLink, tCIDLib::EHJustify::Right, tCIDLib::EVJustify::Center);
                    areaCur.AdjustOrg(-16, 0);
                    piwdgNew->SetArea(areaCur, kCIDLib::False);
                    iwdgEdit.AddChild(piwdgNew);
                }
            }
             else
            {
                piwdgNew = static_cast<TCQCIntfWidget*>(piwdgTileTitle->pobjDuplicate());
                piwdgNew->strWidgetId(TString::strEmpty());
                piwdgNew->Ungroup();
                areaCur = piwdgNew->areaActual();
                areaCur -= areaBase.pntOrg();
                areaCur += areaTile.pntOrg();
                piwdgNew->SetArea(areaCur, kCIDLib::False);
                piwdgNew->strCaption(scliCur.m_strTitle);
                iwdgEdit.AddChild(piwdgNew);
            }

            //
            //  Now let's replicate all of the user's widgets and position them
            //  into the target area.
            //
            const tCIDLib::TCard4 c4WdgCnt = iwdgTile.c4ChildCount();
            for (tCIDLib::TCard4 c4WInd = 0; c4WInd < c4WdgCnt; c4WInd++)
            {
                piwdgNew = static_cast<TCQCIntfWidget*>
                (
                    iwdgTile.iwdgAt(c4WInd).pobjDuplicate()
                );

                piwdgNew->strWidgetId(TString::strEmpty());
                piwdgNew->Ungroup();

                areaCur = piwdgNew->areaActual();
                areaCur += areaTileCnt.pntOrg();
                piwdgNew->SetArea(areaCur, kCIDLib::False);
                iwdgEdit.AddChild(piwdgNew);
            }

            c4TarInd++;
        }
    }


    //
    //  Based on where we got the target slot to, we can resize the template to
    //  fit the rows we created. The tiles are indented vertically by 16 pixels,
    //  but the vertical offset includes the vertical spacing.
    //
    tCIDLib::TCard4 c4Size = (c4TarInd / 3) * i4YOfs;
    if (c4TarInd % 3)
        c4Size += i4YOfs;
    TSize szNew = iwdgEdit.areaActual().szArea();
    szNew.c4Height(c4Size);
    iwdgEdit.SetSize(szNew, kCIDLib::False);
}



//
//  We may have to remove some of the forecast days. Weather drivers have to provide
//  at least 3 days, but can provide more. Our pre-fab weather template has 6 day
//  slots. If the driver supports fewer, we have to remove some.
//
tCIDLib::TVoid
TAutoGenDlg::ProcessWeatherFC(TCQCIntfTemplate& iwdgEdit, const TCQCSysCfgRmInfo& scriCur)
{
    //
    //  This is called without checking whether weather is set up for this room, so we
    //  need to check it.
    //
    if (!scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Weather))
        return;

    tCIDLib::TCard4 c4Index;
    const TSCWeatherInfo& scliWeath = m_scfgGen.scliWeatherByUID
    (
        scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Weather), c4Index
    );

    // Ask the driver how many days it supports
    tCQCKit::TCQCSrvProxy orbcSrv(facCQCKit().orbcCQCSrvAdminProxy(scliWeath.m_strMoniker));
    tCIDLib::TCard4 c4FCDays;
    tCIDLib::TCard4 c4SerNum = 0;
    orbcSrv->bReadCardByName
    (
        c4SerNum, scliWeath.m_strMoniker, L"WEATH#FCDayCount", c4FCDays
    );

    //
    //  The original content has five day's worth, fo if less that that, we need
    //  to prune. The glare image in each day's block is named, so we can find it
    //  and then just delete everything in the same group.
    //
    //  There are 6 blocks, so we have to delete backwards from the end till we
    //  get down to the last one available.
    //
    if (c4FCDays < 3)
    {
        QUpMsg(L"F - The weather driver %(1) reports < 3 forecast days", TCardinal(c4FCDays));
    }
     else if (c4FCDays < 6)
    {
        tCIDLib::TCard4 c4CurDay = 6;
        tCIDLib::TCard4 c4FirstZ;
        TString strName;
        tCQCIntfEng::TChildList colGroup(tCIDLib::EAdoptOpts::NoAdopt);
        tCIDLib::TInt4 i4LastLeft = 0;
        while (c4CurDay > c4FCDays)
        {
            strName = L"Day";
            strName.AppendFormatted(c4CurDay);

            if (!iwdgEdit.bFindGroupMembers(strName, colGroup, c4FirstZ))
            {
                QUpMsg(L"F - Could not find forecast day group %(1)", strName);
                break;
            }

            //
            //  We need to get the glare widget which is the left-most. Remember that
            //  as where we will resize the template to when we hit the last removed
            //  group.
            //
            strName.Append(L"Glare");
            TCQCIntfWidget* piwdgGlare = iwdgEdit.piwdgFindByName(strName, kCIDLib::False);
            i4LastLeft = piwdgGlare->areaActual().i4Left();

            // Now we can remove all of the members of this group
            const tCIDLib::TCard4 c4GroupCnt = colGroup.c4ElemCount();
            for (c4Index = 0; c4Index < c4GroupCnt; c4Index++)
                iwdgEdit.RemoveWidget(colGroup[c4Index]);

            c4CurDay--;
        }

        // Size down the template to fit what's left, assuming we didn't fail
        if (c4CurDay == c4FCDays)
        {
            TArea areaNew = iwdgEdit.areaActual();
            areaNew.i4Right(i4LastLeft);
            iwdgEdit.SetSize(areaNew.szArea(), kCIDLib::False);
        }
    }
}


//
//  Helpers to queue up messages with a various replacement tokens
//
tCIDLib::TVoid TAutoGenDlg::QUpMsg(const tCIDLib::TCh* const pszText)
{
    m_colMsgQ.objPushTop(pszText);
}

tCIDLib::TVoid
TAutoGenDlg::QUpMsg(const   tCIDLib::TCh* const pszText
                    , const MFormattable&       fmtblToken1)
{
    m_strTmpMsg = pszText;
    m_strTmpMsg.eReplaceToken(fmtblToken1, kCIDLib::chDigit1);
    m_colMsgQ.objPushTop(m_strTmpMsg);
}

tCIDLib::TVoid
TAutoGenDlg::QUpMsg(const   tCIDLib::TCh* const pszText
                    , const MFormattable&       fmtblToken1
                    , const MFormattable&       fmtblToken2)
{
    m_strTmpMsg = pszText;
    m_strTmpMsg.eReplaceToken(fmtblToken1, kCIDLib::chDigit1);
    m_strTmpMsg.eReplaceToken(fmtblToken2, kCIDLib::chDigit2);
    m_colMsgQ.objPushTop(m_strTmpMsg);
}


//
//  Removes all of the widgets from the passed template that are part of the indicated
//  group. This is mostly to aid in the removal of tiles from the tiles overlay, for
//  functional areas not enabled for a given room.
//
tCIDLib::TVoid
TAutoGenDlg::RemoveGroup(TCQCIntfTemplate& iwdgEdit, const TString& strGroupName)
{
    //
    //  The collection must be non-adopting. We are just getting references to
    //  widgets owned by the template.
    //
    tCIDLib::TCard4         c4FirstZ;
    tCQCIntfEng::TChildList colMembers(tCIDLib::EAdoptOpts::NoAdopt);
    iwdgEdit.bFindGroupMembers(strGroupName, colMembers, c4FirstZ);

    const tCIDLib::TCard4 c4Count = colMembers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        iwdgEdit.RemoveWidget(colMembers[c4Index]);
}



//
//  We have a special case with media audio sources in that this is one that will
//  potentially be multi-unit. So, in that case, the field name can be different
//  as well as the moniker.
//
//  Our source fields always have a sub-unit since we use the AG_MZAudio driver to
//  do volume fields in the source templates, Z1 is music and Z2 is movies. And
//  that also means we know what the source moniker is as well.
//
//  This is called with the music/movie room function values to indicate which
//  we are doing. The only difference is whether we replace the Z1 or Z1 sub-unit.
//
tCIDLib::TVoid
TAutoGenDlg::ReplaceAudioSrc(       TCQCIntfWidget&         iwdgTar
                            , const TSCMediaInfo&           scliInfo
                            , const tCQCSysCfg::ERmFuncs    eFunc)
{
    //
    //  First do the monikers, to get any device associations or the use of the
    //  monikers in action parms and the like.
    //
    {
        static const tCQCIntfEng::ERepFlags eMonFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ERepFlags::ActionParms
            , tCQCIntfEng::ERepFlags::AssocDevice
            , tCQCIntfEng::ERepFlags::AssocField
            , tCQCIntfEng::ERepFlags::Children
            , tCQCIntfEng::ERepFlags::Expressions
        );

        iwdgTar.Replace
        (
            eMonFlags
            , L"AG_MZAudio"
            , scliInfo.m_strAudioMon
            , kCIDLib::False
            , kCIDLib::False
            , m_regxSaR
        );
    }


    // And now do the part of the fields that would be affected
    {
        static const tCQCIntfEng::ERepFlags eFldFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ERepFlags::ActionParms
            , tCQCIntfEng::ERepFlags::AssocField
            , tCQCIntfEng::ERepFlags::Children
            , tCQCIntfEng::ERepFlags::Expressions
        );

        // The new one may or may not have a sub-unit part
        TString strRepWith(L"AUD#");
        if (!scliInfo.m_strAudioSub.bIsEmpty())
        {
            strRepWith.Append(scliInfo.m_strAudioSub);
            strRepWith.Append(L'~');
        }

        // Set up the known source field
        TString strRep(L"AUD#");
        if (eFunc == tCQCSysCfg::ERmFuncs::Music)
            strRep.Append(L"Z1~");
        else
            strRep.Append(L"Z2~");

        iwdgTar.Replace
        (
            eFldFlags
            , strRep
            , strRepWith
            , kCIDLib::False
            , kCIDLib::False
            , m_regxSaR
        );
    }
}


//
//  This is a helper to do a replace on a moniker. We use moniker names that are
//  not going to be otherwise seen anywhere, purposefully so. So we can do a single
//  replacement pass, for a partial match (so that we get uses within things like
//  parameters.) We don't have to worry about our monikers being part of a larger
//  value that we would mangle.
//
tCIDLib::TVoid
TAutoGenDlg::ReplaceMoniker(        TCQCIntfTemplate&   iwdgTar
                            , const TString&            strSrc
                            , const TString&            strTar)
{
    static const tCQCIntfEng::ERepFlags eMonFlags1 = tCIDLib::eOREnumBits
    (
        tCQCIntfEng::ERepFlags::ActionParms
        , tCQCIntfEng::ERepFlags::AssocField
        , tCQCIntfEng::ERepFlags::AssocDevice
        , tCQCIntfEng::ERepFlags::Children
        , tCQCIntfEng::ERepFlags::Expressions
    );

    iwdgTar.Replace
    (
        eMonFlags1
        ,strSrc
        ,strTar
        ,kCIDLib::False
        ,kCIDLib::False
        ,m_regxSaR
    );
}


//
//  Look up the target path for a given source path. We have one for extra overlay
//  templates one for non-extras.
//
const TString& TAutoGenDlg::strFindTarPath(const TString& strSrcPath) const
{
    for(tCIDLib::TCard4 c4Index = 0; c4Index < m_c4PreTmplCnt; c4Index++)
    {
        if (m_colTPaths[c4Index].strKey().bCompareI(strSrcPath))
            return m_colTPaths[c4Index].strValue();
    }
    return TString::strEmpty();
}

const TString& TAutoGenDlg::strFindXOverTarPath(const TString& strSrcPath) const
{
    // These start past the pre-fab templates
    const tCIDLib::TCard4 c4Count = m_colTPaths.c4ElemCount();
    for(tCIDLib::TCard4 c4Index = m_c4PreTmplCnt; c4Index < c4Count; c4Index++)
    {
        if (m_colTPaths[c4Index].strKey().bCompareI(strSrcPath))
            return m_colTPaths[c4Index].strValue();
    }
    return TString::strEmpty();
}


//
//  We find a list of all of the images referenced by any of our source templates.
//  For each one we create a relative target path for where they will be, within
//  their individual target resolution directories. The target res part will be a
//  replacement token we can just replace for each resolution. We create a master
//  K/V pair list that has the source image path in the key and the target path in
//  the value (with the replacement token still.)
//
//  We then download all of the images. Once we have them, we can do the scaling
//  for each configured resolution.
//
//  For scaling purposes, we add all of the images that are in any referenced
//  scopes (as opposed to specific images) into the image list, so that we scale
//  those as well. But we also need to create a map for the scopes so that we can
//  search and replace them in the templates later.
//
tCIDLib::TVoid TAutoGenDlg::ScaleImages(TThread& thrThis)
{
    tCIDLib::TStrHashSet   colImgs(109, TStringKeyOps(kCIDLib::False));
    tCIDLib::TStrHashSet   colScopes(109, TStringKeyOps(kCIDLib::False));

    //
    //  Do all of the templates, standard and extras
    tCIDLib::TCard4 c4TmplCnt = m_colSrcTmpls.c4ElemCount();
    for(tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        m_colSrcTmpls[c4Index].QueryReferencedImgs
        (
            colImgs, colScopes, kCIDLib::True, kCIDLib::True
        );
    }

    // And any tile templates
    c4TmplCnt = m_colTileTmpls.c4ElemCount();
    for(tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        m_colTileTmpls[c4Index].QueryReferencedImgs
        (
            colImgs, colScopes, kCIDLib::True, kCIDLib::True
        );
    }

    //
    //  Go through the individual images and create the key/value pairs for them.
    //  We have an overall target image path, which is the main output scope, and
    //  under that a scope for each resolution.
    //
    TString strImgTar = m_scfgGen.strTarScope();
    strImgTar.Append(L"/%(r)");

    QUpMsg(L"I - Creating image list...");

    TString strTar;
    TString strTmp;
    {
        tCIDLib::TStrHashSet::TCursor cursImgs(&colImgs);
        for (; cursImgs; ++cursImgs)
        {
            const TString& strSrcImg = *cursImgs;

            //
            //  We recreate the source dir structure under the target, changing
            //  /System to /S and /User to /U. So take the src and cut out the
            //  required characters to create that format.
            //
            strTmp = strSrcImg;
            if (strTmp.bStartsWithI(L"/User"))
                strTmp.Cut(2, 3);
            else if (strTmp.bStartsWithI(L"/System"))
                strTmp.Cut(2, 5);

            //
            //  Build up what it's going to be put under. It's the overall image
            //  output path, plus a place holder for the resolution. Then the above
            //  modified source.
            //
            strTar = strImgTar;
            strTar.Append(strTmp);

            // And now add an entry for this one
            m_colImgMap.objAdd(TKeyValuePair(strSrcImg, strTar));
        }
    }

    //
    //  Remember the current image count. Allof them up to here are the single
    //  images. The rest are from scopes, just put into the list for scaling
    //  purposes.
    //
    m_c4SingleImgCnt = m_colImgMap.c4ElemCount();

    //
    //  Do the scopes now. For each one, we query all of the images in that
    //  scope and add them to the image list in the same way as above.
    //
    {
        // We need to get lists of image paths
        tCIDLib::TStrList colImgPaths;

        tCIDLib::TStrHashSet::TCursor cursScopes(&colScopes);
        for (; cursScopes; ++cursScopes)
        {
            TString strSrcScope = *cursScopes;

            if(strSrcScope.chLast() == kCIDLib::chForwardSlash)
                strSrcScope.DeleteLast();

            // Set this one up in the map for later use
            strTmp = strSrcScope;
            if (strTmp.bStartsWithI(L"/User"))
                strTmp.Cut(2, 3);
            else if (strTmp.bStartsWithI(L"/System"))
                strTmp.Cut(2, 5);
            else
            {
                CIDAssert2(L"The image path doesn't start with /User or /System")
            }

            strTar = strImgTar;
            strTar.Append(strTmp);
            m_colScopeMap.objAdd(TKeyValuePair(strSrcScope, strTar));

            //
            //  Query all of the images in this scope and add them to our image list. Get
            //  items only, not scopes.
            //
            tCIDLib::TCard4 c4ScopeId;
            m_pdsclSrv->bQueryScopeNames
            (
                c4ScopeId
                , strSrcScope
                , tCQCRemBrws::EDTypes::Image
                , colImgPaths
                , tCQCRemBrws::EQSFlags::Items
                , facCQCAdmin.sectUser()
            );

            TString strSrcImg;
            const tCIDLib::TCard4 c4ICnt = colImgPaths.c4ElemCount();
            for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ICnt; c4IInd++)
            {
                strSrcImg = strSrcScope;
                strSrcImg.Append(kCIDLib::chForwardSlash);
                strSrcImg.Append(colImgPaths[c4IInd]);

                // Create the target path (with replacement token)
                strTmp = strSrcImg;
                if (strTmp.bStartsWithI(L"/User"))
                    strTmp.Cut(2, 3);
                else if (strTmp.bStartsWithI(L"/System"))
                    strTmp.Cut(2, 5);

                strTar = strImgTar;
                strTar.Append(strTmp);

                // And add an entry for this one
                m_colImgMap.objAdd(TKeyValuePair(strSrcImg, strTar));
            }
        }
    }
    QUpMsg(L"I - Image list is complete");

    // The rest we can ignore if skipping images
    if (!m_bSkipImages)
    {
        //
        //  Let's query the image data. We use a lower level query here, so there's
        //  no caching done. We create our own little cache by just pre-reading all
        //  of the images into a vector the same size/order as our unique image
        //  list. We know the count now, so just an object array. We will get them
        //  as PNG files.
        //
        QUpMsg(L"I - Downloading images...");

        tCIDLib::TKVPFList colMeta;
        const tCIDLib::TCard4 c4ImgCount = m_colImgMap.c4ElemCount();
        TObjArray<TPNGImage> objaImgData(c4ImgCount);
        for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ImgCount; c4IInd++)
        {
            tCIDLib::TCard4 c4SerialNum = 0;
            tCIDLib::TEncodedTime enctLastChange;
            m_pdsclSrv->bReadImage
            (
                m_colImgMap[c4IInd].strKey()
                , c4SerialNum
                , enctLastChange
                , objaImgData[c4IInd]
                , colMeta
                , facCQCAdmin.sectUser()
                , kCIDLib::True
            );
        }
        QUpMsg(L"I - Image download complete");

        //
        //  OK, we can now loop through the resolutions that are to be supported and
        //  output the scaled versions.
        //
        QUpMsg(L"I -     Scaling images...");

        TString strMsg;
        const tCIDLib::TCard4 c4ResCnt = m_scfgGen.c4ResCount();
        for (tCIDLib::TCard4 c4ResInd= 0; c4ResInd< c4ResCnt; c4ResInd++)
        {
            // If asked to shutdown, then quite now
            if (thrThis.bCheckShutdownRequest())
                break;

            const TSize& szCurRes = m_scfgGen.szResAt(c4ResInd);
            QUpMsg(L"I -         Resolution: %(1)", szCurRes);

            tCIDLib::TFloat8 f8XScale, f8YScale;
            CalcScaleFactors(szCurRes, f8XScale, f8YScale);

            // OK, let's scale all of the images for this resolution
            ScaleImgsOneRes(szCurRes, f8XScale, f8YScale, objaImgData, thrThis);

            QUpMsg(L"I -         Completed");
        }

        QUpMsg(L"I -     Image scaling complete");
    }
}


//
//  This is called from ScaleImgs to scale all of the images for one of the configured
//  resolutions. The passed objaImgs array is the same size as the m_colImgMap
//  list, and has the actual image data in it for the corresponding map slot.
//
tCIDLib::TVoid
TAutoGenDlg::ScaleImgsOneRes(const  TSize&                  szTarRes
                            , const tCIDLib::TFloat8        f8XScale
                            , const tCIDLib::TFloat8        f8YScale
                            , const TObjArray<TPNGImage>&   objaImgs
                            ,       TThread&                thrThis)
{
    //
    //  Format out the resolution. This will be put into the target paths that we
    //  built up before.
    //
    TString strFmtRes;
    szTarRes.FormatToText(strFmtRes, tCIDLib::ERadices::Dec, kCIDLib::chLatin_x);

    //
    //  We remember the list of scopes we have created. We need to insure the scope
    //  for each output image exists, But we don't want to continually make remote
    //  calls to do that. So we track the ones we've created so far.
    //
    //  Tell the keyops object to be non-case sensitive in case there is a wrong
    //  case path in the templates.
    //
    tCIDLib::TStrHashSet colScopesMade(109UL, TStringKeyOps(kCIDLib::False));

    TFacCQCIntfEng&     facIV = facCQCIntfEng();
    TPNGImage           imgCur;
    TString             strScope;
    TString             strTar;
    TSize               szNew;
    TString             strMsg;
    const TSize         szMinImg(1, 1);
    tCIDLib::TKVPFList  colMeta;

    //
    //  Just loop through the images in the map. Scale each image for this target
    //  resolution and upload it.
    //
    const tCIDLib::TCard4 c4ImgCnt = m_colImgMap.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
    {
        // Use a small sleep to reduce overhead and watch for shutdown requests
        if (!thrThis.bSleep(100))
            break;

        const TKeyValuePair& kvalCur = m_colImgMap[c4Index];

        // Get the target path and update it with this target resolution
        strTar = kvalCur.strValue();
        strTar.eReplaceToken(strFmtRes, kCIDLib::chLatin_r);

        // See if we need to create this path
        strScope = strTar;
        if (!facCQCRemBrws().bRemoveLastPathItem(strScope))
        {
            CIDAssert2(L"There should have been a path item to remove")
        }

        if (!colScopesMade.bHasElement(strScope))
        {
            if (m_bVerbose)
                QUpMsg(L"I -             New image scope: %(1)", strScope);

            // Haven't checked it so do so and add it to the checked path list
            colScopesMade.objAdd(strScope);
            m_pdsclSrv->MakePath
            (
                strScope, tCQCRemBrws::EDTypes::Image, facCQCAdmin.cuctxToUse().sectUser()
            );
        }

        //
        //  Get the size and scale it. We have to copy it since we don't want to
        //  affect the original. We need it for subsequent scaling operations.
        //
        imgCur = objaImgs[c4Index];

        szNew = imgCur.szImage();
        szNew.Scale(f8XScale, f8YScale);

        // Don't let it get smaller than 1 pixel in either direction
        szNew.TakeLarger(szMinImg);

        // Scale the image to this new size if different
        if (szNew != imgCur.szImage())
            imgCur.Scale(szNew, 4);

        //
        //  And upload it to its target scope. We use a helper here which will do the
        //  correct flattening and formatting and create the thumb and such. Tell the
        //  data server not to memory cache it if not already done.
        //
        colMeta.RemoveAll();
        facCQCIGKit().UploadImage
        (
            *m_pdsclSrv
            , imgCur
            , strTar
            , m_strmImg
            , m_strmThumb
            , kCIDLib::True
            , colMeta
            , facCQCAdmin.cuctxToUse()
        );
    }
}



//
//  There are various places where we allow the user to invoke a global action. This
//  is called to find the command, which will be disabled in the original source,
//  and will enable it and update the path and commands. We assume we won't be called
//  unless the action is enabled.
//
tCIDLib::TVoid
TAutoGenDlg::UpdateGActCommand(         MCQCCmdSrcIntf&         mcsrcUpdate
                                , const tCQCSysCfg::EGlobActs   eAct
                                , const TCQCSysCfgRmInfo&       scriSrc
                                , const TString&                strEvent)
{
    MCQCCmdSrcIntf::TOpcodeBlock* pcolOnEv = mcsrcUpdate.pcolOpsForEvent(strEvent);

    // This shouldn't get called unless this will work, but just in case
    if (!pcolOnEv)
    {
        QUpMsg
        (
            L"I -        GlobalAct cmd update failed. Room=%(1), Event=%(2)"
            , scriSrc.strName()
            , strEvent
        );
        return;
    }

    const TSCGlobActInfo& scliCur = scriSrc.scliGlobActAt(eAct);

    // Check it for enablement just in case
    if (!scliCur.m_bEnabled)
        return;

    const tCIDLib::TCard4 c4Count = pcolOnEv->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TActOpcode& aocCur = pcolOnEv->objAt(c4Index);

        // We only care about commands that target the system target
        if ((aocCur.m_eOpcode != tCQCKit::EActOps::Cmd)
        ||  (aocCur.m_ccfgStep.strTargetId() != kCQCKit::strCTarId_System))
        {
            continue;
        }

        if (aocCur.m_ccfgStep.strCmdId() == kCQCKit::strCmdId_DoGlobalAct)
        {
            // Enable the command
            aocCur.m_bDisabled = kCIDLib::False;

            //
            //  Build up the parameters, which can have replacement parameters
            //  in them as well, so we have to replace those values. It's a quoted
            //  comma list.
            //
            TString strParms;
            TString strCur;
            for (tCIDLib::TCard4 c4PInd = 0; c4PInd < kCQCSysCfg::c4MaxGActParms; c4PInd++)
            {
                strCur = scliCur.m_colParms[c4PInd];

                // Search and replace any of the standard tokens we support
                strCur.eReplaceToken(scriSrc.strRoomVarAt(0), L'1');
                strCur.eReplaceToken(scriSrc.strRoomVarAt(1), L'2');
                strCur.eReplaceToken(scriSrc.strRoomVarAt(2), L'3');
                strCur.eReplaceToken(scriSrc.strRoomVarAt(3), L'4');
                strCur.eReplaceToken(scriSrc.strName(), L'N');

                TStringTokenizer::BuildQuotedCommaList(strCur, strParms);
            }

            aocCur.m_ccfgStep.SetParmAt(0, scliCur.m_strPath);
            aocCur.m_ccfgStep.SetParmAt(1, strParms);

            // And we are done now
            break;
        }
    }
}



//
//  For the standard templates, we update them for the current room. We know the
//  monikers and fields and whatnot used by the source templates, so we can do
//  search and replace on those. For the user defined extra ones, we don't need
//  do anything but upload them.
//
//  Lights aren't dealt with here. They are handled in temlate specific processing
//  methods already called. The monikers and the fields will be different for them,
//  and there are various other considerations.
//
//  This is only called for the standard (prefab) templates, since only they
//  need it, though it wouldn't like cause harm if it was.
//
tCIDLib::TVoid
TAutoGenDlg::UpdateTmplForRoom(         TThread&            thrThis
                                , const tCIDLib::TCard4     c4TmplIndex
                                ,       TCQCIntfTemplate&   iwdgTar
                                , const TCQCSysCfgRmInfo&   scriCur
                                , const TString&            strResFmt)
{
    tCIDLib::TCard4 c4Index;
    TString         strRepWith;
    try
    {
        if (m_bVerbose)
            QUpMsg(L"I -         Template: %(1)", iwdgTar.strTemplateName());

        //
        //  Update all of the media stuff if enabled. We have to replace the repo
        //  and renderer monikers. And we have to deal with the individual fields
        //  for the audio source, since in that case there might be a sub-unit,
        //  so the fields may be different as well. We have to do that first, then
        //  do the general moniker replacement, else we won't find the fields we
        //  are looking for.
        //
        if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Movies))
        {
            const TSCMediaInfo& scliMovies = m_scfgGen.scliMediaByUID
            (
                scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Movies), c4Index
            );
            ReplaceMoniker(iwdgTar, L"AG_MovieRepo", scliMovies.m_strRepoMon);
            ReplaceMoniker(iwdgTar, L"AG_MovieRend", scliMovies.m_strRendMon);
        }

        if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Music))
        {
            const TSCMediaInfo& scliMusic = m_scfgGen.scliMediaByUID
            (
                scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Music), c4Index
            );
            ReplaceMoniker(iwdgTar, L"AG_MusicRepo", scliMusic.m_strRepoMon);
            ReplaceMoniker(iwdgTar, L"AG_MusicRend", scliMusic.m_strRendMon);
        }


        //
        //  For security we have to deal with the area name. There are a few
        //  fields used that we have to replace.
        //
        if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
        {
            const TSCSecInfo& scliSec = m_scfgGen.scliSecByUID
            (
                scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Security), c4Index
            );

            TString strSrc = L"AG_Security.SEC#Area_House_";
            TString strTar = scliSec.m_strMoniker;
            strTar.Append(L".SEC#Area_");
            strTar.Append(scliSec.m_strAreaName);
            strTar.Append(kCIDLib::chUnderscore);

            static const tCQCIntfEng::ERepFlags eFldFlags = tCIDLib::eOREnumBits
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , tCQCIntfEng::ERepFlags::AssocField
                , tCQCIntfEng::ERepFlags::Children
                , tCQCIntfEng::ERepFlags::Expressions
            );
            iwdgTar.Replace
            (
                eFldFlags, strSrc, strTar, kCIDLib::False, kCIDLib::False, m_regxSaR
            );

            // Do any for just the moniker in action parameters
            static const tCQCIntfEng::ERepFlags eMonFlags = tCIDLib::eOREnumBits
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , tCQCIntfEng::ERepFlags::Children
            );
            iwdgTar.Replace
            (
                eFldFlags
                , L"AG_Security"
                , scliSec.m_strMoniker
                , kCIDLib::False
                , kCIDLib::False
                , m_regxSaR
            );
        }


        if (scriCur.bFuncEnabled(tCQCSysCfg::ERmFuncs::Weather))
        {
            const TSCWeatherInfo& scliWeath = m_scfgGen.scliWeatherByUID
            (
                scriCur.strFuncId(tCQCSysCfg::ERmFuncs::Weather), c4Index
            );
            ReplaceMoniker(iwdgTar, L"AG_Weather", scliWeath.m_strMoniker);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        QUpMsg
        (
            L"I - Failed to update template '%(1)' for room '%(2)'"
            , iwdgTar.strTemplateName()
            , scriCur.strName()
        );
    }

    catch(...)
    {
        QUpMsg
        (
            L"I - System error updating template '%(1)' for room '%(2)'"
            , iwdgTar.strTemplateName()
            , scriCur.strName()
        );
    }
}

