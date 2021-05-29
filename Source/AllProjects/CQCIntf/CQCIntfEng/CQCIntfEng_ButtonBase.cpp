//
// FILE NAME: CQCIntfEng_ButtonBase.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/13/2002
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
//  This file implements the class that understands the grunt work of
//  displaying a button of some type.
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


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfButtonBase,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ButtonBase
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 1 -
        //      Reset the version as of 1.4
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfButtonBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfButtonBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfButtonBase::~TCQCIntfButtonBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfButtonBase: Hidden constructors and operators
// ---------------------------------------------------------------------------

TCQCIntfButtonBase::
TCQCIntfButtonBase( const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName
    )
    , m_bPressed(kCIDLib::False)
    , m_eAutoRepeat(tCQCIntfEng::EAutoReps::None)
{
}

TCQCIntfButtonBase::TCQCIntfButtonBase(const TCQCIntfButtonBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bPressed(iwdgSrc.m_bPressed)
    , m_eAutoRepeat(iwdgSrc.m_eAutoRepeat)
{
}

TCQCIntfButtonBase&
TCQCIntfButtonBase::operator=(const TCQCIntfButtonBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_bPressed      = iwdgSrc.m_bPressed;
        m_eAutoRepeat   = iwdgSrc.m_eAutoRepeat;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfButtonBase: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfButtonBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in our own stuff
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ButtonBase::c2FmtVersion))
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

    // Read in our stuff
    strmToReadFrom  >> m_eAutoRepeat;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfButtonBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_ButtonBase::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << m_eAutoRepeat
                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TCQCIntfButtonBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfButtonBase::bPressed() const
{
    return m_bPressed;
}

tCIDLib::TBoolean
TCQCIntfButtonBase::bPressed(const tCIDLib::TBoolean bToSet)
{
    m_bPressed = bToSet;
    return m_bPressed;
}


//
//  If a derived button wants to provide press/release feedback when clicked
//  they call this.
//
tCIDLib::TVoid TCQCIntfButtonBase::DoClickFeedback()
{
    // Draw our button pressed, pause, and  then unpressed
    const TArea& areaUs = areaActual();

    bPressed(kCIDLib::True);
    Redraw(areaUs);

    TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs / 2);
    bPressed(kCIDLib::False);
    Redraw(areaUs);
}


tCQCIntfEng::EAutoReps TCQCIntfButtonBase::eAutoRepeat() const
{
    return m_eAutoRepeat;
}

tCQCIntfEng::EAutoReps
TCQCIntfButtonBase::eAutoRepeat(const tCQCIntfEng::EAutoReps eToSet)
{
    m_eAutoRepeat = eToSet;
    return m_eAutoRepeat;
}


