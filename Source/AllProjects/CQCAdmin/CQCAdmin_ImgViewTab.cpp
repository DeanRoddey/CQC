//
// FILE NAME: CQCAdmin_ImgViewTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/09/2015
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
//  This file implements a tab for read only viewing of text based content.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_ImgViewTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TImgViewTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TImgViewTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TImgViewTab: Constructors and destructor
// ---------------------------------------------------------------------------
TImgViewTab::TImgViewTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Image", kCIDLib::False)
    , m_pwndScroll(nullptr)
    , m_pwndImage(nullptr)
{
}

TImgViewTab::~TImgViewTab()
{
}


// ---------------------------------------------------------------------------
//  TImgViewTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TImgViewTab::CreateTab(         TTabbedWnd& wndParent
                        , const TString&    strTabText
                        , const TBitmap&    bmpToDisplay)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);

    //
    //  Set the image on the image window and size it up to fit, via the scroll
    //  windwo so it knows and will adjust its scroll bars.
    //
    m_pwndImage->SetBitmap(bmpToDisplay);
    m_pwndScroll->SetChildSize(bmpToDisplay.szBitmap());

    // Now we can show the image window
    m_pwndImage->SetVisibility(kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TImgViewTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TImgViewTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TTabWindow::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our list sized to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndScroll)
        m_pwndScroll->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TImgViewTab::bCreated()
{
    TParent::bCreated();

    // Create the scroller window
    m_pwndScroll = new TScrollArea();
    m_pwndScroll->Create
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaClient()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EScrAStyles::None
    );
    m_pwndScroll->c4Margin(8);

    // Create the image window
    m_pwndImage = new TStaticImg();
    m_pwndImage->CreateStaticImg
    (
        *m_pwndScroll
        , kCIDCtrls::widFirstCtrl + 1
        , TArea(0, 0, 8, 8)
        , tCIDGraphDev::EPlacement::UpperLeft
        , tCIDCtrls::EWndStyles::VisChild
    );

    // Tell the scroll area to treat our image window as it's scrollable child
    m_pwndScroll->SetChild(m_pwndImage);

    // Set them both to the standard window background
    m_pwndScroll->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));
    m_pwndImage->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    return kCIDLib::True;
}


tCIDLib::TVoid TImgViewTab::Destroyed()
{
    //
    //  Delete the scroll window if we created it. It will destroy the image
    //  window.
    //
    if (m_pwndScroll)
    {
        if (m_pwndScroll->bIsValid())
            m_pwndScroll->Destroy();

        delete m_pwndScroll;
        m_pwndScroll= nullptr;
    }

    TParent::Destroyed();
}


