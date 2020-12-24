//
// FILE NAME: CQCIntfEng_AdjustButton.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2004
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
//  This class just exists for backwards file format compatibility. So we keep
//  just enough to handle reading in our contents. We will be discarded after
//  being read.
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
#include    "CQCIntfEng_AdjustButton.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfAdjustButton,TCQCIntfPushButton)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_AdjustButton
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The format version for our persistent storage
        //
        //  Version 2 -
        //      Support for single field association was moved to a mixin from
        //      the base widget class. So we need to know if we have already
        //      converted or not. In actual fact we will no longer see V1
        //      objects anymore post 3.0.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAdjustButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAdjustButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAdjustButton::TCQCIntfAdjustButton() :

    TCQCIntfPushButton
    (
        tCQCIntfEng::ECapFlags::None
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_IncDecButton)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::CReadMWrite))
{
}

TCQCIntfAdjustButton::
TCQCIntfAdjustButton(const TCQCIntfAdjustButton& iwdgToCopy) :

    TCQCIntfPushButton(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfAdjustButton::~TCQCIntfAdjustButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdjustButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfAdjustButton&
TCQCIntfAdjustButton::operator=(const TCQCIntfAdjustButton& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}



// ---------------------------------------------------------------------------
//  TCQCIntfAdjustButton: Public, inherited methods
// ---------------------------------------------------------------------------

// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfAdjustButton::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfAdjustButton&>(iwdgSrc);
}

tCIDLib::TVoid
TCQCIntfAdjustButton::Validate( const   TCQCFldCache&
                                ,       tCQCIntfEng::TErrList&
                                ,       TDataSrvClient&) const
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdjustButton: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Called by the field mixin when our field changes state or value. We only
//  bother actually registering the field if we are int or card type, so we
//  won't ever get any useful values here otherwise.
//
tCIDLib::TVoid
TCQCIntfAdjustButton::FieldChanged(         TCQCFldPollInfo&
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&)
{
}


tCIDLib::TVoid TCQCIntfAdjustButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and check it
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_AdjustButton::c2FmtVersion))
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

    //
    //  In theory version 1 objects would require some adjustment to move over
    //  to the single field mixing scheme for association of fields, but we will
    //  never see such versions anymore. They cannot upgrade to anything post
    //  3.0 until they go to 3.0 first and it does all the refactoring and
    //  conversion.
    //
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    tCIDLib::TBoolean bDummy;
    TString strDummy;
    strmToReadFrom  >> bDummy
                    >> strDummy;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfAdjustButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our stuff, then an end object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_AdjustButton::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << kCIDLib::False
                    << TString::strEmpty()
                    << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfAdjustButton::ValueUpdate(          TCQCPollEngine&
                                    , const tCIDLib::TBoolean
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&
                                    , const TGUIRegion&)
{
}


