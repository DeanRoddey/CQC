//
// FILE NAME: CQCIntfEng_Toolbar.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/30/2005
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
//  This file implements the horizontal tool bar widget.
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
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_ToolBar.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfToolbar, TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfVarToolbar , TCQCIntfToolbar)
RTTIDecls(TCQCIntfToolbar::TTBButton,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ToolBar
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2 -
        //      Implemented the images interface, so that we can provide focus and
        //      pressed images. We also added the vertical adjustment value.
        //
        //  Version 3 -
        //      Add support for marking one of the buttons for special display
        //      emphasis. We added new RTVs and a new image. So we need to update
        //      the version in order to force these things into previously stored
        //      buttons.
        //
        //  Version 4 -
        //      No data change but we need to swap F2/F2 colors to make their
        //      usage consistent with other text color usages.
        //
        //  Version 5 -
        //      We added a new image, a background that can go behind all the buttons,
        //      so we need to add it when streaming in older versions. 4.4.908.
        //
        //  Version 6 -
        //      Added the 'fill slots' option. 4.4.909.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 6;


        // -----------------------------------------------------------------------
        //  And for the buttons themselves so they can change separately
        //
        //  Version 3 -
        //      Split off from the main format above at V3.
        //
        //  Version 4 -
        //      We got rid of on press/release, and move any action commands to
        //      OnClick only.
        //
        //  Version 5 -
        //      Convert paths to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ButtFmtVersion  = 5;


        // -----------------------------------------------------------------------
        //  For the variable mark driven derivative
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The key under which we store the images in the image list interface
        // -----------------------------------------------------------------------
        const TString strFocus(L"Focus");
        const TString strMarked(L"Marked");
        const TString strPressed(L"Pressed");
        const TString strBgn(L"Bgn");


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::TakesFocus
        );

        constexpr tCQCIntfEng::ECapFlags eVarCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfToolBarRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class.
// ---------------------------------------------------------------------------
class TCQCIntfToolBarRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfToolBarRTV( const   tCIDLib::TCard4 c4SelIndex
                            , const tCIDLib::TCard4 c4MarkIndex
                            , const tCIDLib::TCard4 c4FirstIndex
                            , const TString&        strItemText
                            , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_c4LeftIndex(c4FirstIndex)
            , m_c4MarkIndex(c4MarkIndex)
            , m_c4SelIndex(c4SelIndex)
            , m_strItemText(strItemText)
        {
        }

        ~TCQCIntfToolBarRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue( const   TString&    strId
                                    ,       TString&    strToFill)  const
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            // If it's one of ours, then handle it
            if (strId == kCQCKit::strRTVId_CurText)
            {
                strToFill = m_strItemText;
            }
             else if (strId == kCQCKit::strRTVId_FirstIndex)
            {
                strToFill.AppendFormatted(m_c4LeftIndex);
            }
             else if (strId == kCQCKit::strRTVId_MarkIndex)
            {
                // If none, just clear the value
                if (m_c4MarkIndex == kCIDLib::c4MaxCard)
                    strToFill.Clear();
                else
                    strToFill.AppendFormatted(m_c4MarkIndex);
            }
             else if (strId == kCQCKit::strRTVId_SelectIndex)
            {
                strToFill.AppendFormatted(m_c4SelIndex);
            }
             else
            {
                return kCIDLib::False;
            }
            return kCIDLib::True;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LeftIndex
        //      The index of the first button visible on the left.
        //
        //  m_c4MarkIndex
        //      The index of the currently marked button, which can be max
        //      card if there isn't a marked one.
        //
        //  m_c4SelIndex
        //      The index of the button that was clicked to invoke the
        //      action.
        //
        //  m_strItemText
        //      The text of the clicked item.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4LeftIndex;
        tCIDLib::TCard4 m_c4MarkIndex;
        tCIDLib::TCard4 m_c4SelIndex;
        TString         m_strItemText;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfToolbar::TBButton
//  PREFIX: tbb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfToolbar::TTBButton::TTBButton() :

    m_bPressed(kCIDLib::False)
    , m_c1Opacity(0xFF)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_piwdgParent(0)
{
    // Set up our events and RTVs via a private helper
    SetupCmdSrc();
}

TCQCIntfToolbar::TTBButton::TTBButton(  const   TString&                strText
                                        ,       TCQCIntfToolbar* const  piwdgParent) :
    m_bPressed(kCIDLib::False)
    , m_c1Opacity(0xFF)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_piwdgParent(piwdgParent)
    , m_strText(strText)
{
    // Set up our events and RTVs via a private helper
    SetupCmdSrc();
}

TCQCIntfToolbar::TTBButton::TTBButton(const TTBButton& tbbSrc) :

    MCQCCmdSrcIntf(tbbSrc)
    , m_bPressed(tbbSrc.m_bPressed)
    , m_c1Opacity(tbbSrc.m_c1Opacity)
    , m_cptrImg(tbbSrc.m_cptrImg)
    , m_piwdgParent(tbbSrc.m_piwdgParent)
    , m_strImagePath(tbbSrc.m_strImagePath)
    , m_strText(tbbSrc.m_strText)
    , m_szImage(tbbSrc.m_szImage)
{
}

TCQCIntfToolbar::TTBButton::~TTBButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::operator=(const TTBButton& tbbSrc)
{
    if (this != &tbbSrc)
    {
        MCQCCmdSrcIntf::operator=(tbbSrc);
        m_bPressed      = tbbSrc.m_bPressed;
        m_c1Opacity     = tbbSrc.m_c1Opacity;
        m_cptrImg       = tbbSrc.m_cptrImg;
        m_piwdgParent   = tbbSrc.m_piwdgParent;
        m_strImagePath  = tbbSrc.m_strImagePath;
        m_strText       = tbbSrc.m_strText;
        m_szImage       = tbbSrc.m_szImage;
    }
}


tCIDLib::TBoolean
TCQCIntfToolbar::TTBButton::operator==(const TTBButton& tbbSrc) const
{
    if (this == &tbbSrc)
        return kCIDLib::True;

    if (!MCQCCmdSrcIntf::bSameCmds(tbbSrc))
        return kCIDLib::False;

    return
    (
        (m_c1Opacity        == tbbSrc.m_c1Opacity)
        && (m_strImagePath  == tbbSrc.m_strImagePath)
        && (m_strText       == tbbSrc.m_strText)
    );
}

tCIDLib::TBoolean
TCQCIntfToolbar::TTBButton::operator!=(const TTBButton& tbbSrc) const
{
    return !operator==(tbbSrc);
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Gen up one of our runtime value objects. In this case we are really
//  passing info from our containing tool bar, but we are the actual command
//  source and therefore the one to be called.
//
TCQCCmdRTVSrc*
TCQCIntfToolbar::TTBButton::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    return m_piwdgParent->pcrtsMakeNew(cuctxToUse);
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfToolbar::TTBButton::bClrTrans() const
{
    return m_cptrImg->bTransparent();
}


tCIDLib::TBoolean TCQCIntfToolbar::TTBButton::bPressed() const
{
    return m_bPressed;
}

tCIDLib::TBoolean
TCQCIntfToolbar::TTBButton::bPressed(const tCIDLib::TBoolean bToSet)
{
    m_bPressed = bToSet;
    return m_bPressed;
}


tCIDLib::TCard1 TCQCIntfToolbar::TTBButton::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1
TCQCIntfToolbar::TTBButton::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


tCIDLib::TVoid TCQCIntfToolbar::TTBButton::ClearImage()
{
    m_strImagePath.Clear();
    facCQCIntfEng().DropImgRef(m_cptrImg);
}


tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::DrawOn(         TGraphDrawDev&  gdevToUse
                                    , const TArea&          areaTar)
{
    // If no image, then do nothing
    if (m_strImagePath.bIsEmpty())
        return;

    const TArea areaSrc(TPoint::pntOrigin, m_cptrImg->szImage());
    m_cptrImg->bmpImage().AdvDrawOn
    (
        gdevToUse
        , areaSrc
        , areaTar
        , m_cptrImg->bTransparent()
        , m_cptrImg->c4TransClr()
        , m_cptrImg->bmpMask()
        , tCIDGraphDev::EBltTypes::Clip
        , m_c1Opacity
        , !TFacCQCIntfEng::bRemoteMode()
    );
}


tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::LoadImage(const TCQCIntfView& civOwner)
{
    try
    {
        TDataSrvClient dsclLoad;
        LoadImage(civOwner, dsclLoad);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::LoadImage(  const   TCQCIntfView&   civOwner
                                        ,       TDataSrvClient& dsclInit)
{
    if (m_strImagePath.bIsEmpty())
    {
        facCQCIntfEng().DropImgRef(m_cptrImg);
        m_szImage.Zero();
    }
     else
    {
        try
        {
            // Try to load the new image
            m_cptrImg = facCQCIntfEng().cptrGetImage(m_strImagePath, civOwner, dsclInit);
            m_szImage = m_cptrImg->szImage();
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


const TString& TCQCIntfToolbar::TTBButton::strImagePath() const
{
    return m_strImagePath;
}

const TString& TCQCIntfToolbar::TTBButton::strImagePath(const TString& strToSet)
{
    m_strImagePath = strToSet;
    return m_strImagePath;
}


const TString& TCQCIntfToolbar::TTBButton::strText() const
{
    return m_strText;
}

const TString& TCQCIntfToolbar::TTBButton::strText(const TString& strToSet)
{
    m_strText = strToSet;
    return m_strText;
}


const TSize& TCQCIntfToolbar::TTBButton::szImage() const
{
    return m_szImage;
}


//
//  Upon straeming us in, our parent tool bar tells us about himself. For
//  newly created ones, it's done when it's created.
//
tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::SetParent(TCQCIntfToolbar* const piwdgParent)
{
    m_piwdgParent = piwdgParent;
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ToolBar::c2ButtFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Do our mixins
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // And then our stuff
    strmToReadFrom  >> m_strImagePath
                    >> m_c1Opacity
                    >> m_strText;

    // Convert the path to 5.x format
    if (c2FmtVersion < 5)
        facCQCKit().Make50Path(m_strImagePath);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Initalize runtime stuff
    m_bPressed = kCIDLib::False;
    m_piwdgParent = 0;
}


tCIDLib::TVoid
TCQCIntfToolbar::TTBButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_ToolBar::c2ButtFmtVersion;

    // Do our mixins
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Do our stuff and end marker
    strmToWriteTo   << m_strImagePath
                    << m_c1Opacity
                    << m_strText
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar::TBButton: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfToolbar::TTBButton::SetupCmdSrc()
{
    // Add the events we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnClick)
        , kCQCKit::strEvId_OnClick
        , tCQCKit::EActCmdCtx::Standard
    );

    // We have some RTVs as of 4.0.17
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_FirstIndex)
        , kCQCKit::strRTVId_FirstIndex
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_MarkedIndex)
        , kCQCKit::strRTVId_MarkIndex
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
        , kCQCKit::strRTVId_SelectIndex
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_CurText)
        , kCQCKit::strRTVId_CurText
        , tCQCKit::ECmdPTypes::Text
    );
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfToolbar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfToolbar::TCQCIntfToolbar() :

    TCQCIntfWidget
    (
        CQCIntfEng_ToolBar::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Toolbar)
    )
    , MCQCIntfFontIntf()
    , m_bFillSlots(kCIDLib::False)
    , m_bSafePanBgn(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4MarkedIndex(kCIDLib::c4MaxCard)
    , m_c4SelectedIndex(kCIDLib::c4MaxCard)
    , m_c4Spacing(16)
    , m_c4Size(72)
    , m_eLayout(tCQCIntfEng::ETBLayouts::TextBelow)
    , m_i4VertAdjust(0)
{
    // Load up the pressed, focus keys into the image list mixin
    AddNewImgKey(CQCIntfEng_ToolBar::strFocus, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_ToolBar::strPressed, tCQCIntfEng::EImgReqFlags::Optional);

    // As of 4.0.17 we added one for the marked item
    AddNewImgKey(CQCIntfEng_ToolBar::strMarked, tCQCIntfEng::EImgReqFlags::Optional);

    // As of 4.4.908 we added an optional background image
    AddNewImgKey(CQCIntfEng_ToolBar::strBgn, tCQCIntfEng::EImgReqFlags::Optional);

    //
    //  Set our default look. The text is all we can really do, since the rest is
    //  so variable depending on what they want to do.
    //
    bBold(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    m_c4Spacing = 9;
    m_c4Size = 76;

    // Add a default button as a guide
    TTBButton tbbSample(L"Sample", this);
    tbbSample.strImagePath(L"/System/Hardware/Icons/Controller");
    AddButton(tbbSample);

    // Set the action command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Toolbar");
}

TCQCIntfToolbar::TCQCIntfToolbar(const  tCQCIntfEng::ECapFlags  eCaps
                                , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
    , MCQCIntfFontIntf()
    , m_bFillSlots(kCIDLib::False)
    , m_bSafePanBgn(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4MarkedIndex(kCIDLib::c4MaxCard)
    , m_c4SelectedIndex(kCIDLib::c4MaxCard)
    , m_c4Spacing(16)
    , m_c4Size(72)
    , m_eLayout(tCQCIntfEng::ETBLayouts::TextBelow)
    , m_i4VertAdjust(0)
{
    // Load up the pressed, focus keys into the image list mixin
    AddNewImgKey(CQCIntfEng_ToolBar::strFocus, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_ToolBar::strPressed, tCQCIntfEng::EImgReqFlags::Optional);

    // As of 4.0.17 we added one for the marked item
    AddNewImgKey(CQCIntfEng_ToolBar::strMarked, tCQCIntfEng::EImgReqFlags::Optional);

    // As of 4.4.908 we added an optional background image
    AddNewImgKey(CQCIntfEng_ToolBar::strBgn, tCQCIntfEng::EImgReqFlags::Optional);

    //
    //  Set our default look. The text is all we can really do, since the rest is
    //  so variable depending on what they want to do.
    //
    bBold(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    m_c4Spacing = 9;
    m_c4Size = 76;

    // Add a default button as a guide
    TTBButton tbbSample(L"Sample", this);
    tbbSample.strImagePath(L"/System/Hardware/Icons/Controller");
    AddButton(tbbSample);

    // Set the action command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Toolbar");
}

TCQCIntfToolbar::TCQCIntfToolbar(const TCQCIntfToolbar& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_bFillSlots(iwdgSrc.m_bFillSlots)
    , m_bSafePanBgn(iwdgSrc.m_bSafePanBgn)
    , m_c4Spacing(iwdgSrc.m_c4Spacing)
    , m_c4Size(iwdgSrc.m_c4Size)
    , m_colButtons(iwdgSrc.m_colButtons)
    , m_eLayout(iwdgSrc.m_eLayout)
    , m_i4VertAdjust(iwdgSrc.m_i4VertAdjust)

    //
    //  Only really needed at viewing time but we maintain design time
    //  info display by copying them
    //
    , m_areaAdjDisplay(iwdgSrc.m_areaAdjDisplay)
    , m_areaContent(iwdgSrc.m_areaContent)
    , m_areaDisplay(iwdgSrc.m_areaDisplay)
    , m_c4FocusIndex(iwdgSrc.m_c4FocusIndex)
    , m_c4MarkedIndex(iwdgSrc.m_c4MarkedIndex)
    , m_c4SelectedIndex(iwdgSrc.m_c4SelectedIndex)
{
}

TCQCIntfToolbar::~TCQCIntfToolbar()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Public operators
// ---------------------------------------------------------------------------
TCQCIntfToolbar&
TCQCIntfToolbar::operator=(const TCQCIntfToolbar& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);

        m_bFillSlots        = iwdgSrc.m_bFillSlots;
        m_bSafePanBgn       = iwdgSrc.m_bSafePanBgn;
        m_c4Spacing         = iwdgSrc.m_c4Spacing;
        m_c4Size            = iwdgSrc.m_c4Size;
        m_colButtons        = iwdgSrc.m_colButtons;
        m_eLayout           = iwdgSrc.m_eLayout;
        m_i4VertAdjust      = iwdgSrc.m_i4VertAdjust;

        //
        //  Not really needed except at viewing time, but we want to
        //  copy them so that we don't lose fake data set up for design
        //  time.
        //
        m_areaAdjDisplay    = iwdgSrc.m_areaAdjDisplay;
        m_areaContent       = iwdgSrc.m_areaContent;
        m_areaDisplay       = iwdgSrc.m_areaDisplay;
        m_c4FocusIndex      = iwdgSrc.m_c4FocusIndex;
        m_c4MarkedIndex     = iwdgSrc.m_c4MarkedIndex;
        m_c4SelectedIndex   = iwdgSrc.m_c4SelectedIndex;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfToolbar::bCanSizeTo() const
{
    // Our contents is driven by size, not the other way around
    return kCIDLib::False;
}


//
//  If we are already at the boundary in the direction indicated, then we
//  return true to allow the focus to move off us. Else, we just move it
//  to the next slot in that direction and redraw the old and new ones.
//
tCIDLib::TBoolean TCQCIntfToolbar::bMoveFocus(const tCQCKit::EScrNavOps eOp)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    // We special case these, which we translate into scroll operations
    if ((eOp == tCQCKit::EScrNavOps::End)
    ||  (eOp == tCQCKit::EScrNavOps::Home)
    ||  (eOp == tCQCKit::EScrNavOps::PrevPage)
    ||  (eOp == tCQCKit::EScrNavOps::NextPage))
    {
        //
        //  Call the list scrolling helper and translate our op into the
        //  action command parameter that it expects.
        //
        if (eOp == tCQCKit::EScrNavOps::End)
            bRet = bScrollList(L"End", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::Home)
            bRet = bScrollList(L"Home", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::PrevPage)
            bRet = bScrollList(L"Previous", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::NextPage)
            bRet = bScrollList(L"Next", kCIDLib::True);

        // Indicate we kept the focus internal
        return kCIDLib::False;
    }

    // It's not one of those, so it's a focus movement op
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();

    tCIDLib::TCard4 c4NewFocus = m_c4FocusIndex;

    switch(eOp)
    {
        case tCQCKit::EScrNavOps::Up :
        case tCQCKit::EScrNavOps::Left :
            if (c4NewFocus)
                c4NewFocus--;
            break;

        case tCQCKit::EScrNavOps::Down :
        case tCQCKit::EScrNavOps::Right :
            if (c4NewFocus + 1 < c4Count)
                c4NewFocus++;
            break;

        default :
            break;
    };

    // If changed and it's visible enough, take it
    if ((c4NewFocus != m_c4FocusIndex) && bVisibleEnough(c4NewFocus, 0.4F))
    {
        TArea areaNew;
        AreaForIndex(c4NewFocus, areaNew);
        TArea areaUpdate;
        AreaForIndex(m_c4FocusIndex, areaUpdate);
        areaUpdate |= areaNew;

        m_c4FocusIndex = c4NewFocus;
        bRet = kCIDLib::False;

        Redraw(areaUpdate);
    }
    return bRet;
}


//
//  We only allow horizontal panning with inertial, single finger. If in remote mode
//  or no inertia is set on us, we do flicks instead.
//
//  If two fingered or not in our direction, we let it progagate.
//
tCIDLib::TBoolean
TCQCIntfToolbar::bPerGestOpts(  const   TPoint&             pntAt
                                , const tCIDLib::EDirs      eDir
                                , const tCIDLib::TBoolean   bTwoFingers
                                ,       tCIDCtrls::EGestOpts& eToSet
                                ,       tCIDLib::TFloat4&   f4VScale)
{
    if (bTwoFingers
    ||  (eDir == tCIDLib::EDirs::Up)
    ||  (eDir == tCIDLib::EDirs::Down))
    {
        // Let it propogate
        return kCIDLib::True;
    }

    if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
        eToSet= tCIDCtrls::EGestOpts::Flick;
    else
        eToSet= tCIDCtrls::EGestOpts::PanInertia;

    if (m_c4Size < 48)
        f4VScale = 0.85F;
    else if (m_c4Size < 92)
        f4VScale = 0.90F;
    else if (m_c4Size < 148)
        f4VScale = 0.95F;
    else
        f4VScale = 1.0F;

    // No propogation
    return kCIDLib::False;
}



//
//  The gesture engine calls us here to handle drag gestures. WE have to
//  set up the scrollable content in memory images, and then move them
//  around as required during dragging and inertia.
//
tCIDLib::TBoolean
TCQCIntfToolbar::bProcessGestEv(const   tCIDCtrls::EGestEvs eEv
                                , const TPoint&             pntStart
                                , const TPoint&             pntAt
                                , const TPoint&             pntDelta
                                , const tCIDLib::TBoolean   bTwoFinger)
{
    tCIDLib::TBoolean bContinue = kCIDLib::True;

    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        //
        //  Create the bitmap for the scrollable items, which is the size of the
        //  content area. In our case we have to force it to memory because we have
        //  to do some alpha channel manipulation.
        //
        m_bmpCont = TBitmap
        (
            m_areaContent.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
        );

        // Create a zero based version of our display area
        TArea areaZDisplay(m_areaDisplay);
        areaZDisplay.ZeroOrg();

        TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
        if (m_bSafePanBgn)
        {
            //
            //  Draw the bgn contents into our scroll buffer. We have to
            //  be sure we get any gradients aligned, so we create an
            //  area our height, and the content area width, and vertically
            //  center it over the z display area. We can then fill it and
            //  be sure that the display area we scroll will be correctly
            //  aligned relative to our overall background area fill.
            //
            TArea areaFill(m_areaContent);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.JustifyIn
            (
                areaZDisplay, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Center
            );

            DoBgnFill(gdevCont, areaFill);
        }
         else
        {
            //
            //  We need a separate background image to composite the items
            //  over on each round. It only needs to be our size.
            //
            m_bmpBgn = bmpGatherBgn();

            //
            //  We need one for double buffering during the pan in this
            //  case, forced to memory. It only needs to be as big as us.
            //
            m_bmpDBuf = TBitmap
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
        }

        //
        //  Draw the items into our content buffer. We pass the content
        //  area as the relative and the invalid, so we draw all of the
        //  buttons, and no offset so it's zero origined. OUr safe bgn flag
        //  is the opposite of whether the drawing should be alpha safe,
        //  so just negate it.
        //
        DrawButtons
        (
            gdevCont
            , m_areaContent
            , m_areaContent
            , TPoint::pntOrigin
            , !m_bSafePanBgn
        );
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

        //
        //  Remember the current scroll pos and adjust our position by
        //  the delta.
        //
        const tCIDLib::TInt4 i4OrgPos = m_areaAdjDisplay.i4X();

        m_areaAdjDisplay -= pntDelta;
        if (m_areaAdjDisplay.i4X() < m_areaContent.i4X())
            m_areaAdjDisplay.i4X(m_areaContent.i4X());
        else if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);

        //
        //  If we actually moved, then update our scroll offset and scroll.
        //  If not, and we are in inertial mode now, then cancel the rest of
        //  the inertia because we've hit the end.
        //
        if (m_areaAdjDisplay.i4X() != i4OrgPos)
        {
            //
            //  Remember the actual delta and store the new scroll offset.
            //  The delta will be negative for a left scroll.
            //
            tCIDLib::TInt4 i4Delta = m_areaAdjDisplay.i4X() - i4OrgPos;

            //
            //  Clip to the display area plus the most restrictive clipping
            //  area of our ancestor containers.
            //
            TGUIRegion grgnClip;
            bFindMostRestrictiveClip(grgnClip, m_areaDisplay);
            TRegionJanitor janClip
            (
                cptrDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
            );

            if (m_bSafePanBgn)
            {
                // And do the blit
                TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpCont);

                // Move the displayed contents by the new offset
                TArea areaTmpTar;
                cptrDev->ScrollBits
                (
                    m_areaDisplay
                    , m_areaDisplay
                    , i4Delta * -1
                    , 0
                    , areaTmpTar
                    , kCIDLib::True
                );

                TArea areaTmpSrc(areaTmpTar);
                if (i4Delta < 0)
                    areaTmpSrc.LeftJustifyIn(m_areaAdjDisplay, kCIDLib::True);
                else
                    areaTmpSrc.RightJustifyIn(m_areaAdjDisplay, kCIDLib::True);

                // And blit the uncovered area
                cptrDev->CopyBits
                (
                    gdevSrc
                    , areaTmpSrc
                    , areaTmpTar
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::False
                );
            }
             else
            {
                //
                //  We have to do the composite scenario. Set up a zero
                //  based version of our display area.
                //
                TArea areaZDisplay = m_areaDisplay;
                areaZDisplay.ZeroOrg();

                // Copy in the background content to the double buffer
                TGraphMemDev gdevDBuf(civOwner().gdevCompat(), m_bmpDBuf);
                {
                    TGraphMemDev gdevBgn(civOwner().gdevCompat(), m_bmpBgn);
                    gdevDBuf.CopyBits(gdevBgn, areaZDisplay, areaZDisplay);
                }

                //
                //  And alpha blit the content over that. We blit from the
                //  adjusted display area we created above. It's at the right
                //  point over the content.
                //
                {
                    TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
                    gdevDBuf.AlphaBlit
                    (
                        gdevCont
                        , m_areaAdjDisplay
                        , areaZDisplay
                        , 0xFF
                        , kCIDLib::True
                    );
                }

                // And finally copy the result to the target
                cptrDev->CopyBits
                (
                    gdevDBuf
                    , areaZDisplay
                    , m_areaDisplay
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::True
                );
            }
        }
         else if (eEv == tCIDCtrls::EGestEvs::Inertia)
        {
            bContinue = kCIDLib::False;
        }

        // If an end, then we can clean up and do a final redraw
        if (eEv == tCIDCtrls::EGestEvs::End)
        {
            // Release the bitmap memory we used
            m_bmpBgn.Reset();
            m_bmpDBuf.Reset();
            m_bmpCont.Reset();

            // Keep the focus index visible
            KeepFocusVisible();

            // Do a final draw in the end position
            Redraw();
        }
    }

    return bContinue;
}


//
//  We call our parent first. If he says no, we check our buttons. Technical we don't
//  need to call the parent, since we don't mix in the command source interface. But
//  just to be safe for future changes, and the cost is tiny.
//
tCIDLib::TBoolean TCQCIntfToolbar::bReferencesTarId(const TString& strId) const
{
    if (TParent::bReferencesTarId(strId))
        return kCIDLib::True;

    const tCIDLib::TCard4 c4Buttons = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Buttons; c4Index++)
    {
        if (m_colButtons[c4Index].bReferencesTarId(strId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCIntfToolbar::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfToolbar& iwdgOther(static_cast<const TCQCIntfToolbar&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Check our simpler stuff
    if ((m_bFillSlots != iwdgOther.m_bFillSlots)
    ||  (m_c4Spacing != iwdgOther.m_c4Spacing)
    ||  (m_c4Size != iwdgOther.m_c4Size)
    ||  (m_eLayout != iwdgOther.m_eLayout)
    ||  (m_i4VertAdjust != iwdgOther.m_i4VertAdjust))
    {
        return kCIDLib::False;
    }

    // Check our buttons
    const tCIDLib::TCard4 c4Buttons = m_colButtons.c4ElemCount();
    if (c4Buttons != iwdgOther.m_colButtons.c4ElemCount())
        return kCIDLib::False;

    // We have the same number of buttons, so compare them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Buttons; c4Index++)
    {
        const TTBButton& tbbCur = m_colButtons[c4Index];
        if (tbbCur != iwdgOther.m_colButtons[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  The gesture engine calls here if clicked. We see if we hit a button.
//  If so, we do a simulated press/release on it, mark it if that is
//  enabled, and send out an OnClick event.
//
tCIDLib::TVoid TCQCIntfToolbar::Clicked(const TPoint& pntAt)
{
    // See if we hit one of our buttons. If not, we are done
    const tCIDLib::TCard4 c4InvokeInd = c4HitTest(pntAt);
    if (c4InvokeInd == kCIDLib::c4MaxCard)
        return;

    //
    //  Make sure that this one is at least 40% visible. If not, we'll
    //  ignore it.
    //
    tCIDLib::TCard4 c4Dummy = c4InvokeInd;
    if (!bVisibleEnough(c4Dummy, 0.4F))
        return;

    // Redraw the old focus index if any
    TArea areaUpdate;
    if (m_c4FocusIndex != c4InvokeInd)
    {
        AreaForIndex(m_c4FocusIndex, areaUpdate);
        m_c4FocusIndex = c4InvokeInd;
        Redraw(areaUpdate);
    }

    // Draw the target pressed, pause, and then unpressed
    TTBButton& tbbSrc = m_colButtons[c4InvokeInd];
    {
        tCQCIntfEng::TGraphIntfDev cptrGDev = cptrNewGraphDev();
        AreaForIndex(c4InvokeInd, areaUpdate);

        tbbSrc.bPressed(kCIDLib::True);
        Redraw(areaUpdate);

        TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs);

        tbbSrc.bPressed(kCIDLib::False);
        Redraw(areaUpdate);
    }

    //
    //  Post any on click commands. Put the hit item into the selected
    //  index so that it'll be seen when the engine comes bakc to get our
    //  RTVs.
    //
    m_c4SelectedIndex = c4InvokeInd;
    PostOps(tbbSrc, kCQCKit::strEvId_OnClick, tCQCKit::EActOrders::NonNested);
}


//
//  We have to give back a list of our embedded command sources (our buttons)
//  so that the outside world can get a list of command sources even though/
//  we ourself are not a command source.
//
tCIDLib::TCard4
TCQCIntfToolbar::c4QueryEmbeddedCmdSrcs(tCQCKit::TCmdSrcList& colToFill)
{
    // Make sure it's not adopting
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"QueryEmbeddedCmdSrcs - The passed collection must not be adopting"
    );

    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4Buttons = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Buttons; c4Index++)
        colToFill.Add(&m_colButtons[c4Index]);
    return c4Buttons;
}


// Copy our contents from another toolbar
tCIDLib::TVoid TCQCIntfToolbar::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfToolbar&>(iwdgSrc);
}


//
//  Handle commands for us, pass any others to our parent.
//
tCQCKit::ECmdRes
TCQCIntfToolbar::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4CmdIndex
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DelTBItem)
    {
        //
        //  If the provided index is legal, delete the indicated item
        //  from the toolbar. For now, we only make this command
        //  available in pre-load, so we never have to redraw, and
        //  we know the left-most index is at 0 so no adjustments
        //  required.
        //
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(0).m_strValue.c4Val();
        if (c4Index < m_colButtons.c4ElemCount())
            m_colButtons.RemoveAt(c4Index);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FindByText)
    {
        // Get the text to find and see if we can find it
        const TString& strFind = ccfgToDo.piAt(0).m_strValue;
        tCIDLib::TCard4 c4At = 0;

        const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
        for (; c4At < c4Count; c4At++)
        {
            if (m_colButtons[c4At].strText().bCompareI(strFind))
                break;
        }

        bResult = (c4At < c4Count);
        if (bResult)
        {
            // We found it so set the variable
            const TString& strKey = ccfgToDo.piAt(1).m_strValue;
            TCQCActVar& varInd = TStdVarsTar::varFind
            (
                strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varInd.bSetValue(c4At) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varInd.strName(), varInd.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetMarkedIndex)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varMI = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        // The result is whether there is a marked one or not
        bResult = (m_c4MarkedIndex != kCIDLib::c4MaxCard);

        // Return zero if nothing is marked, just to have some valid number
        tCIDLib::TBoolean bChanged;
        if (bResult)
            bChanged = varMI.bSetValue(m_c4MarkedIndex);
        else
            bChanged = varMI.bSetValue(0UL);

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMI.strName(), varMI.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_MarkByIndex)
    {
        const tCIDLib::TCard4 c4ButtInd = ccfgToDo.piAt(0).m_strValue.c4Val();
        const tCIDLib::TBoolean bState = facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue);
        MarkButton(c4ButtInd, bState, bCanRedraw(strEventId));
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollList)
    {
        bResult = bScrollList(ccfgToDo.piAt(0).m_strValue, kCIDLib::True);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetButtonText)
    {
        // Get the index and new text out
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(0).m_strValue.c4Val();
        const TString& strNewText = ccfgToDo.piAt(1).m_strValue;

        // See if the index is valid. If not, throw
        if (c4Index >= m_colButtons.c4ElemCount())
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_BadIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(c4Index)
                , TString(L"button")
                , TString(L"tool bar")
            );
        }

        // Get the button and update his text
        m_colButtons[c4Index].strText(strNewText);

        //
        //  If he is visible, then we need to redraw the area of this buton
        //  we've updated, also if not in an onpreload and if we are visible
        //  overall.
        //
        if (bCanRedraw(strEventId))
            UpdateButtonIfVisible(c4Index);
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
        (
            ccfgToDo
            , c4CmdIndex
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }
    return tCQCKit::ECmdRes::Ok;
}


//
//  If we are getting focus, then we show the emphasis from the item that
//  currently has the focus. All we have to do is redraw the focus item
//  if visible, now that we have the focus.
//
tCIDLib::TVoid TCQCIntfToolbar::GotFocus()
{
    // Just force a redraw of the slot with the focus
    if (m_c4FocusIndex != kCIDLib::c4MaxCard)
    {
        TArea areaUpdate;
        AreaForIndex(m_c4FocusIndex, areaUpdate);
        if (areaUpdate.bIntersects(m_areaDisplay))
            Redraw(areaUpdate);
    }
}


//
//  If we are losing the focus, we remove the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus item
//  if visible, now that we don't have the focus.
//
tCIDLib::TVoid TCQCIntfToolbar::LostFocus()
{
    // Just force a redraw of the slot with the focus
    if (m_c4FocusIndex != kCIDLib::c4MaxCard)
    {
        TArea areaUpdate;
        AreaForIndex(m_c4FocusIndex, areaUpdate);
        if (areaUpdate.bIntersects(m_areaDisplay))
            Redraw(areaUpdate);
    }
}


//
//  We just call our parent, and then calculate where our slots are going to
//  go, based on the loaded width/spacing values.
//
tCIDLib::TVoid
TCQCIntfToolbar::Initialize(TCQCIntfContainer* const    piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    // Run through our buttons and ask each one to load its image.
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TTBButton& tbbCur = m_colButtons[c4Index];
        tbbCur.LoadImage(civOwner(), dsclInit);
    }

    //
    //  Initially calculate any areas and sizes and such. Our AreaChanged and
    //  SizeChanged() methods may get called more than once before we get displayed,
    //  but in case not we do it here first. Pass a bogus old size so that it will
    //  see a size change.
    //
    CalcAreas(areaActual(), TArea::areaEmpty);

    //
    //  Go through and give each of our buttons a pointer to us. WE do it
    //  here because this is after any copying of us that might be done
    //  while getting ready to load the template. At this point, we are
    //  the real one and this pointer will remain valid.
    //
    const tCIDLib::TCard4 c4ButtCnt = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ButtCnt; c4Index++)
        m_colButtons[c4Index].SetParent(this);

    // Make sure our slot bitmap exists, and has the right id
    m_bmpSlot = TBitmap
    (
        TSize(m_c4Size, m_areaDisplay.c4Height())
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
    );

    //
    //  Remember if we have a safe bgn for scrolling (horz). If so, we
    //  only need to create one scrollable bitmap, which will have the
    //  bgn included in it. Else, we need separate content and bgn,
    //  which we will composite together at each new position.
    //
    m_bSafePanBgn = bScrollSafeBgn(kCIDLib::False);
}


//
//  This is called if we are invoked and the focus mechanism is being used.
//  We need to invoke the button that has the focus. So we store the focus
//  index as the selected index and do OnClick.
//
tCIDLib::TVoid TCQCIntfToolbar::Invoke()
{
    if (bHasFocus())
    {
        if (m_c4FocusIndex < m_colButtons.c4ElemCount())
        {
            m_c4SelectedIndex = m_c4FocusIndex;
            TTBButton& tbbSrc = m_colButtons[m_c4FocusIndex];
            PostOps(tbbSrc, kCQCKit::strEvId_OnClick, tCQCKit::EActOrders::NonNested);

            //
            //  Don't do anything else unless you check that we weren't
            //  killed by this action.
            //
        }
    }
}


//
//  We still have to handle flicks for RIVA 1 clients for now and if we
//  are in non-inertial mode. We just translate that into a scroll list
//  command.
//
tCIDLib::TVoid
TCQCIntfToolbar::ProcessFlick(  const   tCIDLib::EDirs  eDir
                                , const TPoint&         pntStart)
{
    if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
    {
        const tCIDLib::TCh* pszCmd = 0;

        //
        //  Not that scrolling orientation is backwards from dragging
        //  and flicking, so we do the opposite command from what you'd
        //  expect.
        //
        if (eDir == tCIDLib::EDirs::Right)
            pszCmd = L"Previous";
        else if (eDir == tCIDLib::EDirs::Left)
            pszCmd = L"Next";

        if (pszCmd)
            bScrollList(pszCmd, kCIDLib::True);
    }

    //
    //  Don't do anything else unless you check that we weren't killed by
    //  actions above.
    //
}


//
//  Call our parent, then add to the list any commands we support in the
//  indicated context.
//
tCIDLib::TVoid
TCQCIntfToolbar::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);


    // Delete a toolbar item by idex. Only allowed in preload.
    if (eContext == tCQCKit::EActCmdCtx::Preload)
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_DelTBItem
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_DelTBItem)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
            )
        );
    }

    //
    //  Get the index of the marked item, if any. Returns boolean to
    //  indicate if anything is marked or not.
    //
    {
        TCQCCmd& cmdGetMI = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetMarkedIndex
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetMarkedIndex)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdGetMI.eType(tCQCKit::ECmdTypes::Both);
    }


    // Find the index of an item by name
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        TCQCCmd& cmdFindBT = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_FindByText
                , facCQCKit().strMsg(kKitMsgs::midCmd_FindByText)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ToFind)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdFindBT.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Make an item by index (unmarks any previously marked one)
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_MarkByIndex
                , facCQCKit().strMsg(kKitMsgs::midCmd_MarkByIndex)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            )
        );
    }

    // Scroll the list
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdScroll = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ScrollList
                , facCQCKit().strMsg(kKitMsgs::midCmd_ScrollList)
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ScrollDir)
                , L"End, Home, Left, Right, Next, Previous"
            )
        );
        cmdScroll.eType(tCQCKit::ECmdTypes::Both);
    }

    // Update the text of a button
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetButtonText
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetButtonText)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_NewText)
            )
        );
    }
}


// Tell the caller about any images we are using
tCIDLib::TVoid TCQCIntfToolbar::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Get any that are referenced by our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);

    const tCIDLib::TCard4 c4Buttons = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Buttons; c4Index++)
    {
        const TString& strPath = m_colButtons[c4Index].strImagePath();
        if (strPath.bIsEmpty())
            continue;

        if (!bIncludeSysImgs && strPath.bStartsWith(kCQCKit::strRepoScope_System))
            continue;

        if (colToFill.bHasElement(strPath))
            continue;

        colToFill.objAdd(strPath);
    }
}


// Return summary information for the summary window
tCIDLib::TVoid
TCQCIntfToolbar::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Set the color names that we keep and aren't already correct
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Marked Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Marked Shadow/FX");

    // Do our mixins
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    //
    //  Do our stuff
    //
    //  The buttons have to really be edited directly in this object, but we still have
    //  to store the original content for undo purposes.
    //
    colAttrs.objPlace
    (
        L"Toolbar Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // We need to flatten the buttons out for this one
    {
        TBinMBufOutStream strmTar(1024UL);
        strmTar << m_colButtons << kCIDLib::FlushIt;
        colAttrs.objPlace
        (
            L"Buttons"
            , kCQCIntfEng::strAttr_ToolBar_Buttons
            , strmTar.mbufData()
            , strmTar.c4CurPos()
            , TString::strEmpty()
            , tCIDMData::EAttrEdTypes::Visual
        );
    }

    TString strLim;
    tCQCIntfEng::FormatETBLayouts
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );

    colAttrs.objPlace
    (
        L"Layout"
        , kCQCIntfEng::strAttr_ToolBar_Layout
        , strLim
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(tCQCIntfEng::strXlatETBLayouts(m_eLayout));

    colAttrs.objPlace
    (
        L"Size"
        , kCQCIntfEng::strAttr_ToolBar_Size
        , L"Range: 8, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::Both
    ).SetCard(m_c4Size);

    colAttrs.objPlace
    (
        L"Spacing"
        , kCQCIntfEng::strAttr_ToolBar_Spacing
        , L"Range: 0, 128"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::Both
    ).SetCard(m_c4Spacing);

    colAttrs.objPlace
    (
        L"Vert Adjust"
        , kCQCIntfEng::strAttr_ToolBar_VertAdjust
        , L"Range: -128, 128"
        , tCIDMData::EAttrTypes::Int
        , tCIDMData::EAttrEdTypes::Both
    ).SetInt(m_i4VertAdjust);

    colAttrs.objPlace
    (
        L"Fill Slots"
        , kCQCIntfEng::strAttr_ToolBar_FillSlots
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::Both
    ).SetBool(m_bFillSlots);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfToolbar::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Let our image list interface have at it first
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);

    // Iterate the buttons and ask each to refresh its image
    const tCIDLib::TCard4 c4ButtCount = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ButtCount; c4Index++)
        m_colButtons[c4Index].LoadImage(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfToolbar::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFull
                        ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    const tCIDLib::TCard4 c4ButtCount = m_colButtons.c4ElemCount();
    TString strChange;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ButtCount; c4Index++)
    {
        // Our caption text is actually in the buttons
        if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Caption))
        {
            strChange = m_colButtons[c4Index].strText();
            const tCIDLib::TBoolean bChanged = facCQCKit().bDoSearchNReplace
            (
                strSrc
                , strTar
                , strChange
                , bRegEx
                , bFull
                , regxFind
            );

            if (bChanged)
                m_colButtons[c4Index].strText(strChange);
        }

        //
        //  Though we use the image list mixin, and updated it above, we
        //  still have to update the image paths in the buttons.
        //
        if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        {
            strChange = m_colButtons[c4Index].strImagePath();
            const tCIDLib::TBoolean bChanged = facCQCKit().bDoSearchNReplace
            (
                strSrc
                , strTar
                , strChange
                , bRegEx
                , bFull
                , regxFind
            );

            if (bChanged)
                m_colButtons[c4Index].strImagePath(strChange);
        }

        // And the actions are on the buttons as well
        if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        {
            m_colButtons[c4Index].CmdSearchNReplace
            (
                strSrc, strTar, bRegEx, bFull, regxFind
            );
        }
    }
}


TSize TCQCIntfToolbar::szDefaultSize() const
{
    return TSize(280, 74);
}


//
//  We override this in order to scale our slot size so that it correctly fits the
//  adjusted font size after rescaling.
//
tCIDLib::TVoid
TCQCIntfToolbar::ScaleInternal( const   tCIDLib::TFloat8 f8XScale
                                , const tCIDLib::TFloat8 f8YScale)
{
    // Scale the slot width by the x factor
    m_c4Size = TMathLib::c4Round(m_c4Size * f8XScale);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfToolbar::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    // And do our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_Buttons)
    {
        // This is generally redudant, but in the case of an undo it's not
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colButtons;

        // Reinitialize the images just in case
        try
        {
            TDataSrvClient dsclLoad;
            RefreshImages(dsclLoad);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbErr(L"The images could not be updated, so they may not display correctly");
            msgbErr.ShowIt(wndAttrEd);
        }
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_FillSlots)
    {
        m_bFillSlots = adatNew.bVal();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_Layout)
    {
        m_eLayout = tCQCIntfEng::eXlatETBLayouts(adatNew.strValue());
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_Size)
    {
        m_c4Size = adatNew.c4Val();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_Spacing)
    {
        m_c4Spacing = adatNew.c4Val();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_ToolBar_VertAdjust)
    {
        m_i4VertAdjust = adatNew.i4Val();
    }
}


tCIDLib::TVoid
TCQCIntfToolbar::Validate(  const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);

    // Validate the buttons
    TString strErr;
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TTBButton& tbbCur = m_colButtons[c4Index];

        // if the image is enabled, we need to deal with that
        if (!tbbCur.strImagePath().bIsEmpty())
        {
            if (!facCQCKit().bIsValidRemTypeRelPath(tbbCur.strImagePath(), strErr, kCIDLib::False))
            {
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        c4UniqueId()
                        , facCQCIntfEng().strMsg
                          (
                            kIEngErrs::errcVal_BadPath, TString(L"toolbar image"), strErr
                          )
                        , kCIDLib::True
                        , strWidgetId()
                    )
                );
            }
             else
            {
                // Looks reasonable, see if it exists
                tCIDLib::TCard4 c4SerNum = 0;
                if (!dsclVal.bFileExists(tbbCur.strImagePath(), tCQCRemBrws::EDTypes::Image))
                {
                    // Put an error in the list
                    strErr.LoadFromMsg
                    (
                        kIEngErrs::errcVal_ImgNotFound, facCQCIntfEng(), tbbCur.strImagePath()
                    );
                    colErrs.objAdd
                    (
                        TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId())
                    );
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add the passed button to our button list
tCIDLib::TVoid TCQCIntfToolbar::AddButton(const TTBButton& tbbToAdd)
{
    // Just stick it into the tool bar at the end
    m_colButtons.objAdd(tbbToAdd);

    CalcAreas(areaActual(), TArea::areaEmpty);
}


// Get or set the fill slots flag
tCIDLib::TBoolean TCQCIntfToolbar::bFillSlots() const
{
    return m_bFillSlots;
}

tCIDLib::TBoolean TCQCIntfToolbar::bFillSlots(const tCIDLib::TBoolean bToSet)
{
    if (m_bFillSlots != bToSet)
    {
        m_bFillSlots = bToSet;
        CalcAreas(areaActual(), TArea::areaEmpty);
    }
    return m_bFillSlots;
}


// Return the count of buttons
tCIDLib::TCard4 TCQCIntfToolbar::c4ButtonCnt() const
{
    return m_colButtons.c4ElemCount();
}


// Get/set the slot spacing. If setting, we have to re-calc our slots
tCIDLib::TCard4 TCQCIntfToolbar::c4Spacing() const
{
    return m_c4Spacing;
}

tCIDLib::TCard4 TCQCIntfToolbar::c4Spacing(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Spacing = c4ToSet;

    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_c4Spacing;
}


// Get/set the slot width. If setting, we have to re-calc our slots
tCIDLib::TCard4 TCQCIntfToolbar::c4Width() const
{
    return m_c4Size;
}

tCIDLib::TCard4 TCQCIntfToolbar::c4Width(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Size = c4ToSet;

    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_c4Size;
}


// Get/set the toolbar layout
tCQCIntfEng::ETBLayouts TCQCIntfToolbar::eLayout() const
{
    return m_eLayout;
}

tCQCIntfEng::ETBLayouts
TCQCIntfToolbar::eLayout(const tCQCIntfEng::ETBLayouts eToSet)
{
    m_eLayout = eToSet;
    return m_eLayout;
}


// Get/set the vertical adjustment.  If setting, we have to re-calc our slots
tCIDLib::TInt4 TCQCIntfToolbar::i4VertAdjust() const
{
    return m_i4VertAdjust;
}

tCIDLib::TInt4 TCQCIntfToolbar::i4VertAdjust(const tCIDLib::TInt4 i4ToSet)
{
    m_i4VertAdjust = i4ToSet;

    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_i4VertAdjust;
}


//
//  Mark or unmark a button as the current button. We return true if we
//  actually changed something.
//
tCIDLib::TVoid
TCQCIntfToolbar::MarkButton(const   tCIDLib::TCard4     c4NewInd
                            , const tCIDLib::TBoolean   bState
                            , const tCIDLib::TBoolean   bRedraw)
{
    //
    //  We only need to do something if they are asking to unmarked the
    //  marked one, or to mark an unmarked one. Otherwise we are already
    //  in the requested state.
    //
    //  Ignore if the index is invalid.
    //
    //  If not redrawing, then we can do a much simpler scenario that doesn't
    /// have to calculate positions and such.
    //
    if ((c4NewInd < m_colButtons.c4ElemCount())
    &&  (((m_c4MarkedIndex == c4NewInd) && !bState)
    ||   ((m_c4MarkedIndex != c4NewInd) && bState)))
    {
        // Remember the current index
        const tCIDLib::TCard4 c4OldIndex = m_c4MarkedIndex;

        // Store the new marked index, or clear the existing one
        if (bState)
            m_c4MarkedIndex = c4NewInd;
        else
            m_c4MarkedIndex = kCIDLib::c4MaxCard;

        //
        //  Handle redrawing if told we should. If the two areas are
        //  contiguous, we can just combine them and do a single redraw.
        //
        //  Else, if we had an old area, and the old area isn't the same
        //  as the new one (we were unmarking the marked one), then we
        //  need to redraw the two areas.
        //
        //  In both cases check to see if the areas to update even
        //  intersect our area, since the list is scrollable and may
        //  be off screen.
        //
        if (bRedraw)
        {
            TArea       areaNew;
            TTBButton&  tbiTar = m_colButtons[c4NewInd];
            AreaForIndex(c4NewInd, areaNew);

            //
            //  If we need to also unmark a previous one, see if the two
            //  are contiguous and get the old area for update.
            //
            tCIDLib::TBoolean bContiguous = kCIDLib::False;
            TArea areaOld;
            if (c4OldIndex != kCIDLib::c4MaxCard)
            {
                AreaForIndex(c4OldIndex, areaOld);
                bContiguous = (c4OldIndex + 1 == c4NewInd)
                              ||  (c4NewInd + 1 == c4OldIndex);
            }

            if (bContiguous)
            {
                // They are side by side so just add them together
                areaNew |= areaOld;
                if (areaNew.bIntersects(m_areaDisplay))
                    Redraw(areaNew);
            }
             else
            {
                // They are separated areas
                if (!areaOld.bIsEmpty()
                &&  (areaOld != areaNew)
                &&  areaOld.bIntersects(m_areaDisplay))
                {
                    Redraw(areaOld);
                }

                if (areaNew.bIntersects(m_areaDisplay))
                    Redraw(areaNew);
            }
        }
    }
}


//
//  Normally this would be a public, virtual, but we aren't the actual
//  command source. The buttons are. So they have to call us to get the
//  RTV object created. We have all the data it needs.
//
TCQCCmdRTVSrc* TCQCIntfToolbar::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    TString strText;
    if (m_c4SelectedIndex != kCIDLib::c4MaxCard)
        strText = m_colButtons[m_c4SelectedIndex].strText();

    return new TCQCIntfToolBarRTV
    (
        m_c4SelectedIndex
        , m_c4MarkedIndex
        , c4FirstVisIndex()
        , strText
        , cuctxToUse
    );
}


//
//  See if there is a button with the indicated text. If so, return a pointer
//  to it and the index it's at.
//
TCQCIntfToolbar::TTBButton*
TCQCIntfToolbar::ptbbWithText(const TString& strToFind,  tCIDLib::TCard4& c4Index)
{
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colButtons[c4Index].strText() == strToFind)
            return &m_colButtons[c4Index];
    }
    return 0;
}


// Remove a button by index
tCIDLib::TVoid TCQCIntfToolbar::RemoveButtonAt(const tCIDLib::TCard4 c4At)
{
    m_colButtons.RemoveAt(c4At);
    CalcAreas(areaActual(), TArea::areaEmpty);
}


//
//  To support the attribute editor stuff. The buttons aren't treated normally, they are
//  edited directly via a dialog back into the widget. But, after that, for undo purposes
//  we have to get the changed content back into the attribute.
//
tCIDLib::TVoid TCQCIntfToolbar::StreamButtons(TBinOutStream& strmTar)
{
    strmTar << m_colButtons << kCIDLib::FlushIt;
}


// Swap the positions of two buttons (for the desiger's use)
tCIDLib::TVoid
TCQCIntfToolbar::SwapButtons(const  tCIDLib::TCard4 c4FirstInd
                            , const tCIDLib::TCard4 c4SecInd)
{
    m_colButtons.SwapItems(c4FirstInd, c4SecInd);
}


// Provides access to the buttons by index
TCQCIntfToolbar::TTBButton& TCQCIntfToolbar::tbbAt(const tCIDLib::TCard4 c4At)
{
    return m_colButtons[c4At];
}

const TCQCIntfToolbar::TTBButton&
TCQCIntfToolbar::tbbAt(const tCIDLib::TCard4 c4At) const
{
    return m_colButtons[c4At];
}


// Remove any current button mark
tCIDLib::TVoid TCQCIntfToolbar::Unmark(const tCIDLib::TBoolean bRedraw)
{
    if (m_c4MarkedIndex != kCIDLib::c4MaxCard)
    {
        // Remember the current index and then set the 'no mark' value
        const tCIDLib::TCard4 c4OldIndex = m_c4MarkedIndex;
        m_c4MarkedIndex = kCIDLib::c4MaxCard;

        // Handle redrawing if told we should
        if (bRedraw)
        {
            TArea       areaOld;
            TTBButton&  tbiTar = m_colButtons[c4OldIndex];
            AreaForIndex(c4OldIndex, areaOld);
            if (areaOld.bIntersects(m_areaDisplay))
                Redraw(areaOld);
        }
    }
}

// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Re-calc our areas and sizes and such. This used to only happen during
//  design time, but now that we support dynamic reisizing it can happen at
//  viewing time as well.
//
tCIDLib::TVoid
TCQCIntfToolbar::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    CalcAreas(areaNew, areaOld);
}


// Stream our level stuf in from the passed string
tCIDLib::TVoid TCQCIntfToolbar::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ToolBar::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Let any mixins stream in
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    // If it's beyond version 1, we stream in the image list
    if (c2FmtVersion > 1)
        MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // Then do our stuff
    strmToReadFrom  >> m_c4Spacing
                    >> m_c4Size
                    >> m_colButtons
                    >> m_eLayout;

    //
    //  For a while the width was allowed to go to zero. This would cause
    //  an error and made no sense. Now it's limited to 8 minimum, so adjust
    //  if it's less.
    //
    if (m_c4Size < 8)
        m_c4Size = 8;

    //
    //  If it's beyond version 1, we stream in the vert adjust, else we
    //  default it.
    //
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_i4VertAdjust;
    else
        m_i4VertAdjust = 0;

    // If V6 or beyond, read in the fill slots flag, else default it
    if (c2FmtVersion > 5)
        strmToReadFrom >> m_bFillSlots;
    else
        m_bFillSlots = kCIDLib::False;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If pre-V3, we need to force the new marked image into the image list
    //  mixin. And we need to go through all of the buttons and add the
    //  new runtime values.
    //
    if (c2FmtVersion < 3)
    {
        AddNewImgKey(CQCIntfEng_ToolBar::strMarked, tCQCIntfEng::EImgReqFlags::Optional);

        const tCIDLib::TCard4 c4ButtCnt = m_colButtons.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ButtCnt; c4Index++)
        {
            TTBButton& tbbCur = m_colButtons[c4Index];

            tbbCur.AddRTValue
            (
                facCQCKit().strMsg(kKitMsgs::midRT_FirstIndex)
                , kCQCKit::strRTVId_FirstIndex
                , tCQCKit::ECmdPTypes::Unsigned
            );

            tbbCur.AddRTValue
            (
                facCQCKit().strMsg(kKitMsgs::midRT_MarkedIndex)
                , kCQCKit::strRTVId_MarkIndex
                , tCQCKit::ECmdPTypes::Unsigned
            );

            tbbCur.AddRTValue
            (
                facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
                , kCQCKit::strRTVId_SelectIndex
                , tCQCKit::ECmdPTypes::Unsigned
            );
        }
    }

    // If less than V4, then swap F2/F3 and move Extra3 to Extra 2
    if (c2FmtVersion < 4)
    {
        TRGBClr rgbSave = rgbFgn2();
        rgbFgn2(rgbFgn3());
        rgbFgn3(rgbSave);

        rgbExtra2(rgbExtra3());
    }

    // If less than V5, we have to add the background image
    if (c2FmtVersion < 5)
        AddNewImgKey(CQCIntfEng_ToolBar::strBgn, tCQCIntfEng::EImgReqFlags::Optional);

    // Init any runtime stuff
    m_c4FocusIndex = 0;
    m_c4MarkedIndex = kCIDLib::c4MaxCard;
    m_c4SelectedIndex = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid TCQCIntfToolbar::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_ToolBar::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // And do our stuff and our end marker
    strmToWriteTo   << m_c4Spacing
                    << m_c4Size
                    << m_colButtons
                    << m_eLayout
                    << m_i4VertAdjust

                    // V6 stuff
                    << m_bFillSlots

                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  Re-calc our areas when this happens. It used to be that this only was
//  called during design time, but now that we support dynamic resizing, it
//  can happen any time.
//
tCIDLib::TVoid
TCQCIntfToolbar::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);

    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    CalcAreas(areaNew, areaOld);
}


tCIDLib::TVoid
TCQCIntfToolbar::StyleFlagChanged(  const   tCIDLib::TCard4 c4OldFlags
                                    , const tCIDLib::TCard4 c4NewFlags)
{
    const tCIDLib::TCard4 c4Old(c4OldFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
    const tCIDLib::TCard4 c4New(c4NewFlags & kCQCIntfEng::c4StyleFlag_HasBorder);

    // If the border flag changed, recalc our area info
    if (c4Old != c4New)
        CalcAreas(areaActual(), TArea::areaEmpty);
}


tCIDLib::TVoid
TCQCIntfToolbar::Update(        TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    //
    //  Do any bgn fill/border. We don't do this if transparent and in fill slots
    //  mode, since we don't have any fill and the border applies to the slots in
    //  that case, so we don't want it to draw that.
    //
    if (!bIsTransparent() || !m_bFillSlots)
        TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Add our display area to the clip region
    grgnClip.CombineWith(m_areaDisplay, tCIDGraphDev::EClipModes::And);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    //
    //  Set up an adjusted content area to draw relative to, relative to our
    //  display area origin. The invalid area will naturally the be relative
    //  to that as well.
    //
    TArea areaAdj(m_areaContent);
    areaAdj.SetOrg(m_areaDisplay.pntOrg());
    areaAdj -= m_areaAdjDisplay.pntOrg();

    //
    //  The area we want to update is the intersection of the invalid area and
    //  display area. We don't use any offset so we just pass a 0,0 point.
    //
    TArea areaUpdate(m_areaDisplay);
    areaUpdate &= areaInvalid;
    DrawButtons
    (
        gdevTarget, areaAdj, areaUpdate, TPoint::pntOrigin, kCIDLib::False
    );
}


// ---------------------------------------------------------------------------
//  TCQCIntfToolbar: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Adjust the passed area by the item slot size. Which can include spacing
//  or not. Once we figure out a starting area, this can be called to just
//  adjust it for each item.
//
tCIDLib::TVoid
TCQCIntfToolbar::AdjustBySlot(          TArea&              areaAdjust
                                , const tCIDLib::TBoolean   bInclSpace) const
{
    tCIDLib::TInt4 i4Ofs = tCIDLib::TInt4(m_c4Size);
    if (bInclSpace)
        i4Ofs += tCIDLib::TInt4(m_c4Spacing);
    areaAdjust.AdjustOrg(i4Ofs, 0);
}


//
//  Calculate the position for the passed index. We have to be able to do it
//  both for display coordinates and scroll buffer coordinates. We have one
//  that defaults to the display coordinate stuff, which calls the other one
//  that takes arbitrary coordinate systems.
//
//  The relative to is the area relative to which the content is being drawn,
//  and therefore to which the item slot areas should be relative.
//
tCIDLib::TVoid
TCQCIntfToolbar::AreaForIndex(  const   tCIDLib::TCard4     c4Index
                                ,       TArea&              areaToFill) const
{
    //
    //  Set up a content area at our display origin, and then adjust it
    //  lft by the scroll offset. That is the normal display relative to
    //  area.
    //
    TArea areaRel(m_areaContent);
    areaRel.SetOrg(m_areaDisplay.pntOrg());
    areaRel -= m_areaAdjDisplay.pntOrg();
    AreaForIndex(c4Index, areaRel, areaToFill);
}

tCIDLib::TVoid
TCQCIntfToolbar::AreaForIndex(  const   tCIDLib::TCard4     c4Index
                                , const TArea&              areaRelTo
                                ,       TArea&              areaToFill) const
{
    //
    //  Start with the target area. However, it may be empty, so we only do
    //  this to get the origin. If empty, its origin will be our return origin.
    //
    areaToFill = areaRelTo;

    // How far from the content origin is this one
    const tCIDLib::TInt4 i4Pos((c4Index * m_c4Size) + (c4Index * m_c4Spacing));

    // Adjust up from the realative to origin
    areaToFill.AdjustOrg(i4Pos, 0);
    areaToFill.SetSize(m_c4Size, m_areaDisplay.c4Height());
}


//
//  This is called when we get a scroll command. We have to adjust the
//  m_c4LeftIndex appropriately to achieve the requested scroll operation.
//  The values we get are the ones we provided as the possible values of our
//  scroll widget command.
//
//  We return true if we actually changed the scroll offset.
//
//  For now, we are just doing a 'jump to position', not smooothly
//  scrolling there.
//
tCIDLib::TBoolean
TCQCIntfToolbar::bScrollList(const  TString&            strScrollOp
                            , const tCIDLib::TBoolean   bRedraw)
{
    //
    //  If the content area is smaller than the display area, then nothing
    //  to do. Else store some info for later.
    //
    if (m_areaDisplay.c4Width() > m_areaContent.c4Width())
        return kCIDLib::False;

    // Remember our current scroll position
    const TArea areaOrg(m_areaAdjDisplay);

    //
    //  For page ops we move by up to a display area width. For next and
    //  previous we move by a quarter page.
    //
    const tCIDLib::TInt4 i4Span = tCIDLib::TInt4(m_areaDisplay.c4Width());

    //
    //  Note that we are moving the display area around, in the positive
    //  direction, over the content, not moving the content under the
    //  display position, so we do things in the same sign as the op.
    //
    if (strScrollOp == L"Home")
    {
        m_areaAdjDisplay.LeftJustifyIn(m_areaContent);
    }
     else if (strScrollOp == L"End")
    {
        m_areaAdjDisplay.RightJustifyIn(m_areaContent);
    }
     else if (strScrollOp == L"Left")
    {
        m_areaAdjDisplay.AdjustOrg(-(i4Span / 4), 0);
    }
     else if ((strScrollOp == L"Next") || (strScrollOp == L"Previous"))
    {
        const tCIDLib::TBoolean bNext(strScrollOp == L"Next");

        if (bNext)
            m_areaAdjDisplay.AdjustOrg(i4Span, 0);
        else
            m_areaAdjDisplay.AdjustOrg(-i4Span, 0);
    }
     else if (strScrollOp == L"Right")
    {
        m_areaAdjDisplay.AdjustOrg(i4Span / 4, 0);
    }

    //
    //  Clip the movement to the content area, and see if we actually
    //  moved or not. Since the content area is zero based, then the
    //  new area X is the adjusted scroll offset.
    //
    if (m_areaAdjDisplay.i4X() < m_areaContent.i4X())
        m_areaAdjDisplay.i4X(m_areaContent.i4X());
    if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
        m_areaAdjDisplay.RightJustifyIn(m_areaContent);

    // If the position changed, and we can redraw, then do it
    if (m_areaAdjDisplay != areaOrg)
    {
        //
        //  If in remote mode just redraw at the new position, else do a
        //  smooth scroll.
        //
        if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
        {
            KeepFocusVisible();
            Redraw();
        }
         else
        {
            SlideList(areaOrg);
        }
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This method will test whether the indicated index is 'visible enough',
//  which is used to decide whether to allow a click on an item or to move
//  the focus to it. We don't want to allow items that might only have a
//  pixel or two visible to be selected in these types of cases.
//
//  If it's not, then the passed index will be adjusted if that is possible
//  to reflect the next closest one that is visible enough.
//
//  It is assumed that the index at least within one item's size of the
//  display area. Else it won't get up to a usefully visible item index.
//
tCIDLib::TBoolean
TCQCIntfToolbar::bVisibleEnough(        tCIDLib::TCard4&    c4Index
                                , const tCIDLib::TFloat4    f4Per) const
{
    // If empty, not much we can do
    if (m_colButtons.bIsEmpty())
        return kCIDLib::True;

    // Get the area of the passed index
    TArea areaTest;
    AreaForIndex(c4Index, areaTest);

    //
    //  How much of an item has to be visible before we consider it a
    //  visible one for our purposes.
    //
    //  They give us a percent that has to be visible. Our stuff below
    //  measures how much is not visible, so flip it.
    //
    const tCIDLib::TInt4 i4Limit = tCIDLib::TInt4(m_c4Size * (1.0 - f4Per));

    // Remember if we modified it
    const tCIDLib::TCard4 c4OrgVal = c4Index;

    // if not enough is visible, then move inwards if possible
    if (areaTest.i4X() < m_areaDisplay.i4X())
    {
        if (m_areaDisplay.i4X() - areaTest.i4X() >= i4Limit)
        {
            if (c4Index + 1 < m_colButtons.c4ElemCount())
               c4Index++;
        }
    }
     else if (areaTest.i4Right() > m_areaDisplay.i4Right())
    {
        if ((areaTest.i4Right() - m_areaDisplay.i4Right() >= i4Limit)
        &&  c4Index)
        {
            c4Index--;
        }
    }

    // If we didn't have to change it, return true
    return (c4OrgVal == c4Index);
}


//
//  We have a special case issue when doing our pans or scrolls that we
//  need to capture any content under the items. That includes any popup
//  layers under us, and any widgets under us in our own layer, and
//  including our own bgn fill if any but not the buttons themselves. So it's
//  sort of a special case.
//
//  We use a special method on the parent container to draw up to, but
//  not including us. Then we draw our background stuff over the top of
//  that. We return a bitmap with that content.
//
//  NOTE: This cannot be called from the regular update, since the big
//  background bitmap is already in use in that case. This is only to
//  be called from the scrolling method.
//
//  NOTE: The bitmap returned is the content within our display area, not
//  our whole widget area, unless they both happen to be the same. We only
//  scroll within the display area so that's all that matters.
//
TBitmap TCQCIntfToolbar::bmpGatherBgn()
{
    //
    //  Get the stuff underneath our display area. We tell it to stop when
    //  it hits our widget, and not to draw it (stop before the one we
    //  indicate.)
    //
    TBitmap bmpBgn = civOwner().bmpQueryContentUnder
    (
        m_areaDisplay, this, kCIDLib::True
    );

    //
    //  Now let's draw our bgn contents on top of that. Center an area
    //  of our full size over the display area and fill over that. to
    //  insure alignment of gradients within the display are bit we move.
    //
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
    if (!bIsTransparent())
    {
        TArea areaFill(areaActual());
        areaFill.CenterIn(m_areaDisplay);
        DoBgnFill(gdevBgn, areaFill);
    }

    // Create a bitmap the size of our display area
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );

    //
    //  And copy the bgn contents over into our return bitmap. WE create a
    //  zero origined version of the display area as the target.
    //
    {
        TGraphMemDev gdevRet(civOwner().gdevCompat(), bmpRet);
        TArea areaTar(m_areaDisplay);
        areaTar.ZeroOrg();
        gdevRet.CopyBits(gdevBgn, m_areaDisplay, areaTar);
    }
    return bmpRet;
}


//
//  Return the closest index to the passed from here index that is mostly
//  visible within our display area. This is for keeping a certain index
//  visible.
//
tCIDLib::TCard4
TCQCIntfToolbar::c4ClosestVisIndex(const tCIDLib::TCard4 c4FromHere) const
{
    // Not much we can do
    if (!m_colButtons.c4ElemCount())
        return c4FromHere;

    // Get the area of the from index
    TArea areaTest;
    AreaForIndex(c4FromHere, areaTest);

    // If it is fully contained within the display area, then it's keeper as is
    if (m_areaDisplay.bContainsArea(areaTest))
        return c4FromHere;

    //
    //  If it doesn't intersect, then it's completely outside, so move to the
    //  first one that's at least reasonably visible. If it does intersect,
    //  then it's a partially visible one, so either way we'll then check
    //  below to make sure it's visible enough.
    //
    tCIDLib::TCard4 c4Ret = c4FromHere;
    if (!m_areaDisplay.bIntersects(areaTest))
    {
        const tCIDLib::TCard4 c4Size = m_c4Size + m_c4Spacing;

        //
        //  Not we have to use the ADJUSTED dislay area here, which
        //  calculating the new index!
        //
        if (areaTest.i4X() < m_areaDisplay.i4X())
            c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4X()) / c4Size;
        else
            c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Right()) / c4Size;
    }

    //
    //  Call the visible enough method which will modify the index if needed.
    //  We don't care if it does or doesn't. We require it to be at least
    //  40% visible.
    //
    bVisibleEnough(c4Ret, 0.4F);

    // And return whatever we ended up with
    return c4Ret;
}


//
//  Calc the first (at least partially) visible index. We have one that
//  takes a visible area and an area it is relative to, which we need in
//  order to deal with both regular display and drawing into scrollable
//  buffers. We have a second one which just calls the first with the right
//  stuff for normal drawing at our widget position.
//
tCIDLib::TCard4
TCQCIntfToolbar::c4FirstVisIndex(const  TArea&  areaVis
                                , const TArea&  areaRelTo) const
{
    // If no names, it has to be zero
    if (m_colButtons.bIsEmpty())
        return 0;

    // See how far we are scrolled from the content origin
    tCIDLib::TCard4 c4Diff;
    c4Diff = tCIDLib::TCard4(areaVis.i4X() - areaRelTo.i4X());

    // And we can just divide to get the indx
    return (c4Diff / (m_c4Size + m_c4Spacing));
}

tCIDLib::TCard4 TCQCIntfToolbar::c4FirstVisIndex() const
{
    // If no names, it has to be zero
    if (m_colButtons.bIsEmpty())
        return 0;

    //
    //  Set up a content area scrolled relative to us, i.e. at our origin
    //  and moved left by the scroll offset. That should leave the visible
    //  bit within our display area. So the adjusted area is the relative to
    //  and the displa area is the visible.
    //
    TArea areaAdj(m_areaContent);
    areaAdj.SetOrg(m_areaDisplay.pntOrg());
    areaAdj -= m_areaAdjDisplay.pntOrg();
    return c4FirstVisIndex(m_areaDisplay, areaAdj);
}



//
//  Tests a point to see if it falls into one of our visible buttons, where
//  visible means it's within our display area currently.
//
tCIDLib::TCard4 TCQCIntfToolbar::c4HitTest(const TPoint& pntTest) const
{
    // Get the count of buttons. If none, then can't have hit a used slot
    const tCIDLib::TCard4 c4ButtCnt = m_colButtons.c4ElemCount();
    if (!c4ButtCnt)
        return kCIDLib::c4MaxCard;

    // If it's not within our display area it can't be good
    if (!m_areaDisplay.bContainsPoint(pntTest))
        return kCIDLib::c4MaxCard;

    //
    //  The difference between our display area origin and the point, plus
    //  the adjusted display area offset, is the full offset to consider.
    //
    const tCIDLib::TCard4 c4Diff = tCIDLib::TCard4
    (
        m_areaAdjDisplay.i4X() + (pntTest.i4X() - m_areaDisplay.i4X())
    );

    // Divide that by the size of each one
    const tCIDLib::TCard4 c4Index = c4Diff / (m_c4Size + m_c4Spacing);

    // If it's beyond our max index, not valid
    if (c4Index >= c4ButtCnt)
        return kCIDLib::c4MaxCard;

    return c4Index;
}


//
//  Calc a few things we need during drawing of the buttons. We don't
//  bother keeping it around and having to worry about keeping it up to
//  date. It's just calculated when needed.
//
//  NOTE: We assume our font has been set before this is called.
//
tCIDLib::TVoid
TCQCIntfToolbar::CalcMaxAndHeight(  TGraphDrawDev&      gdevTarget
                                    , tCIDLib::TCard4&  c4MaxImgVS
                                    , tCIDLib::TCard4&  c4TextHeight)
{
    c4MaxImgVS = 0;
    c4TextHeight = 0;

    // Calculate our text height
    {
        TFontMetrics fmtrCur;
        gdevTarget.QueryFontMetrics(fmtrCur);
        c4TextHeight = fmtrCur.c4Height();

        // If text wrap isn't disabled, allow for 2 lines
        if (!bNoTextWrap())
            c4TextHeight *= 2;
    }

    // Calculate the maximum vertical image size if not doing centered
    const tCIDLib::TCard4 c4ButtCnt = m_colButtons.c4ElemCount();
    if (m_eLayout != tCQCIntfEng::ETBLayouts::Centered)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ButtCnt; c4Index++)
        {
            TTBButton& tbbCur = m_colButtons[c4Index];
            if (tbbCur.strImagePath().bIsEmpty())
                continue;

            if (tbbCur.szImage().c4Height() > c4MaxImgVS)
                c4MaxImgVS = tbbCur.szImage().c4Height();
        }

        // Include the pressed, marked and focus images is set
        TArea areaImgs = areaImgBounds();
        if (areaImgs.c4Height() > c4MaxImgVS)
            c4MaxImgVS = areaImgs.c4Height();
    }
}


//
//  When our button list or size/pos changes, this is called to let us
//  update any area oriented calculations.
//
//  This is called when our size changes or the list is reloaded or changed.
//  We calculate content and display areas and any other stuff we might want
//  to keep around based on size.
//
//  The display area is where we scroll our content, and is within our own
//  area to some degree or another, and at our origin. The content area is
//  zero based and is the full size required to contain all of the buttons.
//
tCIDLib::TVoid
TCQCIntfToolbar::CalcAreas(const TArea& areaNew, const TArea& areaOld)
{
    //
    //  Figure out our display area. Start with our area, then move inwards
    //  for the border if we have one (and we aren't in fill slots mode. If we
    //  in fill slots mode, the border applies to the slots.
    //
    m_areaDisplay = areaNew;
    if (!bIsTransparent() && !m_bFillSlots)
    {
        if (bHasBorder())
            m_areaDisplay.Deflate(2);
    }

    // If we have any rounding, then move in by one more.
    if (c4Rounding())
        m_areaDisplay.Deflate(1);

    //
    //  Calculate the content area. If no content, then we are zero sized at
    //  zero origin, so start with that as the default until proven otherwise
    //  below.
    //
    m_areaContent.ZeroAll();

    // If we have any buttons, then we may need to adjust it
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
    if (c4Count)
    {
        // Calc the width required to hold our buttons and spacings
        const tCIDLib::TCard4 c4Size
        (
            (m_c4Size * c4Count) + (m_c4Spacing * (c4Count - 1))
        );

        //
        //  Set the content to this size in the orientation direction, and
        //  the display area in the other.
        //
        m_areaContent.SetSize(c4Size, m_areaDisplay.c4Height());
    }

    //
    //  Update our adjusted display to the same size as the regular
    //  display area, or the content area if that is smaller
    //
    if (m_areaDisplay.c4Width() > m_areaContent.c4Width())
    {
        // Can't be scrolled anymore, go back to origin and take cont width
        m_areaAdjDisplay.Set
        (
            0, 0, m_areaContent.c4Width(), m_areaDisplay.c4Height()
        );
    }
     else
    {
        // Just take the display size, may need to adjust below
        m_areaAdjDisplay.SetSize(m_areaDisplay.szArea());

        if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);
    }

    //
    //  If the size changed and we are transparent and in fill slots mode, then
    //  reallocate the slot buffer. It has to be forced to memory since we need
    //  to access its pixels.
    //
    if (!areaNew.bSameSize(areaOld) && bIsTransparent() && m_bFillSlots)
    {
        m_bmpSlot = TBitmap
        (
            TSize(m_c4Size, m_areaDisplay.c4Height())
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
            , kCIDLib::True
        );
    }

    // Make sure the focus stays visible
    KeepFocusVisible();
}


//
//  Draws a single button, into the indicated area
//
tCIDLib::TVoid
TCQCIntfToolbar::DrawAButton(       TGraphDrawDev&      gdevTarget
                            ,       TTBButton&          tbbToDraw
                            , const TArea&              areaSlot
                            , const tCIDLib::TCard4     c4MaxImgVS
                            , const tCIDLib::TCard4     c4TextHeight
                            , const tCIDLib::TBoolean   bDoFocus
                            , const tCIDLib::TBoolean   bMarked
                            , const tCIDLib::TBoolean   bForceAlpha)
{
    // Clip the output to the slot area
    TRegionJanitor janClip(&gdevTarget, areaSlot, tCIDGraphDev::EClipModes::And);

    TCQCIntfView& civUs = civOwner();

    //
    //  If transparent and filling slots, then do that. We have a slot bitmap
    //  to do that in a transparent dragging friendly way. It may seem weird we
    //  don't just do a color fill, but that would zero out the alpha channel
    //  bits in the target. We don't have a GDI way of doing it and keeping the
    //  alpha correct. So we have to create use a bitmap. It's presized, so we
    //  just fill it, set the alpha and, and blit it.
    //
    //  However, the WebRIVA client doesn't have to worry about this, so we
    //  just do a fill in that case. He can only do flicks, no smooth scrolling.
    //
    if (bIsTransparent() && m_bFillSlots)
    {
        if (TFacCQCIntfEng::bWebRIVAMode())
        {
            facCQCIntfEng().StdBgnFill(gdevTarget, areaSlot, eBgnStyle(), rgbBgn(), rgbBgn2());
        }
         else
        {
            TArea areaTar(areaSlot);
            areaTar.ZeroOrg();

            TGraphMemDev gdevSlot(gdevTarget, m_bmpSlot);
            facCQCIntfEng().StdBgnFill
            (
                gdevSlot, areaTar, eBgnStyle(), rgbBgn(), rgbBgn2()
            );

            m_bmpSlot.pixaData().SetAllAlpha(0xFF);
            gdevTarget.AlphaBlit(gdevSlot, areaTar, areaSlot, 0xFF, kCIDLib::False);
        }

        // If borders are enabled, they apply to the slot in this case
        if (bHasBorder())
            gdevTarget.Stroke(areaSlot, rgbFgn2());
    }


    // Calculate the position of our text and image
    TArea areaText;
    TArea areaImg;
    {
        // Both areas will be slightly moved inwards horz, for some margin
        areaImg = areaSlot;
        areaText = areaSlot;
        areaImg.Deflate(2, 0);
        areaText.Deflate(2, 0);

        //
        //  If not centering, then set the text to the pre-calculated text
        //  height and the image area to the max image height.
        //
        if (m_eLayout != tCQCIntfEng::ETBLayouts::Centered)
        {
            areaImg.c4Height(c4MaxImgVS);
            areaText.c4Height(c4TextHeight);
        }

        //
        //  Now set up an area that is the combined height of these two
        //  areas, plus a 2 pixel margin and center it in the slot area.
        //  Then we will place the two areas within this area.
        //
        TArea areaWork = areaSlot;
        areaWork.c4Height(areaImg.c4Height() + areaText.c4Height() + 1);
        areaWork.CenterIn(areaSlot);

        // Add the vertical adjustment if any
        if (m_i4VertAdjust)
            areaWork.AdjustOrg(0, m_i4VertAdjust);

        // Now position them within the slot area
        switch(m_eLayout)
        {
            case tCQCIntfEng::ETBLayouts::Centered :
            case tCQCIntfEng::ETBLayouts::ImgOnly :
                //
                //  Just center them both in the slot area, though only the
                //  image will be drawn if image only layout.
                //
                areaImg.CenterIn(areaWork);
                areaText.CenterIn(areaWork);
                break;

            case tCQCIntfEng::ETBLayouts::TextAbove :
                //
                //  Top justify the text, then put the image just below
                //  it with a slight margin.
                //
                areaText.TopJustifyIn(areaWork, kCIDLib::True);
                areaImg.i4Top(areaText.i4Bottom());
                break;

            case tCQCIntfEng::ETBLayouts::TextBelow :
                //
                //  Bottom justify the text, then put the image just above
                //  it with a slight margin.
                //
                areaText.BottomJustifyIn(areaWork, kCIDLib::True);
                areaImg.i4Y
                (
                    areaText.i4Y() - tCIDLib::TInt4(areaImg.c4Height() + 1)
                );
                break;

            default :
                // <TBD> Do a popup here if in debug mode
                break;
        };
    }

    //
    //  For efficiency, get some flags that indicate whether we need to
    //  drop shadow and press effect.
    //
    const tCIDLib::TBoolean bDoShadow = bShadow();

    //
    //  If we hav ea pressed image, we don't want to do faux pressed empahsis on the
    //  image or text. So get that out separately always.
    //
    const MCQCIntfImgListIntf::TImgInfo* pimgiPressed = pimgiForKeyEO
    (
        CQCIntfEng_ToolBar::strPressed
    );

    //
    //  If we are pressed and have a pressed image, that takes precedence over
    //  anything else. Otherwise, if we are doing focus and we have the focus
    //  and this button is in the focus slot, that's next most important. And
    //  finally if this is the marked button and we have a marked image, we
    //  draw that.
    //
    const MCQCIntfImgListIntf::TImgInfo* pimgiCur = 0;
    if (tbbToDraw.bPressed())
    {
        //
        //  We are pressed and we have a pressed image, so let's display
        //  the pressed image.
        //
        pimgiCur = pimgiPressed;
    }
     else if (bDoFocus)
    {
        //
        //  We aren't pressed (or don't have a pressed image), but we have
        //  a focus image, have the focus, and we are the focus button,
        //  so let's display the focus image.
        //
        pimgiCur = pimgiForKeyEO(CQCIntfEng_ToolBar::strFocus);
    }
     else if (bMarked && bImgEnabled(CQCIntfEng_ToolBar::strMarked))
    {
        pimgiCur = pimgiForKeyEO(CQCIntfEng_ToolBar::strMarked);
    }
     else if (bImgEnabled(CQCIntfEng_ToolBar::strBgn))
    {
        pimgiCur = pimgiForKeyEO(CQCIntfEng_ToolBar::strBgn);
    }

    // If we got an image, then display it
    if (pimgiCur)
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , bForceAlpha && !TFacCQCIntfEng::bRemoteMode()
            , pimgiCur->m_cptrImg
            , areaImg
            , areaImg
            , pimgiCur->m_i4HOffset
            , pimgiCur->m_i4VOffset
            , pimgiCur->m_c1Opacity
        );
    }

    //
    //  If the button has an image, then display that if we aren't
    //  displaying a pressed image.
    //
    if (!tbbToDraw.strImagePath().bIsEmpty()) // !tbbToDraw.bPressed()
    {
        // Center the actual image area in the slot image area
        TArea areaImgTar(TPoint::pntOrigin, tbbToDraw.szImage());
        areaImgTar.CenterIn(areaImg);

        //
        //  If this button is in a pressed state, and we don't have a
        //  pressed image, then move both 1,1 to simulate a press.
        //
        if (tbbToDraw.bPressed() && !pimgiPressed)
            areaImgTar.AdjustOrg(1, 1);

        // Ask the button to draw the image
        tbbToDraw.DrawOn(gdevTarget, areaImgTar);
    }

    // And draw the text if we have any and it's not supressed
    const TString& strButtText = tbbToDraw.strText();
    if (!strButtText.bIsEmpty() && (m_eLayout != tCQCIntfEng::ETBLayouts::ImgOnly))
    {
        //
        //  If this button is in a pressed state, and we don't have a pressed
        //  image, then move down 1,1 to simulate a press.
        //
        TArea areaTextTar = areaText;
        if (tbbToDraw.bPressed() && !pimgiPressed)
            areaTextTar.AdjustOrg(1, 1);

        TRGBClr rgbSave = gdevTarget.rgbTextColor();
        gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civUs.DrawTextFX
            (
                gdevTarget
                , strButtText
                , areaTextTar
                , eTextFX()
                , bMarked ? rgbExtra() : rgbFgn()
                , bMarked ? rgbExtra2() : rgbFgn2()
                , tCIDLib::EHJustify::Center
                , tCIDLib::EVJustify::Center
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            //
            //  Clip to the target text area. THe FX text above will do his own
            //  clipping so we only need to do it here.
            //
            TRegionJanitor janTextClip(&gdevTarget, areaTextTar, tCIDGraphDev::EClipModes::And);

            //
            //  Now draw our text. If the shadow style is enabled, then draw
            //  first a drop version in fgn3, then draw the regular one in
            //  fgn1 in the normal position.
            //
            if (bDoShadow)
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaTextTar);
                areaDrop.AdjustOrg(c4Drop, c4Drop);

                if (bMarked)
                    gdevTarget.SetTextColor(rgbExtra2());
                else
                    gdevTarget.SetTextColor(rgbFgn2());

                // If no wrap, use single line draw, else multi-line
                if (bForceAlpha && !TFacCQCIntfEng::bRemoteMode())
                {
                    if (bNoTextWrap())
                    {
                        civUs.DrawTextAlpha
                        (
                            gdevTarget
                            , strButtText
                            , areaDrop
                            , tCIDLib::EHJustify::Center
                            , tCIDLib::EVJustify::Center
                            , tCIDGraphDev::ETextFmts::None
                        );
                    }
                     else
                    {
                        civUs.DrawMTextAlpha
                        (
                            gdevTarget
                            , strButtText
                            , areaDrop
                            , tCIDLib::EHJustify::Center
                            , tCIDLib::EVJustify::Center
                            , kCIDLib::True
                        );
                    }
                }
                 else
                {
                    if (bNoTextWrap())
                    {
                        gdevTarget.DrawString
                        (
                            strButtText
                            , areaDrop
                            , tCIDLib::EHJustify::Center
                            , tCIDLib::EVJustify::Center
                            , tCIDGraphDev::ETextFmts::None
                        );
                    }
                     else
                    {
                        gdevTarget.DrawMText
                        (
                            strButtText
                            , areaDrop
                            , tCIDLib::EHJustify::Center
                            , tCIDLib::EVJustify::Center
                            , kCIDLib::True
                        );
                    }
                }
            }

            if (bMarked)
                gdevTarget.SetTextColor(rgbExtra());
            else
                gdevTarget.SetTextColor(rgbFgn());

            if (bForceAlpha && !TFacCQCIntfEng::bRemoteMode())
            {
                if (bNoTextWrap())
                {
                    civUs.DrawTextAlpha
                    (
                        gdevTarget
                        , strButtText
                        , areaTextTar
                        , tCIDLib::EHJustify::Center
                        , tCIDLib::EVJustify::Center
                        , tCIDGraphDev::ETextFmts::None
                    );
                }
                 else
                {
                    civUs.DrawMTextAlpha
                    (
                        gdevTarget
                        , strButtText
                        , areaTextTar
                        , tCIDLib::EHJustify::Center
                        , tCIDLib::EVJustify::Center
                        , kCIDLib::True
                    );
                }
            }
             else
            {
                if (bNoTextWrap())
                {
                    gdevTarget.DrawString
                    (
                        strButtText
                        , areaTextTar
                        , tCIDLib::EHJustify::Center
                        , tCIDLib::EVJustify::Center
                        , tCIDGraphDev::ETextFmts::None
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        strButtText
                        , areaTextTar
                        , tCIDLib::EHJustify::Center
                        , tCIDLib::EVJustify::Center
                        , kCIDLib::True
                    );
                }
            }
            gdevTarget.SetTextColor(rgbSave);
        }
    }
}


//
//  Draws any buttons that are within that intersect the invalid area. The
//  buttons are drawn relative to the passed relative area, and the invalid
//  area has to be relative to that. Any items that intersect the invalid
//  area are redrawn.
//
//  This allows this method to work for either regular updates at our actual
//  widget area, or to draw into a zero based image for scrolling or dragging.
//
//  In order to support building up side by side pages of content to support
//  the command based scrolling operations, we take a target adjust value,
//  which allows the actual output areas to be offset.
//
tCIDLib::TVoid
TCQCIntfToolbar::DrawButtons(       TGraphDrawDev&      gdevTarget
                            , const TArea&              areaRelTo
                            , const TArea&              areaInvalid
                            , const TPoint&             pntTarAdj
                            , const tCIDLib::TBoolean   bForceAlpha)
{
    // See if we have pressed or focus images
    const tCIDLib::TBoolean bUsePressImg = bImgEnabled(CQCIntfEng_ToolBar::strPressed);
    const tCIDLib::TBoolean bUseFocusImg = bImgEnabled(CQCIntfEng_ToolBar::strFocus);

    // Set our font on our target dvice
    TCQCIntfFontJan janFont(&gdevTarget, *this);

    // Calculate some info we need for redrawing (AFTER setting the font!)
    tCIDLib::TCard4 c4MaxImgVS = 0;
    tCIDLib::TCard4 c4TextHeight = 0;
    CalcMaxAndHeight(gdevTarget, c4MaxImgVS, c4TextHeight);

    // Get the content based based area of the first invalid item
    tCIDLib::TCard4 c4Index = c4FirstVisIndex(areaInvalid, areaRelTo);
    TArea           areaCur;
    AreaForIndex(c4Index, areaRelTo, areaCur);

    //
    //  And now start looping at the first visible index, until we either
    //  run out of buttons or go beyond the target area.
    //
    TArea areaAdjusted;
    const tCIDLib::TCard4 c4Count = m_colButtons.c4ElemCount();
    while (c4Index < c4Count)
    {
        if (areaCur.i4X() > areaInvalid.i4Right())
            break;

        const tCIDLib::TBoolean bDoFocus
        (
            !iwdgRootTemplate().bNoFocus()
            && bHasFocus()
            && (c4Index == m_c4FocusIndex)
            && bUseFocusImg
        );

        // If it intersects the invalid area, draw it
        if (areaCur.bIntersects(areaInvalid))
        {
            // If this is the marked one, tell it to draw marked
            TTBButton& tbbCur = m_colButtons[c4Index];

            // If an adjustment was provided apply that
            areaAdjusted = areaCur;
            areaAdjusted += pntTarAdj;
            DrawAButton
            (
                gdevTarget
                , tbbCur
                , areaAdjusted
                , c4MaxImgVS
                , c4TextHeight
                , bDoFocus
                , m_c4MarkedIndex == c4Index
                , bForceAlpha
            );
        }

        // If in designer mode, draw a box around each one
        if (TFacCQCIntfEng::bDesMode())
        {
            // Set a medium gray pen to draw the uutline
            const tCIDGraphDev::EMixModes eSaveMode = gdevTarget.eMixMode();
            gdevTarget.eMixMode(tCIDGraphDev::EMixModes::XorPen);
            gdevTarget.Stroke(areaCur, facCIDGraphDev().rgbWhite);
            gdevTarget.eMixMode(eSaveMode);
        }

        // Move area and index forward to the next slot
        c4Index++;
        AdjustBySlot(areaCur, kCIDLib::True);
    }
}


//
//  Called after moving the list, to insure that the focus index remains
//  visible.
//
tCIDLib::TVoid TCQCIntfToolbar::KeepFocusVisible()
{
    // Get the area of the focus item for this list
    TArea areaFocus;
    AreaForIndex(m_c4FocusIndex, areaFocus);

    //
    //  If it's not fully visible, then move to the closest one
    //  that is visible enough.
    //
    if (!m_areaDisplay.bContainsArea(areaFocus))
        m_c4FocusIndex = c4ClosestVisIndex(m_c4FocusIndex);
}



//
//  This is called when we need to move the list in a non-drag way, i.e.
//  based on a command, and the new and old areas overlap. In this case we
//  can do a smooth scroll between them. The adjusted display area is
//  already updated, and we get the original area it moved from.
//
tCIDLib::TVoid TCQCIntfToolbar::SlideList(const TArea& areaOrg)
{
    // Set up the whole area we need to scroll
    TArea areaScroll;

    // Figure out which scenario
    TArea areaFirst;
    TArea areaSec;

    TArea areaInvalid;
    tCIDLib::EDirs eDir;
    if (areaOrg.i4X() > m_areaAdjDisplay.i4X())
    {
        eDir = tCIDLib::EDirs::Right;
        areaFirst = m_areaAdjDisplay;
        areaSec = areaOrg;
    }
     else
    {
        eDir = tCIDLib::EDirs::Left;
        areaFirst = areaOrg;
        areaSec = m_areaAdjDisplay;
    }

    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TBitmap bmpScroll;
    if (areaFirst.bIntersects(areaSec)
    ||  (areaFirst.i4Right() == areaSec.i4X()))
    {
        //
        //  The areas overlap or touch, so we can do a contiguous scroll
        //  from one positon to the other.
        //
        //  The scroll area is the intersected width, and the zero origined
        //  version of the combination is the scroll area.
        //
        TArea areaSrc = areaFirst;
        areaSrc |= areaSec;
        areaScroll = areaSrc;
        areaScroll.ZeroOrg();

        // Create the scroll bitmap
        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);

        // If a safe bgn, put it into the content buffer
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);
        }

        //
        //  And draw our contiguous range into the scroll buffer. We have
        //  to create a scrolled content area to use as the 'relative to'
        //  area. This will make the bit we care about come out in our
        //  zero origined buffer.
        //
        TArea areaRelTo = m_areaContent;
        areaRelTo -= areaSrc.pntOrg();
        DrawButtons
        (
            gdevCont, areaRelTo, areaScroll, TPoint::pntOrigin, !m_bSafePanBgn
        );
    }
     else
    {
        //
        //  In this case, we only want to scroll in the new contents. This
        //  one is more complex. The scroll size is twice our width.
        //
        areaScroll.SetSize
        (
            m_areaDisplay.c4Width() * 2, m_areaDisplay.c4Height()
        );

        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );

        // Copy the two src areas out to their appropriate target places
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);

        TArea areaZDisp(m_areaDisplay);
        areaZDisp.ZeroOrg();

        //
        //  If a safe bgn, put it into the content buffer, but we also
        //  have to redraw our on-screen content (without our buttons),
        //  because the efficient scroll will scroll the current contents
        //  and we don't want that since it would probably scroll a partial
        //  one at one end.
        //
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);

            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
            cptrGraphDev->CopyBits(gdevBgn, areaZDisp, m_areaDisplay);
        }

        //
        //  NOTE that we are flipping the directions in this case.
        //
        TArea areaRelTo;
        if (eDir == tCIDLib::EDirs::Left)
        {
            // Put the second page in the first half and scroll right
            areaRelTo = m_areaContent;
            areaRelTo -= areaSec.pntOrg();

            DrawButtons
            (
                gdevCont, areaRelTo, areaZDisp, TPoint::pntOrigin, !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Right;
        }
         else
        {
            //
            //  Put the first page in the second half and scroll the opposite
            //  of our initially selected direction.
            //
            areaRelTo = m_areaContent;
            areaRelTo -= areaFirst.pntOrg();

            DrawButtons
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , areaZDisp.pntUR(1, 0)
                , !m_bSafePanBgn
            );
            eDir = tCIDLib::EDirs::Left;
        }
    }

    //
    //  We have a special case here since we are doing this scroll. We need
    //  to make sure our slot area is clipped to within all our parent's
    //  areas, since the content of the template we are in may be bigger
    //  than the overlay we are in.
    //
    TGUIRegion grgnClip;
    if (!bFindMostRestrictiveClip(grgnClip, m_areaDisplay))
    {
        //
        //  We are completely clipped. Wierd since we shouldn't have seen
        //  the gesture, but anyway...
        //
        return;
    }

    // Clip, and do the scroll
    TRegionJanitor janClip
    (
        cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
    );

    TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);
    TCIDEaseScroller escrSlide;
    if (m_bSafePanBgn)
    {
        // Do the light weight slide
        escrSlide.Scroll2
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }
     else
    {
        //
        //  We have to do the heavier scroll, so set up a bitmap with our
        //  bgn fill in it, and provide a double buffer buffer.
        //
        TBitmap bmpBgn = bmpGatherBgn();
        TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

        // Do the double buffering bitmap
        TBitmap bmpBuf
        (
            m_areaDisplay.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

        escrSlide.Scroll1
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevBuf
            , gdevBgn
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }

    // Make sure the focus stays visible
    KeepFocusVisible();

    // And do a final redraw
    Redraw(m_areaDisplay);
}


//
//  A convenience method mostly for our DoCmd method, to redraw a button that
//  he has changed, if it is visible.
//
tCIDLib::TVoid
TCQCIntfToolbar::UpdateButtonIfVisible(const tCIDLib::TCard4 c4Index)
{
    // Make sure it's a valid index
    if (c4Index >= m_colButtons.c4ElemCount())
        return;

    // Get the area of the target index
    TArea areaIndex;
    AreaForIndex(c4Index, areaIndex);

    // If it doesn't intersect our display area, then it's not visible
    if (!areaIndex.bIntersects(m_areaDisplay))
        return;

    // Redraw the content underneath the area of this button
    Redraw(areaIndex);
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarToolbar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarToolbar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarToolbar::TCQCIntfVarToolbar() :

    TCQCIntfToolbar
    (
        CQCIntfEng_ToolBar::eVarCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Toolbar)
    )
{
}

TCQCIntfVarToolbar::TCQCIntfVarToolbar(const TCQCIntfVarToolbar& iwdgSrc) :

    TCQCIntfToolbar(iwdgSrc)
    , MCQCIntfVarIntf(iwdgSrc)
{
}

TCQCIntfVarToolbar::~TCQCIntfVarToolbar()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarToolbar: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarToolbar&
TCQCIntfVarToolbar::operator=(const TCQCIntfVarToolbar& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfVarIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarToolbar: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarToolbar::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarToolbar& iwdgOther
    (
        static_cast<const TCQCIntfVarToolbar&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarToolbar::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarToolbar&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfVarToolbar::QueryWdgAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mxins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarToolbar::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFullMatch
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFullMatch, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarToolbar::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarToolbar::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarToolbar: Protected, inherited methods
// ---------------------------------------------------------------------------

// We only accept enumerated string types
tCIDLib::TBoolean TCQCIntfVarToolbar::bCanAcceptVar(const TCQCActVar& varToTest)
{
    // Has to be an enumerated string type
    if ((varToTest.eType() != tCQCKit::EFldTypes::String)
    ||  !varToTest.strLimits().bStartsWith(L"Enum"))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarToolbar::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ToolBar::c2VarFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Do the variable mixing
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfVarToolbar::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_ToolBar::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarToolbar::ValueUpdate(        TCQCPollEngine&
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bVarUpdate
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    // If no variables could have changed, don't bother
    if (!bVarUpdate)
        return;

    // Pass it on to our mixin
    VarUpdate(ctarGlobalVars, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the variable state
//  changes out of ready state, to let us know we need to update our display
//  to indicate we don't have a good value. We just remove any mark.
//
tCIDLib::TVoid
TCQCIntfVarToolbar::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    Unmark(!bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarToolbar::VarValChanged(  const   TCQCActVar&         varNew
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    // Make sure it's of the type that we require
    if ((varNew.eType() != tCQCKit::EFldTypes::String)
    ||  !varNew.fldlVar().bIsA(TCQCFldEnumLimit::clsThis()))
    {
        Unmark(!bNoRedraw);
        return;
    }

    // Get the variable's limit object and cast to it's actual type
    const TCQCFldEnumLimit& fldlEnum = dynamic_cast<const TCQCFldEnumLimit&>(varNew.fldlVar());

    // And ask it for the ordinal of the new value
    const tCIDLib::TCard4 c4Ord = fldlEnum.c4QueryOrdinal(varNew.strValue(), kCIDLib::False);

    // If not valid, unmark, else mark
    if (c4Ord == kCIDLib::c4MaxCard)
        Unmark(!bNoRedraw);
    else
        MarkButton(c4Ord, kCIDLib::True, !bNoRedraw);
}

