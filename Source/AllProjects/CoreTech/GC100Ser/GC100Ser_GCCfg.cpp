//
// FILE NAME: GC100Ser_GCCfg.cpp
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
//  This class implements the GC-100 configuration class
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
#include "GC100Ser_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGC100Cfg,TObject)
RTTIDecls(TGC100CfgList,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GC100Ser_GCCfg
{
    // -----------------------------------------------------------------------
    //  Our current persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2       c2FmtVersion        = 1;
    const tCIDLib::TCard2       c2ListFmtVersion    = 1;
}



// ---------------------------------------------------------------------------
//  CLASS: TGC100Cfg
// PREFIX: gccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100Cfg: Public, static methods
// ---------------------------------------------------------------------------

// Support sorting by name
tCIDLib::ESortComps
TGC100Cfg::eCompByName( const   TGC100Cfg&      gccfg1
                        , const TGC100Cfg&      gccfg2)
{
    return gccfg1.m_strName.eCompare(gccfg2.m_strName);
}


const TString& TGC100Cfg::strName(const TGC100Cfg& gccfgSrc)
{
    return gccfgSrc.m_strName;
}


// ---------------------------------------------------------------------------
//  TGC100Cfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TGC100Cfg::TGC100Cfg() :

    m_c4Ports(0)
{
}

TGC100Cfg::TGC100Cfg(const  TString&            strName
                    , const TString&            strAddr
                    , const tCIDLib::TCard4     c4Ports) :

    m_c4Ports(c4Ports)
    , m_strAddr(strAddr)
    , m_strName(strName)
{
}

TGC100Cfg::~TGC100Cfg()
{
}


// ---------------------------------------------------------------------------
//  TGC100Cfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGC100Cfg::operator==(const TGC100Cfg& gccfgSrc) const
{
    return (m_c4Ports == gccfgSrc.m_c4Ports)
            && (m_strAddr == gccfgSrc.m_strAddr)
            && (m_strName == gccfgSrc.m_strName);
}

tCIDLib::TBoolean TGC100Cfg::operator!=(const TGC100Cfg& gccfgSrc) const
{
    return !operator==(gccfgSrc);
}


// ---------------------------------------------------------------------------
//  TGC100Cfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TCard4 TGC100Cfg::c4Ports() const
{
    return m_c4Ports;
}


const TString& TGC100Cfg::strAddr() const
{
    return m_strAddr;
}


const TString& TGC100Cfg::strName() const
{
    return m_strName;
}


tCIDLib::TVoid
TGC100Cfg::Set( const   TString&        strName
                , const TString&        strAddr
                , const tCIDLib::TCard4 c4Ports)
{
    m_c4Ports = c4Ports;
    m_strAddr = strAddr;
    m_strName = strName;
}


// ---------------------------------------------------------------------------
//  TGC100Cfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGC100Cfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > GC100Ser_GCCfg::c2FmtVersion))
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


tCIDLib::TVoid TGC100Cfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << GC100Ser_GCCfg::c2FmtVersion
                    << m_strName
                    << m_strAddr
                    << m_c4Ports
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//  CLASS: TGC100CfgList
// PREFIX: gccl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100CfgList: Constructors and Destructor
// ---------------------------------------------------------------------------
TGC100CfgList::TGC100CfgList()
{
}

TGC100CfgList::~TGC100CfgList()
{
}


// ---------------------------------------------------------------------------
//  TGC100CfgList: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGC100CfgList::operator==(const TGC100CfgList& gcclSrc) const
{
    if (this != &gcclSrc)
    {
        const tCIDLib::TCard4 c4Count = m_colCfgList.c4ElemCount();

        // If a different count, then obviously not equal
        if (c4Count != gcclSrc.m_colCfgList.c4ElemCount())
            return kCIDLib::False;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_colCfgList[c4Index] != gcclSrc.m_colCfgList[c4Index])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TGC100CfgList::operator!=(const TGC100CfgList& gcclSrc) const
{
    return !operator==(gcclSrc);
}


// ---------------------------------------------------------------------------
//  TGC100CfgList: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add a new one to the end of the list and return the new count
tCIDLib::TCard4 TGC100CfgList::c4AddNew(const TGC100Cfg& gccfgNew)
{
    m_colCfgList.objAdd(gccfgNew);
    return m_colCfgList.c4ElemCount();
}


tCIDLib::TCard4 TGC100CfgList::c4Count() const
{
    return m_colCfgList.c4ElemCount();
}


const TGC100Cfg& TGC100CfgList::gccfgAt(const tCIDLib::TCard4 c4At) const
{
    return m_colCfgList[c4At];
}


tCIDLib::TVoid TGC100CfgList::RemoveAll()
{
    m_colCfgList.RemoveAll();
}


tCIDLib::TVoid TGC100CfgList::RemoveAt(const tCIDLib::TCard4 c4At)
{
    m_colCfgList.RemoveAt(c4At);
}


tCIDLib::TVoid
TGC100CfgList::SetAt(const tCIDLib::TCard4 c4At, const TGC100Cfg& gccfgToSet)
{
    m_colCfgList[c4At] = gccfgToSet;
}


tCIDLib::TVoid TGC100CfgList::SortList()
{
    m_colCfgList.Sort(&TGC100Cfg::eCompByName);
}


// ---------------------------------------------------------------------------
//  TGC100CfgList: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGC100CfgList::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > GC100Ser_GCCfg::c2ListFmtVersion))
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
        facGC100Ser().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGC100Errs::errcCfg_InvalidCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Remove any we currently have and start reading in the new ones
    m_colCfgList.RemoveAll();
    TGC100Cfg gccfgCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToReadFrom >> gccfgCur;
        m_colCfgList.objAdd(gccfgCur);

        // We separate them by a frame marker
        strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    }

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TGC100CfgList::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << GC100Ser_GCCfg::c2ListFmtVersion;

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

