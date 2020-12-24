//
// FILE NAME: CQCMedia_MediaInfoClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2003
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
//  This file implements the info class for a CML level media metadata access
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
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCMediaInfoInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMedia_MediaInfoClass
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strLocTypePath(L"MEng.System.CQC.Runtime.CQCMediaInfo.CQCMLocTypes");
        const TString   strMediaTypePath(L"MEng.System.CQC.Runtime.CQCMediaInfo.CQCMediaTypes");
        const TString   strMediaInfo(L"CQCMediaInfo");
        const TString   strMediaInfoClassPath(L"MEng.System.CQC.Runtime.CQCMediaInfo");
        const TString   strMediaInfoBasePath(L"MEng.System.CQC.Runtime");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCMediaInfoInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCMediaInfoInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCMediaInfoInfo::strLocTypePath()
{
    return CQCMedia_MediaInfoClass::strLocTypePath;
}

const TString& TCQCMediaInfoInfo::strMediaTypePath()
{
    return CQCMedia_MediaInfoClass::strMediaTypePath;
}

const TString& TCQCMediaInfoInfo::strPath()
{
    return CQCMedia_MediaInfoClass::strMediaInfoClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCMediaInfoInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMediaInfoInfo::TCQCMediaInfoInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMedia_MediaInfoClass::strMediaInfo
        , CQCMedia_MediaInfoClass::strMediaInfoBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_CookieTypes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_LocTypes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_MediaTypes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_SortOrders(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAllCatCookie(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAudioFmt(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCatList(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColDetails(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetColDetails2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCookieType(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCoverArt(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCurRendArt(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemDetails(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetItemList(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTitleDetails(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTitleDetails2(kCIDMacroEng::c2BadId)
    , m_c4ErrAccess(kCIDLib::c4MaxCard)
    , m_pmeciCookieTypes(0)
    , m_pmeciErrors(0)
    , m_pmeciLocTypes(0)
    , m_pmeciMediaTypes(0)
    , m_pmeciSortOrders(0)
    , m_pmeciStrList(0)
{
}

TCQCMediaInfoInfo::~TCQCMediaInfoInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCMediaInfoInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCMediaInfoInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner, L"CQCMediaErrors", strClassPath(), L"MEng.Enum", 2
        );
        m_c4ErrAccess = m_pmeciErrors->c4AddEnumItem(L"AccessErr", TString::strEmpty());
        m_c4ErrCookieFmt = m_pmeciErrors->c4AddEnumItem(L"CookieFmt", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Create an enum for the cookie type
    {
        m_pmeciCookieTypes = new TMEngEnumInfo
        (
            meOwner, L"CQCMCookieTypes", strClassPath(), L"MEng.Enum", 4
        );
        m_pmeciCookieTypes->c4AddEnumItem(L"Category", L"Catgegory Cookie", tCQCMedia::ECookieTypes::Cat);
        m_pmeciCookieTypes->c4AddEnumItem(L"Title", L"Title Cookie", tCQCMedia::ECookieTypes::Title);
        m_pmeciCookieTypes->c4AddEnumItem(L"Collect", L"Collection Cookie", tCQCMedia::ECookieTypes::Collect);
        m_pmeciCookieTypes->c4AddEnumItem(L"Item", L"Item Cookie", tCQCMedia::ECookieTypes::Item);
        m_pmeciCookieTypes->BaseClassInit(meOwner);
        m_c2EnumId_CookieTypes = meOwner.c2AddClass(m_pmeciCookieTypes);
        bAddNestedType(m_pmeciCookieTypes->strClassPath());
    }

    // Create an enum for the loc type
    {
        m_pmeciLocTypes = new TMEngEnumInfo
        (
            meOwner, L"CQCMLocTypes", strClassPath(), L"MEng.Enum", 3
        );
        m_pmeciLocTypes->c4AddEnumItem
        (
            L"FileCol"
            , tCQCMedia::strXlatELocTypes(tCQCMedia::ELocTypes::FileCol)
            , tCQCMedia::ELocTypes::FileCol
        );
        m_pmeciLocTypes->c4AddEnumItem
        (
            L"FileItem"
            , tCQCMedia::strXlatELocTypes(tCQCMedia::ELocTypes::FileItem)
            , tCQCMedia::ELocTypes::FileItem
        );
        m_pmeciLocTypes->c4AddEnumItem
        (
            L"Changer"
            , tCQCMedia::strXlatELocTypes(tCQCMedia::ELocTypes::Changer)
            , tCQCMedia::ELocTypes::Changer
        );
        m_pmeciLocTypes->BaseClassInit(meOwner);
        m_c2EnumId_LocTypes = meOwner.c2AddClass(m_pmeciLocTypes);
        bAddNestedType(m_pmeciLocTypes->strClassPath());
    }

    // Create an enum for the media type
    {
        m_pmeciMediaTypes = new TMEngEnumInfo
        (
            meOwner, L"CQCMediaTypes", strClassPath(), L"MEng.Enum", 3
        );
        m_pmeciMediaTypes->c4AddEnumItem(L"Movie", L"Movie", tCQCMedia::EMediaTypes::Movie);
        m_pmeciMediaTypes->c4AddEnumItem(L"Music", L"Music", tCQCMedia::EMediaTypes::Music);
        m_pmeciMediaTypes->c4AddEnumItem(L"Pic", L"Picture", tCQCMedia::EMediaTypes::Pic);
        m_pmeciMediaTypes->BaseClassInit(meOwner);
        m_c2EnumId_MediaTypes = meOwner.c2AddClass(m_pmeciMediaTypes);
        bAddNestedType(m_pmeciMediaTypes->strClassPath());
    }

    // Create an enum for the sort orders type
    {
        m_pmeciSortOrders = new TMEngEnumInfo
        (
            meOwner, L"MediaSortOrders", strClassPath(), L"MEng.Enum", 3
        );
        m_pmeciSortOrders->c4AddEnumItem(L"Artist", L"By Artist", tCQCMedia::ESortOrders::Artist);
        m_pmeciSortOrders->c4AddEnumItem(L"Title", L"By Title", tCQCMedia::ESortOrders::Title);
        m_pmeciSortOrders->c4AddEnumItem(L"Year", L"By Year", tCQCMedia::ESortOrders::Year);
        m_pmeciSortOrders->BaseClassInit(meOwner);
        m_c2EnumId_SortOrders = meOwner.c2AddClass(m_pmeciSortOrders);
        bAddNestedType(m_pmeciSortOrders->strClassPath());
    }

    // Create a nested vector of strings type
    {
        TMEngVectorInfo* pmeciNew = new TMEngVectorInfo
        (
            meOwner
            , L"StrList"
            , strClassPath()
            , TMEngVectorInfo::strPath()
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        meOwner.c2AddClass(janNewClass.pobjOrphan());
        m_pmeciStrList = pmeciNew;
    }

    // Create a nested vector of card4 type
    {
        TMEngVectorInfo* pmeciNew = new TMEngVectorInfo
        (
            meOwner
            , L"CardList"
            , strClassPath()
            , TMEngVectorInfo::strPath()
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::Card4)
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        meOwner.c2AddClass(janNewClass.pobjOrphan());
        m_pmeciCardList = pmeciNew;
    }

    // The default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCMediaInfo"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Get the Allxxx category cookie for a given media type
    {
        TMEngMethodInfo methiNew
        (
            L"GetAllCatCookie"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"MediaType", m_c2EnumId_MediaTypes);
        m_c2MethId_GetAllCatCookie = c2AddMethodInfo(methiNew);
    }

    // Get the audio format of a particular title or item
    {
        TMEngMethodInfo methiNew
        (
            L"GetAudioFmt"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Cookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"BitDepth", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"BitRate", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Channels", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"SampleRate", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetAudioFmt = c2AddMethodInfo(methiNew);
    }

    // Get the list of categories
    {
        TMEngMethodInfo methiNew
        (
            L"GetCatList"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"MediaType", m_c2EnumId_MediaTypes);
        methiNew.c2AddInParm(L"NoEmpty", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"Names", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Cookies", m_pmeciStrList->c2Id());
        m_c2MethId_GetCatList = c2AddMethodInfo(methiNew);
    }

    //
    //  Get collection details based on collection cookie. We have a
    //  2 version that gets more info recently added.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"GetColDetails"
            , m_pmeciMediaTypes->c2Id()
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ColCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"TitleName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Artist", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"AspectRatio", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Cast", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Descr", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Label", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"LeadActor", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"LocInfo", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Rating", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ColCount", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Duration", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ItemCnt", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Year", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"LocType", m_pmeciLocTypes->c2Id());
        m_c2MethId_GetColDetails = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetColDetails2"
            , m_pmeciMediaTypes->c2Id()
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ColCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"TitleName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Artist", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"AspectRatio", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Cast", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Descr", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Label", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"LeadActor", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"LocInfo", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Rating", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ColNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Duration", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ItemCnt", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Year", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"LocType", m_pmeciLocTypes->c2Id());
        methiNew.c2AddOutParm(L"MediaFmt", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetColDetails2 = c2AddMethodInfo(methiNew);
    }

    // Figure out a cookie type
    {
        TMEngMethodInfo methiNew
        (
            L"GetCookieType"
            , m_c2EnumId_CookieTypes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Cookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"MediaType", m_c2EnumId_MediaTypes);
        m_c2MethId_GetCookieType = c2AddMethodInfo(methiNew);
    }

    // Query the default cover art for a collection/title cookie
    {
        TMEngMethodInfo methiNew
        (
            L"GetCoverArt"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Cookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ImgData", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddOutParm(L"OutBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"LargeArt", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetCoverArt = c2AddMethodInfo(methiNew);
    }

    // Get the current cover art from a renderer
    {
        TMEngMethodInfo methiNew
        (
            L"GetCurRendArt"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"RendMoniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LargeArt", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"ImgData", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddOutParm(L"OutBytes", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetCurRendArt = c2AddMethodInfo(methiNew);
    }

    // Get item detals based on an item coookie
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemDetails"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ItemCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Artist", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"LocInfo", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetItemDetails = c2AddMethodInfo(methiNew);
    }

    // Get a list of items for the indicated collection from a repo
    {
        TMEngMethodInfo methiNew
        (
            L"GetItemList"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"RepoMoniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ColCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Names", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"ItemCookies", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"LocInfo", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Artists", m_pmeciStrList->c2Id());
        m_c2MethId_GetItemList = c2AddMethodInfo(methiNew);
    }

    // Get title details based on a title cookie
    {
        TMEngMethodInfo methiNew
        (
            L"GetTitleDetails"
            , m_pmeciMediaTypes->c2Id()
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"TitleCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Title", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ColCount", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Names", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Cookies", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Artists", m_pmeciStrList->c2Id());
        m_c2MethId_GetTitleDetails = c2AddMethodInfo(methiNew);
    }

    // A 2 version of the title details query that adds another parm
    {
        TMEngMethodInfo methiNew
        (
            L"GetTitleDetails2"
            , m_pmeciMediaTypes->c2Id()
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"TitleCookie", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Title", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ColCount", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"Names", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Cookies", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Artists", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"MediaFormat", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetTitleDetails2 = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCMediaInfoInfo::pmecvMakeStorage(const   TString&                strName
                                    ,       TCIDMacroEngine&
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCMediaInfoInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCMediaInfoInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    // Don't currently need this
//    TMEngStdClassVal& mecvActual = static_cast<TMEngStdClassVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_GetAllCatCookie)
    {
        // Format out the cookie for the media type requested
        const TMEngEnumVal& mecvMType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
        const tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes
        (
            m_pmeciMediaTypes->c4MapValue(mecvMType)
        );

        TString strTmp;
        facCQCMedia().FormatCatCookie
        (
            eMType
            , eMType == tCQCMedia::EMediaTypes::Music ? kCQCMedia::c2CatId_AllMusic
                                                    : kCQCMedia::c2CatId_AllMovies
            , strTmp
        );

        // Give back the formatted cookie
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvRet.strValue(strTmp);
    }
     else if (c2MethId == m_c2MethId_GetAudioFmt)
    {
        // Get the in params and make the call
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strCookie = meOwner.strStackValAt(c4FirstInd + 1);

        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            tCIDLib::TCard4 c4BitDepth;
            tCIDLib::TCard4 c4BitRate;
            tCIDLib::TCard4 c4Channels;
            tCIDLib::TCard4 c4SampleRate;
            const tCIDLib::TBoolean bRet = facCQCMedia().bQueryAudioFmt
            (
                orbcSrv
                , strMoniker
                , strCookie
                , c4BitDepth
                , c4BitRate
                , c4Channels
                , c4SampleRate
            );

            // If it worked, load up the values
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 2).c4Value(c4BitDepth);
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3).c4Value(c4BitRate);
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 4).c4Value(c4Channels);
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 5).c4Value(c4SampleRate);

            // Return the status
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(bRet);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetCatList)
    {
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TMEngEnumVal& mecvMType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 1);
        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            tCQCMedia::TNameList colNames;
            tCQCMedia::TNameList colCookies;
            tCIDLib::TCard4 c4Count = facCQCMedia().c4QueryCatList
            (
                orbcSrv
                , strMoniker
                , tCQCMedia::EMediaTypes(m_pmeciMediaTypes->c4MapValue(mecvMType))
                , meOwner.bStackValAt(c4FirstInd + 2)
                , colNames
                , colCookies
            );

            // If we got any, then set up the return collections
            TMEngVectorVal& mecvNames = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 3);
            TMEngVectorVal& mecvCookies = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 4);

            mecvCookies.RemoveAll();
            mecvNames.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                mecvNames.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst, colNames[c4Index]
                    )
                );
                mecvCookies.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst, colCookies[c4Index]
                    )
                );
            }

            // Return the count
            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value(c4Count);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else if ((c2MethId == m_c2MethId_GetColDetails)
          ||  (c2MethId == m_c2MethId_GetColDetails2))
    {
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strColCookie = meOwner.strStackValAt(c4FirstInd + 1);
        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            tCIDLib::TCard4 c4Ind = 2;
            TMEngStringVal& mecvTitleName
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvName
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvArtist
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvAspect
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvCast
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvDescr
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvLabel
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvLeadActor
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvLocInfo
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngStringVal& mecvRating
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngCard4Val& mecvColNum
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngCard4Val& mecvDuration
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngCard4Val& mecvItemCnt
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngCard4Val& mecvYear
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + c4Ind);
            c4Ind++;
            TMEngEnumVal& mecvLocType
                    = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + c4Ind);

            //!! Keep the parm index correct since we have more uses for it below
            tCIDLib::TCard4 c4ColCount;
            TMediaCollect   mcolQuery;
            TString         strTitleName;
            const tCQCMedia::EMediaTypes eType = facCQCMedia().eQueryColDetails
            (
                orbcSrv
                , strMoniker
                , strColCookie
                , mcolQuery
                , strTitleName
                , c4ColCount
            );

            mecvTitleName.strValue(strTitleName);
            mecvName.strValue(mcolQuery.strName());
            mecvArtist.strValue(mcolQuery.strArtist());
            mecvAspect.strValue(mcolQuery.strAspectRatio());
            mecvCast.strValue(mcolQuery.strCast());
            mecvDescr.strValue(mcolQuery.strDescr());
            mecvLabel.strValue(mcolQuery.strLabel());
            mecvLeadActor.strValue(mcolQuery.strLeadActor());
            mecvLocInfo.strValue(mcolQuery.strLocInfo());
            mecvRating.strValue(mcolQuery.strRating());

            mecvColNum.c4Value(c4ColCount);
            mecvDuration.c4Value(mcolQuery.c4Duration());
            mecvItemCnt.c4Value(mcolQuery.c4ItemCount());
            mecvYear.c4Value(mcolQuery.c4Year());
            mecvLocType.c4Ordinal(tCIDLib::c4EnumOrd(mcolQuery.eLocType()));

            // If it's the 2 version, give back the extra stuff
            if (c2MethId == m_c2MethId_GetColDetails2)
            {
                c4Ind++;
                TMEngStringVal& mecvMediaFmt
                        = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + c4Ind);
                mecvMediaFmt.strValue(mcolQuery.strMediaFormat());
            }

            // Set the return value
            TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
            mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eType));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetCookieType)
    {
        const TString& strCookie = meOwner.strStackValAt(c4FirstInd);
        try
        {
            tCQCMedia::EMediaTypes eMType;
            tCIDLib::TCard2        c2CatId;
            tCIDLib::TCard2        c2TitleId;
            tCIDLib::TCard2        c2ColId;
            tCIDLib::TCard2        c2ItemId;
            tCQCMedia::ECookieTypes eCookieType = facCQCMedia().eCheckCookie
            (
                strCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
            );

            TMEngEnumVal& mecvMType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 1);
            mecvMType.c4Ordinal(tCIDLib::c4EnumOrd(eMType));

            TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
            mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eCookieType));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCookieFmt, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetCoverArt)
    {
        // Get the moniker and cookie we'll retrieve for
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strCookie = meOwner.strStackValAt(c4FirstInd + 1);

        // Get the output memory buffer
        TMEngMemBufVal& mecvOut = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 2);

        // Call a helper on the media facility to do the grunt work
        tCIDLib::TCard4 c4OutBytes = 0;
        tCIDLib::TBoolean bRet = facCQCMedia().bQueryRepoDrvArt
        (
            strMoniker
            , strCookie
            , meOwner.bStackValAt(c4FirstInd + 4) ? tCQCMedia::ERArtTypes::LrgCover
                                                  : tCQCMedia::ERArtTypes::SmlCover
            , c4OutBytes
            , mecvOut.mbufValue()
        );

        // Get the out bytes and return value parms and set them
        TMEngCard4Val& mecvSz = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvSz.c4Value(c4OutBytes);
        mecvRet.bValue(bRet);
    }
     else if (c2MethId == m_c2MethId_GetCurRendArt)
    {
        TMEngMemBufVal& mecvBuf = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 2);

        tCIDLib::TCard4 c4Bytes = 0;
        const tCIDLib::TBoolean bRes = facCQCMedia().bQueryCurRendArt
        (
            meOwner.strStackValAt(c4FirstInd)
            , c4Bytes
            , mecvBuf.mbufValue()
            , meOwner.bStackValAt(c4FirstInd + 1)
        );

        TMEngCard4Val& mecvBytes = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);

        mecvBytes.c4Value(c4Bytes);
        mecvRet.bValue(bRes);
    }
     else if (c2MethId == m_c2MethId_GetItemDetails)
    {
        const TString& strRepoMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strItemCookie = meOwner.strStackValAt(c4FirstInd + 1);
        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strRepoMoniker
            );

            // Get the output parms
            TMEngStringVal& mecvName = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2);
            TMEngStringVal& mecvArtist = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 3);
            TMEngStringVal& mecvLoc = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 4);

            TMediaItem mitemQuery;
            facCQCMedia().QueryItemDetails
            (
                orbcSrv
                , strRepoMoniker
                , strItemCookie
                , mitemQuery
            );

            mecvName.strValue(mitemQuery.strName());
            mecvArtist.strValue(mitemQuery.strArtist());
            mecvLoc.strValue(mitemQuery.strLocInfo());
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetItemList)
    {
        const TString& strRepoMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strColCookie = meOwner.strStackValAt(c4FirstInd + 1);
        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strRepoMoniker
            );

            TVector<TMediaItem> colItems;

            const tCIDLib::TCard4 c4Count = facCQCMedia().c4QueryColItems
            (
                orbcSrv, strRepoMoniker, strColCookie, colItems
            );

            TMEngVectorVal& mecvNames = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 2);
            mecvNames.RemoveAll();
            TMEngVectorVal& mecvCookies = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 3);
            mecvCookies.RemoveAll();
            TMEngVectorVal& mecvLocs = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 4);
            mecvLocs.RemoveAll();
            TMEngVectorVal& mecvArtists = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 5);
            mecvArtists.RemoveAll();

            TString strItemCookie;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TMediaItem& mitemCur = colItems[c4Index];

                // Build up the item cookie for this one
                strItemCookie = strColCookie;
                strItemCookie.Append(kCIDLib::chComma);
                strItemCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);

                mecvNames.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , mitemCur.strName()
                    )
                );

                mecvCookies.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , strItemCookie
                    )
                );

                mecvLocs.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , mitemCur.strLocInfo()
                    )
                );

                mecvArtists.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , mitemCur.strArtist()
                    )
                );
            }

            // Return the count
            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value(c4Count);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else if ((c2MethId == m_c2MethId_GetTitleDetails)
          ||  (c2MethId == m_c2MethId_GetTitleDetails2))
    {
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strTitleCookie = meOwner.strStackValAt(c4FirstInd + 1);
        try
        {
            // Try to get a proxy for the indicated driver
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            TMEngStringVal& mecvTitle
                    = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2);
            TMEngCard4Val& mecvColCount
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3);
            TMEngVectorVal& mecvNames
                    = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 4);
            TMEngVectorVal& mecvCookies
                    = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 5);
            TMEngVectorVal& mecvArtists
                    = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 6);

            // Parse the data out
            tCIDLib::TCard4      c4ColCount;
            tCIDLib::TCard4      c4Year;
            tCQCMedia::TNameList colArtists;
            tCQCMedia::TNameList colCookies;
            tCQCMedia::TNameList colNames;
            TString              strArtist;
            TString              strMediaFmt;
            const tCQCMedia::EMediaTypes eType = facCQCMedia().eQueryTitleDetails
            (
                strMoniker
                , strTitleCookie
                , mecvTitle.strValue()
                , strArtist
                , strMediaFmt
                , c4ColCount
                , c4Year
                , colNames
                , colCookies
                , colArtists
            );

            //
            //  We need to now load up the outgoing cookie list with CML level
            //  strings.
            //
            mecvCookies.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
            {
                mecvNames.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst, colNames[c4Index]
                    )
                );

                mecvCookies.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst, colCookies[c4Index]
                    )
                );

                mecvArtists.AddObject
                (
                    new TMEngStringVal
                    (
                        TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst, colArtists[c4Index]
                    )
                );
            }

            // If the 2 version, give back the media format
            if (c2MethId == m_c2MethId_GetTitleDetails2)
            {
                TMEngStringVal& mecvMediaFmt
                        = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 7);
                mecvMediaFmt.strValue(strMediaFmt);
            }

            // Return the count of collections
            mecvColCount.c4Value(c4ColCount);

            // Set the return value
            TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
            mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eType));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch);
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCMediaInfoInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCMediaInfoInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
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

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


