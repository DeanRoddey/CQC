//
// FILE NAME: CQCIntfEng_MediaCatBrowser.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/11/2005
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
//  This file implements the media category browser
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
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_MediaCatBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfMCatBRTV, TCQCIntfHVBrwRTV)
AdvRTTIDecls(TCQCIntfMediaCatBrowser,TCQCIntfHVBrowserBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_MediaCatBrowser
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 1 -
        //      Reset to version 1 for the 2.0 release since we have to do a
        //      manual conversion during the ugprade.
        //
        //  Version 2 -
        //      We bumped the version because we got rid of an intervening parent
        //      class in 2.4.15. So we now handle reading in its data and toss it
        //      alway on write back out.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::TakesFocus
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfHBrwRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMCatBRTV: Public Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMCatBRTV::TCQCIntfMCatBRTV( const   TString&        strCookie
                                    , const TString&        strText
                                    , const tCIDLib::TCard4 c4SelectIndex
                                    , const TCQCUserCtx&    cuctxToUse) :
    TCQCIntfHVBrwRTV
    (
        strText
        , c4SelectIndex
        , kCQCMedia::strRTVId_CategoryName
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    )
    , m_strCookie(strCookie)
{
}

TCQCIntfMCatBRTV::~TCQCIntfMCatBRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMCatBRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfMCatBRTV::bRTValue(const  TString&    strId
                                            ,       TString&    strToFill) const
{
    // Call our parent first
    if (TParent::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // If it's one of ours, then handle it
    if (strId == kCQCMedia::strRTVId_CategoryCookie)
    {
        strToFill = m_strCookie;
        return kCIDLib::True;
    }

    // Dunno what it is
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaCatBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMediaCatBrowser::TCQCIntfMediaCatBrowser() :

    TCQCIntfHVBrowserBase
    (
        CQCIntfEng_MediaCatBrowser::eCapFlags
        , facCQCMedia().strMsg(kMedMsgs::midRTVal_CategoryName)
        , kCQCMedia::strRTVId_CategoryName
        , facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
        , kCQCKit::strRTVId_SelectIndex
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MCatBrowser)
    )
    , m_eInitMediaType(tCQCMedia::EMediaTypes::Music)
    , m_eMediaFlags(tCQCMedia::EMTFlags::None)
    , m_eMediaType(tCQCMedia::EMediaTypes::Count)
{
    //
    //  We provide one extra runtime value beyond what our parent browser class
    //  provides, the cookie for the selected category. Our base class handles
    //  the h/v browser specific stuff, but we do force the use of our own
    //  RTV name/key in his ctor call above. We use the default for the
    //  selected index.
    //
    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CatCookie)
        , kCQCMedia::strRTVId_CategoryCookie
        , tCQCKit::ECmdPTypes::Text
    );

    // Default to horz centered text
    eHJustify(tCIDLib::EHJustify::Center);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaCatBrowser");
}

TCQCIntfMediaCatBrowser::TCQCIntfMediaCatBrowser(const TCQCIntfMediaCatBrowser& iwdgSrc) :

    TCQCIntfHVBrowserBase(iwdgSrc)
    , m_colCats(iwdgSrc.m_colCats)
    , m_eInitMediaType(iwdgSrc.m_eInitMediaType)
    , m_eMediaFlags(iwdgSrc.m_eMediaFlags)
    , m_eMediaType(iwdgSrc.m_eMediaType)
    , m_strRepoMoniker(iwdgSrc.m_strRepoMoniker)
{
}

TCQCIntfMediaCatBrowser::~TCQCIntfMediaCatBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfMediaCatBrowser&
TCQCIntfMediaCatBrowser::operator=(const TCQCIntfMediaCatBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_colCats        = iwdgSrc.m_colCats;
        m_eInitMediaType = iwdgSrc.m_eInitMediaType;
        m_eMediaFlags    = iwdgSrc.m_eMediaFlags;
        m_eMediaType     = iwdgSrc.m_eMediaType;
        m_strRepoMoniker = iwdgSrc.m_strRepoMoniker;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfMediaCatBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMediaCatBrowser& iwdgOther
    (
        static_cast<const TCQCIntfMediaCatBrowser&>(iwdgSrc)
    );

    return
    (
        (m_colCats == iwdgOther.m_colCats)
        && (m_eInitMediaType == iwdgOther.m_eInitMediaType)
        && (m_eMediaFlags == iwdgOther.m_eMediaFlags)
        && (m_eMediaType == iwdgOther.m_eMediaType)
        && (m_strRepoMoniker == iwdgOther.m_strRepoMoniker)
    );
}


tCIDLib::TVoid
TCQCIntfMediaCatBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMediaCatBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfMediaCatBrowser::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetMediaType)
    {
        // Make sure it's a type our repository supports
        const tCQCMedia::EMediaTypes eToSet = facCQCMedia().eXlatCmdMediaType
        (
            ccfgToDo.piAt(0).m_strValue
        );

        if (eToSet == tCQCMedia::EMediaTypes::Count)
            ThrowBadParmVal(ccfgToDo, 0);

        // If not, then do nothing, else set the type and reload
        if (facCQCMedia().bTestMediaFlags(eToSet, m_eMediaFlags))
        {
            m_eMediaType = eToSet;

            // If not in the preload, then reload
            if (strEventId != kCQCKit::strEvId_OnPreload)
                LoadCategories();
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        //
        //  This reassociates us with a different repository driver. So we
        //  have to clean out our category list, store the new values, and
        //  re-initialize.
        //

        // Make sure it's a type our repository supports
        const tCQCMedia::EMediaTypes eMType = facCQCMedia().eXlatCmdMediaType
        (
            ccfgToDo.piAt(1).m_strValue
        );

        if (eMType == tCQCMedia::EMediaTypes::Count)
            ThrowBadParmVal(ccfgToDo, 1);

        m_eMediaType = eMType;
        m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;
        m_colCats.RemoveAll();

        // If not in the preload, then reload
        if (strEventId != kCQCKit::strEvId_OnPreload)
            LoadCategories();
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
        (
            ccfgToDo
            , c4Index
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }
    return tCQCKit::ECmdRes::Ok;
}


//
//  Call our parent first, then find our repository driver and load up the
//  category list.
//
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::Initialize(TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


// Gen up one of our runtime value objects
TCQCCmdRTVSrc*
TCQCIntfMediaCatBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    const tCIDLib::TCard4 c4Sel = c4SelectedIndex();
    return new TCQCIntfMCatBRTV
    (
        (c4Sel == kCIDLib::c4MaxCard) ? TString::strEmpty()
                                      : m_colCats[c4Sel].strValue()
        , (c4Sel == kCIDLib::c4MaxCard) ? TString::strEmpty()
                                        : m_colCats[c4Sel].strKey()
        , c4Sel
        , cuctxToUse
    );
}


//
//  Called to do post init loading of data. We do a load of our categories.
//
tCIDLib::TVoid TCQCIntfMediaCatBrowser::PostInit()
{
    // Only do this if not in designer mode
    if (!TFacCQCIntfEng::bDesMode())
        LoadCategories();
}


// This is called to gather up supported commands for the cmd editing dialog
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We dont' allow anything during IV events
    if (eContext == tCQCKit::EActCmdCtx::IVEvent)
        return;

    // Add the set media type command
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetMediaType
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetMediaType)
            , facCQCMedia().strMsg(kMedMsgs::midCmdP_MediaType)
            , facCQCMedia().strMTypeOpts()
        )
    );


    // Set us up to associate with a different repository
    {
        TCQCCmd cmdSetRepo
        (
            kCQCMedia::strCmdId_SetRepository
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetRepository)
            , 2
        );

        cmdSetRepo.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
                , tCQCKit::ECmdPTypes::Driver
            )
        );

        cmdSetRepo.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCMedia().strMsg(kMedMsgs::midCmdP_MediaType)
                , facCQCMedia().strMTypeOpts()
            )
        );

        colCmds.objAdd(cmdSetRepo);
    }
}


//
//  We override these next two to allow for moniker replacement, mostly for
//  template package import/export. We have a repository moniker that we need
//  to account for.
//
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}



// We call our parent first, then add a few bits of our own
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Do our stuff
    TString strLim;
    tCQCMedia::FormatEMediaTypes
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"Init Type"
        , kCQCIntfEng::strAttr_CatBrws_InitType
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCMedia::strXlatEMediaTypes(m_eInitMediaType));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_CatBrws_RepoMon
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strRepoMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAdd(adatTmp);
}


// Do search and replace at our level
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocDevice))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strRepoMoniker, bRegEx, bFull, regxFind
        );
    }
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_CatBrws_InitType)
        m_eInitMediaType = tCQCMedia::eXlatEMediaTypes(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_CatBrws_RepoMon)
        m_strRepoMoniker = adatNew.strValue();
}


// Validate our configuration for the designer
tCIDLib::TVoid
TCQCIntfMediaCatBrowser::Validate(  const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Make sure our referenced media repo is available
    if (m_strRepoMoniker.bIsEmpty() ||
        !facCQCKit().bDrvIsLoaded(m_strRepoMoniker))
    {
        TString strErr
        (
            kIEngErrs::errcVal_DrvNotFound, facCQCIntfEng(), m_strRepoMoniker
        );
        colErrs.objAdd
        (
            TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId())
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the initial media type
tCQCMedia::EMediaTypes TCQCIntfMediaCatBrowser::eInitMediaType() const
{
    return m_eInitMediaType;
}

tCQCMedia::EMediaTypes
TCQCIntfMediaCatBrowser::eInitMediaType(const tCQCMedia::EMediaTypes eToSet)
{
    m_eInitMediaType = eToSet;
    return m_eInitMediaType;
}


// Get/set the media flags
tCQCMedia::EMTFlags TCQCIntfMediaCatBrowser::eMediaFlags() const
{
    return m_eMediaFlags;
}

tCQCMedia::EMTFlags
TCQCIntfMediaCatBrowser::eMediaFlags(const tCQCMedia::EMTFlags eToSet)
{
    m_eMediaFlags = eToSet;
    return m_eMediaFlags;
}


// Get/set the current media type
tCQCMedia::EMediaTypes TCQCIntfMediaCatBrowser::eMediaType() const
{
    return m_eMediaType;
}

tCQCMedia::EMediaTypes
TCQCIntfMediaCatBrowser::eMediaType(const tCQCMedia::EMediaTypes eToSet)
{
    m_eMediaType = eToSet;
    return m_eMediaType;
}


const TString& TCQCIntfMediaCatBrowser::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfMediaCatBrowser::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfMediaCatBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MediaCatBrowser::c2FmtVersion))
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
    //  If this is V1, then eat the stuff from an old parent class that was
    //  factored out. It had no data, so just the start/stop markers and
    //  format version.
    //
    if (c2FmtVersion < 2)
    {
        strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);
        tCIDLib::TCard2 c2Tmp;
        strmToReadFrom >> c2Tmp;
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    if (c2FmtVersion < 2)
    {
        strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

        // Also force it to horizontal, since these always originally were
        bHorizontal(kCIDLib::True);
    }

    // Read in our stuff
    strmToReadFrom  >> m_strRepoMoniker
                    >> m_eMediaFlags
                    >> m_eMediaType
                    >> m_eInitMediaType;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfMediaCatBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_MediaCatBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff and end marker
    strmToWriteTo   << m_strRepoMoniker
                    << m_eMediaFlags
                    << m_eMediaType
                    << m_eInitMediaType
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the base horz list browser class know how many display values we have
//  available.
//
tCIDLib::TCard4 TCQCIntfMediaCatBrowser::c4NameCount() const
{
    return m_colCats.c4ElemCount();
}


//
//  Lets the base horz list browser class get access to our display values
//  when he needs them.
//
const TString&
TCQCIntfMediaCatBrowser::strDisplayTextAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colCats[c4Index].strKey();
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaCatBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to get us to download our list of categories from the
//  associated media repository driver.
//
tCIDLib::TVoid TCQCIntfMediaCatBrowser::LoadCategories()
{
    if (!m_strRepoMoniker.bIsEmpty())
    {
        try
        {
            // Get a ref to the latest cached database for our repo
            TString strDBSerialNum;
            TFacCQCMedia::TMDBPtr cptrMetaData;
            facCQCMedia().eGetMediaDB
            (
                m_strRepoMoniker, strDBSerialNum, cptrMetaData
            );

            // If we got the database, then let's set up
            if (cptrMetaData.pobjData())
            {
                // Store the media type flags
                m_eMediaFlags = cptrMetaData->eMTFlags();

                // Choose an initial type, if one isn't set yet
                if (m_eMediaType == tCQCMedia::EMediaTypes::Count)
                {
                    // Assume the initial value
                    m_eMediaType = m_eInitMediaType;

                    // If the initial type is not supported, then select one
                    if (!facCQCMedia().bTestMediaFlags(m_eMediaType, m_eMediaFlags))
                    {
                        if (tCIDLib::bAllBitsOn(m_eMediaFlags, tCQCMedia::EMTFlags::Music))
                            m_eMediaType = tCQCMedia::EMediaTypes::Music;
                        else if (tCIDLib::bAllBitsOn(m_eMediaFlags, tCQCMedia::EMTFlags::Movie))
                            m_eMediaType = tCQCMedia::EMediaTypes::Movie;
                        else
                            m_eMediaType = tCQCMedia::EMediaTypes::Pic;
                    }
                }

                const TMediaDB& mdbData = cptrMetaData->mdbData();

                // Get a sorted list of cats for our current media type
                TRefVector<const TMediaCat> colCats(tCIDLib::EAdoptOpts::NoAdopt);
                const tCIDLib::TCard4 c4Count = mdbData.c4QueryCatList
                (
                    m_eMediaType, colCats, &TMediaCat::eCompByName
                );

                TString strCatCookie;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TMediaCat* pmcatCur = colCats[c4Index];

                    // Format the cookie for this guy
                    facCQCMedia().FormatCatCookie
                    (
                        m_eMediaType, pmcatCur->c2Id(), strCatCookie
                    );

                    m_colCats.objAdd
                    (
                        TKeyValuePair(pmcatCur->strName(), strCatCookie)
                    );
                }

                // Let the base horz browser class know we reloaded
                ReloadedList();

                Redraw();
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bLogFailures())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIEngMsgs::midStatus_CantLoadMCats
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strRepoMoniker
                );
            }
        }
    }
}


