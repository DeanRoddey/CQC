//
// FILE NAME: CQCMedia_MediaInfoClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2007
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
//  This file implements the CML classes for a CML level play list item class
//  and play list manager. In both cases, they just wrap an existing C++
//  class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  We have to bring in the otherwise private CML field def class header, so
//  that we can create value objects here.
// ---------------------------------------------------------------------------
#include    "CQCMEng_FieldDefClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCPLItemInfo,TMEngClassInfo)
RTTIDecls(TCQCPLItemValue,TMEngClassVal)
RTTIDecls(TCQCPLMgrInfo,TMEngClassInfo)
RTTIDecls(TCQCPLMgrValue,TMEngClassVal)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMedia_PLItemClass
{
    // -----------------------------------------------------------------------
    //  The names for the types that we support here. Each derivative has to
    //  be able to return strings that contain its name and full name.
    // -----------------------------------------------------------------------
    const TString   strActsClass(L"PLMgrActs");
    const TString   strBasePath(L"MEng.System.CQC.Runtime");
    const TString   strItemClass(L"CQCPLItem");
    const TString   strItemClassPath(L"MEng.System.CQC.Runtime.CQCPLItem");
    const TString   strMgrClass(L"CQCPLMgr");
    const TString   strMgrClassPath(L"MEng.System.CQC.Runtime.CQCPLMgr");
    const TString   strSelResClass(L"PLMgrSelRes");
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCPLItemValue
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPLItemValue: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCQCPLItemValue::TCQCPLItemValue(const  TString&                strName
                                , const tCIDLib::TCard2         c2ClassId
                                , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCQCPLItemValue::~TCQCPLItemValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCPLItemValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCPLItemValue::bDbgFormat(        TTextOutStream&     strmTarget
                            , const TMEngClassInfo&
                            , const tCIDMacroEng::EDbgFmts eFormat
                            , const tCIDLib::ERadices   eRadix
                            , const TCIDMacroEngine&    ) const
{
    if (eFormat == tCIDMacroEng::EDbgFmts::Short)
    {
        strmTarget << m_mpliValue.strTitleName();
        return kCIDLib::True;
    }
     else
    {
        strmTarget  << L"Title=" << m_mpliValue.strTitleName() << kCIDLib::NewLn
                    << L"Loc=" << m_mpliValue.strLocInfo() << kCIDLib::NewLn
                    << L"Col=" << m_mpliValue.strColName() << kCIDLib::NewLn
                    << L"MType="
                    << tCQCMedia::strXlatEMediaTypes(m_mpliValue.eMediaType())
                    << kCIDLib::EndLn;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCPLItemValue: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the value
const TCQCMediaPLItem& TCQCPLItemValue::mpliValue() const
{
    return m_mpliValue;
}

TCQCMediaPLItem& TCQCPLItemValue::mpliValue()
{
    return m_mpliValue;
}

const TCQCMediaPLItem&
TCQCPLItemValue::mpliValue(const TCQCMediaPLItem& mpliToSet)
{
    m_mpliValue = mpliToSet;
    return m_mpliValue;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCPLItemInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPLItemInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCPLItemInfo::strPath()
{
    return CQCMedia_PLItemClass::strItemClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCPLItemInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPLItemInfo::TCQCPLItemInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMedia_PLItemClass::strItemClass
        , CQCMedia_PLItemClass::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_LocTypes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_MediaTypes(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)

    , m_c2MethId_FormatTotalTime(kCIDMacroEng::c2BadId)

    , m_c2MethId_GetColArtist(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColAspect(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColCast(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColDescr(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColLabel(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColLeadActor(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColName(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColRating(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColYear(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemArtist(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemName(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemSeconds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetListItemId(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLocInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLocType(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetMediaFormat(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetRepoMoniker(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTitleCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTitleName(kCIDMacroEng::c2BadId)

    , m_c2MethId_SetColArtist(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColAspect(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColCast(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColDescr(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColLabel(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColLeadActor(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColName(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColRating(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetColYear(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetItemArtist(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetItemCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetItemName(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetItemSeconds(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetLocInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetLocType(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetMediaFormat(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetRepoMoniker(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetTitleCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetTitleName(kCIDMacroEng::c2BadId)

    , m_pmeciLocTypes(nullptr)
    , m_pmeciMediaTypes(nullptr)
{
    // Import the media info class since we use some of its nested types
    bAddClassImport(TCQCMediaInfoInfo::strPath());
}

TCQCPLItemInfo::~TCQCPLItemInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCPLItemInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCPLItemInfo::Init(TCIDMacroEngine& meOwner)
{
    // Look up some types we need
    m_pmeciLocTypes = meOwner.pmeciFindAs<TMEngEnumInfo>
    (
        TCQCMediaInfoInfo::strLocTypePath(), kCIDLib::True
    );
    m_c2EnumId_LocTypes = m_pmeciLocTypes->c2Id();

    m_pmeciMediaTypes = meOwner.pmeciFindAs<TMEngEnumInfo>
    (
        TCQCMediaInfoInfo::strMediaTypePath(), kCIDLib::True
    );
    m_c2EnumId_MediaTypes = m_pmeciMediaTypes->c2Id();


    // The default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCPLItem"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Format the total time in standard format
    {
        TMEngMethodInfo methiNew
        (
            L"FormatTotalTime"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FormatTotalTime = c2AddMethodInfo(methiNew);
    }

    // Get the col artist member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColArtist"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColArtist = c2AddMethodInfo(methiNew);
    }

    // Get the col aspect member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColAspect"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColAspect = c2AddMethodInfo(methiNew);
    }

    // Get the col cast member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColCast"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColCast = c2AddMethodInfo(methiNew);
    }

    // Get the col cookie member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColCookie"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColCookie = c2AddMethodInfo(methiNew);
    }

    // Get the col description member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColDescr"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColDescr = c2AddMethodInfo(methiNew);
    }

    // Get the col label member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColLabel"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColLabel = c2AddMethodInfo(methiNew);
    }

    // Get the col lead actor member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColLeadActor"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColLeadActor = c2AddMethodInfo(methiNew);
    }

    // Get the col name member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColName = c2AddMethodInfo(methiNew);
    }

    // Get the col rating member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColRating"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColRating = c2AddMethodInfo(methiNew);
    }

    // Get the col year member
    {
        TMEngMethodInfo methiNew
        (
            L"GetColYear"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetColYear = c2AddMethodInfo(methiNew);
    }

    // Get the item artist member
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemArtist"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetItemArtist = c2AddMethodInfo(methiNew);
    }

    // Get the item cookie member
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemCookie"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetItemCookie = c2AddMethodInfo(methiNew);
    }

    // Get the item duration (seconds)
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemSeconds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetItemSeconds = c2AddMethodInfo(methiNew);
    }

    // Get the list item id
    {
        TMEngMethodInfo methiNew
        (
            L"GetListItemId"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetListItemId = c2AddMethodInfo(methiNew);
    }

    // Get the item name member
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetItemName = c2AddMethodInfo(methiNew);
    }

    // Get the loc info member
    {
        TMEngMethodInfo methiNew
        (
            L"GetLocInfo"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetLocInfo = c2AddMethodInfo(methiNew);
    }

    // Get the location type member
    {
        TMEngMethodInfo methiNew
        (
            L"GetLocType"
            , m_c2EnumId_LocTypes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetLocType = c2AddMethodInfo(methiNew);
    }

    // Get the media format
    {
        TMEngMethodInfo methiNew
        (
            L"GetMediaFormat"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetMediaFormat = c2AddMethodInfo(methiNew);
    }

    // Get the repo moniker
    {
        TMEngMethodInfo methiNew
        (
            L"GetRepoMoniker"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetRepoMoniker = c2AddMethodInfo(methiNew);
    }

    // Get the Title cookie member
    {
        TMEngMethodInfo methiNew
        (
            L"GetTitleCookie"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetTitleCookie = c2AddMethodInfo(methiNew);
    }

    // Get the title name member
    {
        TMEngMethodInfo methiNew
        (
            L"GetTitleName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetTitleName = c2AddMethodInfo(methiNew);
    }


    // Set the col artist member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColArtist"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColArtist = c2AddMethodInfo(methiNew);
    }

    // Set the col aspect member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColAspect"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColAspect = c2AddMethodInfo(methiNew);
    }

    // Set the col cast member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColCast"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColCast = c2AddMethodInfo(methiNew);
    }

    // Set the col cookie member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColCookie"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColCookie = c2AddMethodInfo(methiNew);
    }

    // Set the col description member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColDescr"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColDescr = c2AddMethodInfo(methiNew);
    }

    // Set the col label member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColLabel"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColLabel = c2AddMethodInfo(methiNew);
    }

    // Set the col lead actor member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColLeadActor"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColLeadActor = c2AddMethodInfo(methiNew);
    }

    // Set the col name member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColName"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColName = c2AddMethodInfo(methiNew);
    }

    // Set the col rating member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColRating"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetColRating = c2AddMethodInfo(methiNew);
    }

    // Set the col year member
    {
        TMEngMethodInfo methiNew
        (
            L"SetColYear"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetColYear = c2AddMethodInfo(methiNew);
    }

    // Set the item artist member
    {
        TMEngMethodInfo methiNew
        (
            L"SetItemArtist"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetItemArtist = c2AddMethodInfo(methiNew);
    }

    // Set the item cookie member
    {
        TMEngMethodInfo methiNew
        (
            L"SetItemCookie"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetItemCookie = c2AddMethodInfo(methiNew);
    }

    // Set the item duration (seconds)
    {
        TMEngMethodInfo methiNew
        (
            L"SetItemSeconds"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetItemSeconds = c2AddMethodInfo(methiNew);
    }

    // Set the item name member
    {
        TMEngMethodInfo methiNew
        (
            L"SetItemName"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetItemName = c2AddMethodInfo(methiNew);
    }

    // Set the loc info member
    {
        TMEngMethodInfo methiNew
        (
            L"SetLocInfo"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetLocInfo = c2AddMethodInfo(methiNew);
    }

    // Set the location type member
    {
        TMEngMethodInfo methiNew
        (
            L"SetLocType"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", m_c2EnumId_LocTypes);
        m_c2MethId_SetLocType = c2AddMethodInfo(methiNew);
    }

    // Set the media format
    {
        TMEngMethodInfo methiNew
        (
            L"SetMediaFormat"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetMediaFormat = c2AddMethodInfo(methiNew);
    }

    // Set the repo moniker
    {
        TMEngMethodInfo methiNew
        (
            L"SetRepoMoniker"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetRepoMoniker = c2AddMethodInfo(methiNew);
    }

}


// Create a new value of our type upon request
TMEngClassVal*
TCQCPLItemInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCPLItemValue(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCPLItemInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCPLItemInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    // Don't currently need this
    TCQCPLItemValue& mecvActual = static_cast<TCQCPLItemValue&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Reset the value object
        mecvActual.mpliValue().Reset();
    }
     else if (c2MethId == m_c2MethId_FormatTotalTime)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd);
        facCQCMedia().FormatDuration
        (
            mecvActual.mpliValue().c4ItemSeconds(), mecvToFill.strValue()
        );
    }
     else if (c2MethId == m_c2MethId_GetColArtist)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColArtist());
    }
     else if (c2MethId == m_c2MethId_GetColAspect)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColAspectRatio());
    }
     else if (c2MethId == m_c2MethId_GetColCast)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColCast());
    }
     else if (c2MethId == m_c2MethId_GetColCookie)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColCookie());
    }
     else if (c2MethId == m_c2MethId_GetColDescr)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColDescription());
    }
     else if (c2MethId == m_c2MethId_GetColLabel)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColLabel());
    }
     else if (c2MethId == m_c2MethId_GetColLeadActor)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColLeadActor());
    }
     else if (c2MethId == m_c2MethId_GetColName)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColName());
    }
     else if (c2MethId == m_c2MethId_GetColRating)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strColRating());
    }
     else if (c2MethId == m_c2MethId_GetColYear)
    {
        TMEngCard4Val& mecvToFill = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvToFill.c4Value(mecvActual.mpliValue().c4ColYear());
    }
     else if (c2MethId == m_c2MethId_GetItemArtist)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strItemArtist());
    }
     else if (c2MethId == m_c2MethId_GetItemCookie)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strItemCookie());
    }
     else if (c2MethId == m_c2MethId_GetItemName)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strItemName());
    }
     else if (c2MethId == m_c2MethId_GetItemSeconds)
    {
        TMEngCard4Val& mecvToFill = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvToFill.c4Value(mecvActual.mpliValue().c4ItemSeconds());
    }
     else if (c2MethId == m_c2MethId_GetListItemId)
    {
        TMEngCard4Val& mecvToFill = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvToFill.c4Value(mecvActual.mpliValue().c4ListItemId());
    }
     else if (c2MethId == m_c2MethId_GetLocInfo)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strLocInfo());
    }
     else if (c2MethId == m_c2MethId_GetLocType)
    {
        TMEngEnumVal& mecvLocType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvLocType.c4Ordinal(tCIDLib::c4EnumOrd(mecvActual.mpliValue().eLocType()));
    }
     else if (c2MethId == m_c2MethId_GetMediaFormat)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strMediaFormat());
    }
     else if (c2MethId == m_c2MethId_GetRepoMoniker)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strRepoMoniker());
    }
     else if (c2MethId == m_c2MethId_GetTitleCookie)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strTitleCookie());
    }
     else if (c2MethId == m_c2MethId_GetTitleName)
    {
        TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvToFill.strValue(mecvActual.mpliValue().strTitleName());
    }
     else if (c2MethId == m_c2MethId_SetColArtist)
    {
        mecvActual.mpliValue().strColArtist(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColAspect)
    {
        mecvActual.mpliValue().strColAspectRatio(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColCast)
    {
        mecvActual.mpliValue().strColCast(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColCookie)
    {
        mecvActual.mpliValue().strColCookie(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColDescr)
    {
        mecvActual.mpliValue().strColDescription(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColLabel)
    {
        mecvActual.mpliValue().strColLabel(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColLeadActor)
    {
        mecvActual.mpliValue().strColLeadActor(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColName)
    {
        mecvActual.mpliValue().strColName(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColRating)
    {
        mecvActual.mpliValue().strColRating(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetColYear)
    {
        mecvActual.mpliValue().c4ColYear(meOwner.c4StackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetItemArtist)
    {
        mecvActual.mpliValue().strItemArtist(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetItemCookie)
    {
        mecvActual.mpliValue().strItemCookie(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetItemName)
    {
        mecvActual.mpliValue().strItemName(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetItemSeconds)
    {
        mecvActual.mpliValue().c4ItemSeconds(meOwner.c4StackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetLocInfo)
    {
        mecvActual.mpliValue().strLocInfo(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetLocType)
    {
        const TMEngEnumVal& mecvLocType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvActual.mpliValue().eLocType(tCQCMedia::ELocTypes(mecvLocType.c4Ordinal()));
    }
     else if (c2MethId == m_c2MethId_SetMediaFormat)
    {
        mecvActual.mpliValue().strMediaFormat(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetRepoMoniker)
    {
        mecvActual.mpliValue().strRepoMoniker(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetTitleCookie)
    {
        mecvActual.mpliValue().strTitleCookie(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetTitleName)
    {
        mecvActual.mpliValue().strTitleName(meOwner.strStackValAt(c4FirstInd));
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCPLMgrValue
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPLMgrValue: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCQCPLMgrValue::TCQCPLMgrValue( const   TString&                strName
                                , const tCIDLib::TCard2         c2ClassId
                                , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCQCPLMgrValue::~TCQCPLMgrValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCPLMgrValue: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get access to the underlying playlist manager object
const TCQCMediaPLMgr& TCQCPLMgrValue::mplmValue() const
{
    return m_mplmValue;
}

TCQCMediaPLMgr& TCQCPLMgrValue::mplmValue()
{
    return m_mplmValue;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCPLMgrInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPLMgrInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCPLMgrInfo::strPath()
{
    return CQCMedia_PLItemClass::strMgrClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCPLMgrInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPLMgrInfo::TCQCPLMgrInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMedia_PLItemClass::strMgrClass
        , CQCMedia_PLItemClass::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_PLMgrActs(kCIDMacroEng::c2BadId)
    , m_c2EnumId_PLMgrSelRes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_PLModes(kCIDMacroEng::c2BadId)

    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddMedia(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCurCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCurIndex(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCurItem(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCurItemId(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemAt(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemCount(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSerialNum(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetPLMode(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetVerbose(kCIDMacroEng::c2BadId)
    , m_c2MethId_FormatCurPLItem(kCIDMacroEng::c2BadId)
    , m_c2MethId_FormatPL(kCIDMacroEng::c2BadId)
    , m_c2MethId_HasItemPlayed(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsEmpty(kCIDMacroEng::c2BadId)
    , m_c2MethId_LoadFromRendPL(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryStdRendFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_RandomCatUpdate(kCIDMacroEng::c2BadId)
    , m_c2MethId_RemoveById(kCIDMacroEng::c2BadId)
    , m_c2MethId_RemoveItemAt(kCIDMacroEng::c2BadId)
    , m_c2MethId_Reset(kCIDMacroEng::c2BadId)
    , m_c2MethId_SelectById(kCIDMacroEng::c2BadId)
    , m_c2MethId_SelectNewItem(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetPLMode(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetVerbose(kCIDMacroEng::c2BadId)

    , m_c2TypeId_FldDef(kCIDMacroEng::c2BadId)
    , m_c2TypeId_FldList(kCIDMacroEng::c2BadId)

    , m_c4ErrId_AddFailed(kCIDLib::c4MaxCard)
    , m_c4ErrId_EmptyList(kCIDLib::c4MaxCard)
    , m_c4ErrId_IndexErr(kCIDLib::c4MaxCard)
    , m_c4ErrId_ListLoad(kCIDLib::c4MaxCard)

    , m_pmeciErrors(0)
    , m_pmeciFldList(0)
    , m_pmeciPLMgrActs(0)
    , m_pmeciPLMgrSelRes(0)
    , m_pmeciPLModes(0)
{
    // Import the playlist item class since we use it our interface
    bAddClassImport(CQCMedia_PLItemClass::strItemClassPath);
}

TCQCPLMgrInfo::~TCQCPLMgrInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCPLMgrInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCPLMgrInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create our exception enum
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner, L"CQCPLMgrErrs", strClassPath(), L"MEng.Enum", 4
        );
        m_c4ErrId_AddFailed = m_pmeciErrors->c4AddEnumItem(L"AddFailed", TString::strEmpty());
        m_c4ErrId_EmptyList = m_pmeciErrors->c4AddEnumItem(L"EmptyList", L"The playlist is empty");
        m_c4ErrId_IndexErr = m_pmeciErrors->c4AddEnumItem(L"IndexErr", L"Index %(1) is not valid for the playlist");
        m_c4ErrId_ListLoad = m_pmeciErrors->c4AddEnumItem(L"ListLoad", L"Could not load from renderer playlist");
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Create the playlist manager actions enum
    {
        m_pmeciPLMgrActs = new TMEngEnumInfo
        (
            meOwner, L"CQCPLMgrActs", strClassPath(), L"MEng.Enum", 5
        );
        m_pmeciPLMgrActs->c4AddEnumItem(L"Rejected", L"Rejected", tCQCMedia::EPLMgrActs::Rejected);
        m_pmeciPLMgrActs->c4AddEnumItem(L"NoAction", L"NoAction", tCQCMedia::EPLMgrActs::NoAction);
        m_pmeciPLMgrActs->c4AddEnumItem(L"NewItem",  L"NewItem",  tCQCMedia::EPLMgrActs::NewItem);
        m_pmeciPLMgrActs->c4AddEnumItem(L"NewSelection", L"NewSelection", tCQCMedia::EPLMgrActs::NewSelection);
        m_pmeciPLMgrActs->c4AddEnumItem(L"Stop", L"Stop", tCQCMedia::EPLMgrActs::Stop);
        m_pmeciPLMgrActs->BaseClassInit(meOwner);
        m_c2EnumId_PLMgrActs = meOwner.c2AddClass(m_pmeciPLMgrActs);
        bAddNestedType(m_pmeciPLMgrActs->strClassPath());
    }

    // Create the playlist selection result enum
    {
        m_pmeciPLMgrSelRes = new TMEngEnumInfo
        (
            meOwner, L"CQCPLMgrSelRes", strClassPath(), L"MEng.Enum", 3
        );
        m_pmeciPLMgrSelRes->c4AddEnumItem(L"Empty", L"Empty", tCQCMedia::EPLMgrSelRes::Empty);
        m_pmeciPLMgrSelRes->c4AddEnumItem(L"NewItem", L"NewItem", tCQCMedia::EPLMgrSelRes::NewItem);
        m_pmeciPLMgrSelRes->c4AddEnumItem(L"NewAndChanged", L"NewAndChanged", tCQCMedia::EPLMgrSelRes::NewAndChanged);
        m_pmeciPLMgrSelRes->BaseClassInit(meOwner);
        m_c2EnumId_PLMgrSelRes = meOwner.c2AddClass(m_pmeciPLMgrSelRes);
        bAddNestedType(m_pmeciPLMgrSelRes->strClassPath());
    }

    //
    //  Create the playlist mode enum. We have to keep text here in sync with
    //  the C++ world since this is used to set up the playlist mode field
    //  in the CML world, but the driver's fields are set up by the standard
    //  renderer field query on the facility class. So we set the text part
    //  from the C++ enum. The name part is only used in the CML world so
    //  it doesn't have to be synced.
    //
    {
        m_pmeciPLModes = new TMEngEnumInfo
        (
            meOwner, L"CQCPLModes", strClassPath(), L"MEng.Enum", 4
        );
        m_pmeciPLModes->c4AddEnumItem
        (
            L"Normal"
            , tCQCMedia::strXlatEPLModes(tCQCMedia::EPLModes::Normal)
            , tCQCMedia::EPLModes::Normal
        );
        m_pmeciPLModes->c4AddEnumItem
        (
            L"Jukebox"
            , tCQCMedia::strXlatEPLModes(tCQCMedia::EPLModes::Jukebox)
            , tCQCMedia::EPLModes::Jukebox
        );
        m_pmeciPLModes->c4AddEnumItem
        (
            L"Shuffle"
            , tCQCMedia::strXlatEPLModes(tCQCMedia::EPLModes::Shuffle)
            , tCQCMedia::EPLModes::Shuffle
        );
        m_pmeciPLModes->c4AddEnumItem
        (
            L"RandomCat"
            , tCQCMedia::strXlatEPLModes(tCQCMedia::EPLModes::RandomCat)
            , tCQCMedia::EPLModes::RandomCat
        );
        m_pmeciPLModes->BaseClassInit(meOwner);
        m_c2EnumId_PLModes = meOwner.c2AddClass(m_pmeciPLModes);
        bAddNestedType(m_pmeciPLModes->strClassPath());
    }


    //
    //  We need a nested type that is a vector of field defs. This is needed
    //  by our standard field defs query method. Save the field def class id
    //  for later, too, since we need it to create value objects.
    //
    {
        m_c2TypeId_FldDef = meOwner.c2FindClassId(TCQCFldDefInfo::strClassPath());

        TMEngVectorInfo* m_pmeciFldList = new TMEngVectorInfo
        (
            meOwner
            , L"CQCMediaFldList"
            , CQCMedia_PLItemClass::strMgrClassPath
            , TMEngVectorInfo::strPath()
            , m_c2TypeId_FldDef
        );
        TJanitor<TMEngVectorInfo> janNewClass(m_pmeciFldList);
        m_pmeciFldList->BaseClassInit(meOwner);
        bAddNestedType(m_pmeciFldList->strClassPath());
        m_c2TypeId_FldList = meOwner.c2AddClass(janNewClass.pobjOrphan());
    }


    // Create some literals for some backdoor driver values
    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kRenderDrv_QueryArt"
            , tCIDMacroEng::EIntrinsics::String
            , new TMEngStringVal
              (
                L"kRenderDrv_QueryArt"
                , tCIDMacroEng::EConstTypes::Const
                , kCQCMedia::strQuery_QueryArt
              )
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kRenderDrv_QueryCurPLItem"
            , tCIDMacroEng::EIntrinsics::String
            , new TMEngStringVal
              (
                L"kRenderDrv_QueryCurPLItem"
                , tCIDMacroEng::EConstTypes::Const
                , kCQCMedia::strQuery_CurPLItem
              )
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kRenderDrv_QueryPLItems"
            , tCIDMacroEng::EIntrinsics::String
            , new TMEngStringVal
              (
                L"kRenderDrv_QueryPLItems"
                , tCIDMacroEng::EConstTypes::Const
                , kCQCMedia::strQuery_PLItems
              )
        )
    );


    //
    //  Define constants for all of the standard media driver fields,
    //  so that every driver doesn't end up ad hoc'ing them.
    //
    AddStrLiteral(L"kMediaFld_ActiveRepository", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::ActiveRepo));
    AddStrLiteral(L"kMediaFld_AdjVolume", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::AdjustVolume));
    AddStrLiteral(L"kMediaFld_ClearPlaylist", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::ClearPL));
    AddStrLiteral(L"kMediaFld_CurAspectRatio", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurAspect));
    AddStrLiteral(L"kMediaFld_CurCast", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurCast));
    AddStrLiteral(L"kMediaFld_CurColArtist", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurColArtist));
    AddStrLiteral(L"kMediaFld_CurColCookie", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurColCookie));
    AddStrLiteral(L"kMediaFld_CurColName", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurColName));
    AddStrLiteral(L"kMediaFld_CurDescr", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurDescr));
    AddStrLiteral(L"kMediaFld_CurItemName", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurItName));
    AddStrLiteral(L"kMediaFld_CurItemArtist", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurItArtist));
    AddStrLiteral(L"kMediaFld_CurItemCookie", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurItCookie));
    AddStrLiteral(L"kMediaFld_CurItemTime", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurItTime));
    AddStrLiteral(L"kMediaFld_CurItemTotal", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurItTotal));
    AddStrLiteral(L"kMediaFld_CurLabel", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurLabel));
    AddStrLiteral(L"kMediaFld_CurLeadActor", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurLeadActor));
    AddStrLiteral(L"kMediaFld_CurRating", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurRating));
    AddStrLiteral(L"kMediaFld_CurTitleName", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurTiName));
    AddStrLiteral(L"kMediaFld_CurTitleCookie", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurTiCookie));
    AddStrLiteral(L"kMediaFld_CurYear", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::CurYear));
    AddStrLiteral(L"kMediaFld_DelPlaylistItem", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::DelPLItem));
    AddStrLiteral(L"kMediaFld_EnqueueMedia", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::EnqMedia));
    AddStrLiteral(L"kMediaFld_MediaState", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::MediaState));
    AddStrLiteral(L"kMediaFld_Mute", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::Mute));
    AddStrLiteral(L"kMediaFld_PBPercent", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PBPercent));
    AddStrLiteral(L"kMediaFld_PlayMedia", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PlayMedia));
    AddStrLiteral(L"kMediaFld_PlaylistMode", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLMode));
    AddStrLiteral(L"kMediaFld_PLIndex", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLIndex));
    AddStrLiteral(L"kMediaFld_PLItemCnt", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLItemCnt));
    AddStrLiteral(L"kMediaFld_PLItemKey", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLItemKey));
    AddStrLiteral(L"kMediaFld_PLSerialNum", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLSerialNum));
    AddStrLiteral(L"kMediaFld_Power", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::Power));
    AddStrLiteral(L"kMediaFld_PowerStatus", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PowerStatus));
    AddStrLiteral(L"kMediaFld_SelPlaylistItem", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::SelPLItem));
    AddStrLiteral(L"kMediaFld_SetRandomCat", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::SetRandomCat));
    AddStrLiteral(L"kMediaFld_SetRepository", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::SetRepo));
    AddStrLiteral(L"kMediaFld_Transport", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::Transport));
    AddStrLiteral(L"kMediaFld_Volume", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::Volume));
    AddStrLiteral(L"kMediaFld_ZoneNum", TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::ZoneNum));



    //
    //  Look up the playlist item class id, because we have to reference
    //  it in some method parms below.
    //
    const tCIDLib::TCard2 c2PLItemId = meOwner.c2FindClassId
    (
        CQCMedia_PLItemClass::strItemClassPath
    );


    // The default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCPLMgr"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AddMedia"
            , m_c2EnumId_PLMgrActs
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"DefRepo", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToPlay", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Enqueue", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"UserData", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddMedia = c2AddMethodInfo(methiNew);
    }

    // Format the current PL item in the form correct to return to the viewer
    {
        TMEngMethodInfo methiNew
        (
            L"FormatCurPLItem"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_FormatCurPLItem = c2AddMethodInfo(methiNew);
    }

    // Format the play list in the form correct to return to the client
    {
        TMEngMethodInfo methiNew
        (
            L"FormatPL"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_FormatPL = c2AddMethodInfo(methiNew);
    }

    // Get the repo and cookie (item if there, else col) for the current item
    {
        TMEngMethodInfo methiNew
        (
            L"GetCurCookie"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Repo", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Cookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"IsItem", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetCurCookie = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetCurIndex"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetCurIndex = c2AddMethodInfo(methiNew);
    }

    // Get the current playlist item. If empty, it will throw
    {
        TMEngMethodInfo methiNew
        (
            L"GetCurItem"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ItemToFill", c2PLItemId);
        m_c2MethId_GetCurItem = c2AddMethodInfo(methiNew);
    }

    // Get the id fo the current playlist item, if there is one
    {
        TMEngMethodInfo methiNew
        (
            L"GetCurItemId"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ItemToFill", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetCurItemId = c2AddMethodInfo(methiNew);
    }

    // Get the playlist item at the indicated index
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemAt"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Index", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ItemToFill", c2PLItemId);
        m_c2MethId_GetItemAt = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetItemCount"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetItemCount = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetSerialNum"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetSerialNum = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetPLMode"
            , m_c2EnumId_PLModes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetPLMode = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetVerbose"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetVerbose = c2AddMethodInfo(methiNew);
    }

    // Return whether the item at the indicted index has been played
    {
        TMEngMethodInfo methiNew
        (
            L"HasItemPlayed"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Index", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_HasItemPlayed = c2AddMethodInfo(methiNew);
    }

    // Return whether our playlist is empty or not
    {
        TMEngMethodInfo methiNew
        (
            L"IsEmpty"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_IsEmpty = c2AddMethodInfo(methiNew);
    }

    // Load us up from a renderer driver's playlist
    {
        TMEngMethodInfo methiNew
        (
            L"LoadFromRendPL"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"RendMoniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetIds", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_LoadFromRendPL = c2AddMethodInfo(methiNew);
    }


    //
    //  We provide a helper to load up all of the field defs for the standard
    //  renderer fields. This avoids redundant code in the various drivers.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"QueryStdRendFlds"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"DoesMovies", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"DoesMusic", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"DrvArchVer", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Flags", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryStdRendFlds = c2AddMethodInfo(methiNew);
    }

    // Do a random category update
    {
        TMEngMethodInfo methiNew
        (
            L"RandomCatUpdate"
            , m_c2EnumId_PLMgrActs
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"RepoMon", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CatCookie", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_RandomCatUpdate = c2AddMethodInfo(methiNew);
    }

    // Remove an item by its unique item id
    {
        TMEngMethodInfo methiNew
        (
            L"RemoveById"
            , m_c2EnumId_PLMgrActs
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ItemId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_RemoveById = c2AddMethodInfo(methiNew);
    }

    // Remove an item at a specific index
    {
        TMEngMethodInfo methiNew
        (
            L"RemoveItemAt"
            , m_c2EnumId_PLMgrActs
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Index", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_RemoveItemAt = c2AddMethodInfo(methiNew);
    }

    // Reset the playlist to empty and default settings
    {
        TMEngMethodInfo methiNew
        (
            L"Reset"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Reset = c2AddMethodInfo(methiNew);
    }

    // Select an item by its unique item id
    {
        TMEngMethodInfo methiNew
        (
            L"SelectById"
            , m_c2EnumId_PLMgrActs
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ItemId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SelectById = c2AddMethodInfo(methiNew);
    }

    // Set the next or previous item in teh list, get back the index
    {
        TMEngMethodInfo methiNew
        (
            L"SelectNewItem"
            , m_c2EnumId_PLMgrSelRes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Next", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"NewIndex", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SelectNewItem = c2AddMethodInfo(methiNew);
    }

    // Set the driver moniker for our driver
    {
        TMEngMethodInfo methiNew
        (
            L"SetMoniker"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetMoniker = c2AddMethodInfo(methiNew);
    }

    // Set the playlist mode for this playlist
    {
        TMEngMethodInfo methiNew
        (
            L"SetPLMode"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ModeToSet", m_c2EnumId_PLModes);
        m_c2MethId_SetPLMode = c2AddMethodInfo(methiNew);
    }

    // Set verbose logging level
    {
        TMEngMethodInfo methiNew
        (
            L"SetVerbose"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SetVerbose = c2AddMethodInfo(methiNew);
    }
}


// Create a new value of our type upon request
TMEngClassVal*
TCQCPLMgrInfo::pmecvMakeStorage(const   TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCPLMgrValue(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCPLMgrInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCPLMgrInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                            , const TMEngMethodInfo&    methiTarget
                            ,       TMEngClassVal&      mecvInstance)
{
    // Get the this object out and the playlist manager it holds
    TCQCPLMgrValue& mecvActual = static_cast<TCQCPLMgrValue&>(mecvInstance);
    TCQCMediaPLMgr& mplmThis = mecvActual.mplmValue();

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Reset the value object
        mplmThis.Reset();
    }
     else if (c2MethId == m_c2MethId_AddMedia)
    {
        try
        {
            tCQCMedia::EPLMgrActs eAct = mplmThis.eAddMedia
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.bStackValAt(c4FirstInd + 2)
                , tCQCMedia::EMTFlags::MultiMedia
                , meOwner.strStackValAt(c4FirstInd + 3)
            );

            // Translate the action enum back to the CML equiv
            TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
            mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eAct));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrId_AddFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetCurCookie)
    {
        TMEngStringVal& mecvRepo = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd);
        TMEngStringVal& mecvCookie = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1);

        tCIDLib::TBoolean bIsItem;
        const tCIDLib::TBoolean bRet = mplmThis.bQueryCurCookie
        (
            mecvRepo.strValue(), mecvCookie.strValue(), bIsItem
        );

        TMEngBooleanVal& mecvIsItem = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd + 2);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);

        mecvIsItem.bValue(bIsItem);
        mecvRet.bValue(bRet);
    }
     else if (c2MethId == m_c2MethId_GetCurIndex)
    {
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(mplmThis.c4CurIndex());
    }
     else if (c2MethId == m_c2MethId_GetCurItem)
    {
        if (mplmThis.bIsEmpty())
            ThrowAnErr(meOwner, m_c4ErrId_EmptyList);
        else if (mplmThis.c4CurIndex() == kCIDLib::c4MaxCard)
            ThrowAnErr(meOwner, m_c4ErrId_IndexErr);

        TCQCPLItemValue& mecvToFill = meOwner.mecvStackAtAs<TCQCPLItemValue>(c4FirstInd);
        mecvToFill.mpliValue(mplmThis.mpliCurrent());
    }
     else if (c2MethId == m_c2MethId_GetCurItemId)
    {
        tCIDLib::TCard4 c4CurId;
        tCIDLib::TBoolean bRet = mplmThis.bQueryCurItemId(c4CurId);

        // Give back the return and the output parm
        TMEngCard4Val& mecvId = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd);
        mecvId.c4Value(c4CurId);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(bRet);
    }
     else if (c2MethId == m_c2MethId_GetItemAt)
    {
        const tCIDLib::TCard4 c4Index = c4CheckIParm(mecvActual, meOwner, c4FirstInd);
        TCQCPLItemValue& mecvToFill = meOwner.mecvStackAtAs<TCQCPLItemValue>(c4FirstInd + 1);
        mecvToFill.mpliValue(mplmThis.mpliAt(c4Index));
    }
     else if (c2MethId == m_c2MethId_GetItemCount)
    {
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(mplmThis.c4ItemCount());
    }
     else if (c2MethId == m_c2MethId_GetSerialNum)
    {
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(mplmThis.c4SerialNum());
    }
     else if (c2MethId == m_c2MethId_GetPLMode)
    {
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(mplmThis.ePlayListMode()));
    }
     else if (c2MethId == m_c2MethId_GetVerbose)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue
        (
            mplmThis.eVerboseLevel() > tCQCKit::EVerboseLvls::Off
        );
    }
     else if (c2MethId == m_c2MethId_FormatCurPLItem)
    {
        // Get the output memory buffer to fill in
        TMEngMemBufVal& mecvOut = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const tCIDLib::TCard4 c4Size = facCQCMedia().c4FormatCurPLItem
        (
            mplmThis, mecvOut.mbufValue()
        );

        // The return value is the bytes formatted
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4Size);
    }
     else if (c2MethId == m_c2MethId_FormatPL)
    {
        // Get the output memory buffer to fill in
        TMEngMemBufVal& mecvOut = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const tCIDLib::TCard4 c4Size = facCQCMedia().c4FormatPLItems
        (
            mplmThis, mecvOut.mbufValue()
        );

        // The return value is the bytes formatted
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4Size);
    }
     else if (c2MethId == m_c2MethId_HasItemPlayed)
    {
        const tCIDLib::TCard4 c4Index = c4CheckIParm(mecvActual, meOwner, c4FirstInd);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(mplmThis.bHasItemPlayed(c4Index));
    }
     else if (c2MethId == m_c2MethId_IsEmpty)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(mplmThis.bIsEmpty());
    }
     else if (c2MethId == m_c2MethId_LoadFromRendPL)
    {
        try
        {
            const TString& strRendDrv = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strRendDrv
            );

            // Query the list of items from the driver if we can
            TVector<TCQCMediaPLItem> colItems;
            facCQCMedia().c4QueryPLItems(orbcSrv, strRendDrv, colItems);

            // Load us up with the returned items
            mplmThis.SetItemList(colItems, meOwner.bStackValAt(c4FirstInd + 1));
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrId_ListLoad, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_QueryStdRendFlds)
    {
        //
        //  We have to create a temp local C++ list and do the query, then copy
        //  those values over to the caller's CML vector. The caller tells us
        //  if he supports music and or movie content. We turn this into a
        //  media type flags enum.
        //
        const tCIDLib::TBoolean bMovies = meOwner.bStackValAt(c4FirstInd + 1);
        const tCIDLib::TBoolean bMusic = meOwner.bStackValAt(c4FirstInd + 2);
        tCQCMedia::EMTFlags eFlags = tCQCMedia::EMTFlags::None;
        if (bMovies && bMusic)
            eFlags = tCQCMedia::EMTFlags::MultiMedia;
        else if (bMovies)
            eFlags = tCQCMedia::EMTFlags::Movie;
        else if (bMusic)
            eFlags = tCQCMedia::EMTFlags::Music;

        // Get the caller's vector
        TMEngVectorVal& mecvToFill = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);

        // If no flags, just empty it, else laod it up.
        if (eFlags == tCQCMedia::EMTFlags::None)
        {
            mecvToFill.RemoveAll();
        }
         else
        {
            TVector<TCQCFldDef> colFlds;
            TCQCStdMediaRendDrv::QueryStdRendFields
            (
                colFlds
                , eFlags
                , meOwner.c4StackValAt(c4FirstInd + 3)
                , meOwner.c4StackValAt(c4FirstInd + 4)
            );

            // Now iterate the C++ list and copy to the CML vector
            const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                mecvToFill.AddObject
                (
                    new TCQCFldDefVal
                    (
                        TString::strEmpty()
                        , m_c2TypeId_FldDef
                        , tCIDMacroEng::EConstTypes::NonConst
                        , colFlds[c4Index]
                    )
                );
            }
        }
    }
     else if (c2MethId == m_c2MethId_RandomCatUpdate)
    {
        // Just pass it on to the playlist manager
        const tCQCMedia::EPLMgrActs eAct = mplmThis.eRandomCatUpdate
        (
            meOwner.strStackValAt(c4FirstInd)
            , meOwner.strStackValAt(c4FirstInd + 1)
        );

        // And give back the resulting action enum
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eAct));
    }
     else if (c2MethId == m_c2MethId_RemoveById)
    {
        const tCQCMedia::EPLMgrActs eAct = mplmThis.eRemoveItemById
        (
            meOwner.c4StackValAt(c4FirstInd)
        );

        // Translate the action enum back to the CML equiv
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eAct));
    }
     else if (c2MethId == m_c2MethId_RemoveItemAt)
    {
        const tCIDLib::TCard4 c4Index = c4CheckIParm(mecvActual, meOwner, c4FirstInd);
        tCQCMedia::EPLMgrActs eAct = mplmThis.eRemoveItemAt(c4Index);

        // Translate the action enum back to the CML equiv
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eAct));
    }
     else if (c2MethId == m_c2MethId_Reset)
    {
        mplmThis.Reset();
    }
     else if (c2MethId == m_c2MethId_SelectById)
    {
        const tCQCMedia::EPLMgrActs eAct = mplmThis.eSelectItemById
        (
            meOwner.c4StackValAt(c4FirstInd)
        );

        // Translate the action enum back to the CML equiv
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eAct));
    }
     else if (c2MethId == m_c2MethId_SelectNewItem)
    {
        tCIDLib::TCard4 c4NewIndex;
        tCQCMedia::EPLMgrSelRes eRes = mplmThis.eSelectNewItem
        (
            meOwner.bStackValAt(c4FirstInd), c4NewIndex
        );

        // Translate the result enum back to the CML equiv
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eRes));

        // And store the new index
        TMEngCard4Val& mecvInd = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 1);
        mecvInd.c4Value(c4NewIndex);
    }
     else if (c2MethId == m_c2MethId_SetMoniker)
    {
        mplmThis.strMoniker(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetPLMode)
    {
        const TMEngEnumVal& mecvMode = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
        mplmThis.ePlayListMode(tCQCMedia::EPLModes(mecvMode.c4Ordinal()));
    }
     else if (c2MethId == m_c2MethId_SetVerbose)
    {
        if (meOwner.bStackValAt(c4FirstInd))
            mplmThis.eVerboseLevel(tCQCKit::EVerboseLvls::High);
        else
            mplmThis.eVerboseLevel(tCQCKit::EVerboseLvls::Off);
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TCQCPLMgrInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Just a helper to cut down on the verbiage of adding a lot of literals
tCIDLib::TVoid
TCQCPLMgrInfo::AddStrLiteral(const TString& strName, const TString& strValue)
{
    AddLiteral
    (
        new TMEngLiteralVal
        (
            strName
            , tCIDMacroEng::EIntrinsics::String
            , new TMEngStringVal(strName, tCIDMacroEng::EConstTypes::Const, strValue)
        )
    );
}


//
//  A helper to pull a playlist index parameter off off the stack and check
//  it, throwing an exception if it's bad.
//
tCIDLib::TCard4
TCQCPLMgrInfo::c4CheckIParm(        TCQCPLMgrValue&     mecvTarget
                            ,       TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ParmInd)
{
    const tCIDLib::TCard4 c4Index = meOwner.c4StackValAt(c4ParmInd);
    if (c4Index >= mecvTarget.mplmValue().c4ItemCount())
        ThrowAnErr(meOwner, m_c4ErrId_IndexErr, TCardinal(c4Index));
    return c4Index;
}


// Helpers for throwing CML exceptions for our errors
tCIDLib::TVoid
TCQCPLMgrInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow) const
{
    TString strText(m_pmeciErrors->strTextValue(c4ToThrow));

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strText
        , meOwner.c4CurLine()
    );

    // And throw the C++ exception that represents a macro level exception
    throw TExceptException();
}


tCIDLib::TVoid
TCQCPLMgrInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const MFormattable&       fmtblToken1) const
{
    TString strText(m_pmeciErrors->strTextValue(c4ToThrow));
    strText.eReplaceToken(fmtblToken1, kCIDLib::chDigit1);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strText
        , meOwner.c4CurLine()
    );

    // And throw the C++ exception that represents a macro level exception
    throw TExceptException();
}

tCIDLib::TVoid
TCQCPLMgrInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TError&             errCaught) const
{
    // Log the C++ error if in verbose log mode
    if (facCQCMedia().bShouldLog(errCaught))
        TModule::LogEventObj(errCaught);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , errCaught.strErrText()
        , meOwner.c4CurLine()
    );

    // And throw the C++ exception that represents a macro level exception
    throw TExceptException();
}


