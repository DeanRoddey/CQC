//
// FILE NAME: CQCKit_StdCmdSrc.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/19/2005
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
//  This file implements some standard command source derivatives that are
//  used when the command source doesn't need to be mixed into something else
//  but are just blocks of commands stored under some key.
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
//  Local types and data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_StdCmdSrc
    {
        // -----------------------------------------------------------------------
        //  Persistent format version for the standard command src
        //
        //  Version 2 -
        //      Added a couple of flags just for the convenience of client code.
        //      No functional change, we just need to persist these new flags.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VersionFmt        = 2;


        // -----------------------------------------------------------------------
        //  Persistent format version for the keyed standard command src
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2KeyedVersionFmt   = 1;
    }
}


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TBasicCmdSrc,TObject)
RTTIDecls(TCQCStdCmdSrc,TObject)
RTTIDecls(TCQCStdKeyedCmdSrc,TCQCStdCmdSrc)



// ---------------------------------------------------------------------------
//   CLASS: TBasicCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBasicCmdSrc: Constructors and Destructor
// ---------------------------------------------------------------------------
TBasicCmdSrc::TBasicCmdSrc()
{
}

TBasicCmdSrc::TBasicCmdSrc(const TBasicCmdSrc& csrcSrc) :

    MCQCCmdSrcIntf(csrcSrc)
{
}

TBasicCmdSrc::~TBasicCmdSrc()
{
}


// ---------------------------------------------------------------------------
//  TBasicCmdSrc: Public operators
// ---------------------------------------------------------------------------
TBasicCmdSrc& TBasicCmdSrc::operator=(const  TBasicCmdSrc& csrcSrc)
{
    MCQCCmdSrcIntf::operator=(csrcSrc);
    return *this;
}

tCIDLib::TBoolean TBasicCmdSrc::operator==(const TBasicCmdSrc& csrcSrc) const
{
    return MCQCCmdSrcIntf::bSameCmds(csrcSrc);
}

tCIDLib::TBoolean TBasicCmdSrc::operator!=(const TBasicCmdSrc& csrcSrc) const
{
    return !MCQCCmdSrcIntf::bSameCmds(csrcSrc);
}


// ---------------------------------------------------------------------------
//  TBasicCmdSrc: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TBasicCmdSrc::StreamFrom(TBinInStream& strmToReadFrom)
{
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);
}

tCIDLib::TVoid TBasicCmdSrc::StreamTo(TBinOutStream& strmToWriteTo) const
{
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCStdCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdCmdSrc: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStdCmdSrc::TCQCStdCmdSrc() :

    m_bNoGUI(kCIDLib::False)
    , m_bNoPropogate(kCIDLib::False)
    , m_bUsed(kCIDLib::False)
    , m_c4UserId(kCIDLib::c4MaxCard)
{
}

TCQCStdCmdSrc::TCQCStdCmdSrc(const TString& strTitle) :

    m_bNoGUI(kCIDLib::False)
    , m_bNoPropogate(kCIDLib::False)
    , m_bUsed(kCIDLib::False)
    , m_c4UserId(kCIDLib::c4MaxCard)
    , m_strTitle(strTitle)
{
}

TCQCStdCmdSrc::~TCQCStdCmdSrc()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdCmdSrc: Public operators
// ---------------------------------------------------------------------------

// User id is not considered here!
tCIDLib::TBoolean TCQCStdCmdSrc::operator==(const TCQCStdCmdSrc& csrcSrc) const
{
    if (this == &csrcSrc)
        return kCIDLib::True;

    // Check the easy stuff first
    if ((m_bNoGUI       != csrcSrc.m_bNoGUI)
    ||  (m_bNoPropogate != csrcSrc.m_bNoPropogate)
    ||  (m_bUsed        != csrcSrc.m_bUsed)
    ||  (m_strTitle     != csrcSrc.m_strTitle))
    {
        return kCIDLib::False;
    }

    // That's all ok, so compare the opcodes
    return MCQCCmdSrcIntf::bSameCmds(csrcSrc);
}

tCIDLib::TBoolean TCQCStdCmdSrc::operator!=(const TCQCStdCmdSrc& csrcSrc) const
{
    return !operator==(csrcSrc);
}



// ---------------------------------------------------------------------------
//  TCQCStdCmdSrc: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the no GUI flag
tCIDLib::TBoolean TCQCStdCmdSrc::bNoGUI() const
{
    return m_bNoGUI;
}

tCIDLib::TBoolean TCQCStdCmdSrc::bNoGUI(const tCIDLib::TBoolean bToSet)
{
    m_bNoGUI = bToSet;
    return m_bNoGUI;
}


// Get/set the no propogate flag
tCIDLib::TBoolean TCQCStdCmdSrc::bNoPropogate() const
{
    return m_bNoPropogate;
}

tCIDLib::TBoolean TCQCStdCmdSrc::bNoPropogate(const tCIDLib::TBoolean bToSet)
{
    m_bNoPropogate = bToSet;
    return m_bNoPropogate;
}


//
//  Get/ set the used flag. If it is cleared, then all opcodes are removed
//  from the source.
//
tCIDLib::TBoolean TCQCStdCmdSrc::bUsed() const
{
    return m_bUsed;
}

tCIDLib::TBoolean TCQCStdCmdSrc::bUsed(const tCIDLib::TBoolean bToSet)
{
    // If they are setting it false, then remove any opcodes
    if (!bToSet)
        ResetAllOps(kCIDLib::False);
    m_bUsed = bToSet;
    return m_bUsed;
}


// Get/set the user id
tCIDLib::TCard4 TCQCStdCmdSrc::c4UserId() const
{
    return m_c4UserId;
}

tCIDLib::TCard4 TCQCStdCmdSrc::c4UserId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4UserId = c4ToSet;
    return m_c4UserId;
}


// Get/set the title string
const TString& TCQCStdCmdSrc::strTitle() const
{
    return m_strTitle;
}

const TString& TCQCStdCmdSrc::strTitle(const TString& strToSet)
{
    m_strTitle = strToSet;
    return m_strTitle;
}


// ---------------------------------------------------------------------------
//  TCQCStdCmdSrc: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCStdCmdSrc::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_StdCmdSrc::c2VersionFmt))
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

    // Do the mixin class
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Looks ok, so stream in our stuff
    strmToReadFrom  >> m_bUsed
                    >> m_strTitle;

    // If V1, default the V2 stuff, else read it in
    if (c2FmtVersion == 1)
    {
        m_bNoGUI = kCIDLib::False;
        m_bNoPropogate = kCIDLib::False;
    }
     else
    {
        strmToReadFrom >> m_bNoGUI >> m_bNoPropogate;
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset runtime only stuff
    m_c4UserId = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid TCQCStdCmdSrc::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_StdCmdSrc::c2VersionFmt;

    // Do the mixin class
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    strmToWriteTo   << m_bUsed
                    << m_strTitle
                    << m_bNoGUI
                    << m_bNoPropogate
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCStdKeyedCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdKeyedCmdSrc: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCStdKeyedCmdSrc::strKey(const TCQCStdKeyedCmdSrc& csrcSrc)
{
    return csrcSrc.m_strId;
}


// ---------------------------------------------------------------------------
//  TCQCStdKeyedCmdSrc: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStdKeyedCmdSrc::TCQCStdKeyedCmdSrc()
{
}

TCQCStdKeyedCmdSrc::TCQCStdKeyedCmdSrc( const   TString& strTitle
                                        , const TString& strId) :
    TCQCStdCmdSrc(strTitle)
    , m_strId(strId)
{
}

TCQCStdKeyedCmdSrc::~TCQCStdKeyedCmdSrc()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdKeyedCmdSrc: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCStdKeyedCmdSrc::operator==(const TCQCStdKeyedCmdSrc& csrcSrc) const
{
    if (this == &csrcSrc)
        return kCIDLib::True;

    // Check our parent first
    if (!TParent::operator==(csrcSrc))
        return kCIDLib::False;

    // Good so far so compare our stuff
    return (m_strId == csrcSrc.m_strId);
}

tCIDLib::TBoolean TCQCStdKeyedCmdSrc::operator!=(const TCQCStdKeyedCmdSrc& csrcSrc) const
{
    return !operator==(csrcSrc);
}


// ---------------------------------------------------------------------------
//  TCQCStdKeyedCmdSrc: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TCQCStdKeyedCmdSrc::strId() const
{
    return m_strId;
}

const TString& TCQCStdKeyedCmdSrc::strId(const TString& strToSet)
{
    m_strId = strToSet;
    return m_strId;
}


// ---------------------------------------------------------------------------
//  TCQCStdKeyedCmdSrc: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCStdKeyedCmdSrc::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_StdCmdSrc::c2KeyedVersionFmt)
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

    // Looks ok, so stream in our stuff
    strmToReadFrom  >> m_strId;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCStdKeyedCmdSrc::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_StdCmdSrc::c2KeyedVersionFmt;

    // Call our parent
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff
    strmToWriteTo   << m_strId
                    << tCIDLib::EStreamMarkers::EndObject;
}


