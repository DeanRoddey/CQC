//
// FILE NAME: CQCKit_SecurityToken.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2002
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
//  This is the implementation file for the security token class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSecToken,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_SecurityToken
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2 -
        //      Removed the old marker hash
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSecToken
//  PREFIX: sect
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSecToken: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSecToken::TCQCSecToken() :

    m_c4Count(0)
    , m_mbufData()
{
}

TCQCSecToken::~TCQCSecToken()
{
}


// ---------------------------------------------------------------------------
//  TCQCSecToken: Public, non-virtual methods
// ---------------------------------------------------------------------------
const THeapBuf& TCQCSecToken::mbufData() const
{
    return m_mbufData;
}



tCIDLib::TVoid TCQCSecToken::Reset()
{
    m_c4Count = 0;
    m_mbufData.Set(0);
}


tCIDLib::TVoid TCQCSecToken::Set(const  TMemBuf&                mbufToSet
                                , const tCIDLib::TCard4         c4Bytes)
{
    // Make sure the buffer has at least the bytes they indicated
    if (mbufToSet.c4Size() < c4Bytes)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_BadTokenBufSize
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , TCardinal(mbufToSet.c4Size())
            , TCardinal(c4Bytes)
        );
    }

    //
    //  Looks ok, so store the new data. We let them pass in any kind of
    //  buffer, so we can't just assign the buffer. We have to copy in the
    //  bytes.
    //
    m_c4Count = c4Bytes;
    m_mbufData.CopyIn(mbufToSet, c4Bytes);
}


// ---------------------------------------------------------------------------
//  TCQCSecToken: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSecToken::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_SecurityToken::c2FmtVersion)
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

    //
    //  Looks reasonable to stream in the fields. If V1 we read in the old hash
    //  and discard it.
    //
    if (c2FmtVersion < 2)
    {
        TMD5Hash mhashTmp;
        strmToReadFrom  >> mhashTmp;
    }

    strmToReadFrom  >> m_c4Count
                    >> m_mbufData;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSecToken::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_SecurityToken::c2FmtVersion

                    // V1 stuff
                    << m_c4Count
                    << m_mbufData

                    << tCIDLib::EStreamMarkers::EndObject;
}


