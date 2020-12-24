//
// FILE NAME: CQCIntfView_StateData.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/08/2002
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
//  The implementation for the interface viewer's persistent state data
//  class.
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
#include    "CQCIntfView.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfViewState,TCQCFrameState)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfView_StateData
{
    namespace
    {
        const tCIDLib::TCard2   c2FmtVersion = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfViewState
//  PREFIX: fst
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfViewState: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfViewState::TCQCIntfViewState()
{
}

TCQCIntfViewState::TCQCIntfViewState(const  TArea&      areaFrame
                                    , const TString&    strIntfId) :

    TCQCFrameState(areaFrame)
    , m_strIntfId(strIntfId)
{
}

TCQCIntfViewState::TCQCIntfViewState(const TString& strIntfId) :

    TCQCFrameState()
    , m_strIntfId(strIntfId)
{
}

TCQCIntfViewState::TCQCIntfViewState(const TCQCIntfViewState& fstToCopy) :

    TCQCFrameState(fstToCopy)
    , m_strIntfId(fstToCopy.m_strIntfId)
{
}

TCQCIntfViewState::~TCQCIntfViewState()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfViewState: Public operators
// ---------------------------------------------------------------------------
TCQCIntfViewState&
TCQCIntfViewState::operator=(const TCQCIntfViewState& fstToAssign)
{
    if (this == &fstToAssign)
        return *this;

    // Call our parent first, then do our stuff
    TParent::operator=(fstToAssign);

    m_strIntfId = fstToAssign.m_strIntfId;
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfViewState: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfViewState::ClearLastTmpl()
{
    m_strIntfId.Clear();
}


const TString& TCQCIntfViewState::strLastTmpl() const
{
    return m_strIntfId;
}

const TString& TCQCIntfViewState::strLastTmpl(const TString& strIntfId)
{
    m_strIntfId = strIntfId;
    return m_strIntfId;
}


// ---------------------------------------------------------------------------
//  TCQCIntfViewState: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfViewState::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    //
    //  And next should be the format version. We only have one version
    //  now, so we just check that it matches that.
    //
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIntfView_StateData::c2FmtVersion)
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

    // And get our stuff out
    strmToReadFrom  >>  m_strIntfId;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfViewState::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Call our parent first
    TParent::StreamTo(strmToWriteTo);

    //
    //  And dump our our stuff, with a frame and trailing end object marker.
    //  We also put out a format version, to support later automatic
    //  upgrading of the format.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfView_StateData::c2FmtVersion
                    << m_strIntfId
                    << tCIDLib::EStreamMarkers::EndObject;
}




