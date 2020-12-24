//
// FILE NAME: JAPwrSer_PortCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This class implements the Just Add Power port configuration classes
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
#include "JAPwrSer_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TJAPwrCfg,TObject)
RTTIDecls(TJAPwrCfgList,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace JAPwrSer_PortCfg
{
    // -----------------------------------------------------------------------
    //  Our current persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2       c2FmtVersion        = 1;
    const tCIDLib::TCard2       c2ListFmtVersion    = 1;
}



// ---------------------------------------------------------------------------
//  CLASS: TJAPwrCfg
// PREFIX: japcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TJAPwrCfg: Public, static methods
// ---------------------------------------------------------------------------

// Support sorting by name
tCIDLib::ESortComps
TJAPwrCfg::eCompByName( const   TJAPwrCfg&      japcfg1
                        , const TJAPwrCfg&      japcfg2)
{
    return japcfg1.m_strName.eCompare(japcfg2.m_strName);
}


const TString& TJAPwrCfg::strName(const TJAPwrCfg& japcfgSrc)
{
    return japcfgSrc.m_strName;
}


// ---------------------------------------------------------------------------
//  TJAPwrCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TJAPwrCfg::TJAPwrCfg() :

    m_c4Ports(0)
{
}

TJAPwrCfg::TJAPwrCfg(const  TString&            strName
                    , const TString&            strAddr
                    , const tCIDLib::TCard4     c4Ports) :

    m_c4Ports(c4Ports)
    , m_strAddr(strAddr)
    , m_strName(strName)
{
}

TJAPwrCfg::~TJAPwrCfg()
{
}


// ---------------------------------------------------------------------------
//  TJAPwrCfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TJAPwrCfg::operator==(const TJAPwrCfg& japcfgSrc) const
{
    return (m_c4Ports == japcfgSrc.m_c4Ports)
            && (m_strAddr == japcfgSrc.m_strAddr)
            && (m_strName == japcfgSrc.m_strName);
}

tCIDLib::TBoolean TJAPwrCfg::operator!=(const TJAPwrCfg& japcfgSrc) const
{
    return !operator==(japcfgSrc);
}


// ---------------------------------------------------------------------------
//  TJAPwrCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TCard4 TJAPwrCfg::c4Ports() const
{
    return m_c4Ports;
}


const TString& TJAPwrCfg::strAddr() const
{
    return m_strAddr;
}


const TString& TJAPwrCfg::strName() const
{
    return m_strName;
}


tCIDLib::TVoid
TJAPwrCfg::Set( const   TString&        strName
                , const TString&        strAddr
                , const tCIDLib::TCard4 c4Ports)
{
    m_c4Ports = c4Ports;
    m_strAddr = strAddr;
    m_strName = strName;
}


// ---------------------------------------------------------------------------
//  TJAPwrCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TJAPwrCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > JAPwrSer_PortCfg::c2FmtVersion))
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

    // Pull in the info
    strmToReadFrom  >> m_strName
                    >> m_strAddr
                    >> m_c4Ports;

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TJAPwrCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << JAPwrSer_PortCfg::c2FmtVersion
                    << m_strName
                    << m_strAddr
                    << m_c4Ports
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//  CLASS: TJAPwrCfgList
// PREFIX: japl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TJAPwrCfgList: Constructors and Destructor
// ---------------------------------------------------------------------------
TJAPwrCfgList::TJAPwrCfgList()
{
}

TJAPwrCfgList::~TJAPwrCfgList()
{
}


// ---------------------------------------------------------------------------
//  TJAPwrCfgList: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TJAPwrCfgList::operator==(const TJAPwrCfgList& japlSrc) const
{
    if (this != &japlSrc)
    {
        const tCIDLib::TCard4 c4Count = m_colCfgList.c4ElemCount();

        // If a different count, then obviously not equal
        if (c4Count != japlSrc.m_colCfgList.c4ElemCount())
            return kCIDLib::False;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colCfgList[c4Index] != japlSrc.m_colCfgList[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TJAPwrCfgList::operator!=(const TJAPwrCfgList& japlSrc) const
{
    return !operator==(japlSrc);
}



// ---------------------------------------------------------------------------
//  TJAPwrCfgList: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add a new one to the end of the list and return the new count
tCIDLib::TCard4 TJAPwrCfgList::c4AddNew(const TJAPwrCfg& japcfgNew)
{
    m_colCfgList.objAdd(japcfgNew);
    return m_colCfgList.c4ElemCount();
}


tCIDLib::TCard4 TJAPwrCfgList::c4Count() const
{
    return m_colCfgList.c4ElemCount();
}


const TJAPwrCfg& TJAPwrCfgList::japcfgAt(const tCIDLib::TCard4 c4At) const
{
    return m_colCfgList[c4At];
}


tCIDLib::TVoid TJAPwrCfgList::RemoveAll()
{
    m_colCfgList.RemoveAll();
}


tCIDLib::TVoid TJAPwrCfgList::RemoveAt(const tCIDLib::TCard4 c4At)
{
    m_colCfgList.RemoveAt(c4At);
}


tCIDLib::TVoid
TJAPwrCfgList::SetAt(const tCIDLib::TCard4 c4At, const TJAPwrCfg& japcfgToSet)
{
    m_colCfgList[c4At] = japcfgToSet;
}


tCIDLib::TVoid TJAPwrCfgList::SortList()
{
    m_colCfgList.Sort(&TJAPwrCfg::eCompByName);
}


// ---------------------------------------------------------------------------
//  TJAPwrCfgList: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TJAPwrCfgList::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > JAPwrSer_PortCfg::c2ListFmtVersion))
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

    // Get the count and XOR'd count and test it
    tCIDLib::TCard4 c4Count, c4XORCount;
    strmToReadFrom  >> c4Count >> c4XORCount;

    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facJAPwrSer().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kJAPwrErrs::errcCfg_InvalidCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Remove any we currently have and start reading in the new ones
    m_colCfgList.RemoveAll();
    TJAPwrCfg japcfgCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToReadFrom >> japcfgCur;
        m_colCfgList.objAdd(japcfgCur);

        // We separate them by a frame marker
        strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    }

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TJAPwrCfgList::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << JAPwrSer_PortCfg::c2ListFmtVersion;

    const tCIDLib::TCard4 c4Count = m_colCfgList.c4ElemCount();
    strmToWriteTo   << c4Count
                    << tCIDLib::TCard4(c4Count ^ kCIDLib::c4MaxCard);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo << m_colCfgList[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}

