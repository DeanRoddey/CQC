//
// FILE NAME: CQCSysCfg_Parts.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/02/2014
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
//  This file implements the smaller classes that make up parts of the configuration
//  data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSysCfg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TSysCfgListItem,TObject)
AdvRTTIDecls(TSCGlobActInfo,TSysCfgListItem)
AdvRTTIDecls(TSCHVACInfo,TSysCfgListItem)
AdvRTTIDecls(TSCLoadInfo,TSysCfgListItem)
AdvRTTIDecls(TSCMediaInfo,TSysCfgListItem)
AdvRTTIDecls(TSCSecInfo,TSysCfgListItem)
AdvRTTIDecls(TSCWeatherInfo,TSysCfgListItem)
AdvRTTIDecls(TSCXOverInfo,TSysCfgListItem)


namespace
{
    namespace CQCSysCfg_Parts
    {
        // -----------------------------------------------------------------------
        //  The persistent format number for the base class.
        //
        //  2 - Add an 'alt name' value. It is used for voice control stuff.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtBaseInfo  = 2;


        // -----------------------------------------------------------------------
        //  The persistent format number for the load info class.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtLoadInfo  = 1;


        // -----------------------------------------------------------------------
        //  The persistent format number for the global action info class.
        //
        //  Version 2 -
        //      Convert the path to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtGActInfo  = 2;


        // -----------------------------------------------------------------------
        //  The persistent format number for the HVAC info class.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtHVACInfo  = 1;


        // -----------------------------------------------------------------------
        //  The persistent format number for the media info class.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtMediaInfo  = 1;


        // -----------------------------------------------------------------------
        //  The persistent format number for the security panel info class.
        //
        //  2 - Added an arming code for use with voice control
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtSecInfo  = 2;


        // -----------------------------------------------------------------------
        //  The persistent format number for the weather info class.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtWeatherInfo  = 1;


        // -----------------------------------------------------------------------
        //  The persistent format number for the exra overlays info class.
        //
        //  Version 2 -
        //      Forgot to move the title down to the base class. Probably no one has
        //      created any of these, but just in case we need to read in the title
        //      and push it to the base class then discard it.
        //
        //  Version 3 -
        //      Convert the template paths to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtXOverInfo  = 3;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TSysCfgListItem
//  PREFIX: scli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSysCfgListItem: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TSysCfgListItem::eCompByTitle(  const   TSysCfgListItem&    scliL
                                , const TSysCfgListItem&    scliR)
{
    return TString::eComp(scliL.m_strTitle, scliR.m_strTitle);
}

// ---------------------------------------------------------------------------
//  TSysCfgListItem: Constructor and Destructor
// ---------------------------------------------------------------------------
TSysCfgListItem::TSysCfgListItem() :

    m_strUniqueId(TUniqueId::strMakeId())
{
}

TSysCfgListItem::~TSysCfgListItem()
{
}


// ---------------------------------------------------------------------------
//  TSysCfgListItem: Public, inherited methods
// ---------------------------------------------------------------------------

// The edit id isn't part of equality, it's just for editing purposes
tCIDLib::TBoolean TSysCfgListItem::bCompare(const TSysCfgListItem& scliSrc) const
{
    return
    (
        (m_strAltName == scliSrc.m_strAltName)
        && (m_strTitle == scliSrc.m_strTitle)
         && (m_strUniqueId == scliSrc.m_strUniqueId)
     );
}


// Support polymorphic copying. WE
tCIDLib::TVoid TSysCfgListItem::CopyFrom(const TSysCfgListItem& scliSrc)
{
    if (&scliSrc != this)
    {
        m_strAltName    = scliSrc.m_strAltName;
        m_strTitle      = scliSrc.m_strTitle;
        m_strUniqueId   = scliSrc.m_strUniqueId;
    }
}

tCIDLib::TVoid TSysCfgListItem::Reset()
{
    m_strAltName.Clear();
    m_strTitle.Clear();

    // Create a new unique id in case this is reused
    m_strUniqueId = TUniqueId::strMakeId();
}


// ---------------------------------------------------------------------------
//  TSysCfgListItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSysCfgListItem::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtBaseInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSysCfgListItem")
        );
    }

    // Read in our data
    strmSrc >> m_strTitle
            >> m_strUniqueId;

    // If v2 or beyond, read in the 'alt name' field, else default it
    if (c2FmtVersion > 1)
        strmSrc >> m_strAltName;
    else
        m_strAltName.Clear();

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSysCfgListItem::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtBaseInfo

            << m_strTitle
            << m_strUniqueId

            // V2 stuff
            << m_strAltName

            << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TSCGlobActInfo
//  PREFIX: scli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCGlobActInfo: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSCGlobActInfo::bComp(const TSCGlobActInfo& scli1, const TSCGlobActInfo& scli2)
{
    return (scli1 == scli2);
}



// ---------------------------------------------------------------------------
//  TSCGlobActInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCGlobActInfo::TSCGlobActInfo() :

    m_bEnabled(kCIDLib::False)
    , m_colParms(kCQCSysCfg::c4MaxGActParms)
{
}

TSCGlobActInfo::~TSCGlobActInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCGlobActInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCGlobActInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCGlobActInfo::clsThis()), L"Source is not a global action info");
    const TSCGlobActInfo& scliGA = static_cast<const TSCGlobActInfo&>(scliSrc);
    return
    (
        (m_bEnabled == scliGA.m_bEnabled)
        && (m_strPath == scliGA.m_strPath)
        && (m_colParms == scliGA.m_colParms)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCGlobActInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCGlobActInfo::clsThis()), L"Source is not a global action info");
    *this = static_cast<const TSCGlobActInfo&>(scliSrc);
}

tCIDLib::TVoid TSCGlobActInfo::Reset()
{
    TParent::Reset();

    m_bEnabled = kCIDLib::False;
    m_colParms.SetAll(TString::strEmpty());
    m_strPath.Clear();
}


// ---------------------------------------------------------------------------
//  TSCGlobActInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCGlobActInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtGActInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCGlobActInfo")
        );
    }

    // Do our parent first
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_bEnabled
            >> m_colParms
            >> m_strPath;

    if (c2FmtVersion < 2)
        facCQCKit().Make50Path(m_strPath);

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCGlobActInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtGActInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    // And our stuff
    strmTar << m_bEnabled
            << m_colParms
            << m_strPath

            << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TSCHVACInfo
//  PREFIX: li
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCHVACInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCHVACInfo::TSCHVACInfo()
{
}

TSCHVACInfo::~TSCHVACInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCHVACInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCHVACInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCHVACInfo::clsThis()), L"Source is not an HVAC info");
    const TSCHVACInfo& scliHV = static_cast<const TSCHVACInfo&>(scliSrc);
    return
    (
        (m_strMoniker == scliHV.m_strMoniker)
        && (m_strThermoSub == scliHV.m_strThermoSub)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCHVACInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCHVACInfo::clsThis()), L"Source is not an HVAC info");
    *this = static_cast<const TSCHVACInfo&>(scliSrc);
}


tCIDLib::TVoid TSCHVACInfo::Reset()
{
    TParent::Reset();
    m_strMoniker.Clear();
    m_strThermoSub.Clear();
}


// ---------------------------------------------------------------------------
//  TSCHVACInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCHVACInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtHVACInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCHVACInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_strMoniker
            >> m_strThermoSub;

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCHVACInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtHVACInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    // Do our stuff
    strmTar << m_strMoniker
            << m_strThermoSub
            << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TSCLoadInfo
//  PREFIX: scli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCLoadInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCLoadInfo::TSCLoadInfo()
{
}

TSCLoadInfo::~TSCLoadInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCLoadInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCLoadInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCLoadInfo::clsThis()), L"Source is not a load info");
    const TSCLoadInfo& scliLd = static_cast<const TSCLoadInfo&>(scliSrc);
    return
    (
        (m_strClrField == scliLd.m_strClrField)
        && (m_strField == scliLd.m_strField)
        && (m_strField2 == scliLd.m_strField2)
        && (m_strMoniker == scliLd.m_strMoniker)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCLoadInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCLoadInfo::clsThis()), L"Source is not a load info");
    *this = static_cast<const TSCLoadInfo&>(scliSrc);
}

tCIDLib::TVoid TSCLoadInfo::Reset()
{
    TParent::Reset();

    m_strClrField.Clear();
    m_strField.Clear();
    m_strField2.Clear();
    m_strMoniker.Clear();
}



// ---------------------------------------------------------------------------
//  TSCLoadInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// If the main field is a dimmer, return true
tCIDLib::TBoolean TSCLoadInfo::bIsDimmer() const
{
    return m_strField.bStartsWith(L"LGHT#Dim_");
}


// ---------------------------------------------------------------------------
//  TSCLoadInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCLoadInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtLoadInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCLoadInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_strMoniker
            >> m_strClrField
            >> m_strField
            >> m_strField2;

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCLoadInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtLoadInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    strmTar << m_strMoniker
            << m_strClrField
            << m_strField
            << m_strField2

            << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TSCMediaInfo
//  PREFIX: scli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCMediaInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCMediaInfo::TSCMediaInfo()
{
}

TSCMediaInfo::~TSCMediaInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCMediaInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCMediaInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCMediaInfo::clsThis()), L"Source is not a media info");
    const TSCMediaInfo& scliMedia = static_cast<const TSCMediaInfo&>(scliSrc);
    return
    (
        (m_strAudioMon == scliMedia.m_strAudioMon)
        && (m_strAudioSub == scliMedia.m_strAudioSub)
        && (m_strRendMon== scliMedia.m_strRendMon)
        && (m_strRepoMon == scliMedia.m_strRepoMon)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCMediaInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCMediaInfo::clsThis()), L"Source is not a media info");
    *this = static_cast<const TSCMediaInfo&>(scliSrc);
}


tCIDLib::TVoid TSCMediaInfo::Reset()
{
    TParent::Reset();

    m_strAudioMon.Clear();
    m_strAudioSub.Clear();
    m_strRendMon.Clear();
    m_strRepoMon.Clear();
}


// ---------------------------------------------------------------------------
//  TSCMediaInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCMediaInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtMediaInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCMediaInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_strAudioMon
            >> m_strAudioSub
            >> m_strRendMon
            >> m_strRepoMon;

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCMediaInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtMediaInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    strmTar << m_strAudioMon
            << m_strAudioSub
            << m_strRendMon
            << m_strRepoMon

            << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TSCSecInfo
//  PREFIX: li
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCSecInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCSecInfo::TSCSecInfo()
{
}

TSCSecInfo::~TSCSecInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCSecInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCSecInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCSecInfo::clsThis()), L"Source is not a security info");
    const TSCSecInfo& scliSec = static_cast<const TSCSecInfo&>(scliSrc);
    return
    (
        (m_strArmingCode == scliSec.m_strArmingCode)
        && (m_strAreaName == scliSec.m_strAreaName)
        && (m_strMoniker == scliSec.m_strMoniker)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCSecInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCSecInfo::clsThis()), L"Source is not a security info");
    *this = static_cast<const TSCSecInfo&>(scliSrc);
}


tCIDLib::TVoid TSCSecInfo::Reset()
{
    TParent::Reset();

    m_strArmingCode.Clear();
    m_strAreaName.Clear();
    m_strMoniker.Clear();
}


// ---------------------------------------------------------------------------
//  TSCSecInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCSecInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtSecInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCSecInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_strMoniker
            >> m_strAreaName;

    // If V2 or beyond, read in the arming code, else default it
    if (c2FmtVersion > 1)
        strmSrc >> m_strArmingCode;
    else
        m_strArmingCode.Clear();

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCSecInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtSecInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    // Do our stuff
    strmTar << m_strMoniker
            << m_strAreaName

            // V2 stuff
            << m_strArmingCode

            << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TSCWeatherInfo
//  PREFIX: li
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCWeatherInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCWeatherInfo::TSCWeatherInfo() :

    m_bFScale(kCIDLib::True)
{
}

TSCWeatherInfo::~TSCWeatherInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCWeatherInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCWeatherInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCWeatherInfo::clsThis()), L"Source is not a weather info");
    const TSCWeatherInfo& scliWeather = static_cast<const TSCWeatherInfo&>(scliSrc);
    return
    (
        (m_bFScale == scliWeather.m_bFScale)
        && (m_strMoniker == scliWeather.m_strMoniker)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCWeatherInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCWeatherInfo::clsThis()), L"Source is not a weather info");
    *this = static_cast<const TSCWeatherInfo&>(scliSrc);
}


tCIDLib::TVoid TSCWeatherInfo::Reset()
{
    TParent::Reset();

    m_bFScale = kCIDLib::True;
    m_strMoniker.Clear();
}


// ---------------------------------------------------------------------------
//  TSCWeatherInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCWeatherInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtWeatherInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCWeatherInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    // Read in our data
    strmSrc >> m_strMoniker
            >> m_bFScale;

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCWeatherInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtWeatherInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    // Do our stuff
    strmTar << m_strMoniker
            << m_bFScale
            << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TSCXOverInfo
//  PREFIX: li
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSCXOverInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TSCXOverInfo::TSCXOverInfo()
{
}

TSCXOverInfo::~TSCXOverInfo()
{
}


// ---------------------------------------------------------------------------
//  TSCXOverInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TSCXOverInfo::bCompare(const TSysCfgListItem& scliSrc) const
{
    if (!TParent::bCompare(scliSrc))
        return kCIDLib::False;

    CIDAssert(scliSrc.bIsA(TSCXOverInfo::clsThis()), L"Source is not an overlay info");
    const TSCXOverInfo& scliOver = static_cast<const TSCXOverInfo&>(scliSrc);
    return
    (
        (m_strContentTmpl == scliOver.m_strContentTmpl)
        && (m_strTileTmpl == scliOver.m_strTileTmpl)
    );
}


// Support polymorphic copying
tCIDLib::TVoid TSCXOverInfo::CopyFrom(const TSysCfgListItem& scliSrc)
{
    CIDAssert(scliSrc.bIsA(TSCXOverInfo::clsThis()), L"Source is not an overlay info");
    *this = static_cast<const TSCXOverInfo&>(scliSrc);
}


tCIDLib::TVoid TSCXOverInfo::Reset()
{
    TParent::Reset();

    m_strContentTmpl.Clear();
    m_strTileTmpl.Clear();
}


// ---------------------------------------------------------------------------
//  TSCXOverInfo: Protected, inherted methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSCXOverInfo::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_Parts::c2FmtXOverInfo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TSCXOverInfo")
        );
    }

    // Do our parent
    TParent::StreamFrom(strmSrc);

    //
    //  Read in our data. If V1, then read in the title we used to keep here and move
    //  it to the base class.
    //
    if (c2FmtVersion == 1)
        strmSrc >> m_strTitle;

    strmSrc >> m_strContentTmpl
            >> m_strTileTmpl;

    if (c2FmtVersion < 3)
    {
        facCQCKit().Make50Path(m_strContentTmpl);
        facCQCKit().Make50Path(m_strTileTmpl);
    }
    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TSCXOverInfo::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_Parts::c2FmtXOverInfo;

    // Do our parent
    TParent::StreamTo(strmTar);

    // Do our stuff
    strmTar << m_strContentTmpl
            << m_strTileTmpl
            << tCIDLib::EStreamMarkers::EndObject;
}


