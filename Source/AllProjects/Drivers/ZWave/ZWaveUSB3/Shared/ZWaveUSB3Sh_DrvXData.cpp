//
// FILE NAME: ZWaveUSB3Sh_DriverXData.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/17/2017
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
//  The implementation for our small Z-Wave class information class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWDriverXData, TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_ZWDriverXData
{
    const tCIDLib::TCard2 c2FmtVersion = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWDriverXData
//  PREFIX: zwdxd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWDriverXData: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWDriverXData::TZWDriverXData()
{
    Reset();
}

//
//  When we first find out about a class being supported via discovery, this is
//  all we know.
//
TZWDriverXData::TZWDriverXData(const TZWDriverXData& zwdxdSrc) :

    m_bAwake(zwdxdSrc.m_bAwake)
    , m_bFlag1(zwdxdSrc.m_bFlag1)
    , m_bStatus(zwdxdSrc.m_bStatus)
    , m_c1EPId(zwdxdSrc.m_c1EPId)
    , m_c1UnitId(zwdxdSrc.m_c1UnitId)
    , m_c1Val1(zwdxdSrc.m_c1Val1)
    , m_c1Val2(zwdxdSrc.m_c1Val2)
    , m_c4Val1(zwdxdSrc.m_c4Val1)
    , m_c4BufSz(zwdxdSrc.m_c4BufSz)
    , m_i4Val1(zwdxdSrc.m_i4Val1)
    , m_mbufData(zwdxdSrc.m_mbufData)
    , m_strCmd(zwdxdSrc.m_strCmd)
    , m_strMsg(zwdxdSrc.m_strMsg)
{
}

TZWDriverXData::~TZWDriverXData()
{
}


// ---------------------------------------------------------------------------
//  TZWDriverXData: Public operators
// ---------------------------------------------------------------------------
TZWDriverXData& TZWDriverXData::operator=(const TZWDriverXData& zwdxdSrc)
{
    if (&zwdxdSrc != this)
    {
        m_bAwake        = zwdxdSrc.m_bAwake;
        m_bFlag1        = zwdxdSrc.m_bFlag1;
        m_bStatus       = zwdxdSrc.m_bStatus;
        m_c1EPId        = zwdxdSrc.m_c1EPId;
        m_c1UnitId      = zwdxdSrc.m_c1UnitId;
        m_c1Val1        = zwdxdSrc.m_c1Val1;
        m_c1Val2        = zwdxdSrc.m_c1Val2;
        m_c4Val1        = zwdxdSrc.m_c4Val1;
        m_c4BufSz       = zwdxdSrc.m_c4BufSz;
        m_i4Val1        = zwdxdSrc.m_i4Val1;
        m_mbufData      = zwdxdSrc.m_mbufData;
        m_strCmd        = zwdxdSrc.m_strCmd;
        m_strMsg        = zwdxdSrc.m_strMsg;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWDriverXData: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Full resets this object
tCIDLib::TVoid TZWDriverXData::Reset()
{
    m_bAwake    = kCIDLib::False;
    m_bFlag1    = kCIDLib::False;
    m_c1EPId    = 0xFF;
    m_c1UnitId  = 0;
    m_c1Val1    = 0;
    m_c1Val2    = 0;
    m_c4Val1    = 0;
    m_c4BufSz   = 0;
    m_i4Val1    = 0;

    m_strCmd.Clear();
    m_strMsg.Clear();

    //
    //  This is typically used to send from the client, so we need to set the status
    //  to true so that all the members get sent.
    //
    m_bStatus   = kCIDLib::True;
}


//
//  Mostly for the server side to let it just reset the object it streamed in and
//  then set it up for the return. We don't reset the command, assuming that it is
//  already set to what it should be and it is always returned in replies as a
//  sanity check.
//
//  We have a couple convenience ones that will format info into the msg. And one
//  that just returns a result
//
tCIDLib::TVoid TZWDriverXData::StatusReset(const tCIDLib::TBoolean bStatus)
{
    // Save the command, reset and then put it back
    TString strSaveCmd = m_strCmd;
    Reset();
    m_strCmd = strSaveCmd;
    m_bStatus = bStatus;
}


tCIDLib::TVoid
TZWDriverXData::StatusReset(const tCIDLib::TBoolean bStatus, const TString& strMsg)
{
    // Save the command, reset and then put it back
    TString strSaveCmd = m_strCmd;
    Reset();
    m_strCmd = strSaveCmd;
    m_bStatus = bStatus;
    m_strMsg = strMsg;
}

tCIDLib::TVoid
TZWDriverXData::StatusReset(const   tCIDLib::TBoolean   bStatus
                            , const TString&            strMsg1
                            , const MFormattable&       mfmtblToken1
                            , const MFormattable&       mfmtblToken2
                            , const MFormattable&       mfmtblToken3)
{
    // Save the command, reset and then put it back
    TString strSaveCmd = m_strCmd;
    Reset();
    m_strCmd = strSaveCmd;
    m_bStatus = bStatus;

    m_strMsg = strMsg1;

    if (!MFormattable::bIsNullObject(mfmtblToken1))
        m_strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit1);

    if (!MFormattable::bIsNullObject(mfmtblToken2))
        m_strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit2);

    if (!MFormattable::bIsNullObject(mfmtblToken3))
        m_strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit3);
}



// ---------------------------------------------------------------------------
//  TZWDriverXData: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWDriverXData::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_ZWDriverXData::c2FmtVersion))
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
    //  Reset us and then read in the command, status flag, and msg. The command is
    //  always read, since it's returned as a sanity check in replies.
    //
    Reset();
    strmToReadFrom  >> m_strCmd
                    >> m_bStatus
                    >> m_strMsg;

    // If successful we have to read in the other stuff
    if (m_bStatus)
    {
        strmToReadFrom  >> m_bAwake
                        >> m_bFlag1
                        >> m_c1EPId
                        >> m_c1UnitId
                        >> m_c1Val1
                        >> m_c1Val2
                        >> m_c4Val1
                        >> m_c4BufSz
                        >> m_i4Val1;


        // If any buffer bytes, read those
        tCIDLib::TCard4 c4XorCnt;
        strmToReadFrom >> m_c4BufSz >> c4XorCnt;
        if ((c4XorCnt ^ kCIDLib::c4MaxCard) != m_c4BufSz)
        {
            // <TBD> Something is awry

            m_c4BufSz = 0;
        }

        // Read in that many bytes
        if (m_c4BufSz)
        {
            if (m_mbufData.c4Size() < m_c4BufSz)
                m_mbufData.Reallocate(m_c4BufSz, kCIDLib::False);
            strmToReadFrom.c4ReadBuffer(m_mbufData, m_c4BufSz);
        }
    }


    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWDriverXData::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_ZWDriverXData::c2FmtVersion;

    strmToWriteTo << m_strCmd << m_bStatus << m_strMsg;

    // If successful we have to stream the other stuff
    if (m_bStatus)
    {
        strmToWriteTo   << m_bAwake
                        << m_bFlag1
                        << m_c1EPId
                        << m_c1UnitId
                        << m_c1Val1
                        << m_c1Val2
                        << m_c4Val1
                        << m_c4BufSz
                        << m_i4Val1;

        // Write out the data buffer stuff if any
        strmToWriteTo   << m_c4BufSz
                        << tCIDLib::TCard4(m_c4BufSz ^ kCIDLib::c4MaxCard);
        if (m_c4BufSz)
            strmToWriteTo.c4WriteBuffer(m_mbufData, m_c4BufSz);
    }

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}
