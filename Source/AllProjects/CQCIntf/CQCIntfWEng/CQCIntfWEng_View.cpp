//
// FILE NAME: CQCIntfWEng_View.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This file implements the TCQCIntfWView class, which provides a display
//  mechanism for user drawn interfaces onto a standard window.
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
#include    "CQCIntfWEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfWView,TCQCIntfStdView)




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWView
//  PREFIX: civ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWView::TCQCIntfWView(       TCQCIntfViewWnd* const  pwndOwner
                            , const TString&                strTitle
                            ,       MCQCIntfAppHandler* const pmiahToUse
                            , const tCIDLib::TFloat8        f8Lat
                            , const tCIDLib::TFloat8        f8Long
                            , const TCQCUserCtx&            cuctxToUse) :

    TCQCIntfStdView
    (
        strTitle
        , pmiahToUse
        , pwndOwner->rgbBgnFill()
        , f8Lat
        , f8Long
        , kCIDLib::True
        , cuctxToUse
    )
    , m_pgdevCompat(0)
    , m_pwndOwner(pwndOwner)
{
    CIDAssert(m_pwndOwner != 0, L"The owner window cannot be null");
}

TCQCIntfWView::TCQCIntfWView(const  TCQCIntfTemplate&       iwdgTemplate
                            ,       TCQCIntfViewWnd* const  pwndOwner
                            , const TString&                strTitle
                            ,       MCQCIntfAppHandler* const pmiahToUse
                            , const tCIDLib::TFloat8        f8Lat
                            , const tCIDLib::TFloat8        f8Long
                            , const TCQCUserCtx&            cuctxToUse) :

    TCQCIntfStdView
    (
        strTitle
        , iwdgTemplate
        , pmiahToUse
        , pwndOwner->rgbBgnFill()
        , f8Lat
        , f8Long
        , kCIDLib::True
        , cuctxToUse
    )
    , m_pgdevCompat(nullptr)
    , m_pwndOwner(pwndOwner)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window cannot be null");
}

TCQCIntfWView::~TCQCIntfWView()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfWView: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Return the full possible area of the monitor on which the view is currently
//  displayed.
//
TArea TCQCIntfWView::areaDisplayRes() const
{
    CIDAssert(m_pwndOwner != nullptr, L"The view's owner window is not set");

    TArea areaRet;
    facCIDCtrls().QueryMonArea(*m_pwndOwner, areaRet, kCIDLib::False);
    return areaRet;
}


//
//  Return the current area of the actual view area, which for us is the
//  area of the window, at the origin.
//
TArea TCQCIntfWView::areaView() const
{
    CIDAssert(m_pwndOwner != 0, L"The owner window has not been set on the view");
    return m_pwndOwner->areaWndSize();
}


// Returns a graphic device for the engine to use
tCQCIntfEng::TGraphIntfDev TCQCIntfWView::cptrNewGraphDev() const
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    return tCQCIntfEng::TGraphIntfDev(new TGraphWndDev(*m_pwndOwner));
}


// We override this to handle any cleanup. Calling our parent class last
tCIDLib::TVoid TCQCIntfWView::Destroyed()
{
    // Clean up the compatible device
    if (m_pgdevCompat)
    {
        delete m_pgdevCompat;
        m_pgdevCompat = 0;
    }

    // And call our parent now
    TParent::Destroyed();
}


//
//  We override this to get callbacks from the command engine that runs
//  interface based actions. In our case, we pass along the info we get
//  to our owning window.
//
tCIDLib::TVoid TCQCIntfWView::DoActEvent(tCQCIntfEng::TIntfCmdEv& iceToDo)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");

    //
    //  This guy handles any exceptions, which is VERY important. We cannot
    //  allow any exceptions or we'll never get back again to the background
    //  thread that called us here.
    //
    m_pwndOwner->ProcessActEvent(iceToDo);
}


tCIDLib::TVoid
TCQCIntfWView::DoModalLoop(         tCIDLib::TBoolean&  bBreakFlag
                            , const tCIDLib::TBoolean   bNoEscape)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");

    //
    //  They can set a flag (via action command) on the parent view class that
    //  tells us to supress mouse input to child windows.
    //
    tCIDCtrls::EModalFlags eModalFlags = tCIDCtrls::EModalFlags::None;
    if (bNoChildMouse())
    {
        eModalFlags = tCIDLib::eOREnumBits
        (
            eModalFlags, tCIDCtrls::EModalFlags::NoChildMouse
        );
    }

    // If exit through escape is not disallowed, set that flag
    if (!bNoEscape)
    {
        eModalFlags = tCIDLib::eOREnumBits
        (
            eModalFlags, tCIDCtrls::EModalFlags::BreakOnEsc
        );
    }

    // We just call the standard modal loop on our owner window
    m_pwndOwner->eDoModalLoop(bBreakFlag, eModalFlags, nullptr, kCIDLib::c4MaxCard);
}


//
//  We override this to do some extra processing for some commands.
//
tCQCKit::ECmdRes
TCQCIntfWView::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4Index
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    // Pass it on to our parent
    const tCQCKit::ECmdRes eRes = TParent::eDoCmd
    (
        ccfgToDo, c4Index, strUserId, strEventId, ctarGlobals, bResult, acteTar
    );

    // If it's the fill border color command then do a full redraw
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetViewBorderClr)
    {
        ForceRedraw();
    }

    return eRes;
}


// Just ask our window to fully redraw
tCIDLib::TVoid TCQCIntfWView::ForceRedraw()
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    m_pwndOwner->ForceRepaint();
}


const TGraphDrawDev& TCQCIntfWView::gdevCompat() const
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");

    // Create our compatible device if not done yet
    if (!m_pgdevCompat)
        m_pgdevCompat = new TGraphWndDev(*m_pwndOwner);

    return *m_pgdevCompat;
}


tCIDLib::TVoid TCQCIntfWView::InvalidateArea(const TArea& areaToRedraw)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    m_pwndOwner->InvalidateArea(areaToRedraw);
}


tCIDLib::TVoid TCQCIntfWView::PauseTimers(const tCIDLib::TBoolean bPauseState)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");

    // Pass it on to our owning window
    m_pwndOwner->PauseTimers(bPauseState);
}


tCIDLib::TVoid TCQCIntfWView::PlayWAV(  const   TString&            strToPlay
                                        , const tCIDLib::EWaitModes eWait)
{
    TAudio::PlayWAVFile(strToPlay, eWait);
}


tCIDLib::TVoid TCQCIntfWView::Redraw()
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    m_pwndOwner->Redraw(m_pwndOwner->areaWnd(), tCIDCtrls::ERedrawFlags::Immediate);
}


tCIDLib::TVoid TCQCIntfWView::Redraw(const TArea& areaToRedraw)
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    m_pwndOwner->Redraw(areaToRedraw, tCIDCtrls::ERedrawFlags::Immediate);
}

tCIDLib::TVoid TCQCIntfWView::Redraw(const TGUIRegion& grgnToRedraw)

{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    m_pwndOwner->Redraw(grgnToRedraw, tCIDCtrls::ERedrawFlags::Immediate);
}


tCIDLib::TVoid
TCQCIntfWView::ShowErr( const   TString&    strTitle
                        , const TString&    strText
                        , const TError&     errToShow)
{
    //
    //  Just pass it on to a more generl error shower that will look for certain
    //  common errors and show them as a simple msg box, else it will show the
    //  general error dialog.
    //
    facCQCGKit().ShowError(*m_pwndOwner, strTitle, strText, errToShow);
}


tCIDLib::TVoid
TCQCIntfWView::ShowMsg( const   TString&            strTitle
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

    if (bIsError)
    {
        TErrBox msgbErr(strTitle, strMsg);
        msgbErr.ShowIt(*m_pwndOwner);
    }
     else
    {
        TOkBox msgbOk(strTitle, strMsg);
        msgbOk.ShowIt(*m_pwndOwner);
    }
}


tCIDLib::TVoid TCQCIntfWView::TakeFocus()
{
    m_pwndOwner->TakeFocus();
}


const TWindow& TCQCIntfWView::wndOwner() const
{
    return *m_pwndOwner;
}

TWindow& TCQCIntfWView::wndOwner()
{
    return *m_pwndOwner;
}



// ---------------------------------------------------------------------------
//  TCQCIntfWView: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Returns our type specific owner window, as opposed to the generic one
//  we inherit from the base view class above.
//
const TCQCIntfViewWnd& TCQCIntfWView::wndOwner2() const
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    return *m_pwndOwner;
}

TCQCIntfViewWnd& TCQCIntfWView::wndOwner2()
{
    CIDAssert(m_pwndOwner != nullptr, L"The owner window has not been set on the view");
    return *m_pwndOwner;
}


