//
// FILE NAME: CQCBulkLoader_CfgInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This file implements the classes that we use to store the data from the
//  config file.
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
#include "CQCBulkLoader.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TBLPromptInfo,TObject)
RTTIDecls(TBLDrvInfo,TObject)
RTTIDecls(TBLEventInfo,TObject)
RTTIDecls(TBLEvScopeInfo,TObject)
RTTIDecls(TBLImageInfo,TObject)
RTTIDecls(TBLImgScopeInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TBLPromptInfo
//  PREFIX: pi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLPromptInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLPromptInfo::TBLPromptInfo()
{
}

TBLPromptInfo::TBLPromptInfo(const TString& strKey) :

    m_strKey(strKey)
{
}

TBLPromptInfo::~TBLPromptInfo()
{
}


// ---------------------------------------------------------------------------
//  TBLPromptInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TBLPromptInfo::AddSubKey(const TString& strKey, const TString& strValue)
{
    m_colSubKeys.objAdd(TKeyValuePair(strKey, strValue));
}



// ---------------------------------------------------------------------------
//   CLASS: TBLDrvInfo
//  PREFIX: drvi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLDrvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLDrvInfo::TBLDrvInfo()
{
}

TBLDrvInfo::TBLDrvInfo( const   TString&    strMoniker
                        , const TString&    strMake
                        , const TString&    strModel
                        , const TString&    strHost
                        , const TString&    strConnInfo
                        , const TString&    strFldExport) :

    m_strConnInfo(strConnInfo)
    , m_strFldExport(strFldExport)
    , m_strHost(strHost)
    , m_strMake(strMake)
    , m_strModel(strModel)
    , m_strMoniker(strMoniker)
{
}

TBLDrvInfo::~TBLDrvInfo()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TBLDrvInfo::AddPrompt(const TBLPromptInfo& piToAdd)
{
    m_colPrompts.objAdd(piToAdd);
}


tCIDLib::TCard4 TBLDrvInfo::c4PromptCount() const
{
    return m_colPrompts.c4ElemCount();
}


// Getters and setters for the stuff that needs it
const TCQCDriverObjCfg& TBLDrvInfo::cqcdcInfo() const
{
    return m_cqcdcInfo;
}


//
//  During the initialization, as the configured drivers are looked up,
//  this will be called to set the static level driver config (the stuff
//  from the manifest file.) We set that as the base content of our driver
//  instance level config, which will then subsequently then be set up
//  with driver prompt info. We return a ref to our config object for that
//  purpose.
//
TCQCDriverObjCfg& TBLDrvInfo::cqcdcSetManifest(const TCQCDriverCfg& cqcdcToSet)
{
    // It's the base class of the full actual driver instance config
    m_cqcdcInfo.UpdateBase(cqcdcToSet);
    return m_cqcdcInfo;
}

const TBLPromptInfo& TBLDrvInfo::piAt(const tCIDLib::TCard4 c4At) const
{
    return m_colPrompts[c4At];
}

const TString& TBLDrvInfo::strConnInfo() const
{
    return m_strConnInfo;
}

const TString& TBLDrvInfo::strFldExport() const
{
    return m_strFldExport;
}

const TString& TBLDrvInfo::strHost() const
{
    return m_strHost;
}

const TString& TBLDrvInfo::strMake() const
{
    return m_strMake;
}

const TString& TBLDrvInfo::strModel() const
{
    return m_strModel;
}

const TString& TBLDrvInfo::strMoniker() const
{
    return m_strMoniker;
}



// ---------------------------------------------------------------------------
//   CLASS: TBLEventInfo
//  PREFIX: evi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLEventInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLEventInfo::TBLEventInfo()
{
}

TBLEventInfo::TBLEventInfo(const TString& strName) :

    m_strName(strName)
{
}

TBLEventInfo::~TBLEventInfo()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TBLEventInfo::strName() const
{
    return m_strName;
}




// ---------------------------------------------------------------------------
//   CLASS: TBLEvScopeInfo
//  PREFIX: scpi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLEvScopeInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLEvScopeInfo::TBLEvScopeInfo()
{
}

TBLEvScopeInfo::TBLEvScopeInfo( const   TString&                strSrcDir
                                , const TString&                strTarScope
                                , const tCQCRemBrws::EDTypes    eType) :
    m_eType(eType)
    , m_strName(strSrcDir)
    , m_strTarScope(strTarScope)
{
}

// No need to copy the images since they wouldn't be loaded where this is copied from
TBLEvScopeInfo::TBLEvScopeInfo(const TBLEvScopeInfo& scpiToCopy) :

    m_eType(scpiToCopy.m_eType)
    , m_strName(scpiToCopy.m_strName)
    , m_strTarScope(scpiToCopy.m_strTarScope)
{
}

TBLEvScopeInfo::~TBLEvScopeInfo()
{
}


// ---------------------------------------------------------------------------
//  TBLEvScopeInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TBLEvScopeInfo::AddEvent(const TString& strName)
{
    m_colEvents.objAdd(TBLEventInfo(strName));
}


//
//  Sub-dirs only have a name, so we pass an empty string for the target scope. That is
//  only used for the top level scope of a given config file block.
//
TBLEvScopeInfo& TBLEvScopeInfo::scpiAddSubDir(const TString& strName)
{
    return m_colSubDirs.objAdd(TBLEvScopeInfo(strName, TString::strEmpty(), m_eType));
}


tCIDLib::TCard4 TBLEvScopeInfo::c4EvCount() const
{
    return m_colEvents.c4ElemCount();
}

tCIDLib::TCard4 TBLEvScopeInfo::c4SubDirCount() const
{
    return m_colSubDirs.c4ElemCount();
}


const TBLEventInfo& TBLEvScopeInfo::eviAt(const tCIDLib::TCard4 c4At) const
{
    return m_colEvents[c4At];
}

const TBLEvScopeInfo& TBLEvScopeInfo::scpiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSubDirs[c4At];
}


const TString& TBLEvScopeInfo::strName() const
{
    return m_strName;
}


const TString& TBLEvScopeInfo::strTarScope() const
{
    return m_strTarScope;
}



// ---------------------------------------------------------------------------
//   CLASS: TBLmgInfo
//  PREFIX: imgi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLImageInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLImageInfo::TBLImageInfo() :

    m_eTransType(tCQCBulkLoader::EImgTrans_None)
{
}

TBLImageInfo::TBLImageInfo( const   tCQCBulkLoader::EImgTrans   eTransType
                            , const TString&                    strName
                            , const TPoint&                     pntTransClr
                            , const TRGBClr&                    rgbTransClr) :

    m_eTransType(eTransType)
    , m_pntTransClr(pntTransClr)
    , m_rgbTransClr(rgbTransClr)
    , m_strName(strName)
{
}

TBLImageInfo::~TBLImageInfo()
{
}


// ---------------------------------------------------------------------------
//  TBLImageInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCBulkLoader::EImgTrans TBLImageInfo::eTransType() const
{
    return m_eTransType;
}

const TPoint& TBLImageInfo::pntTransClr() const
{
    return m_pntTransClr;
}

const TRGBClr& TBLImageInfo::rgbTransClr() const
{
    return m_rgbTransClr;
}

const TString& TBLImageInfo::strName() const
{
    return m_strName;
}



// ---------------------------------------------------------------------------
//   CLASS: TBLmgScopeInfo
//  PREFIX: scpi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBLImgScopeInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBLImgScopeInfo::TBLImgScopeInfo()
{
}

TBLImgScopeInfo::TBLImgScopeInfo(const  TString&    strSrcDir
                                , const TString&    strTarScope) :

    m_strName(strSrcDir)
    , m_strTarScope(strTarScope)
{
}

// No need to copy the images since they wouldn't be loaded where this is copied from
TBLImgScopeInfo::TBLImgScopeInfo(const TBLImgScopeInfo& scpiToCopy) :

    m_strName(scpiToCopy.m_strName)
    , m_strTarScope(scpiToCopy.m_strTarScope)
{
}

TBLImgScopeInfo::~TBLImgScopeInfo()
{
}


// ---------------------------------------------------------------------------
//  TBLImgScopeInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TBLImgScopeInfo::AddImage(  const   tCQCBulkLoader::EImgTrans   eTransType
                            , const TString&                    strName
                            , const TPoint&                     pntTransClr
                            , const TRGBClr&                    rgbTransClr)
{
    m_colImages.objAdd(TBLImageInfo(eTransType, strName, pntTransClr, rgbTransClr));
}


TBLImgScopeInfo& TBLImgScopeInfo::scpiAddSubDir(const TString& strName)
{
    // No separate src/tar names at this level, so pass an empty string
    return m_colSubDirs.objAdd(TBLImgScopeInfo(strName, TString::strEmpty()));
}


tCIDLib::TCard4 TBLImgScopeInfo::c4ImgCount() const
{
    return m_colImages.c4ElemCount();
}

tCIDLib::TCard4 TBLImgScopeInfo::c4SubDirCount() const
{
    return m_colSubDirs.c4ElemCount();
}


const TBLImageInfo& TBLImgScopeInfo::imgiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colImages[c4At];
}

const TBLImgScopeInfo& TBLImgScopeInfo::scpiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSubDirs[c4At];
}


const TString& TBLImgScopeInfo::strName() const
{
    return m_strName;
}

const TString& TBLImgScopeInfo::strTarScope() const
{
    return m_strTarScope;
}


