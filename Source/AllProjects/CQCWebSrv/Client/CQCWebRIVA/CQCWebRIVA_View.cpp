//
// FILE NAME: CQCWebRIVA_View.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2009
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
//  This file implements the TCQCWebRIVAView class, which provides a display
//  mechanism for user drawn interfaces via a remoted graphical device object.
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
#include    "CQCWebRIVA_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCWebRIVAView, TCQCIntfStdView)



// ---------------------------------------------------------------------------
//   CLASS: TCQCWebRIVAView
//  PREFIX: civ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCWebRIVAView::TCQCWebRIVAView(const  TString&                    strTitle
                                , const tCIDLib::TFloat8            f8Lat
                                , const tCIDLib::TFloat8            f8Long
                                , const TSize&                      szDevRes
                                , const tCQCIntfEng::TGraphIntfDev  cptrRemDev
                                ,       TWebRIVAGraphDev*           pgdevToUse
                                ,       TWebRIVAWIntf* const        pwrwiOwner
                                ,       MCQCIntfAppHandler* const   pmiahOwner
                                , const TCQCUserCtx&                cuctxToUse
                                , const tCIDLib::TFloat8            f8ClientLat
                                , const tCIDLib::TFloat8            f8ClientLong) :
    TCQCIntfStdView
    (
        strTitle
        , pmiahOwner
        , facCIDGraphDev().rgbBlack
        , f8Lat
        , f8Long
        , kCIDLib::False
        , cuctxToUse
    )
    , m_bMouseCaptured(kCIDLib::False)
    , m_c4ModalDepth(0)
    , m_cptrRemDev(cptrRemDev)
    , m_f8ClientLat(f8ClientLat)
    , m_f8ClientLong(f8ClientLong)
    , m_gdevCompat
      (
        TSize(10, 10)
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::EPixFmts::TrueAlpha
      )
    , m_pgdevToUse(pgdevToUse)
    , m_pmiahOwner(pmiahOwner)
    , m_pwrwiOwner(pwrwiOwner)
    , m_szDevRes(szDevRes)
{
}

TCQCWebRIVAView::TCQCWebRIVAView(const  TCQCIntfTemplate&           iwdgTemplate
                                , const TString&                    strTitle
                                , const tCIDLib::TFloat8            f8Lat
                                , const tCIDLib::TFloat8            f8Long
                                , const TSize&                      szDevRes
                                , const tCQCIntfEng::TGraphIntfDev  cptrRemDev
                                ,       TWebRIVAGraphDev*           pgdevToUse
                                ,       TWebRIVAWIntf* const        pwrwiOwner
                                ,       MCQCIntfAppHandler* const   pmiahOwner
                                , const TCQCUserCtx&                cuctxToUse
                                , const tCIDLib::TFloat8            f8ClientLat
                                , const tCIDLib::TFloat8            f8ClientLong) :
    TCQCIntfStdView
    (
        strTitle
        , iwdgTemplate
        , pmiahOwner
        , facCIDGraphDev().rgbBlack
        , f8Lat
        , f8Long
        , kCIDLib::False
        , cuctxToUse
    )
    , m_bMouseCaptured(kCIDLib::False)
    , m_c4ModalDepth(0)
    , m_cptrRemDev(cptrRemDev)
    , m_f8ClientLat(f8ClientLat)
    , m_f8ClientLong(f8ClientLong)
    , m_gdevCompat
      (
        TSize(10, 10)
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::EPixFmts::TrueAlpha
      )
    , m_pgdevToUse(pgdevToUse)
    , m_pmiahOwner(pmiahOwner)
    , m_pwrwiOwner(pwrwiOwner)
    , m_szDevRes(szDevRes)
{
}

TCQCWebRIVAView::~TCQCWebRIVAView()
{
}


// ---------------------------------------------------------------------------
//  TCQCWebRIVAView: Public, inherited methods
// ---------------------------------------------------------------------------

// Return the full possible area of the display 'device'
TArea TCQCWebRIVAView::areaDisplayRes() const
{
    return TArea(TPoint::pntOrigin, m_szDevRes);
}


//
//  Return the current area of the actual view area, which for us is the
//  same as the device resolution.
//
TArea TCQCWebRIVAView::areaView() const
{
    return TArea(TPoint::pntOrigin, m_szDevRes);
}


// Return our remote graphics device
tCQCIntfEng::TGraphIntfDev TCQCWebRIVAView::cptrNewGraphDev() const
{
    return m_cptrRemDev;
}


//
//  We override this to get callbacks from the command engine that runs
//  interface based actions. In order to make this work right, we have to
//  emulate the way it normally works and dispatch this to the 'GUI' thread,
//  and let him handle it. Otherwise, there are issues because we'd be
//  accessing widgets at the same time he is.
//
//  So this will block until the GUI thread has processed the event and
//  then return.
//
tCIDLib::TVoid TCQCWebRIVAView::DoActEvent(tCQCIntfEng::TIntfCmdEv& iceToDo)
{
    m_pwrwiOwner->DispatchActEvent(iceToDo);
}


//
//  Because of the usually GUI based nature of the interface engine, it uses modal
//  loops when popups occur. So we have to emulate this. So when the engine calls us
//  to to do a modal loop we emulate that by just calling back into the owning
//  thread's faux GUI loop (since he also emulates the GUI scenario as well with a
//  message loop.)
//
//  Currently, there's no need to enforce this modality on the client since there's
//  noting they can do that would require it. The user could get to to menus or close
//  the app, but that wouldn't matter to us since there's no command the menus could
//  send us that would cause a problem, and if they close, we'll see the socket go
//  down and just unwind.
//
tCIDLib::TVoid
TCQCWebRIVAView::DoModalLoop(tCIDLib::TBoolean& bBreakFlag, const tCIDLib::TBoolean)
{
    // Bump our modal depth
    m_c4ModalDepth++;
    try
    {
        m_pwrwiOwner->FauxGUILoop(bBreakFlag);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Drop the modal depth and rethrow
        m_c4ModalDepth--;
        throw;
    }

    catch(...)
    {
        // Drop the modal depth and rethrow
        m_c4ModalDepth--;
        throw;
    }

    // Drop the modeal depth back
    m_c4ModalDepth--;
}


//
//  We override this to do some extra processing for some commands and to override
//  some others.
//
tCQCKit::ECmdRes
TCQCWebRIVAView::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4Index
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    const TString& strTargetId = ccfgToDo.strTargetId();

    //
    //  Do any commands that we just want to completely override
    //
    if (strTargetId == kCQCIntfEng::strCTarId_IntfViewer)
    {
        if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RIVACmd)
        {
            // Pass it on to the client
            TWebRIVATools wrtToUse;
            tCIDLib::TCard4 c4Bytes;
            TMemBuf* pmbufNew = wrtToUse.pmbufFormatRIVACmd
            (
                c4Bytes
                , ccfgToDo.piAt(0).m_strValue
                , ccfgToDo.piAt(1).m_strValue
                , ccfgToDo.piAt(2).m_strValue
                , ccfgToDo.piAt(3).m_strValue
            );
            m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
            return tCQCKit::ECmdRes::Ok;
        }

        // Any others we let go
    }
     else if (strTargetId == kCQCKit::strCTarId_System)
    {
        // We send wave playback and TTS to the client to do
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PlayWAV)
        {
            // Pass it on to the client
        }
         else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RIVACmd)
        {
            // Pass it on to the client
            TWebRIVATools wrtToUse;
            tCIDLib::TCard4 c4Bytes;
            TMemBuf* pmbufNew = wrtToUse.pmbufFormatRIVACmd
            (
                c4Bytes
                , ccfgToDo.piAt(0).m_strValue
                , ccfgToDo.piAt(1).m_strValue
                , ccfgToDo.piAt(2).m_strValue
                , ccfgToDo.piAt(3).m_strValue
            );
            m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
            return tCQCKit::ECmdRes::Ok;
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayText)
        {
            // Pass it on to the client
        }

        // We don't pass these on, we are overriding them
        return tCQCKit::ECmdRes::Ok;
    }


    // Some that we may want to override, else we fall through to below
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Exit)
    {
        //
        //  If we are currently on the base template, then it's not an exit
        //  of a popup, it's a request to exit the program, so pass that to
        //  the client. If it's not zero, pass it on
        //
        if (c4StackSize() == 1)
        {
            TWebRIVATools wrtToUse;
            tCIDLib::TCard4 c4Bytes;
            TMemBuf* pmbufNew = wrtToUse.pmbufFormatExitViewer(c4Bytes);
            m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
            return tCQCKit::ECmdRes::Ok;
        }
    }


    //
    //  Do any commands that we want to modify ingoing parameters for or that we want to
    //  process after the fact.
    //
    tCQCKit::ECmdRes eRes;

    if ((strTargetId == kCQCKit::strCTarId_System)
    &&  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CalcGeoDistance)
    &&  (ccfgToDo.piAt(0).m_strValue.bCompareI(L"[CLIENT]")
    ||   ccfgToDo.piAt(1).m_strValue.bCompareI(L"[CLIENT]")))
    {
        //
        //  If the geo distance command references the special client location,
        //  value, we need to update the special parameters that reference the
        //  client coordinates, providing whatever we last were provided by
        //  the client.
        //
        //  Format out our client side lat/long info. If not available, then
        //  we throw an exception.
        //
        if ((m_f8ClientLat == kCQCWebRIVA::f8LocNotSet)
        ||  (m_f8ClientLong == kCQCWebRIVA::f8LocNotSet))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCfg_NoLocInfo
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotReady
                , TString(L"Client")
            );
        }

        // Make a copy of the command we can modify
        TCQCCmdCfg ccfgMod = ccfgToDo;

        // Format out the values appropriately for the command
        TString strFmt;
        strFmt.AppendFormatted(m_f8ClientLat);
        strFmt.Append(L' ');
        strFmt.AppendFormatted(m_f8ClientLong);

        // And update the references
        if (ccfgMod.piAt(0).m_strValue.bCompareI(L"[CLIENT]"))
            ccfgMod.piAt(0).m_strValue = strFmt;

        if (ccfgMod.piAt(1).m_strValue.bCompareI(L"[CLIENT]"))
            ccfgMod.piAt(1).m_strValue = strFmt;

        // Pass it on to our parent now
        eRes = TParent::eDoCmd
        (
            ccfgMod, c4Index, strUserId, strEventId, ctarGlobals, bResult, acteTar
        );
    }
     else
    {
        // These we let the parent see, and then do some extra stuff afterwards
        eRes = TParent::eDoCmd
        (
            ccfgToDo, c4Index, strUserId, strEventId, ctarGlobals, bResult, acteTar
        );

        //
        //  Do any post processing of commands after the fact
        //
        if (strTargetId == kCQCIntfEng::strCTarId_IntfViewer)
        {
            if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetViewBorderClr)
            {
                TRGBClr rgbNew;
                if (rgbNew.bParseFromText(ccfgToDo.piAt(0).m_strValue, tCIDLib::ERadices::Auto, L' '))
                {
                    // Pass it on to the client
                    TWebRIVATools wrtToUse;
                    tCIDLib::TCard4 c4Bytes;

                    //
                    //  The stupid DIV won't take rgb() formatted values without a lot of
                    //  silly quoting, so we make this one a string and format the color
                    //  ourself.
                    //
                    TString strClr(L"#", 6);
                    if (rgbNew.c1Red() < 0x10)
                        strClr.Append(kCIDLib::chDigit0);
                    strClr.AppendFormatted(rgbNew.c1Red(), tCIDLib::ERadices::Hex);
                    if (rgbNew.c1Green() < 0x10)
                        strClr.Append(kCIDLib::chDigit0);
                    strClr.AppendFormatted(rgbNew.c1Green(), tCIDLib::ERadices::Hex);
                    if (rgbNew.c1Blue() < 0x10)
                        strClr.Append(kCIDLib::chDigit0);
                    strClr.AppendFormatted(rgbNew.c1Blue(), tCIDLib::ERadices::Hex);

                    TMemBuf* pmbufNew = wrtToUse.pmbufFormatSetTmplBorderClr(c4Bytes, strClr);
                    m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
                }
            }
        }
    }

    return eRes;
}


//
//  Just call Redraw() which is the same for us since we don't have any distinction between
//  redrawing synchronously or asynchronously.
//
tCIDLib::TVoid TCQCWebRIVAView::ForceRedraw()
{
    Redraw();
}


//
//  We return a dummy memory device that we created in the ctor for all
//  compatible device uses. It can't actually be used, it's just for this
//  places where the engine requires a compatible device.
//
const TGraphDrawDev& TCQCWebRIVAView::gdevCompat() const
{
    return m_gdevCompat;
}


tCIDLib::TVoid TCQCWebRIVAView::InvalidateArea(const TArea& areaToRedraw)
{
    // Just call Redraw() which is the same for us
    Redraw(areaToRedraw);
}


//
//  The worker thread simulates the timers that are done with Window timers in
//  the real IV view. So we just pass this on to him.
//
tCIDLib::TVoid TCQCWebRIVAView::PauseTimers(const tCIDLib::TBoolean bPauseState)
{
    m_pwrwiOwner->PauseTimers(bPauseState);
}


//
//  This needs to be passed to the client. They may or may not do anything
//  with it. The wait mode is meaningless in the remote viewer system. The
//  client will always play it asynchronously.
//
tCIDLib::TVoid TCQCWebRIVAView::PlayWAV(  const   TString&            strToPlay
                                        , const tCIDLib::EWaitModes )
{
    //
    //  If it's empty for some reason, do nothing. We shouldn't get called if
    //  it is but best not to assume.
    //
    if (strToPlay.bIsEmpty())
        return;

    #if 0
    TRemVPoolJan janBuf(m_pwrwiOwner, strToPlay.c4Length() * 4);
    tCQCRemVComm::TGenOpText* pData
    (
        janBuf.pPrepAsWithText<tCQCRemVComm::TGenOpText>
        (
            kCQCRemVComm::c1OpCode_PlayWAV, strToPlay
        )
    );
    janBuf.QueueUpItem();
    #endif
}


//
//  Just force a redraw of everything. This will send all of the commands
//  to the client. Bracket it with start/end commands.
//
tCIDLib::TVoid TCQCWebRIVAView::Redraw()
{
    //
    //  Ask the underlying view for the area the covers the base and all
    //  popups. That because our redraw area.
    //
    TArea areaUpdate;
    QueryCombinedAreas(areaUpdate);

    m_pgdevToUse->StartDraw(areaUpdate);
    try
    {
        // Redraw all the widgets under the area
        DrawUnder(*m_pgdevToUse, areaUpdate);

        // And tell the view what area was updated
        m_pgdevToUse->EndDraw(areaUpdate);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Make sure we end the drawing bracket
        m_pgdevToUse->EndDraw(areaUpdate);
        throw;
    }
}


tCIDLib::TVoid TCQCWebRIVAView::Redraw(const TArea& areaToRedraw)
{
    m_pgdevToUse->StartDraw(areaToRedraw);
    try
    {
        // Redraw all fo the widgets under the redraw area
        DrawUnder(*m_pgdevToUse, areaToRedraw);

        // And tell the view what area was updated
        m_pgdevToUse->EndDraw(areaToRedraw);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Make sure we end the drawing bracket
        m_pgdevToUse->EndDraw(areaToRedraw);
        throw;
    }
}

tCIDLib::TVoid TCQCWebRIVAView::Redraw(const TGUIRegion& grgnToRedraw)
{
    // We can't do this one, and it shouldn't happen
    CIDAssert(kCIDLib::False, L"Can't do region based redraw");
}


//
//  These just pass errors on to the client by queuing them up as messages. This
//  is different from the real IV, where this would be a blocking operation, but we
//  can't really do that here.
//
tCIDLib::TVoid
TCQCWebRIVAView::ShowErr(const  TString&    strTitle
                        , const TString&    strText
                        , const TError&     errToShow)
{
    TWebRIVATools wrtTmp;
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = wrtTmp.pmbufFormatShowException
    (
        c4Bytes, strTitle, strText, errToShow
    );
    m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid
TCQCWebRIVAView::ShowMsg(const  TString&            strTitle
                        , const TString&            strText
                        , const tCIDLib::TBoolean   bIsError
                        , const MFormattable&       mfmtbl1
                        , const MFormattable&       mfmtbl2
                        , const MFormattable&       mfmtbl3)
{
    TString strMsg(strText);
    if (!MFormattable::bIsNullObject(mfmtbl1))
        strMsg.eReplaceToken(mfmtbl1, L'1');
    if (!MFormattable::bIsNullObject(mfmtbl2))
        strMsg.eReplaceToken(mfmtbl2, L'2');
    if (!MFormattable::bIsNullObject(mfmtbl3))
        strMsg.eReplaceToken(mfmtbl3, L'3');

    TWebRIVATools wrtTmp;
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = wrtTmp.pmbufFormatShowMsg(c4Bytes, strTitle, strMsg);
    m_pwrwiOwner->SendMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid TCQCWebRIVAView::TakeFocus()
{
    // A no-op for us, so we never pass it on to the templates
}


// We have no such thing, so we just have to throw if called
const TWindow& TCQCWebRIVAView::wndOwner() const
{
    CIDAssert(kCIDLib::False, L"This call has no meaning here");
    return TWindow::Nul_TWindow();
}

TWindow& TCQCWebRIVAView::wndOwner()
{
    CIDAssert(kCIDLib::False, L"This call has no meaning here");
    return TWindow::Nul_TWindow();
}


// ---------------------------------------------------------------------------
//  TCQCWebRIVAView: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  There's nothing to do here per se, but we do set a flag that indicates the client
//  has captured the mouse (and told us about it by sending a message to our owning work
//  thread who passed it to us. This way we can prevent certain things when the mouse is
//  captured potentially.
//
tCIDLib::TVoid TCQCWebRIVAView::CaptureMouse()
{
    m_bMouseCaptured = kCIDLib::True;
}


//
//  Return whatever the last point we were told about by the owning thread.
//
TPoint TCQCWebRIVAView::pntTouchPos() const
{
    return m_pntTouch;
}


//
//  There's nothing to do here per se, but we do set a flag that indicates the client
//  has captured the mouse (and told us about it by sending a message to our owning work
//  thread who passed it to us. This way we can prevent certain things when the mouse is
//  captured potentially.
//
tCIDLib::TVoid TCQCWebRIVAView::ReleaseMouseCapture()
{
    m_bMouseCaptured = kCIDLib::False;
}


//
//  If the client's position changes over time and he wants to report lat/long
//  info, the worker thread will get those msgs and call us here to update it.
//
tCIDLib::TVoid
TCQCWebRIVAView::SetClientLocInfo(const   tCIDLib::TFloat8    f8ClientLat
                                , const tCIDLib::TFloat8    f8ClientLong)
{
    m_f8ClientLat = f8ClientLat;
    m_f8ClientLong = f8ClientLong;
}


//
//  The faux GUI thread calls this to tell us where the remote client has moved
//  the pointer position. The interface engine gets info on the current
//  touch position from the owning view (us in this case.)
//
tCIDLib::TVoid TCQCWebRIVAView::SetTouchPos(const TPoint& pntPos)
{
    m_pntTouch = pntPos;
}

