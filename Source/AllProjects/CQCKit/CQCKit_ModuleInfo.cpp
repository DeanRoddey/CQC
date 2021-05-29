//
// FILE NAME: CQCKit_ModleInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2000
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
//  This file implements the TCQCDriverCfg class.
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCModuleInfo,TCIDModuleInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_ModuleInfo
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCModuleInfi
//  PREFIX: modi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCModuleInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCModuleInfo::TCQCModuleInfo() :

    m_eSides(tCIDLib::ECSSides::Both)
{
}

TCQCModuleInfo::TCQCModuleInfo( const   TString&            strBaseName
                                , const tCIDLib::ECSSides   eSides
                                , const tCIDLib::EModTypes  eModType
                                , const tCIDLib::TCard4     c4MajVersion
                                , const tCIDLib::TCard4     c4MinVersion
                                , const tCIDLib::EModFlags  eFlags) :

    TCIDModuleInfo(strBaseName, eModType, c4MajVersion, c4MinVersion, eFlags)
    , m_eSides(eSides)
{
}

TCQCModuleInfo::TCQCModuleInfo(const    TString&            strBaseName
                                , const tCIDLib::ECSSides   eSides
                                , const tCIDLib::EModTypes  eModType
                                , const tCIDLib::EModFlags  eFlags) :

    TCIDModuleInfo(strBaseName, eModType, eFlags)
    , m_eSides(eSides)
{
}



// ---------------------------------------------------------------------------
//  TCQCModuleInfo: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCModuleInfo::operator==(const TCQCModuleInfo& modiSrc) const
{
    if (this == &modiSrc)
        return kCIDLib::True;

    if (m_eSides != modiSrc.m_eSides)
        return kCIDLib::False;

    return TParent::operator==(modiSrc);
}

tCIDLib::TBoolean TCQCModuleInfo::operator!=(const TCQCModuleInfo& modiSrc) const
{
    if (this == &modiSrc)
        return kCIDLib::False;

    if (m_eSides == modiSrc.m_eSides)
        return kCIDLib::False;

    return TParent::operator!=(modiSrc);
}


// ---------------------------------------------------------------------------
//  TCQCModuleInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::ECSSides TCQCModuleInfo::eSides() const
{
    return m_eSides;
}


// ---------------------------------------------------------------------------
//  TCQCModuleInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCModuleInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Do our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_ModuleInfo::c2FmtVersion))
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

    // And get our stuff, and check for the trailing end marker
    strmToReadFrom >> m_eSides;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCModuleInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our parent first
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff, followed by an end object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_ModuleInfo::c2FmtVersion
                    << m_eSides
                    << tCIDLib::EStreamMarkers::EndObject;
}

