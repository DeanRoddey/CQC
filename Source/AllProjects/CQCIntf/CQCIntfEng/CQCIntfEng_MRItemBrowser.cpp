//
// FILE NAME: CQCIntfEng_MRItemBrowser.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2006
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
//  This file implements the media repository item list browser.
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
#include    "CQCIntfEng_MItemBrowser.hpp"
#include    "CQCIntfEng_MRItemBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfMRIBrwRTV,TCQCIntfHVBrwRTV)
AdvRTTIDecls(TCQCIntfMRItemBrowser,TCQCIntfHVBrowserBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_MRItemBrowser
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2 -
        //      We bumped the version because we got rid of an intervening parent
        //      class in 2.4.15. So we now handle reading in its data and toss it
        //      alway on write back out.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The caps flags for our class
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::TakesFocus
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMRIBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMRIBrwRTV: Public Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMRIBrwRTV::TCQCIntfMRIBrwRTV(const TCQCUserCtx& cuctxToUse) :

    TCQCIntfHVBrwRTV
    (
        TString::strEmpty()
        , kCIDLib::c4MaxCard
        , kCQCMedia::strRTVId_ItemName
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    )
{
}

TCQCIntfMRIBrwRTV::TCQCIntfMRIBrwRTV(const  TMediaItem&     mitemSel
                                    , const TString&        strColCookie
                                    , const TString&        strRepoMoniker
                                    , const tCIDLib::TCard4 c4SelectIndex
                                    , const TCQCUserCtx&    cuctxToUse) :
    TCQCIntfHVBrwRTV
    (
        mitemSel.strName()
        , c4SelectIndex
        , kCQCMedia::strRTVId_ItemName
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    )
    , m_mitemSelected(mitemSel)
    , m_strColCookie(strColCookie)
    , m_strRepoMoniker(strRepoMoniker)
{
}

TCQCIntfMRIBrwRTV::~TCQCIntfMRIBrwRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRIBrwRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfMRIBrwRTV::bRTValue(const   TString&    strId
                            ,       TString&    strToFill)  const
{
    // Call our parent first
    if (TParent::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // Not one of his so see if it's one of ours
    if (strId == kCQCMedia::strRTVId_ColCookie)
    {
        strToFill = m_strColCookie;
    }
     else if (strId == kCQCMedia::strRTVId_ItemCookie)
    {
        //
        //  Build up the item cookie. Note that we don't use the item id
        //  of the item. The ids in the cookies are just the 1 based offsets
        //  of the item with the collection.
        //
        strToFill = m_strColCookie;
        strToFill.Append(kCIDLib::chComma);
        strToFill.AppendFormatted(c4SelIndex() + 1, tCIDLib::ERadices::Hex);
    }
    else if (strId == kCQCMedia::strRTVId_LocInfo)
        strToFill = m_mitemSelected.strLocInfo();
    else if (strId == kCQCMedia::strRTVId_ItemArtist)
        strToFill = m_mitemSelected.strArtist();
    else if (strId == kCQCMedia::strRTVId_MediaRepoMoniker)
        strToFill = m_strRepoMoniker;
    else
        return kCIDLib::False;

    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMRItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMRItemBrowser::TCQCIntfMRItemBrowser() :

    TCQCIntfHVBrowserBase
    (
        CQCIntfEng_MRItemBrowser::eCapFlags
        , facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemName)
        , kCQCMedia::strRTVId_ItemName
        , facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
        , kCQCKit::strRTVId_SelectIndex
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MRItemBrowser)
    )
{
    //
    //  In addition to the RTVs provided by our parent class (and we tell him
    //  in the ctor call above to use our own RTV name/key for the actual item
    //  text, which is the name of the media item), we provide the other info
    //  available for the item.
    //
    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemArtist)
        , kCQCMedia::strRTVId_ItemArtist
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CollectCookie)
        , kCQCMedia::strRTVId_ColCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemCookie)
        , kCQCMedia::strRTVId_ItemCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_MediaRepoMoniker)
        , kCQCMedia::strRTVId_MediaRepoMoniker
        , tCQCKit::ECmdPTypes::Driver
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_LocInfo)
        , kCQCMedia::strRTVId_LocInfo
        , tCQCKit::ECmdPTypes::Text
    );

    // Default to no text wrap
    bNoTextWrap(kCIDLib::True);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaRepoItemBrowser");
}

TCQCIntfMRItemBrowser::
TCQCIntfMRItemBrowser(const TCQCIntfMRItemBrowser& iwdgSrc) :

    TCQCIntfHVBrowserBase(iwdgSrc)
    , m_colItems(iwdgSrc.m_colItems)
    , m_strColCookie(iwdgSrc.m_strColCookie)
    , m_strRepoMoniker(iwdgSrc.m_strRepoMoniker)
{
}

TCQCIntfMRItemBrowser::~TCQCIntfMRItemBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfMRItemBrowser&
TCQCIntfMRItemBrowser::operator=(const TCQCIntfMRItemBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_colItems       = iwdgSrc.m_colItems;
        m_strColCookie   = iwdgSrc.m_strColCookie;
        m_strRepoMoniker = iwdgSrc.m_strRepoMoniker;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfMRItemBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMRItemBrowser& iwdgOther
    (
        static_cast<const TCQCIntfMRItemBrowser&>(iwdgSrc)
    );
    return (m_strRepoMoniker == iwdgOther.m_strRepoMoniker);
}


tCIDLib::TVoid
TCQCIntfMRItemBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMRItemBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfMRItemBrowser::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetColCookie)
    {
        //
        //  We get one parameter, which is the collection cookie. Just call
        //  our helper method to update us for that cookie.
        //
        if (bLoadItems(ccfgToDo.piAt(0).m_strValue))
        {
            // Tell our base class that we reloaded the list
            ReloadedList();

            // If not hiddden, then redraw with the new contents
            if (bCanRedraw(strEventId))
                Redraw();
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        //
        //  Just store the new repository. It'll be used the next time they
        //  set a new cookie on us.
        //
        m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;
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


tCIDLib::TVoid
TCQCIntfMRItemBrowser::Initialize(  TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    m_colItems.RemoveAll();
    m_strColCookie.Clear();
}


//
//  We override these next two to allow for moniker replacement, mostly for
//  template package import/export. We have a repository moniker that we need
//  to account for.
//
tCIDLib::TVoid TCQCIntfMRItemBrowser::
QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}


//
//  We have the same runtime values as the other media item browser class,
//  so we use his RTV class.
//
TCQCCmdRTVSrc*
TCQCIntfMRItemBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    const tCIDLib::TCard4 c4Sel = c4SelectedIndex();
    if (c4Sel == kCIDLib::c4MaxCard)
        return new TCQCIntfMRIBrwRTV(cuctxToUse);

    return new TCQCIntfMRIBrwRTV
    (
        m_colItems[c4Sel]
        , m_strColCookie
        , m_strRepoMoniker
        , c4Sel
        , cuctxToUse
    );
}


tCIDLib::TVoid
TCQCIntfMRItemBrowser::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We don't support anything in an event
    if (eContext == tCQCKit::EActCmdCtx::IVEvent)
        return;

    // Set a new cookie on us
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCMedia::strCmdId_SetColCookie
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SetColCookie)
                , tCQCKit::ECmdPTypes::Text
                , facCQCMedia().strMsg(kMedMsgs::midCmdP_CollectCookie)
            )
        );
    }

    // Set us to a new repository
    {
        TCQCCmd cmdSetRepo
        (
            kCQCMedia::strCmdId_SetRepository
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetRepository)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
        );
        colCmds.objAdd(cmdSetRepo);
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfMRItemBrowser::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_MRItemBrws_RepoMon
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strRepoMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));
}


tCIDLib::TVoid
TCQCIntfMRItemBrowser::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfMRItemBrowser::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_MRItemBrws_RepoMon)
        m_strRepoMoniker = adatNew.strValue();
}


tCIDLib::TVoid
TCQCIntfMRItemBrowser::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Make sure our referenced media repo is available
    if (m_strRepoMoniker.bIsEmpty() || !facCQCKit().bDrvIsLoaded(m_strRepoMoniker))
    {
        TString strErr
        (
            kIEngErrs::errcVal_DrvNotFound, facCQCIntfEng(), m_strRepoMoniker
        );
        colErrs.objPlace(c4UniqueId(), strErr, kCIDLib::True, strWidgetId());
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TCQCIntfMRItemBrowser::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfMRItemBrowser::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfMRItemBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MRItemBrowser::c2FmtVersion))
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
        strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Read in our stuff
    strmToReadFrom >> m_strRepoMoniker;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear the list and collection cookie
    m_strColCookie.Clear();
    m_colItems.RemoveAll();
}


tCIDLib::TVoid
TCQCIntfMRItemBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_MRItemBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do our stuff and end the end marker
    strmToWriteTo   << m_strRepoMoniker
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the base vert list browser class know how many display values we have
//  available.
//
tCIDLib::TCard4 TCQCIntfMRItemBrowser::c4NameCount() const
{
    return m_colItems.c4ElemCount();
}


//
//  Lets the base vert list browser class get access to our display values
//  when he needs them.
//
const TString&
TCQCIntfMRItemBrowser::strDisplayTextAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colItems[c4Index].strName();
}


// ---------------------------------------------------------------------------
//  TCQCIntfMRItemBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to get us to download our list of items from the
//  associated media repository.
//
tCIDLib::TBoolean
TCQCIntfMRItemBrowser::bLoadItems(const TString& strColCookie)
{
    // Assume we won't find it until proven otherwise
    m_strColCookie.Clear();
    m_colItems.RemoveAll();

    if (!m_strRepoMoniker.bIsEmpty())
    {
        try
        {
            // See if we have local cached media info for our repo
            TString strDBSerialNum;
            TFacCQCMedia::TMDBPtr cptrMetaData;
            facCQCMedia().eGetMediaDB
            (
                m_strRepoMoniker, strDBSerialNum, cptrMetaData
            );

            // If we got the database, then see if we can load the items
            if (cptrMetaData.pobjData())
            {
                const TMediaDB& mdbData = cptrMetaData->mdbData();

                // Find the indicated collection
                tCQCMedia::EMediaTypes  eMType;
                const TMediaTitleSet*   pmtsFound;
                const TMediaCollect*    pmcolFound;
                const TMediaItem*       pmitemFound;

                tCIDLib::TBoolean bRes = mdbData.bFindByCookie
                (
                    strColCookie, eMType, pmtsFound, pmcolFound, pmitemFound
                );

                if (bRes && pmcolFound)
                {
                    // Iterate the items in this collection and store them
                    const tCIDLib::TCard4 c4Count = pmcolFound->c4ItemCount();
                    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                    {
                        m_colItems.objAdd
                        (
                            pmcolFound->mitemAt(mdbData, c4Index)
                        );
                    }

                    // It all went well, so store the new cookie
                    m_strColCookie = strColCookie;
                }
            }
        }

        catch(const TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            // Tell the user it failed
            civOwner().ShowErr
            (
                civOwner().strTitle()
                , facCQCIntfEng().strMsg
                  (
                    kIEngMsgs::midStatus_CantLoadItems, m_strRepoMoniker
                  )
                , errToCatch
            );

            m_colItems.RemoveAll();
            return kCIDLib::False;
        }
    }

    // If we stored the cookie, it worked
    return !m_strColCookie.bIsEmpty();
}

