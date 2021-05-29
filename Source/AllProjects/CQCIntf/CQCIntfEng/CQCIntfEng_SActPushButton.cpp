//
// FILE NAME: CQCIntfEng_SActPushButton.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/04/2004
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
//  This file implements a derivative of the push button class, which in this
//  case performs one of a set of canned built in actions.
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
#include    "CQCIntfEng_PushButton.hpp"
#include    "CQCIntfEng_SActPushButton.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfSActPushButton,TCQCIntfPushButton)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_SActPushButton
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;


        // -----------------------------------------------------------------------
        //  Out capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::TakesFocus
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfSActPushButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfSActPushButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfSActPushButton::TCQCIntfSActPushButton() :

    TCQCIntfPushButton
    (
        CQCIntfEng_SActPushButton::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_SpecialAction)
    )
    , m_eAction(tCQCIntfEng::ESpecActs::Blanker)
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/SpecialActButton");
}

TCQCIntfSActPushButton::
TCQCIntfSActPushButton(const TCQCIntfSActPushButton& iwdgSrc) :

    TCQCIntfPushButton(iwdgSrc)
    , m_eAction(iwdgSrc.m_eAction)
{
}

TCQCIntfSActPushButton::~TCQCIntfSActPushButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfSActPushButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfSActPushButton&
TCQCIntfSActPushButton::operator=(const TCQCIntfSActPushButton& iwdgSrc)
{
    if (this == &iwdgSrc)
        return *this;

    TParent::operator=(iwdgSrc);
    m_eAction = iwdgSrc.m_eAction;

    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfSActPushButton: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfSActPushButton::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfSActPushButton& iwdgOther(static_cast<const TCQCIntfSActPushButton&>(iwdgSrc));
    return (m_eAction == iwdgOther.m_eAction);
}


tCIDLib::TVoid
TCQCIntfSActPushButton::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfSActPushButton&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfSActPushButton::Initialize( TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // For now, we just pass it on to our parent
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  At this level, the text is just our caption text. A derived class
    //  may do this again after calling us, but this gets it right for our
    //  level.
    //
    SetText(TString::strEmpty(), kCIDLib::True, kCIDLib::False);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfSActPushButton::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Add our stuff
    adatTmp.Set(L"Special Act. Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // We just have our special action to do
    TString strLim;
    tCQCIntfEng::FormatESpecActs
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"Special Action"
        , kCQCIntfEng::strAttr_SpcActButt_Action
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatESpecActs(m_eAction));
    colAttrs.objAdd(adatTmp);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfSActPushButton::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_SpcActButt_Action)
        m_eAction = tCQCIntfEng::eXlatESpecActs(adatNew.strValue());
}



// ---------------------------------------------------------------------------
//  TCQCIntfSActPushButton: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCIntfEng::ESpecActs TCQCIntfSActPushButton::eAction() const
{
    return m_eAction;
}

tCQCIntfEng::ESpecActs
TCQCIntfSActPushButton::eAction(const tCQCIntfEng::ESpecActs eToSet)
{
    m_eAction = eToSet;
    return m_eAction;
}


// ---------------------------------------------------------------------------
//  TCQCIntfSActPushButton: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfSActPushButton::Clicked(const TPoint&)
{
    TCQCIntfTemplate& iwdgRoot = civOwner().iwdgTopMost();
    if (!iwdgRoot.bIsPopupType())
    {
        // Do click feedback and then do the action
        DoClickFeedback();
        civOwner().miahOwner().DoSpecialAction(m_eAction, kCIDLib::False, civOwner());
    }
}


tCIDLib::TVoid TCQCIntfSActPushButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And now stream in our own stuff now. Start with the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIntfEng_SActPushButton::c2FmtVersion)
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

    // Do our stuff
    strmToReadFrom >> m_eAction;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfSActPushButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream a start marker and our format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_SActPushButton::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << m_eAction
                    << tCIDLib::EStreamMarkers::EndObject;
}

