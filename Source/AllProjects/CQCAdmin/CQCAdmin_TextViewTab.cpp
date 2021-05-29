//
// FILE NAME: CQCAdmin_TextViewTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/03/2015
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


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTextViewTab, TContentTab)



namespace
{
    namespace CQCAdmin_TextViewTab
    {
        // Faux menu commands for use with our accellerator table
        constexpr tCIDLib::TResId ridAccel_GotoLine = 1000;
        constexpr tCIDLib::TResId ridAccel_Find     = 1001;
        constexpr tCIDLib::TResId ridAccel_FindNext = 1002;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TTextViewTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTextViewTab: Constructors and destructor
// ---------------------------------------------------------------------------
TTextViewTab::TTextViewTab( const   TString&    strPath
                            , const TString&    strRelPath
                            , const TString&    strType) :

    TContentTab(strPath, strRelPath, strType, kCIDLib::False)
    , m_bFullMatchOnly(kCIDLib::False)
    , m_pwndText(nullptr)
{
}

TTextViewTab::~TTextViewTab()
{
}


// ---------------------------------------------------------------------------
//  TTextViewTab: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TTextViewTab::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // Pass it to our accellerator table
    return m_accelTextView.bProcessAccel(*this, pMsgData);
}


// ---------------------------------------------------------------------------
//  TTextViewTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TTextViewTab::CreateTab(        TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TString&            strContText
                        , const tCIDLib::TBoolean   bIsCode)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);

    // Set up our accelerator table
    tCIDLib::TStrList colKeyStrs;
    tCIDLib::TCardList fcolCmds;

    colKeyStrs.objAdd(L"Ctrl-G");
    fcolCmds.c4AddElement(CQCAdmin_TextViewTab::ridAccel_GotoLine);

    colKeyStrs.objAdd(L"Ctrl-F");
    fcolCmds.c4AddElement(CQCAdmin_TextViewTab::ridAccel_Find);

    colKeyStrs.objAdd(L"F3");
    fcolCmds.c4AddElement(CQCAdmin_TextViewTab::ridAccel_FindNext);
    m_accelTextView.Create(colKeyStrs, fcolCmds);


    //
    //  If it's code style, then set a non-proportional font on the text window. We
    //  temporarily set the default font and use that to set up a new font. THis way
    //  we know we keep the same font size.
    //
    if (bIsCode)
    {
        TFontSelAttrs fselNew;
        {
            // Use a janitor to set the font long enough to get the selection attrs
            TGraphWndDev gdevTmp(*this);
            TFontJanitor janFont(&gdevTmp, &TGUIFacility::gfontDefNorm());
            fselNew = TFontSelAttrs(gdevTmp);
        }

        fselNew.bDevicePreferred(kCIDLib::True);
        fselNew.ePitch(tCIDGraphDev::EFontPitches::Fixed);
        fselNew.c4Height(12);
        fselNew.strFaceName(L"Courier New");

        m_pwndText->SetNewFont(fselNew);
    }

    // Now set the text on the text window and show it
    m_pwndText->strWndText(strContText);
    m_pwndText->SetVisibility(kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TTextViewTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TTextViewTab::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our text window sized to fit, but with some margin
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndText)
    {
        TArea areaText(areaClient());
        areaText.Deflate(0, 4);
        m_pwndText->SetSizePos(areaText, kCIDLib::False);
    }
}


tCIDLib::TBoolean TTextViewTab::bCreated()
{
    TParent::bCreated();

    TArea areaInit = areaClient();
    areaInit.Deflate(0, 4);
    m_pwndText= new TMultiEdit();
    m_pwndText->Create
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaInit
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::Child
            , tCIDCtrls::EWndStyles::HScroll
            , tCIDCtrls::EWndStyles::VScroll
          )
        , tCIDCtrls::EMLEStyles::StdReadOnly
    );

    // Set us and the text to use the standard window background
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));
    m_pwndText->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Give the text window the focus
    m_pwndText->TakeFocus();

    return kCIDLib::True;
}


tCIDLib::TVoid TTextViewTab::Destroyed()
{
    if (m_pwndText)
    {
        try
        {
            m_pwndText->Destroy();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete m_pwndText;
        m_pwndText = nullptr;
    }

    TParent::Destroyed();
}


//
//  We don't have a menu but have an accel table, so we need to handle them via this
//  override.
//
tCIDLib::TVoid
TTextViewTab::MenuCommand(  const   tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == CQCAdmin_TextViewTab::ridAccel_GotoLine)
    {
        // Just call our helper that gets the line and does the move
        GotoLine();
    }
     else if ((ridItem == CQCAdmin_TextViewTab::ridAccel_Find)
          ||  (ridItem == CQCAdmin_TextViewTab::ridAccel_FindNext))
    {
        //
        //  If find, then get the search values. Else, if there is a search value set,
        //  just search again.
        //
        tCIDLib::TBoolean bRes;
        tCIDLib::TBoolean bFirst(ridItem == CQCAdmin_TextViewTab::ridAccel_Find);
        if (bFirst)
        {
            // We can't support selection based search, so we eat that
            tCIDLib::TBoolean bSelOnly = kCIDLib::False;
            bRes = facCIDWUtils().bGetSearchOpts
            (
                *this
                , m_strToFind
                , m_colSearchList
                , m_bFullMatchOnly
                , bSelOnly
                , tCIDWUtils::EFindFlags::None
            );
        }
         else
        {
            bRes = !m_strToFind.bIsEmpty();
        }

        if (bRes)
        {
            tCIDLib::TCard4 c4StartOfs;
            tCIDLib::TCard4 c4EndOfs;
            bRes = m_pwndText->bFindText
            (
                m_strToFind, c4StartOfs, c4EndOfs, m_bFullMatchOnly, bFirst
            );

            if (bRes)
            {
                m_pwndText->SelectRange(c4StartOfs, c4EndOfs);
            }
             else
            {
                TOkBox msgbDone(L"No more matches were found");
                msgbDone.ShowIt(*this);
            }
        }
    }
}



// ---------------------------------------------------------------------------
//  TTextViewTab: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TTextViewTab::GotoLine()
{
    TString strVal;
    tCIDLib::TBoolean bRes = facCIDWUtils().bListSelect2
    (
        *this
        , L"Go To Line"
        , m_colGotoList
        , strVal
        , tCIDWUtils::EListValFmts::NZCardinal
    );

    if (bRes)
    {
        // Don't let the list get crazy
        if (m_colGotoList.c4ElemCount() >= 32)
            m_colGotoList.bRemoveLast();

        tCIDLib::TCard4 c4Line;
        if (strVal.bToCard4(c4Line, tCIDLib::ERadices::Auto))
        {
            CIDAssert(c4Line != 0, L"The line number should not be zero");

            // Tell the multi-edit to go there (he takes 1 based lines)
            m_pwndText->GoToLine(c4Line);
        }
    }
}

