//
// FILE NAME: CQCVoice_VisTextWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/09/2017
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
//  Implements the visual text window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVoiceVisTextWnd, TWindow)
RTTIDecls(TCQCVoiceVisTextFrame, TFrameWnd)



// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceVisTextWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCVoiceVisTextWnd::TCQCVoiceVisTextWnd() :

    TWindow(kCIDLib::False)
    , m_bSpeaking(kCIDLib::False)
    , m_rgbSpeaking(0xFF, 0x77, 0x77)
{
}

TCQCVoiceVisTextWnd::~TCQCVoiceVisTextWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Remove any existing text
tCIDLib::TVoid TCQCVoiceVisTextWnd::ClearText()
{
    m_strText.Clear();
    InvalidateArea(m_areaText, kCIDLib::True);
}


// Create ourself
tCIDLib::TVoid
TCQCVoiceVisTextWnd::Create(const TWindow& wndParent, const TArea& areaInit)
{
    // Call our parent window to do the creation
    TParent::CreateWnd
    (
        wndParent.hwndThis()
        , kCIDCtrls::pszCustClassHV
        , kCIDLib::pszEmptyZStr
        , areaInit
        , tCIDCtrls::EWndStyles::VisChild
        , tCIDCtrls::EExWndStyles::None
        , kCIDCtrls::widFirstCtrl
    );
}


//
//  We need to set store the new info and redraw. We just force redraws, so that the
//  drawing happens in the GUI thread. This is called from the BT thread.
//
tCIDLib::TVoid
TCQCVoiceVisTextWnd::ShowText( const   TString&            strMsg
                                , const tCIDLib::TBoolean   bWaitForSpoken)
{
    // If the speaking indicator state is different, then store and redraw
    if (m_bSpeaking != bWaitForSpoken)
    {
        m_bSpeaking = bWaitForSpoken;
        InvalidateArea(m_areaSpeaking);
    }

    // Now store the text and redraw the text area
    m_strText = strMsg;
    InvalidateArea(m_areaText, kCIDLib::True);
}


tCIDLib::TVoid TCQCVoiceVisTextWnd::ReadyToListen()
{
    // We are longer speaking
    m_bSpeaking = kCIDLib::False;
    InvalidateArea(m_areaSpeaking);
}


// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// We override this to handle re-calculating our display areas
tCIDLib::TVoid
TCQCVoiceVisTextWnd::AreaChanged(  const   TArea&                  areaPrev
                                    , const TArea&                  areaNew
                                    , const tCIDCtrls::EPosStates   ePosState
                                    , const tCIDLib::TBoolean       bOrgChanged
                                    , const tCIDLib::TBoolean       bSizeChanged
                                    , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged
    (
        areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged
    );

    // Recalculate our areas based on our new size
    CalcAreas(areaNew);
}


//
//  We need to create our dialog resource driven contents and load up
//  our tabs.
//
tCIDLib::TBoolean TCQCVoiceVisTextWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Do an initial area calculation
    CalcAreas(areaWndSize());

    // Set up our font that we will use, starting with our standard bold one
    TGraphWndDev gdevTmp(*this);
    TFontSelAttrs fselToUse = TGUIFacility::gfontDefBold().fselCurrent();

    //
    //  Now size it up considerably.
    //
    fselToUse.SetHeightAsPoints(gdevTmp, 36);
    m_gfontText.SetSelAttrs(fselToUse);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCVoiceVisTextWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    // If the area intersects the text and we have any, draw it
    if (!m_strText.bIsEmpty() && areaUpdate.bIntersects(m_areaText))
    {
        TFontJanitor janFont(&gdevToUse, &m_gfontText);
        gdevToUse.DrawMText
        (
            m_strText
            , m_areaText
            , tCIDLib::EHJustify::Center
            , tCIDLib::EVJustify::Center
        );
    }

    // If the area intersects the speaking indicator, draw it
    if (areaUpdate.bIntersects(m_areaSpeaking))
        DrawSpeaking(gdevToUse, areaUpdate);

    return kCIDLib::True;
}


// We stop our timer if its running
tCIDLib::TVoid TCQCVoiceVisTextWnd::Destroyed()
{
    // Nothing to do currently

    TParent::Destroyed();
}


// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We set a minimum tracking size that will prevent us from getting small enough to
//  cause any weirdness nere.
//
tCIDLib::TVoid TCQCVoiceVisTextWnd::CalcAreas(const TArea& areaSrc)
{
    //
    //  Take the area, which should be the available display area, and move it in
    //  to leave some margin.
    //
    TArea areaAvail(areaSrc);
    areaAvail.Deflate(48, 24);

    //
    //  Now split that into an area the bottom for the indicator and an area above
    //  that for the text, and then deflate them in a bit each.
    //
    m_areaText = areaAvail;
    m_areaText.AdjustSize(0, -64);

    m_areaSpeaking = areaAvail;
    m_areaSpeaking.i4Top(m_areaText.i4Bottom());
    m_areaSpeaking.c4Height(64);

    m_areaText.Deflate(0, 24);
    m_areaSpeaking.Deflate(48, 8);
}


//
//  This will update the speaking indocator based on its current state.
//
tCIDLib::TVoid
TCQCVoiceVisTextWnd::DrawSpeaking(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    // Get the actual area we need to update
    TArea areaRedraw(areaUpdate & m_areaSpeaking);

    // Shouldn't be called if no overlap, but just in case
    if (areaRedraw.bIsEmpty())
        return;

    //
    //  If it's off right now, then fill with the background, else fill with
    //  the indicator color.
    //
    if (m_bSpeaking)
        gdevToUse.Fill(areaRedraw, m_rgbSpeaking);
    else
        gdevToUse.Fill(areaRedraw, rgbBgnFill());
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceVisTextFrame
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextFrame: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCVoiceVisTextFrame::TCQCVoiceVisTextFrame() :

    m_pwndClient(nullptr)
{
}

TCQCVoiceVisTextFrame::~TCQCVoiceVisTextFrame()
{
}


// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextFrame: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Pass it on to our client window if created
tCIDLib::TVoid TCQCVoiceVisTextFrame::ClearText()
{
    if (m_pwndClient)
        m_pwndClient->ClearText();
}


// Create ourself
tCIDLib::TVoid TCQCVoiceVisTextFrame::Create()
{
    TParent::CreateFrame
    (
        nullptr
        , facCIDCtrls().areaDefWnd()
        , facCQCVoice.strMsg(kCQCVoiceMsgs::midVisText_Title)
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );
}


// Pass it on to our client if it exists
tCIDLib::TVoid
TCQCVoiceVisTextFrame::ShowText(const  TString&            strMsg
                                , const tCIDLib::TBoolean   bWaitForSpoken)
{
    if (m_pwndClient)
        m_pwndClient->ShowText(strMsg, bWaitForSpoken);
}


// Pass it on to our client if it exists
tCIDLib::TVoid TCQCVoiceVisTextFrame::ReadyToListen()
{
    if (m_pwndClient)
        m_pwndClient->ReadyToListen();
}


// ---------------------------------------------------------------------------
//  TCQCVoiceVisTextFrame: Protected, inherited methods
// ---------------------------------------------------------------------------

// We need to keep our client window sized to fit
tCIDLib::TVoid
TCQCVoiceVisTextFrame::AreaChanged(const   TArea&                  areaPrev
                                    , const TArea&                  areaNew
                                    , const tCIDCtrls::EPosStates   ePosState
                                    , const tCIDLib::TBoolean       bOrgChanged
                                    , const tCIDLib::TBoolean       bSizeChanged
                                    , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (m_pwndClient)
        m_pwndClient->SetSizePos(areaClient(), kCIDLib::True);
}


//
//  We override this and instead of closing, we just minimize ourself. Otherwise
//  this would close the application since this is a top level frame. We return false
//  to say don't shutdown.
//
tCIDLib::TBoolean TCQCVoiceVisTextFrame::bAllowShutdown()
{
    Minimize();
    return kCIDLib::False;
}


//
//  We create the client window and set it as our client
//
tCIDLib::TBoolean TCQCVoiceVisTextFrame::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Create it as a child of us, and sized initially to our client area
    m_pwndClient = new TCQCVoiceVisTextWnd();
    m_pwndClient->Create(*this, areaClient());

    // Set our icon to the program icon
    facCQCVoice.SetFrameIcon(*this, L"CQCVoice");

    // Set min/max tracking sizes
    SetMinMaxSize(TSize(640, 480), TWindow::wndDesktop().areaWndSize().szArea());

    return kCIDLib::True;
}
