//
// FILE NAME: CQCInst_InfoPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This file implements the base information panel class. There are a set
//  of classes derived from this class that handle getting various pieces of
//  installation information.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstInfoPanel,TGenericWnd)


// ---------------------------------------------------------------------------
//   CLASS: TInstInfoPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstInfoPanel: Public Destructor
// ---------------------------------------------------------------------------
TInstInfoPanel::~TInstInfoPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstInfoPanel: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Some panels should only be displayed in certain circumstances. This gives them a
//  chance to tell the main window whether they should be shown or not. This default
//  implementation just returns true.
//
tCIDLib::TBoolean TInstInfoPanel::bPanelIsVisible() const
{
    return kCIDLib::True;
}


// Some panels don't need any validation, so we provide a default
tCIDLib::TBoolean TInstInfoPanel::bValidate(tCIDCtrls::TWndId&, TString&)
{
    return kCIDLib::True;
}

// When we are about to make this one the active one
tCIDLib::TVoid TInstInfoPanel::Entered()
{
}


//
//  Lets them know that another panel has changed state, so that they can react to
//  it. Most don't need to so we provide a default.
//
tCIDLib::TVoid TInstInfoPanel::ReactToChanges()
{
}


// If not overridden, we say allow both next or previous
tCQCInst::ENavFlags TInstInfoPanel::eNavFlags() const
{
    return tCQCInst::ENavFlags::AllowAll;
}


//
//  Very few panels provide this one, so we provide default that says just stay on the
//  panel and wait for user input.
//
tCQCInst::EProcRes TInstInfoPanel::eProcess()
{
    return tCQCInst::EProcRes::Stay;
}


// Many panels use no ports, so the default just does nothing
tCIDLib::TVoid TInstInfoPanel::ReportPortUsage(tCQCInst::TPortCntList&)
{
}


// Various panels have no content to save so we provide a default
tCIDLib::TVoid TInstInfoPanel::SaveContents()
{
}


// ---------------------------------------------------------------------------
//  TInstInfoPanel: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstInfoPanel::bValidated() const
{
    return m_bValidated;
}

tCIDLib::TBoolean TInstInfoPanel::bValidated(const tCIDLib::TBoolean bToSet)
{
    m_bValidated = bToSet;
    return m_bValidated;
}


tCIDLib::TVoid
TInstInfoPanel::CreateInfoPanel(const   TWindow&            wndInitParent
                                , const TArea&              areaContent
                                , const tCIDCtrls::TWndId   widToUse)
{
    CreateGenWnd
    (
        wndInitParent
        , areaContent
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , widToUse
    );

    //
    //  Set this panel's name as our text. It won't be visible but it insures that any
    //  popups invoked by the panel gets this as the default title text.
    //
    strWndText(m_strName);
}


const TString& TInstInfoPanel::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TInstInfoPanel: Hidden constructors
// ---------------------------------------------------------------------------
TInstInfoPanel::TInstInfoPanel( const   TString&                strName
                                ,       TCQCInstallInfo* const  pinfoCur
                                ,       TMainFrameWnd* const    pwndParent) :

    m_bValidated(kCIDLib::False)
    , m_pinfoCur(pinfoCur)
    , m_pwndParent(pwndParent)
    , m_strName(strName)
{
}


// ---------------------------------------------------------------------------
//  TInstInfoPanel: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Make the install info available to our derivatives
const TCQCInstallInfo& TInstInfoPanel::infoCur() const
{
    return *m_pinfoCur;
}

TCQCInstallInfo& TInstInfoPanel::infoCur()
{
    return *m_pinfoCur;
}


// A helper for our derivatives to create dialog description driven controls
tCIDLib::TVoid TInstInfoPanel::LoadDlgItems(const tCIDLib::TResId ridToLoad)
{
    // Load the dialog description
    TDlgDesc dlgdChildren;
    facCQCInst.bCreateDlgDesc(ridToLoad, dlgdChildren);

    // And use it to create the dialog items
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);
}


// Make the main frame window available to our derivatives
const TMainFrameWnd& TInstInfoPanel::wndParent() const
{
    return *m_pwndParent;
}

TMainFrameWnd& TInstInfoPanel::wndParent()
{
    return *m_pwndParent;
}

