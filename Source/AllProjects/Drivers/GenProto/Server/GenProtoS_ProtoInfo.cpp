//
// FILE NAME: GenProtoS_ProtoInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements the protocol information storage class, which is used
//  to store and hold at runtime the basic information about the protocol
//  at large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GenProtoS_ProtoInfo
{
    // Our current persistent format version
    const   tCIDLib::TCard2     c2FmtVersion = 1;
}


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoInfo
//  PREFIX: pinfo
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoInfo::TGenProtoInfo() :

    m_bLittleEndian(kCIDLib::False)
    , m_c4EndDelimCount(1)
    , m_c4StartDelimCount(0)
    , m_c4LenOfs(0)
    , m_eDelim(tGenProtoS::EMsgDelims::Term)
    , m_eFormat(tGenProtoS::EProtoFmts::ASCII)
{
    m_ac1StartDelim[0] = 0;
    m_ac1TermDelim[0] = 0x13;
}

TGenProtoInfo::TGenProtoInfo(const TGenProtoInfo& gpinfoToCopy) :

    m_bLittleEndian(gpinfoToCopy.m_bLittleEndian)
    , m_c4EndDelimCount(gpinfoToCopy.m_c4EndDelimCount)
    , m_c4StartDelimCount(gpinfoToCopy.m_c4StartDelimCount)
    , m_c4LenOfs(gpinfoToCopy.m_c4LenOfs)
    , m_eDelim(gpinfoToCopy.m_eDelim)
    , m_eFormat(gpinfoToCopy.m_eFormat)
{
    // Just copy the whole things instead of what's indicated
    TRawMem::CopyMemBuf
    (
        m_ac1StartDelim
        , gpinfoToCopy.m_ac1StartDelim
        , kGenProtoS::c4MaxSeqBytes
    );

    TRawMem::CopyMemBuf
    (
        m_ac1TermDelim
        , gpinfoToCopy.m_ac1TermDelim
        , kGenProtoS::c4MaxSeqBytes
    );
}

TGenProtoInfo::~TGenProtoInfo()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoInfo: Public operators
// ---------------------------------------------------------------------------
TGenProtoInfo& TGenProtoInfo::operator=(const TGenProtoInfo& gpinfoToAssign)
{
    if (this != &gpinfoToAssign)
    {
        m_bLittleEndian     = gpinfoToAssign.m_bLittleEndian;
        m_c4EndDelimCount   = gpinfoToAssign.m_c4EndDelimCount;
        m_c4StartDelimCount = gpinfoToAssign.m_c4StartDelimCount;
        m_c4LenOfs          = gpinfoToAssign.m_c4LenOfs;
        m_eDelim            = gpinfoToAssign.m_eDelim;
        m_eFormat           = gpinfoToAssign.m_eFormat;

        // Just copy the whole things instead of what's indicated
        TRawMem::CopyMemBuf
        (
            m_ac1StartDelim
            , gpinfoToAssign.m_ac1StartDelim
            , kGenProtoS::c4MaxSeqBytes
        );

        TRawMem::CopyMemBuf
        (
            m_ac1TermDelim
            , gpinfoToAssign.m_ac1TermDelim
            , kGenProtoS::c4MaxSeqBytes
        );
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
const tCIDLib::TCard1* TGenProtoInfo::ac1StartDelim() const
{
    return m_ac1StartDelim;
}


const tCIDLib::TCard1* TGenProtoInfo::ac1TermDelim() const
{
    return m_ac1TermDelim;
}


tCIDLib::TBoolean TGenProtoInfo::bLittleEndian() const
{
    return m_bLittleEndian;
}


tCIDLib::TCard4 TGenProtoInfo::c4EndDelimCount() const
{
    return m_c4EndDelimCount;
}


tCIDLib::TCard4 TGenProtoInfo::c4StartDelimCount() const
{
    return m_c4StartDelimCount;
}


tCIDLib::TCard4 TGenProtoInfo::c4LenOfs() const
{
    return m_c4LenOfs;
}


tGenProtoS::EMsgDelims TGenProtoInfo::eDelim() const
{
    return m_eDelim;
}


tGenProtoS::EProtoFmts TGenProtoInfo::eFormat() const
{
    return m_eFormat;
}


// ---------------------------------------------------------------------------
//  TGenProtoInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (c2FmtVersion != GenProtoS_ProtoInfo::c2FmtVersion)
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

    strmToReadFrom  >> m_bLittleEndian
                    >> m_c4EndDelimCount
                    >> m_c4StartDelimCount
                    >> m_c4LenOfs
                    >> m_eDelim
                    >> m_eFormat;

    strmToReadFrom.ReadArray(m_ac1StartDelim, kGenProtoS::c4MaxSeqBytes);
    strmToReadFrom.ReadArray(m_ac1TermDelim, kGenProtoS::c4MaxSeqBytes);
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TGenProtoInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << GenProtoS_ProtoInfo::c2FmtVersion
                    << m_bLittleEndian
                    << m_c4EndDelimCount
                    << m_c4StartDelimCount
                    << m_c4LenOfs
                    << m_eDelim
                    << m_eFormat;

    strmToWriteTo.WriteArray(m_ac1StartDelim, kGenProtoS::c4MaxSeqBytes);
    strmToWriteTo.WriteArray(m_ac1TermDelim, kGenProtoS::c4MaxSeqBytes);
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


