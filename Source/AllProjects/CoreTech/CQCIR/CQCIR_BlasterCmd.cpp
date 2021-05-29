//
// FILE NAME: CQCIR_BlasterCmd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This class implements the IR command object, which holds the info for
//  a single command.
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
#include "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRBlasterCmd,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIR_BlasterCmd
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterCmd
//  PREFIX: irbc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRBlasterCmd: Public, static methods
// ---------------------------------------------------------------------------
const TString& TIRBlasterCmd::strKey(const TIRBlasterCmd& irbcSrc)
{
    return irbcSrc.m_strName;
}


// ---------------------------------------------------------------------------
//  TIRBlasterCmd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRBlasterCmd::TIRBlasterCmd() :

    m_c4DataLen(0)
{
}

TIRBlasterCmd::TIRBlasterCmd(const TString& strName) :

    m_c4DataLen(0)
    , m_mbufData(64UL)
    , m_strName(strName)
{
}

TIRBlasterCmd::TIRBlasterCmd(const  TString&        strName
                            , const TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4Len) :

    m_c4DataLen(c4Len)
    , m_mbufData(c4Len)
    , m_strName(strName)
{
    m_mbufData.CopyIn(mbufData, c4Len);
}

TIRBlasterCmd::TIRBlasterCmd(const  TString&                strName
                            , const tCIDLib::TSCh* const    pchData) :

    m_c4DataLen(TRawStr::c4StrLen(pchData))
    , m_mbufData((tCIDLib::TCard1*)pchData, m_c4DataLen)
    , m_strName(strName)
{
}

TIRBlasterCmd::~TIRBlasterCmd()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterCmd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TIRBlasterCmd::c4DataLen() const
{
    return m_c4DataLen;
}


tCIDLib::TVoid TIRBlasterCmd::FormatData(TString& strToFill) const
{
    strToFill.Clear();

    //
    //  We can do a trivial cast to a wide char, since the data is all
    //  simple ASCII data.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4DataLen; c4Index++)
        strToFill.Append(tCIDLib::TCh(m_mbufData[c4Index]));
}


const TMemBuf& TIRBlasterCmd::mbufData() const
{
    return m_mbufData;
}


const TString& TIRBlasterCmd::strName() const
{
    return m_strName;
}

const TString& TIRBlasterCmd::strName(const TString& strNew)
{
    m_strName = strNew;
    return m_strName;
}


tCIDLib::TVoid
TIRBlasterCmd::Set(const TString& strName, const TString& strData)
{
    //
    //  The name we take as is. The data is assumed to be convertable to
    //  ASCII, so we do a cheap cast to bytes to put it into the buffer.
    //
    m_strName = strName;

    // Do some basic sanity checks
    if (strData.bIsEmpty())
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_BadDataFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strName
        );
    }

    const tCIDLib::TCard4 c4Count = strData.c4Length();
    for (m_c4DataLen = 0; m_c4DataLen < c4Count; m_c4DataLen++)
    {
        const tCIDLib::TCh chCur = strData[m_c4DataLen];

        // Make sure it's really convertable
        if (chCur > 127)
        {
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcImp_BadDataFmt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , m_strName
            );
        }
        m_mbufData.PutCard1(tCIDLib::TCard1(chCur), m_c4DataLen);
    }
}


tCIDLib::TVoid
TIRBlasterCmd::SetData(const TMemBuf& mbufData, const tCIDLib::TCard4 c4Len)
{
    m_c4DataLen = c4Len;
    m_mbufData.CopyIn(mbufData, c4Len);
}


// ---------------------------------------------------------------------------
//  TIRBlasterCmd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterCmd::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIR_BlasterCmd::c2FmtVersion)
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

    // Pull in the name
    strmToReadFrom  >>  m_strName;

    //
    //  Now get the data into our buffer. We should next get the bytes
    //  and an XOR'd version of it. If it checks out, read in the indicated
    //  number of bytes.
    //
    tCIDLib::TCard4 c4XLen;
    strmToReadFrom >> m_c4DataLen >> c4XLen;
    if ((m_c4DataLen ^ kCIDLib::c4MaxCard) != c4XLen)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_BadCmdByteCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    strmToReadFrom.c4ReadBuffer(m_mbufData, m_c4DataLen);

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TIRBlasterCmd::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIR_BlasterCmd::c2FmtVersion
                    << m_strName;

    //
    //  We don't just stream out the buffer, since some of its allocation
    //  size might not be used. So we just write out the bytes actually
    //  used. This is byte data, so endianness is not an issue. We write
    //  out a XOR'd version of the bytes, to catch errors during streaming
    //  it back in.
    //
    strmToWriteTo   << m_c4DataLen
                    << tCIDLib::TCard4(m_c4DataLen ^ kCIDLib::c4MaxCard);
    strmToWriteTo.c4WriteBuffer(m_mbufData, m_c4DataLen);
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


