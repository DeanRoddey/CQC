//
// FILE NAME: CQCIR_BlasterDevModelInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/30/2002
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
//  This class implements the IR device header info class, that holds the non-
//  IR data part of the device information. Clients get a collection of these
//  when they ask for a list of available devices.
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
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRBlasterDevModelInfo,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_BlasterDevModelInfo
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterDevModelInfo
//  PREFIX: irbdmi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRBlasterDevModelInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString&
TIRBlasterDevModelInfo::strKey(const TIRBlasterDevModelInfo& irbdmiSrc)
{
    return irbdmiSrc.m_strModel;
}


// ---------------------------------------------------------------------------
//  TIRBlasterDevModelInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRBlasterDevModelInfo::TIRBlasterDevModelInfo() :

    m_eCategory(tCQCKit::EDevCats::Miscellaneous)
{
}

TIRBlasterDevModelInfo::
TIRBlasterDevModelInfo( const   TString&            strModel
                        , const TString&            strMake
                        , const TString&            strDescr
                        , const TString&            strDataFile
                        , const tCQCKit::EDevCats   eCategory) :

    m_eCategory(eCategory)
    , m_strDataFile(strDataFile)
    , m_strDescr(strDescr)
    , m_strMake(strMake)
    , m_strModel(strModel)
{
}

TIRBlasterDevModelInfo::
TIRBlasterDevModelInfo( const   TString&            strDataFile
                        , const TIRBlasterDevModel& irbdmSrc) :

    m_eCategory(irbdmSrc.eCategory())
    , m_strDataFile(strDataFile)
    , m_strDescr(irbdmSrc.strDescription())
    , m_strMake(irbdmSrc.strMake())
    , m_strModel(irbdmSrc.strModel())
{
}

TIRBlasterDevModelInfo::~TIRBlasterDevModelInfo()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterDevModelInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EDevCats TIRBlasterDevModelInfo::eCategory() const
{
    return m_eCategory;
}

tCQCKit::EDevCats
TIRBlasterDevModelInfo::eCategory(const tCQCKit::EDevCats eToSet)
{
    m_eCategory = eToSet;
    return m_eCategory;
}


const TString& TIRBlasterDevModelInfo::strDataFile() const
{
    return m_strDataFile;
}

const TString& TIRBlasterDevModelInfo::strDataFile(const TString& strToSet)
{
    m_strDataFile = strToSet;
    return m_strDataFile;
}


const TString& TIRBlasterDevModelInfo::strDescription() const
{
    return m_strDescr;
}

const TString& TIRBlasterDevModelInfo::strDescription(const TString& strToSet)
{
    m_strDescr = strToSet;
    return m_strDescr;
}


const TString& TIRBlasterDevModelInfo::strMake() const
{
    return m_strMake;
}

const TString& TIRBlasterDevModelInfo::strMake(const TString& strToSet)
{
    m_strMake = strToSet;
    return m_strMake;
}


const TString& TIRBlasterDevModelInfo::strModel() const
{
    return m_strModel;
}

const TString& TIRBlasterDevModelInfo::strModel(const TString& strToSet)
{
    m_strModel = strToSet;
    return m_strModel;
}


tCIDLib::TVoid
TIRBlasterDevModelInfo::UpdateFrom(const TIRBlasterDevModel& irbdmSrc)
{
    m_eCategory = irbdmSrc.eCategory();
    m_strDescr  = irbdmSrc.strDescription();
    m_strMake   = irbdmSrc.strMake();
    m_strModel  = irbdmSrc.strModel();
}



// ---------------------------------------------------------------------------
//  TIRBlasterDevModelInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterDevModelInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_BlasterDevModelInfo::c2FmtVersion)
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

    strmToReadFrom  >> m_strModel
                    >> m_strMake
                    >> m_strDescr
                    >> m_strDataFile
                    >> m_eCategory;

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TIRBlasterDevModelInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_BlasterDevModelInfo::c2FmtVersion
                    << m_strModel
                    << m_strMake
                    << m_strDescr
                    << m_strDataFile
                    << m_eCategory
                    << tCIDLib::EStreamMarkers::EndObject;
}


