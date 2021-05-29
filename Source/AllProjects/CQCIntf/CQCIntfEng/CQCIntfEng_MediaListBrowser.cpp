//
// FILE NAME: CQCIntfEng_MediaListBrowser.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2010
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
//  This file implements the list based media browser.
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
#include    "CQCIntfEng_MediaListBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
//
//  Normally the enum tricks stuff would be public, but we are the only one
//  who needs to use that functionality, so no use burdening everyone else.
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfMediaListBrowser, TCQCIntfWidget)
StdEnumTricks(TCQCIntfMediaListBrowser::EDispLvls)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_MediaListBrws
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2 -
        //      No data change, but we need to rearrange our use of colors to make
        //      them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The amount of time we'll let a list in our cache hang around before
        //  we check it again.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctListTime = kCIDLib::enctOneHour;


        // -----------------------------------------------------------------------
        //  The max number of items we'll keep in our list cache before we start
        //  tossing out old ones.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4MaxCacheSz = 128;


        // -----------------------------------------------------------------------
        //  The margin we move in for clipping purposes.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4Margin = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::TakesFocus
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMLBRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class, to add any extra info
//  we support above and beyond the base runtime values. The browser will
//  provide us with the mininum of info, since it doesn't keep much around
//  due to the need for speed.
//
//  The other stuff we provide, we'll grab as the user asks for it.
// ---------------------------------------------------------------------------
class TCQCIntfMLBRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMLBRTV
        (
            const   TString&                strCatName
            , const TString&                strCatCookie
            , const TString&                strTitleArtist
            , const TString&                strTitleCookie
            , const TString&                strTitleName
            , const TString&                strColArtist
            , const TString&                strColCookie
            , const TString&                strColName
            , const TString&                strItemArtist
            , const TString&                strItemCookie
            , const TString&                strItemName
            , const TString&                strRepoMoniker
            , const TString&                strLocInfo
            , const tCQCMedia::ELocTypes    eLocType
            , const tCQCMedia::EMediaTypes  eMediaType
            , const TCQCIntfMediaListBrowser::EDispLvls eDispLevel
            , const TCQCIntfMediaListBrowser::EDispLvls eSelLevel
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TCQCIntfMLBRTV();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid GetLocType() const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eDispLevel
        //      The current display level
        //
        //  m_eLocType
        //      The location type indicator, which controls how to interpret
        //      the location info. This is faulted in if asked for.
        //
        //  m_eMediaType
        //      The media type currently displayed in the browser, so that
        //      the target widgets can respond accordingly. The browser gives
        //      us this.
        //
        //  m_eSelLevel
        //      The level in which a selection was made, which is often not
        //      the same as the display level, since the new level will have
        //      been swapped in by the time the action is run. So this
        //      represents the level whose contents is being viewed. If a
        //      selection is made at item level, they are the same.
        //
        //  m_pstrLocInfo
        //      The location info, which we fault in.
        //
        //  m_strCurName
        //      This is the name of whatever was selected, if any, so that
        //      the handler of the event doesn't have to figure this out
        //      himself. So it'll be a artist, title, collection, or item
        //      name, depending on what was selected.
        //
        //  m_strXXXX
        //      This stuff is all just the usual stuff that the browser has
        //      to have around in order to do his thing, so he passes this
        //      stuff to us.
        //
        //  m_strMediaFmt
        //      We fault this in if it's asked for.
        //
        //  m_strRepoMoniker
        //      The repository driver that we are associated with. The browser
        //      gives us this since he already has it.
        // -------------------------------------------------------------------
        TCQCIntfMediaListBrowser::EDispLvls m_eDispLevel;
        mutable tCQCMedia::ELocTypes        m_eLocType;
        tCQCMedia::EMediaTypes              m_eMediaType;
        TCQCIntfMediaListBrowser::EDispLvls m_eSelLevel;
        mutable TString*                    m_pstrLocInfo;
        TString                             m_strCatCookie;
        TString                             m_strCatName;
        TString                             m_strColArtist;
        TString                             m_strColCookie;
        TString                             m_strColName;
        TString                             m_strCurName;
        mutable TString                     m_strMediaFmt;
        TString                             m_strItemArtist;
        TString                             m_strItemCookie;
        TString                             m_strItemName;
        TString                             m_strRepoMoniker;
        TString                             m_strTitleArtist;
        TString                             m_strTitleCookie;
        TString                             m_strTitleName;
};


// ---------------------------------------------------------------------------
//  TCQCIntfMLBRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMLBRTV::
TCQCIntfMLBRTV( const   TString&                            strCatName
                , const TString&                            strCatCookie
                , const TString&                            strTitleArtist
                , const TString&                            strTitleCookie
                , const TString&                            strTitleName
                , const TString&                            strColArtist
                , const TString&                            strColCookie
                , const TString&                            strColName
                , const TString&                            strItemArtist
                , const TString&                            strItemCookie
                , const TString&                            strItemName
                , const TString&                            strRepoMoniker
                , const TString&                            strLocInfo
                , const tCQCMedia::ELocTypes                eLocType
                , const tCQCMedia::EMediaTypes              eMediaType
                , const TCQCIntfMediaListBrowser::EDispLvls eDispLevel
                , const TCQCIntfMediaListBrowser::EDispLvls eSelLevel
                , const TCQCUserCtx&                        cuctxToUse) :

    TCQCCmdRTVSrc(cuctxToUse)
    , m_eDispLevel(eDispLevel)
    , m_eSelLevel(eSelLevel)
    , m_eLocType(eLocType)
    , m_eMediaType(eMediaType)
    , m_pstrLocInfo(0)
    , m_strCatCookie(strCatCookie)
    , m_strCatName(strCatName)
    , m_strColArtist(strColArtist)
    , m_strColCookie(strColCookie)
    , m_strColName(strColName)
    , m_strItemArtist(strItemArtist)
    , m_strItemCookie(strItemCookie)
    , m_strItemName(strItemName)
    , m_strRepoMoniker(strRepoMoniker)
    , m_strTitleArtist(strTitleArtist)
    , m_strTitleCookie(strTitleCookie)
    , m_strTitleName(strTitleName)
{
    // If we got a loc info up front, store it
    if (!strLocInfo.bIsEmpty())
        m_pstrLocInfo = new TString(strLocInfo);
}

TCQCIntfMLBRTV::~TCQCIntfMLBRTV()
{
    // Clean up any stuff that we faulted in
    delete m_pstrLocInfo;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMLBRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfMLBRTV::bRTValue(const TString& strId,  TString& strToFill) const
{
    // Call our parent first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // See if it's one of ours
    if (strId == kCQCMedia::strRTVId_Artist)
    {
        // Depending on the selection level, give the appropriate artist info
        if (m_eSelLevel == TCQCIntfMediaListBrowser::EDispLvls::Top)
        {
            // If the media type is movies, clear this, else it's the title artist
            if (m_eMediaType == tCQCMedia::EMediaTypes::Movie)
                strToFill.Clear();
            else
                strToFill = m_strTitleArtist;
        }
        else if (m_eSelLevel == TCQCIntfMediaListBrowser::EDispLvls::Title)
            strToFill = m_strTitleArtist;
        else if (m_eSelLevel == TCQCIntfMediaListBrowser::EDispLvls::Col)
            strToFill = m_strColArtist;
        else if (m_eSelLevel == TCQCIntfMediaListBrowser::EDispLvls::Item)
            strToFill = m_strItemArtist;
    }
     else if (strId == kCQCMedia::strRTVId_CategoryCookie)
    {
        strToFill = m_strCatCookie;
    }
     else if (strId == kCQCMedia::strRTVId_ColCookie)
    {
        strToFill = m_strColCookie;
    }
     else if (strId == kCQCMedia::strRTVId_ItemArtist)
    {
        strToFill = m_strItemArtist;
    }
     else if (strId == kCQCMedia::strRTVId_ItemCookie)
    {
        strToFill = m_strItemCookie;
    }
     else if (strId == kCQCMedia::strRTVId_ItemName)
    {
        strToFill = m_strItemName;
    }
     else if (strId == kCQCMedia::strRTVId_LocInfo)
    {
        if (!m_pstrLocInfo)
        {
            // If we've not gotten the location type yet, then get it
            if (m_eLocType == tCQCMedia::ELocTypes::Count)
                GetLocType();

            //
            //  The collection level is handled in the GetLocType above. If
            //  it's item level and an item was selected, we need to get that.
            //
            if ((m_eLocType == tCQCMedia::ELocTypes::FileItem)
            &&  (m_eSelLevel == TCQCIntfMediaListBrowser::EDispLvls::Item))
            {
                tCQCKit::TCQCSrvProxy orbcSrv
                (
                    facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
                );

                //
                //  A little of this info is redundant since we already
                //  have it, but this is the most targeted query we have.
                //
                TMediaItem mitemDet;
                facCQCMedia().QueryItemDetails
                (
                    orbcSrv
                    , m_strRepoMoniker
                    , m_strItemCookie
                    , mitemDet
                );
                m_pstrLocInfo = new TString(mitemDet.strLocInfo());
            }
        }

        // If we could get it, set it
        if (m_pstrLocInfo)
            strToFill = *m_pstrLocInfo;
        else
            strToFill.Clear();
    }
     else if (strId == kCQCMedia::strRTVId_LocType)
    {
        // If we haven't got it yet, then get it if we can
        if (m_eLocType == tCQCMedia::ELocTypes::Count)
            GetLocType();

        switch(m_eLocType)
        {
            case tCQCMedia::ELocTypes::FileCol :
                strToFill = L"FileCol";
                break;

            case tCQCMedia::ELocTypes::FileItem:
                strToFill = L"FileItem";
                break;

            case tCQCMedia::ELocTypes::Changer :
                strToFill = L"Changer";
                break;

            default :
                strToFill.Clear();
                break;
        };
    }
     else if (strId == kCQCMedia::strRTVId_MediaFmt)
    {
        // This one we fault in as required
        if (m_strMediaFmt.bIsEmpty())
        {
            tCQCKit::TCQCSrvProxy orbcSrv
            (
                facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
            );

            orbcSrv->bQueryTextVal
            (
                m_strRepoMoniker
                , kCQCMedia::strQuery_MediaFmt
                , m_strColCookie
                , m_strMediaFmt
            );

            // If it's empty, to avoid getting it again, set it to ???
            if (m_strMediaFmt.bIsEmpty())
                m_strMediaFmt = L"????";
        }
        strToFill = m_strMediaFmt;
    }
     else if (strId == kCQCMedia::strRTVId_MediaType)
    {
        switch(m_eMediaType)
        {
            case tCQCMedia::EMediaTypes::Movie :
                strToFill = L"Movie";
                break;

            case tCQCMedia::EMediaTypes::Music :
                strToFill = L"Music";
                break;

            case tCQCMedia::EMediaTypes::Pic :
                strToFill = L"Picture";
                break;

            default :
                strToFill.Clear();
                break;
        };
    }
     else if (strId == kCQCMedia::strRTVId_Name)
    {
        //
        //  Depending on the display level, give the appropriate name info.
        //  We are showing the containing list name.
        //
        if (m_eDispLevel == TCQCIntfMediaListBrowser::EDispLvls::Top)
        {
            if (m_eMediaType == tCQCMedia::EMediaTypes::Movie)
                strToFill = facCQCIntfEng().strMsg(kIEngMsgs::midGen_CategoryList);
            else
                strToFill = facCQCIntfEng().strMsg(kIEngMsgs::midGen_ArtistList);
        }
         else if (m_eDispLevel == TCQCIntfMediaListBrowser::EDispLvls::Title)
        {
            strToFill = m_strTitleArtist;
        }
         else if ((m_eDispLevel == TCQCIntfMediaListBrowser::EDispLvls::Col)
              ||  (m_eDispLevel == TCQCIntfMediaListBrowser::EDispLvls::Item))
        {
            // Put in the title name
            strToFill = m_strTitleName;

            //
            //  If the collection has a name and it's not the same as the
            //  title name, then add that to the title level name.
            //
            if (!m_strColName.bIsEmpty() && (m_strColName != m_strTitleName))
            {
                strToFill.Append(L" - ");
                strToFill.Append(m_strColName);
            }
        }
    }
     else if (strId == kCQCMedia::strRTVId_TitleArtist)
    {
        strToFill = m_strTitleArtist;
    }
     else if (strId == kCQCMedia::strRTVId_TitleCookie)
    {
        strToFill = m_strTitleCookie;
    }
     else if (strId == kCQCMedia::strRTVId_TitleName)
    {
        strToFill = m_strTitleName;
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMLBRTV: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfMLBRTV::GetLocType() const
{
    // We only do this one if we have a collection cookie
    if (m_strColCookie.bIsEmpty())
        return;

    tCQCKit::TCQCSrvProxy orbcSrv
    (
        facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
    );

    TString strLocInfo;
    m_eLocType = facCQCMedia().eQueryColLoc
    (
        orbcSrv, m_strRepoMoniker, m_strColCookie, strLocInfo
    );

    //
    //  If it indicates collection level info, then go ahead now and
    //  store that, to avoid having to do it again later pehaps.
    //
    if (m_eLocType != tCQCMedia::ELocTypes::FileItem)
        m_pstrLocInfo = new TString(strLocInfo);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaListBrowser::TSlotInfo
//  PREFIX: si
//
//  Just a simple nested class for storing the info we need in each list.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser::TSlotInfo: Public, static methods
// ---------------------------------------------------------------------------

// So we can be sorted
tCIDLib::ESortComps
TCQCIntfMediaListBrowser::TSlotInfo::eCompI(const   TSlotInfo&      si1
                                            , const TSlotInfo&      si2)
{
    return si1.m_strName.eCompareI(si2.m_strName);
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser::TSlotInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMediaListBrowser::TSlotInfo::TSlotInfo()
{
}

TCQCIntfMediaListBrowser
::TSlotInfo::TSlotInfo( const   TString&        strArtist
                        , const TString&        strCookie
                        , const TString&        strName
                        , const tCIDLib::TCard2 c2Id
                        , const tCIDLib::TCard4 c4ChildCount) :

    m_bPressed(kCIDLib::False)
    , m_c2Id(c2Id)
    , m_c4ChildCount(c4ChildCount)
    , m_strArtist(strArtist)
    , m_strCookie(strCookie)
    , m_strName(strName)
{
}

TCQCIntfMediaListBrowser::TSlotInfo::~TSlotInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser::TSlotInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfMediaListBrowser::TSlotInfo::bPressed() const
{
    return m_bPressed;
}

tCIDLib::TBoolean
TCQCIntfMediaListBrowser::TSlotInfo::bPressed(const tCIDLib::TBoolean bToSet)
{
    m_bPressed = bToSet;
    return m_bPressed;
}


tCIDLib::TCard2 TCQCIntfMediaListBrowser::TSlotInfo::c2Id() const
{
    return m_c2Id;
}


tCIDLib::TCard4 TCQCIntfMediaListBrowser::TSlotInfo::c4ChildCount() const
{
    return m_c4ChildCount;
}

tCIDLib::TCard4
TCQCIntfMediaListBrowser::TSlotInfo::c4ChildCount(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ChildCount = c4ToSet;
    return m_c4ChildCount;
}


const TString& TCQCIntfMediaListBrowser::TSlotInfo::strArtist() const
{
    return m_strArtist;
}


const TString& TCQCIntfMediaListBrowser::TSlotInfo::strCookie() const
{
    return m_strCookie;
}


const TString& TCQCIntfMediaListBrowser::TSlotInfo::strName() const
{
    return m_strName;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaListBrowser::TListInfo
//  PREFIX: li
//
//  And another simple class that we use to track lists, and which is used
//  to cache previously downloaded lists for re-use, with the cookie being
//  the key.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser::TListInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMediaListBrowser::TListInfo::
TListInfo(const TCQCIntfMediaListBrowser::EDispLvls eLevel
        , const TString&                            strMoniker
        , const TString&                            strCookie) :

    m_c4FocusIndex(0)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_eLevel(eLevel)
    , m_strCookie(strCookie)
{
}

TCQCIntfMediaListBrowser::TListInfo::~TListInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser::TListInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Return our content area set to the relative to origin, and then adjusted
//  upwards by our current scrolled position (inverse of our adjusted display
//  area's y position.)
//
TArea TCQCIntfMediaListBrowser::
TListInfo::areaContentsRelTo(const TArea& areaRelTo) const
{
    TArea areaRet(areaRelTo.pntOrg(), m_areaContent.szArea());
    areaRet.AdjustOrg(0, -m_areaAdjDisp.i4Y());

    return areaRet;
}


tCIDLib::TCard4 TCQCIntfMediaListBrowser::TListInfo::c4Count() const
{
    return m_colList.c4ElemCount();
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::
TListInfo::Resize(  const   tCIDLib::TCard4 c4SlotHeight
                    , const TArea&          areaDisplay)
{
    const tCIDLib::TCard4 c4ListHeight = c4SlotHeight * m_colList.c4ElemCount();

    // Calculate the new content area
    m_areaContent.Set(0, 0, areaDisplay.c4Width(), c4ListHeight);

    //
    //  Update the display area size, It's always the width of the display
    //  area. For the height we take the display area height, unless the
    //  is not tall enough to fill the display area, in which case we
    //  take the list height.
    //
    m_areaAdjDisp.SetSize
    (
        areaDisplay.c4Width()
        , c4ListHeight < areaDisplay.c4Height() ? c4ListHeight
                                                : areaDisplay.c4Height()
    );

    // Make sure that we are still within our content area
    if (m_areaAdjDisp.i4Y() < m_areaContent.i4Y())
        m_areaAdjDisp.i4Y(m_areaContent.i4Y());
    else if (m_areaAdjDisp.i4Bottom() > m_areaContent.i4Bottom())
        m_areaAdjDisp.BottomJustifyIn(m_areaContent);
}


TCQCIntfMediaListBrowser::TSlotInfo&
TCQCIntfMediaListBrowser::TListInfo::siAt(const tCIDLib::TCard4 c4At)
{
    return m_colList[c4At];
}

const TCQCIntfMediaListBrowser::TSlotInfo&
TCQCIntfMediaListBrowser::TListInfo::siAt(const tCIDLib::TCard4 c4At) const
{
    return m_colList[c4At];
}


const TCQCIntfMediaListBrowser::TSlotInfo&
TCQCIntfMediaListBrowser::TListInfo::siSelected() const
{
    CIDAssert
    (
        m_c4SelIndex <= m_colList.c4ElemCount()
        , L"The list selection index is invalid"
    );
    return m_colList[m_c4SelIndex];
}


tCIDLib::TVoid TCQCIntfMediaListBrowser::TListInfo::Sort()
{
    m_colList.Sort(TSlotInfo::eCompI);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaListBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMediaListBrowser::TCQCIntfMediaListBrowser() :

    TCQCIntfWidget
    (
        CQCIntfEng_MediaListBrws::eCapFlags
        ,  facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MediaListBrowser)
    )
    , m_c4SlotHeight(12)
    , m_eInitMediaType(tCQCMedia::EMediaTypes::Music)
    , m_objaData(EDispLvls::Count)
{
    // Set default values for all the runtime stuff
    SetDefaults();

    // Load up the runtime values we support at this level
    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_Artist)
        , kCQCMedia::strRTVId_Artist
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CatCookie)
        , kCQCMedia::strRTVId_CategoryCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CategoryName)
        , kCQCMedia::strRTVId_CategoryName
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
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemArtist)
        , kCQCMedia::strRTVId_ItemArtist
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
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemName)
        , kCQCMedia::strRTVId_ItemName
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_LocInfo)
        , kCQCMedia::strRTVId_LocInfo
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_LocType)
        , kCQCMedia::strRTVId_LocType
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_MediaFmt)
        , kCQCMedia::strRTVId_MediaFmt
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_MediaType)
        , kCQCMedia::strRTVId_MediaType
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_Name)
        , kCQCMedia::strRTVId_Name
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleArtist)
        , kCQCMedia::strRTVId_TitleArtist
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleCookie)
        , kCQCMedia::strRTVId_TitleCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleName)
        , kCQCMedia::strRTVId_TitleName
        , tCQCKit::ECmdPTypes::Text
    );

    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCMedia().strMsg(kMedMsgs::midCmdEv_OnSelectCol)
        , kCQCMedia::strEvId_OnSelectCol
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCMedia().strMsg(kMedMsgs::midCmdEv_OnSelectItem)
        , kCQCMedia::strEvId_OnSelectItem
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSet)
        , kCQCKit::strEvId_OnSet
        , tCQCKit::EActCmdCtx::NoContChange
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaListBrowser");
}

TCQCIntfMediaListBrowser::
TCQCIntfMediaListBrowser(const TCQCIntfMediaListBrowser& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , m_areaDisplay(iwdgSrc.m_areaDisplay)
    , m_c4SlotHeight(iwdgSrc.m_c4SlotHeight)
    , m_eInitMediaType(iwdgSrc.m_eInitMediaType)
    , m_objaData(EDispLvls::Count)
    , m_strRepoMoniker(iwdgSrc.m_strRepoMoniker)
{
    SetDefaults();
}

TCQCIntfMediaListBrowser::~TCQCIntfMediaListBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Public operators
// ---------------------------------------------------------------------------

TCQCIntfMediaListBrowser&
TCQCIntfMediaListBrowser::operator=(const TCQCIntfMediaListBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);

        // Copy over persistent stuff
        m_eInitMediaType = iwdgSrc.m_eInitMediaType;
        m_strRepoMoniker = iwdgSrc.m_strRepoMoniker;

        // These are runtime only, but needed for maintaining design time display
        m_c4SlotHeight  = iwdgSrc.m_c4SlotHeight;
        m_areaDisplay   = iwdgSrc.m_areaDisplay;
        SetDefaults();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfMediaListBrowser::bCanSizeTo() const
{
    // Our contents is driven by size, not the other way around
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCIntfMediaListBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMediaListBrowser& iwdgOther
    (
        static_cast<const TCQCIntfMediaListBrowser&>(iwdgSrc)
    );

    // Do our mixins
    if (!MCQCCmdSrcIntf::bSameCmds(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther))
    {
        return kCIDLib::False;
    }

    // And our stuff
    return
    (
        (m_eInitMediaType == iwdgOther.m_eInitMediaType)
        && (m_strRepoMoniker == iwdgOther.m_strRepoMoniker)
    );
}


//
//  If going right, we allow a flick, single finger. We don't allow left at
//  all. For up or down we allow intertial panning, single finger. But if
//  our 'no inertia' flag is set, we do vertical flicks as well.
//
//  We allow two fingers to propigate, else we don't.
//
tCIDLib::TBoolean
TCQCIntfMediaListBrowser::bPerGestOpts( const   TPoint&             pntAt
                                        , const tCIDLib::EDirs      eDir
                                        , const tCIDLib::TBoolean   bTwoFingers
                                        ,       tCIDCtrls::EGestOpts& eToSet
                                        ,       tCIDLib::TFloat4&   f4VScale)
{
    if (bTwoFingers)
        return kCIDLib::True;

    if (eDir == tCIDLib::EDirs::Right)
    {
        //
        //  If we aren't already at the root level list, allow a move back to the previous
        //  level via flick gesture.
        //
        if (m_eDispLevel > EDispLvls::Top)
            eToSet = tCIDCtrls::EGestOpts::Flick;
    }
     else if ((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Down))
    {
        // Get the current display list
        TListPtr cptrList = m_objaData[m_eDispLevel];

        // Unless the content is larger than us, then nothing to do
        if (cptrList->m_areaContent.c4Height() > m_areaDisplay.c4Height())
        {
            //
            //  If no inertia or in remote mode, then do a flick, else
            //  do a normal inertial pan.
            //
            if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;
        }
    }

    if (m_c4SlotHeight < 20)
        f4VScale = 0.8F;
    else if (m_c4SlotHeight < 48)
        f4VScale = 0.9F;
    else if (m_c4SlotHeight < 64)
        f4VScale = 0.95F;
    else
        f4VScale = 1.0F;

    return kCIDLib::False;
}



//
//  If we are already at the boundary in the direction indicated, then we
//  return tru to allow the focus to move off us. Else, we just move it to
//  the next slot in that direction and redraw the old and new ones.
//
tCIDLib::TBoolean
TCQCIntfMediaListBrowser::bMoveFocus(const tCQCKit::EScrNavOps eOp)
{
    TListPtr cptrList = m_objaData[m_eDispLevel];

    //
    //  We have some special cases. Some we just translate into scrolling
    //  operations. The focus will be moved internally if necessary to keep
    //  it on a visible item. The others we move the focus, potentially
    //  off of us, but never scroll.
    //
    tCIDLib::TBoolean bRet = kCIDLib::True;

    if ((eOp == tCQCKit::EScrNavOps::End)
    ||  (eOp == tCQCKit::EScrNavOps::Home)
    ||  (eOp == tCQCKit::EScrNavOps::PrevPage)
    ||  (eOp == tCQCKit::EScrNavOps::NextPage))
    {
        //
        //  Call the list scrolling helper and translate our op into the
        //  action command parameter that it expects.
        //
        if (eOp == tCQCKit::EScrNavOps::End)
            ScrollList(L"End");
        else if (eOp == tCQCKit::EScrNavOps::Home)
            ScrollList(L"Home");
        else if (eOp == tCQCKit::EScrNavOps::PrevPage)
            ScrollList(L"Previous Page");
        else if (eOp == tCQCKit::EScrNavOps::NextPage)
            ScrollList(L"Next Page");

        // Indicate we kept the focus internal
        bRet = kCIDLib::False;
    }
     else if (eOp == tCQCKit::EScrNavOps::Back)
    {
        // Go back to the previous list
        GoBack();

        // Indicate we kept the focus internal
        bRet = kCIDLib::False;
    }
     else
    {
        // It's not one of those, so it's a focus movement op
        tCIDLib::TCard4 c4NewFocus = cptrList->m_c4FocusIndex;
        switch(eOp)
        {
            case tCQCKit::EScrNavOps::Up :
                if (c4NewFocus)
                    c4NewFocus--;
                break;

            case tCQCKit::EScrNavOps::Down :
                if (c4NewFocus + 1 < cptrList->c4Count())
                    c4NewFocus++;
                break;

            default :
                break;
        };

        if ((c4NewFocus != cptrList->m_c4FocusIndex)
        &&  bVisibleEnough(c4NewFocus, 0.4F))
        {
            TArea areaUpdate;
            AreaForIndex(cptrList->m_c4FocusIndex, areaUpdate);
            TArea areaNew;
            AreaForIndex(c4NewFocus, areaNew);
            areaUpdate |= areaNew;

            cptrList->m_c4FocusIndex = c4NewFocus;
            bRet = kCIDLib::False;

            Redraw(areaUpdate);
        }
    }
    return bRet;
}


//
//  This is called to let us react to a gesture. Note that we only accept
//  gestures vertical, so we know that these will be vertical. The horizontal
//  ones are always flicks.
//
tCIDLib::TBoolean
TCQCIntfMediaListBrowser::bProcessGestEv(const  tCIDCtrls::EGestEvs   eEv
                                        , const TPoint&             pntStart
                                        , const TPoint&             pntAt
                                        , const TPoint&             pntDelta
                                        , const tCIDLib::TBoolean   bTwoFinger)
{
    tCIDLib::TBoolean bContinue = kCIDLib::True;
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        // Get the current list and some refs to stuff
        TListPtr cptrList = m_objaData[m_eDispLevel];
        const TArea& areaContent = cptrList->m_areaContent;
        const TArea& areaListDisp = cptrList->m_areaAdjDisp;
        const TArea& areaUs = areaActual();

        m_areaPanCont = areaContent;
        {
            TArea areaScreen = TGUIFacility::areaVirtualScreen();

            const tCIDLib::TInt4 i4Span(tCIDLib::TInt4(areaScreen.c4Height()));

            // Set the size to five spans
            m_areaPanCont.c4Height(i4Span * 5);

            // Put the current display content in the middel one
            m_areaPanCont.i4Y(areaListDisp.i4Y() - (i4Span * 2));

            // And clip to the content area
            if (m_areaPanCont.i4Y() < areaContent.i4Y())
                m_areaPanCont.i4Y(areaContent.i4Y());
            if (m_areaPanCont.i4Bottom() > areaContent.i4Bottom())
                m_areaPanCont.i4Bottom(areaContent.i4Bottom());
        }

        //
        //  Now set up the actual bit of that pan content that we will initially
        //  draw. We will do just enough to handle the worst case actual move
        //  of the finger by the user, which will be two times our width. This is
        //  as far as they could possibly move us in the actual drag, before
        //  inertia.
        //
        //  Note it's 3x our whole height, not the display height, since dragging
        //  continues to the edge of the whole widget.
        //
        tCIDLib::TCard4 c4InitialHeight = tCIDLib::TCard4(areaUs.c4Height() * 3);

        //
        //  Set up an area of that size, such that the middle page of the three
        //  widths is layed over the current display content. This will be the
        //  initial 'drawn so far' area, by starting with the display are and
        //  moving it left one width.
        //
        m_areaPanSoFar = areaListDisp;
        m_areaPanSoFar.c4Height(c4InitialHeight);
        m_areaPanSoFar.AdjustOrg(0, -tCIDLib::TInt4(areaUs.c4Height()));

        // Clip to the pan content area if needed
        if (m_areaPanSoFar.i4Y() < m_areaPanCont.i4Y())
            m_areaPanSoFar.i4Y(m_areaPanCont.i4Y());
        if (m_areaPanSoFar.i4Bottom() > m_areaPanCont.i4Bottom())
            m_areaPanSoFar.i4Bottom(m_areaPanCont.i4Bottom());

        // Create the bitmap for the pannable content
        m_bmpCont = TBitmap
        (
            m_areaPanCont.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
        );

        //
        //  Get a version of the initial 'so far' area adjusted relative to the
        //  pan content, so we can do some drawing into it below. The bitmap is
        //  the zero origined pan content area.
        //
        TArea areaInitial(m_areaPanSoFar);
        areaInitial -= m_areaPanCont.pntOrg();

        TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
        if (m_bSafePanBgn)
        {
            //
            //  We have a safe bgn so we can put our background into the
            //  contents itself. We have to insure that any gradients line
            //  up so get a copy of the content area, and set the width
            //  to our width. Then center it horizontally over our content
            //  area and fill that.
            //
            TArea areaFill(areaInitial);
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaInitial);
            DoBgnFill(gdevCont, areaFill);
        }
         else
        {
            //
            //  We need a separate background image to composite the items over on
            //  each round. It only needs to be our size, but atthe origin of course.
            //
            m_bmpBgn = bmpGatherBgn();

            //
            //  We need one for double buffering during the pan in this case, forced
            //  forced to memory. It only needs to be as big as us.
            //
            m_bmpDBuf = TBitmap
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
        }

        //
        //  Draw the items into our content buffer, set the alpha mode
        //  flag to the opposite of whether we have a safe pan bgn.
        //
        TArea areaRelTo(areaContent);
        areaRelTo -= m_areaPanCont.pntOrg();
        DrawItems
        (
            gdevCont
            , areaRelTo
            , areaInitial
            , TPoint::pntOrigin
            , m_objaData[m_eDispLevel]
            , !m_bSafePanBgn
        );
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        // Get the current list and some refs to stuff
        TListPtr cptrList = m_objaData[m_eDispLevel];
        const TArea& areaContent = cptrList->m_areaContent;

        // Remember the current scroll pos
        const tCIDLib::TInt4 i4OrgPos = cptrList->m_areaAdjDisp.i4Y();

        // Apply the delta and clip back to the pan content area
        cptrList->m_areaAdjDisp.AdjustOrg(0, -pntDelta.i4Y());

        if (cptrList->m_areaAdjDisp.i4Y() < m_areaPanCont.i4Y())
            cptrList->m_areaAdjDisp.i4Y(m_areaPanCont.i4Y());
        else if (cptrList->m_areaAdjDisp.i4Bottom() > m_areaPanCont.i4Bottom())
            cptrList->m_areaAdjDisp.BottomJustifyIn(m_areaPanCont);

        //
        //  If we actually moved, then do the movement. If not, and we
        //  are getting inertia at this point, cancel any more inertia.
        //
        if (cptrList->m_areaAdjDisp.i4Y() != i4OrgPos)
        {
            // Calc the actual delta for use below
            tCIDLib::TInt4 i4NewOfs
            (
                cptrList->m_areaAdjDisp.i4Y() - i4OrgPos
            );

            //
            //  Clip to within our display plus the most restrictive of
            //  any of our ancestor containers.
            //
            tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

            TGUIRegion grgnClip;
            bFindMostRestrictiveClip(grgnClip, m_areaDisplay);
            TRegionJanitor janClip
            (
                cptrDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::And
            );

            //
            //  We now need to adjust the adjusted display area, because
            //  it is currently relative to the overall content origin,
            //  and we need ot make it relative to the pan content. Now
            //  it's over the actual pan content at the appropriate place
            //  to blit content from. So we can blit from here to our
            //  display area.
            //
            TArea areaBlitSrc = cptrList->m_areaAdjDisp;
            areaBlitSrc -= m_areaPanCont.pntOrg();

            if (m_bSafePanBgn)
            {
                //
                //  In this case we can just blit the contents directly since
                //  background and content are in one. But we can move the
                //  screen contents and only blit the missing bit.
                //
                TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpCont);

                // Scroll the display area over
                TArea areaTmpTar;
                cptrDev->ScrollBits
                (
                    m_areaDisplay
                    , m_areaDisplay
                    , 0
                    , i4NewOfs * -1
                    , areaTmpTar
                    , kCIDLib::True
                );

                //
                //  Create a source area from the target update area the scroll gave
                //  us. It needs to be either at the top of bottom edge of the src
                //  blit area. Left Justify it as well, since the value we get
                //  from the scroll is relative to the window, and this is zero
                //  based.
                //
                TArea areaTmpSrc(areaTmpTar);
                if (i4NewOfs < 0)
                    areaTmpSrc.TopJustifyIn(areaBlitSrc);
                 else
                    areaTmpSrc.BottomJustifyIn(areaBlitSrc);
                areaTmpSrc.i4X(areaBlitSrc.i4X());

                // And blit the uncovered area
                cptrDev->CopyBits
                (
                    gdevSrc
                    , areaTmpSrc
                    , areaTmpTar
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::False
                );
            }
             else
            {
                //
                //  We have to do the composite scenario. Set up a zero
                //  based version of our display area. We need this to blit from
                //  the background to our double buffer, and from that to
                //  our display area.
                //
                TArea areaZDisp = m_areaDisplay;
                areaZDisp.ZeroOrg();

                TGraphMemDev gdevDBuf(civOwner().gdevCompat(), m_bmpDBuf);

                // Copy in the background content to the double buffer
                {
                    TGraphMemDev gdevBgn(civOwner().gdevCompat(), m_bmpBgn);
                    gdevDBuf.CopyBits(gdevBgn, areaZDisp, areaZDisp);
                }

                // And alpha blit the content over that
                {
                    TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
                    gdevDBuf.AlphaBlit(gdevCont, areaBlitSrc, areaZDisp, 0xFF, kCIDLib::True);
                }

                // And finally copy the result to the target.
                cptrDev->CopyBits
                (
                    gdevDBuf
                    , areaZDisp
                    , m_areaDisplay
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::True
                );
            }
        }
         else if (eEv == tCIDCtrls::EGestEvs::Inertia)
        {
            bContinue = kCIDLib::False;
        }

        // If an end, then we can clean up and do a final redraw
        if (eEv == tCIDCtrls::EGestEvs::End)
        {
            // Release the bitmap memory we used
            m_bmpBgn.Reset();
            m_bmpDBuf.Reset();
            m_bmpCont.Reset();

            // Keep the focus index visible
            KeepFocusVisible();

            // Do a final draw in the end position
            Redraw();
        }
    }
    return bContinue;

}


tCIDLib::TVoid TCQCIntfMediaListBrowser::Clicked(const TPoint& pntAt)
{
    const tCIDLib::TCard4 c4HitInd = c4HitTest(pntAt);
    if (c4HitInd == kCIDLib::c4MaxCard)
        return;

    //
    //  Make sure that this one is at least 40% visible. If not, we'll
    //  ignore it.
    //
    tCIDLib::TCard4 c4Dummy = c4HitInd;
    if (!bVisibleEnough(c4Dummy, 0.4F))
        return;

    try
    {
        ItemInvoked(c4HitInd);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMediaListBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfMediaListBrowser::eDoCmd(const  TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Back)
    {
        GoBack();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Reset)
    {
        // Get the latest metadata for our repo
        facCQCMedia().eGetMediaDB
        (
            m_strRepoMoniker, m_strMDBSerialNum, m_cptrMetaData
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollList)
    {
        // Call a helper since we have to do this from more than one place
        const TString& strScrollOp = ccfgToDo.piAt(0).m_strValue;
        ScrollList(strScrollOp);
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetMediaType)
    {
        // Make sure it's a type our repository supports
        const tCQCMedia::EMediaTypes eToSet = facCQCMedia().eXlatCmdMediaType
        (
            ccfgToDo.piAt(0).m_strValue
        );

        if (eToSet == tCQCMedia::EMediaTypes::Count)
            ThrowBadParmVal(ccfgToDo, 0);

        //
        //  If this is a preload event, then just store the initial type.
        //  Else, load the default category with the new media type.
        //
        m_eInitMediaType = eToSet;
        SetDefaults();
        if (strEventId != kCQCKit::strEvId_OnPreload)
        {
            LoadTopList();
            Redraw();
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        // Get the initial media type and make sure it's good
        const tCQCMedia::EMediaTypes eToSet = facCQCMedia().eXlatCmdMediaType
        (
            ccfgToDo.piAt(1).m_strValue
        );

        if (eToSet == tCQCMedia::EMediaTypes::Count)
            ThrowBadParmVal(ccfgToDo, 1);

        //
        //  Now just store the new repository moniker and do the default
        //  category load as though we'd just been created. If this is an
        //  OnPreload event, then don't actually update, just store the
        //  info.
        //
        m_eInitMediaType = eToSet;
        m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;
        m_strMDBSerialNum.Clear();
        SetDefaults();

        if (strEventId != kCQCKit::strEvId_OnPreload)
        {
            LoadTopList();
            Redraw();
        }
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
TCQCIntfMediaListBrowser::GestInertiaIdle(  const   tCIDLib::TEncodedTime   enctEnd
                                            , const tCIDLib::TCard4         c4Millis
                                            , const tCIDLib::EDirs          eDir)
{
    //
    //  If we have have already draw all the way to the edge in the direction of
    //  inertia, then we are done. Our pan content and so far areas are relative
    //  to the zero origin of the bitmap, so going left if we hit zero we are done.
    //
    //  Else, see how close the display area is to how far we've drawn. If it's
    //  within two widget widths, we will draw out up to another width.
    //
    const tCIDLib::TInt4    i4Span(areaActual().c4Height() / 4);
    const tCIDLib::TInt4    i42Span(i4Span * 2);

    TListPtr cptrList = m_objaData[m_eDispLevel];
    const TArea& areaContent = cptrList->m_areaContent;
    const TArea& areaListDisp = cptrList->m_areaAdjDisp;

    TArea   areaUpdate;
    TArea   areaNewSoFar(m_areaPanSoFar);

    if (eDir == tCIDLib::EDirs::Up)
    {
        if (areaNewSoFar.i4Bottom() >= m_areaPanCont.i4Bottom())
            return;

        // As long as we have more than two widths, don't do anything
        if (areaNewSoFar.i4Bottom() - areaListDisp.i4Bottom() > i42Span)
            return;

        areaNewSoFar.AdjustSize(0, i4Span);

        // If it goes too far. clip it back
        if (areaNewSoFar.i4Bottom() > m_areaPanCont.i4Bottom())
            areaNewSoFar.i4Bottom(m_areaPanCont.i4Bottom());

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4Y(m_areaPanSoFar.i4Bottom(), kCIDLib::True);
    }
     else if (eDir == tCIDLib::EDirs::Down)
    {
        if (!areaNewSoFar.i4Y())
            return;

        if (areaListDisp.i4Y() - areaNewSoFar.i4Y() > i42Span)
            return;

        // Keep the right where it is, move the top up by a span
        areaNewSoFar.i4Top(areaNewSoFar.i4Y() - i4Span, kCIDLib::True);

        // If it goes below the origin, clip it back
        if (areaNewSoFar.i4Y() < 0)
            areaNewSoFar.i4Y(0, kCIDLib::True);

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4Bottom(m_areaPanSoFar.i4Y() - 1);
    }

    // If by some chance, it ended up empty, then do nothing
    if (areaUpdate.bIsEmpty())
        return;

    // Update the so far area
    m_areaPanSoFar = areaNewSoFar;

    //
    //  Subtract the pan content area's origin from teh update area. This will put
    //  it at the right place in the zero based content area bitmap.
    //
    areaUpdate -= m_areaPanCont.pntOrg();

    //
    //  And create a correctly adjusted content area so that the pan content area
    //  will fall at zero origin relative to it. This is our relative to drawing
    //  area.
    //
    TArea areaRelTo(areaContent);
    areaRelTo.ZeroOrg();
    areaRelTo -= m_areaPanCont.pntOrg();

    //
    //  Reset the remaining bit before we draw into it. For the safe bgn scenario
    //  we need to do a background fill. Since we keep the pannable content left
    //  justified and it fills the whole area, so we don't have to worry about
    //  gradient alignment.
    //
    const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();
    TGraphMemDev gdevCont(gdevCompat, m_bmpCont);

    if (m_bSafePanBgn)
        DoBgnFill(gdevCont, areaUpdate);

    //
    //  We have to clip the output to the actual update area. Otherwise, we could
    //  allow content to be drawn over twice. If that content was drawn with alpha,
    //  it would just add up.
    //
    TRegionJanitor janClip(&gdevCont, areaUpdate, tCIDGraphDev::EClipModes::And);

    DrawItems
    (
        gdevCont
        , areaRelTo
        , areaUpdate
        , TPoint::pntOrigin
        , m_objaData[m_eDispLevel]
        , !m_bSafePanBgn
    );
}



//
//  We just call our parent, and then calculate where our slots are going to
//  go, based on the loaded width/spacing values.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Remember if we have a safe bgn for scrolling. If so, we only need to
    //  create one scrollable bitmap, which will have the bgn in it. Else,
    //  we need the content plus the bgn, which we will composite together
    //  at each new position.
    //
    //  We only check for vertical, which is the drag direction. We check for
    //  check for the horizontal version separately when we have to move
    //  between list levels.
    //
    m_bSafePanBgn = bScrollSafeBgn(kCIDLib::True);

    // Do an initial recalculation of size related stuff
    Recalc(areaActual(), areaActual());
}


//
//  We have the focus and the user pressed Enter, or whatever equates to
//  that in the containing application. We act as though the item with
//  the focus was clicked on.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::Invoke()
{
    try
    {
        TListPtr cptrList = m_objaData[m_eDispLevel];
        ItemInvoked(cptrList->m_c4FocusIndex);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  We return a simple derivative of the RTV source since we have some RTVs
//  of our own. Note that we use the selection level here to load the
//  info up, since otherwise we'd load info for the displayed level which
//  has not been actually selected from yet. Unless we were already at the
//  item level, the selection has already caused another layer to load, so
//  the display level is not what was selected.
//
TCQCCmdRTVSrc*
TCQCIntfMediaListBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    try
    {
        //
        //  Double check that our list info is coherent, and it's at least up to the
        //  top level, since no events should be invoked if we are less than that.
        //
        #if CID_DEBUG_ON
        CheckListLevel(EDispLvls::Top);
        #endif

        //
        //  Get the available info into locals. We always have to be at at least root level,
        //  so we don't have to check the level. But it's possible it could be empty, and at
        //  design time isn't even set.
        //
        TString strCatCookie;
        TString strCatName;
        if (m_objaData[EDispLvls::Top].pobjData())
        {
            TListPtr cptrList = m_objaData[EDispLvls::Top];
            if (cptrList->c4Count())
            {
                const TSlotInfo& siCat = cptrList->siSelected();
                strCatName = siCat.strName();
                strCatCookie = siCat.strCookie();
            }
        }

        // We might be able to provide location info, else we pass in defaults
        tCQCMedia::ELocTypes    eLocType = tCQCMedia::ELocTypes::Count;
        TString                 strLocInfo;

        // Look up title info if we have it
        TString strTitleArtist;
        TString strTitleCookie;
        TString strTitleName;
        tCIDLib::TCard4 c4ColCnt = 0;
        if ((m_eSelectLevel != EDispLvls::Count)
        &&  (m_eSelectLevel >= EDispLvls::Title))
        {
            const TSlotInfo& siTitle = m_objaData[EDispLvls::Title]->siSelected();
            strTitleArtist = siTitle.strArtist();
            strTitleCookie = siTitle.strCookie();
            strTitleName = siTitle.strName();

            c4ColCnt = siTitle.c4ChildCount();
        }

        // Look up collection info if we have it
        TString strColArtist;
        TString strColCookie;
        TString strColName;
        if (m_eSelectLevel >= EDispLvls::Col)
        {
            if (m_eSelectLevel != EDispLvls::Count)
            {
                const TSlotInfo& siCol = m_objaData[EDispLvls::Col]->siSelected();
                strColArtist = siCol.strArtist();
                strColCookie = siCol.strCookie();
                strColName = siCol.strName();
            }
        }
         else if (c4ColCnt == 1)
        {
            //
            //  We have to special case the scenario where they click on a title
            //  set with a single collection. We treat that as a click on the
            //  collection itself, so we have to set up for that.
            //
            strColCookie = strTitleCookie;
            strColCookie.Append(L",1");

            // And now query the artist/title info
            tCQCKit::TCQCSrvProxy orbcSrv
            (
                facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
            );

            TMediaCollect mcolDet;
            tCIDLib::TCard4 c4ColCnt;
            TString strDummy;
            facCQCMedia().eQueryColDetails
            (
                orbcSrv
                , m_strRepoMoniker
                , strColCookie
                , mcolDet
                , strDummy
                , c4ColCnt
            );

            strColArtist = mcolDet.strArtist();
            strColName = mcolDet.strName();

            // And we can in this case provide some location info
            eLocType = mcolDet.eLocType();
            if (eLocType != tCQCMedia::ELocTypes::FileItem)
                strLocInfo = mcolDet.strLocInfo();
        }

        // Get item info if we have it
        TString strItemArtist;
        TString strItemCookie;
        TString strItemName;
        if ((m_eSelectLevel != EDispLvls::Count)
        &&  (m_eSelectLevel >= EDispLvls::Item))
        {
            const TSlotInfo& siItem = m_objaData[EDispLvls::Item]->siSelected();
            strItemArtist = siItem.strArtist();
            strItemCookie = siItem.strCookie();
            strItemName = siItem.strName();
        }

        return new TCQCIntfMLBRTV
        (
            strCatName
            , strCatCookie
            , strTitleArtist
            , strTitleCookie
            , strTitleName
            , strColArtist
            , strColCookie
            , strColName
            , strItemArtist
            , strItemCookie
            , strItemName
            , m_strRepoMoniker
            , strLocInfo
            , eLocType
            , m_eCurMediaType
            , m_eDispLevel
            , m_eSelectLevel
            , cuctxToUse
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Called for post init loading. We get ready for the top level list.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::PostInit()
{
    try
    {
        // Get an initial media database if we can
        facCQCMedia().eGetMediaDB
        (
            m_strRepoMoniker, m_strMDBSerialNum, m_cptrMetaData
        );

        // If we got it, then load up the top list and redraw
        if (m_cptrMetaData.pobjData())
        {
            LoadTopList();
            Invalidate();
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
}


//
//  We stil have to accept flicks for up and down for RIVA clients. For
//  right flicks those we allow locally so we handle them always. They are
//  used to go back to the previous list.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::ProcessFlick( const   tCIDLib::EDirs  eDir
                                        , const TPoint&         pntStart)
{
    if ((eDir == tCIDLib::EDirs::Up)
    ||  (eDir == tCIDLib::EDirs::Down))
    {
        const tCIDLib::TCh* pszCmd = 0;

        if (eDir == tCIDLib::EDirs::Down)
            pszCmd = L"Previous Page";
        else if (eDir == tCIDLib::EDirs::Up)
            pszCmd = L"Next Page";

        if (pszCmd)
            ScrollList(pszCmd);
    }
     else if (eDir == tCIDLib::EDirs::Right)
    {
        // Call the back handler to do this
        GoBack();
    }
}


// This is called to gather up supported commands for the cmd editing dialog
tCIDLib::TVoid
TCQCIntfMediaListBrowser::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    //
    //  We don't allow any of setting type stuff in an event driven action,
    //  and mostly not in a preload.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Go back to the previous list if possible
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Back
                , facCQCKit().strMsg(kKitMsgs::midCmd_Back)
                , 0
            )
        );
    }

    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        // Reset the cache
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Reset
                , facCQCKit().strMsg(kKitMsgs::midCmd_Reset)
                , 0
            )
        );
    }

    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Scroll the current list if it can be
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ScrollList
                , facCQCKit().strMsg(kKitMsgs::midCmd_ScrollList)
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ScrollDir)
                , L"Home, Next Page, Previous Page, End"
            )
        );
    }

    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        // Set a new media type
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
}


//
//  We override these next two to allow for moniker replacement, mostly for
//  template package import/export. We have a repository moniker that we need
//  to account for.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Focus Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Focus Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3, L"Press Emph.");

    // Do our mixins. Our actions are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Browser Actions"
    );
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);

    // And do our attributes
    adatTmp.Set(L"Browser Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_Media_Repo
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strRepoMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAdd(adatTmp);

    TString strLim;
    tCQCMedia::FormatEMediaTypes(strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
    adatTmp.Set
    (
        L"Init Media Type"
        , kCQCIntfEng::strAttr_Media_Type
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCMedia::strXlatEMediaTypes(m_eInitMediaType));
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfMediaListBrowser::szDefaultSize() const
{
    return TSize(320, 480);
}


//
//  We override this in order to scale our slot size so that it correctly fits the
//  adjusted font size after rescaling.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::ScaleInternal(const   tCIDLib::TFloat8 f8XScale
                                        , const tCIDLib::TFloat8 f8YScale)
{
    m_c4SlotHeight = TMathLib::c4Round(m_c4SlotHeight * f8YScale);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMediaListBrowser::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);

    // And do ours
    if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Type)
        m_eInitMediaType = tCQCMedia::eXlatEMediaTypes(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Repo)
        m_strRepoMoniker = adatNew.strValue();
}


tCIDLib::TVoid TCQCIntfMediaListBrowser::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    // Not one of ours, pass long to our parent
    TParent::SetDefParms(ccfgToSet);
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::Validate( const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

}



// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the initial media type
tCQCMedia::EMediaTypes TCQCIntfMediaListBrowser::eInitMediaType() const
{
    return m_eInitMediaType;
}

tCQCMedia::EMediaTypes
TCQCIntfMediaListBrowser::eInitMediaType(const tCQCMedia::EMediaTypes eToSet)
{
    m_eInitMediaType = eToSet;
    return m_eInitMediaType;
}


// Get or set the moniker of the repository driver we are associated with
const TString& TCQCIntfMediaListBrowser::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfMediaListBrowser::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


//
//  Our config tab needs to be able to force us to recalc if the font changes.
//  And we need to do this at viewing time when we are initialized or our
//  size changes.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::Recalc(const TArea& areaNew, const TArea& areaOld)
{
    //
    //  We need to set up our configued font so that we can get the font
    //  metrics. So we just create a temporary graphic device.
    //
    tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();
    TCQCIntfFontJan janFont(cptrDev.pobjData(), *this);
    TFontMetrics fmtrCur(*cptrDev);

    //
    //  Store the font height plus descender and some pading. This will
    //  drive our area per display slot.
    //
    m_c4SlotHeight = fmtrCur.c4Height() + fmtrCur.c4Descender() + 10;

    // Set up our display area
    m_areaDisplay = areaActual();

    // Move in two if we have a border, one if not
    if (bHasBorder())
        m_areaDisplay.Deflate(2);
    else
        m_areaDisplay.Deflate(1);

    // Move in another one if rounded
    if (c4Rounding())
        m_areaDisplay.Deflate(1);

    //
    //  Update any current lists. We may have to update their content areas
    //  and adjust their display area if it's now off the content area.
    //
    EDispLvls eLevel = EDispLvls::Min;
    while (eLevel <= EDispLvls::Max)
    {
        TListPtr cptrList = m_objaData[eLevel];
        if (cptrList.pobjData())
            cptrList->Resize(m_c4SlotHeight, m_areaDisplay);

        eLevel++;
    }

    // Make sure the currently visible list's focus stays visible
    KeepFocusVisible();
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

// Re-calc our slot position based on the new area
tCIDLib::TVoid
TCQCIntfMediaListBrowser::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    // Do any size calcs
    Recalc(areaNew, areaOld);
}


//
//  If we are getting focus, then we show the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we have the focus.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::GotFocus()
{
    // If the focus item for the current top item is visible, redraw it
    const TListPtr cptrCur = m_objaData[m_eDispLevel];
    if (cptrCur.pobjData())
    {
        TArea areaRedraw;
        AreaForIndex(cptrCur->m_c4FocusIndex, areaRedraw);
        Redraw(areaRedraw);
    }
}


//
//  If we are losing the focus, we remove the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we don't have the focus.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::LostFocus()
{
    // If the focus item for the current top item is visible, redraw it
    const TListPtr cptrCur = m_objaData[m_eDispLevel];
    if (cptrCur.pobjData())
    {
        TArea areaRedraw;
        AreaForIndex(cptrCur->m_c4FocusIndex, areaRedraw);
        Redraw(areaRedraw);
    }
}


tCIDLib::TVoid TCQCIntfMediaListBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    try
    {
        // Our stuff should start with a start marker
        strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

        // Check our version info
        tCIDLib::TCard2 c2FmtVersion;
        strmToReadFrom  >> c2FmtVersion;
        if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MediaListBrws::c2FmtVersion))
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

        // Call our parent
        TParent::StreamFrom(strmToReadFrom);

        // Do any mixins
        MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);
        MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

        // And do our stuff
        strmToReadFrom  >> m_strRepoMoniker
                        >> m_eInitMediaType;

        // And it should all end with an end object marker
        strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

        // Clear other runtime stuff
        SetDefaults();

        //
        //  If V1, then we need to adjust some colors, so that the F2 color is
        //  always used for shadow/fx. Currently F3 is being used for shadow and
        //  F2 for FX, and F2 is also being used for the border. And, F2 was
        //  also being used for the pressed fill effect. Move that to Extra1.
        //
        if (c2FmtVersion == 1)
        {
            // Save the original values we are going to change
            TRGBClr rgbF2 = rgbFgn2();
            TRGBClr rgbF3 = rgbFgn3();

            // Put F2 into F3, which is now the border color
            rgbFgn3(rgbF2);

            // And into Extra3 which is now the press effect
            rgbExtra3(rgbF2);

            //
            //  If not in text fx mode, then move F3 to F2, so that whatever
            //  was being used for the shadow will continue to be.
            //
            if (eTextFX() == tCIDGraphDev::ETextFX::None)
                rgbFgn2(rgbF3);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    try
    {
        // Write out our marker and version
        strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                        << CQCIntfEng_MediaListBrws::c2FmtVersion;

        // Call our parent, so he can stream his stuff out
        TParent::StreamTo(strmToWriteTo);

        // Do any mixins
        MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);
        MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);

        // And do our stuff and end marker
        strmToWriteTo   << m_strRepoMoniker
                        << m_eInitMediaType
                        << tCIDLib::EStreamMarkers::EndObject;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Though this currently only hapepns during design time, where it doesn't
//  make much difference to us, at some point we'll support such resizing,
//  we override and call our recalc method.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);

    // Recalc anything size related
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    Recalc(areaNew, areaOld);
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::StyleFlagChanged( const   tCIDLib::TCard4 c4OldFlags
                                            , const tCIDLib::TCard4 c4NewFlags)
{
    //
    //  If the border flag changed, recalc anything size related, since the
    //  displayed contents is going to move inwards.
    //
    const tCIDLib::TCard4 c4Old(c4OldFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
    const tCIDLib::TCard4 c4New(c4NewFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
    if (c4Old != c4New)
        Recalc(areaActual(), TArea::areaEmpty);
}


tCIDLib::TVoid
TCQCIntfMediaListBrowser::Update(       TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Add our display area to the clip region
    grgnClip.CombineWith(m_areaDisplay, tCIDGraphDev::EClipModes::And);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    try
    {
        //
        //  If in designer mode, we just draw outlines of where the images would
        //  go. Else, we need to draw our real contents.
        //
        if (TFacCQCIntfEng::bDesMode())
        {
            //
            //  Create a pen for a XOR style outline, saving the current
            //  mix mode to put back after we are done.
            //
            TCosmeticPen gpenOutline(facCIDGraphDev().rgbWhite);
            TPenJanitor janPen(&gdevTarget, &gpenOutline);
            const tCIDGraphDev::EMixModes eSaveMode = gdevTarget.eMixMode();
            gdevTarget.eMixMode(tCIDGraphDev::EMixModes::XorPen);

            // Set our font, and turn off bgn fill for text
            TCQCIntfFontJan janFont(&gdevTarget, *this);
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

            //
            //  Set up the first area. We can't call AreaForIndex() because
            //  we don't have any actual list content set up.
            //
            TArea areaCur;
            areaCur = m_areaDisplay;
            areaCur.c4Height(m_c4SlotHeight);

            // Set up a dummy item to draw
            TSlotInfo siDummy
            (
                L"Bubba Jones", L"1,1,1,1", L"Sample Text Value", 1
            );

            // Temp buffers that are passed in for transparency and F/X drawing
            TBitmap bmpBuf;
            TBitmap bmpMask;

            while (areaCur.i4Y() < m_areaDisplay.i4Bottom())
            {
                DrawItemText
                (
                    gdevTarget
                    , areaCur
                    , siDummy
                    , kCIDLib::False
                    , kCIDLib::False
                );
                areaCur.Move(tCIDLib::EDirs::Down);
            }
            gdevTarget.eMixMode(eSaveMode);
        }
         else
        {
            //
            //  Do the normal draw of the slots, from the current top item
            //  of the current list. This will draw up to a full page's
            //  worth if we have that many, so figure out how many we can
            //  display. Make sure that the list is even set yet, which it
            //  might not be.
            //
            const TListPtr cptrList = m_objaData[m_eDispLevel];
            if (cptrList.pobjData())
            {
                //
                //  Get his content adjusted to be relative to our display
                //  area, which is what we want to draw relative to, and
                //  our display area becomes the target area.
                //
                TArea areaRelTo = cptrList->areaContentsRelTo(m_areaDisplay);

                // The update area is the intersect of display and invalid
                TArea areaUpdate(m_areaDisplay);
                areaUpdate &= areaInvalid;
                DrawItems
                (
                    gdevTarget
                    , areaRelTo
                    , areaUpdate
                    , TPoint::pntOrigin
                    , cptrList
                    , kCIDLib::False
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfMediaListBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Calculate the position for the passed index. We have to be able to do it
//  both for display coordinates and scroll buffer coordinates. We hve one
//  that defaults to the display coordinate stuff, which calls the other one
//  that takes arbitrary coordinate systems.
//
//  The relative to is the area where the content is being drawn. It's either
//  relative to the display area or some zero based buffer we are drawing into
//  for scrolling.
//
//  We assume the top-most level.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::AreaForIndex( const   tCIDLib::TCard4 c4Index
                                        ,       TArea&          areaToFill) const
{
    // Get the top list
    const TListPtr cptrList = m_objaData[m_eDispLevel];

    //
    //  Ask it for an adjusted content area, relative to our display area.
    //  This is the relative to area for normal display purposes.
    //
    TArea areaRelTo = cptrList->areaContentsRelTo(m_areaDisplay);
    AreaForIndex(c4Index, areaRelTo, areaToFill);
}

tCIDLib::TVoid
TCQCIntfMediaListBrowser::AreaForIndex( const   tCIDLib::TCard4 c4Index
                                        , const TArea&          areaRelTo
                                        ,       TArea&          areaToFill) const
{
    //
    //  Start with the target area. However, it may be empty, so we only do
    //  this to get the origin. If empty, its origin will be our return origin.
    //
    areaToFill = areaRelTo;

    // How far from the content origin is this one
    const tCIDLib::TInt4 i4Pos(c4Index * m_c4SlotHeight);

    // Adjust from the origin by this much and set the slot height
    areaToFill.AdjustOrg(0, i4Pos);
    areaToFill.c4Height(m_c4SlotHeight);
}



//
//  A selection has been made from the current list. If we need to display
//  the next level down, then do that. Then if it's a collection level or
//  item level (or title set level for a title set with only one collection)
//  we need to tell the caller that he should invoke a user action.
//
tCIDLib::TBoolean
TCQCIntfMediaListBrowser::bHandleSelection(tCIDLib::TCard4& c4SelIndex)
{
    try
    {
        // Make sure the display level is valid
        CIDAssert
        (
            (m_eDispLevel >= EDispLvls::Min) && (m_eDispLevel <= EDispLvls::Max)
            , L"Invalid selection level for OnClick"
        );

        // Get the list info for the current level
        TListPtr cptrCur = m_objaData[m_eDispLevel];
        CIDAssert(cptrCur.pobjData(), L"Selection seen for unset slot item");

        //
        //  Remember the level that was set before we do anything, and store
        //  the selection index for this level.
        //
        m_eSelectLevel = m_eDispLevel;
        cptrCur->m_c4SelIndex = c4SelIndex;

        //
        //  If we are in movie mode and at collection level, then just return
        //  true to invoke an action.
        //
        if ((m_eCurMediaType == tCQCMedia::EMediaTypes::Movie)
        &&  (m_eDispLevel == EDispLvls::Col))
        {
            return kCIDLib::True;
        }

        // If we are at item level, then return true to invoke an action
        if (m_eDispLevel == EDispLvls::Item)
            return kCIDLib::True;

        // Get the selected one
        const TSlotInfo& siCur = cptrCur->siSelected();

        //
        //  If we are in movie mode and on the title set level, and this title
        //  set only has 1 collection, we don't do anything. We'll just return
        //  true to indicate an action should be invoked. When we are asked to
        //  create an RTV value, we'll see this and act like the one collection
        //  was selected.
        //
        if ((m_eCurMediaType == tCQCMedia::EMediaTypes::Movie)
        &&  (m_eDispLevel == EDispLvls::Title)
        &&  (siCur.c4ChildCount() == 1))
        {
            return kCIDLib::True;
        }

        // Move to the next level and load up that list
        EDispLvls eNewLevel = m_eDispLevel;
        eNewLevel++;

        // Load up a list for the new level
        TListPtr cptrNew;
        if (eNewLevel == EDispLvls::Title)
            cptrNew = cptrLoadTitles(siCur, m_eCurSortOrder);
        else if (eNewLevel == EDispLvls::Col)
            cptrNew = cptrLoadCols(siCur);
        else if (eNewLevel == EDispLvls::Item)
            cptrNew = cptrLoadItems(siCur);

        // Update to the new level if we got a new list
        if (cptrNew.pobjData())
        {
            // Set up the state info to put us on the newly loaded list
            m_eDispLevel = eNewLevel;
            m_objaData[m_eDispLevel] = cptrNew;

            //
            //  If we are in music mode, and now showing a collection, and
            //  there's only one collection in the parent title set, then we
            //  want to actually skip forward to the item list.
            //
            if ((m_eCurMediaType == tCQCMedia::EMediaTypes::Music)
            &&  (m_eDispLevel == EDispLvls::Col)
            &&  (siCur.c4ChildCount() == 1))
            {
                const TSlotInfo& siCol = cptrNew->siAt(0);

                TListPtr  cptrItems = cptrLoadItems(siCol);

                // And once again, update our display info to the new level
                m_eDispLevel++;
                m_objaData[m_eDispLevel] = cptrItems;

                // Update the new level we are moving to to the item level
                eNewLevel++;

                //
                //  And we have to push the selection level up as well, and
                //  set the selected index to zero to fake a selection on
                //  the 0th entry of this new level.
                //
                m_eSelectLevel++;
                c4SelIndex = 0;

                // Update the collection list as well
                cptrNew->m_c4SelIndex = 0;
            }
        }

        //
        //  If remote mode or no inertia for this widget, just force a
        //  redraw. Else, do a sliding scroll to the new page.
        //
        if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
            Redraw();
        else
            SlideListLevel(m_eSelectLevel, eNewLevel);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // For now, we always invoke a command
    return kCIDLib::True;
}


//
//  This method will test whether the indicated index is 'visible enough',
//  which is used to decide whether to allow a click on an item or to move
//  the focus to it. We don't want to allow items that might only have a
//  pixel or two visible to be selected in these types of cases.
//
//  If it's not, then the passed index will be adjusted if that is possible
//  to reflect the next closest one that is visible enough.
//
//  It is assumed that the index at least within one item's size of the
//  display area. Else it won't get up to a usefully visible item index.
//
tCIDLib::TBoolean
TCQCIntfMediaListBrowser::bVisibleEnough(       tCIDLib::TCard4&    c4Index
                                        , const tCIDLib::TFloat4    f4Per) const
{
    TListPtr cptrCur = m_objaData[m_eDispLevel];

    // Not much we can do
    if (!cptrCur.pobjData())
        return kCIDLib::True;;

    // Get the area of the passed index
    TArea areaTest;
    AreaForIndex(c4Index, areaTest);

    //
    //  How much of an item has to be visible before we consider it a
    //  visible one for our purposes.
    //
    //  They give us a percent that has to be visible. Our stuff below
    //  measures how much is not visible, so flip it.
    //
    const tCIDLib::TInt4 i4Limit = tCIDLib::TInt4(m_c4SlotHeight * (1.0 - f4Per));

    // Remember if we modified it
    const tCIDLib::TCard4 c4OrgVal = c4Index;

    // if not enough is visible, then move inwards if possible
    if (areaTest.i4Y() < m_areaDisplay.i4Y())
    {
        if (m_areaDisplay.i4Y() - areaTest.i4Y() >= i4Limit)
        {
            if (c4Index + 1 < cptrCur->c4Count())
               c4Index++;
        }
    }
     else if (areaTest.i4Bottom() > m_areaDisplay.i4Bottom())
    {
        if ((areaTest.i4Bottom() - m_areaDisplay.i4Bottom() >= i4Limit)
        &&  c4Index)
        {
            c4Index--;
        }
    }

    // If we didn't have to change it, return true
    return (c4OrgVal == c4Index);
}


//
//  Capture the background under us, plus our background, but no foreground
//  list contents. This is used in the inertial scrolling methods.
//
//  We only capture the display area, and we have to insure that any gradients
//  align with our overall area background fill.
//
TBitmap TCQCIntfMediaListBrowser::bmpGatherBgn()
{
    //
    //  Get the stuff underneath our display area. We tell it to stop when
    //  it hits our widget, and not to draw it (stop before the one we
    //  indicate.)
    //
    TBitmap bmpBgn = civOwner().bmpQueryContentUnder
    (
        m_areaDisplay, this, kCIDLib::True
    );

    // Draw our background content (if any) over that
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

    //
    //  Do our background fill if any over that. We insure alighment of
    //  gradients by centering an area of our overall size over our display
    //  area and filling that.
    //
    TArea areaFill = areaActual();
    areaFill.CenterIn(m_areaDisplay);
    DoBgnFill(gdevBgn, areaFill);

    // Create a bitmap the size of our display area
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );

    //
    //  And copy the bgn contents within the drawing area over into our
    //  return bitmap.
    //
    {
        TGraphMemDev gdevRet(civOwner().gdevCompat(), bmpRet);

        TArea areaTar(m_areaDisplay);
        areaTar.ZeroOrg();
        gdevRet.CopyBits(gdevBgn, m_areaDisplay, areaTar);
    }
    return bmpRet;
}


//
//  Return the closest index to the passed from here index that is mostly
//  visible within our display area. This is for keeping a certain index
//  visible.
//
tCIDLib::TCard4
TCQCIntfMediaListBrowser::c4ClosestVisIndex(const tCIDLib::TCard4 c4FromHere) const
{
    TListPtr cptrCur = m_objaData[m_eDispLevel];

    // Not much we can do
    if (!cptrCur.pobjData())
        return c4FromHere;

    // Get the area of the from index
    TArea areaTest;
    AreaForIndex(c4FromHere, areaTest);

    // If it is fully contained within the display area, then it's keeper as is
    if (m_areaDisplay.bContainsArea(areaTest))
        return c4FromHere;

    //
    //  If it doesn't intersect, then it's completely outside, so move to the
    //  first one that's at least reasonably visible. If it does intersect,
    //  then it's a partially visible one, so either way we'll then check
    //  below to make sure it's visible enough.
    //
    tCIDLib::TCard4 c4Ret = c4FromHere;
    if (!m_areaDisplay.bIntersects(areaTest))
    {
        //
        //  Not we have to use the ADJUSTED dislay area here, which
        //  calculating the new index!
        //
        if (areaTest.i4Y() < m_areaDisplay.i4Y())
            c4Ret = tCIDLib::TCard4(cptrCur->m_areaAdjDisp.i4Y()) / m_c4SlotHeight;
        else
            c4Ret = tCIDLib::TCard4(cptrCur->m_areaAdjDisp.i4Bottom()) / m_c4SlotHeight;
    }

    //
    //  Call the visible enough method which will modify the index if needed.
    //  We don't care if it does or doesn't. We require it to be at least
    //  40% visible.
    //
    bVisibleEnough(c4Ret, 0.4F);

    // And return whatever we ended up with
    return c4Ret;
}


//
//  Calc the first (at least partially) visible index, given a visible area
//  and another area it's relative to.
//
tCIDLib::TCard4
TCQCIntfMediaListBrowser::c4FirstVisIndex(  const   TArea&  areaVis
                                            , const TArea&  areaRelTo) const
{
    const TListPtr cptrCur = m_objaData[m_eDispLevel];
    if (!cptrCur.pobjData())
        return 0;

    // See how far we are scrolled from the content origin
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4(areaVis.i4Y() - areaRelTo.i4Y());

    // And we can just divide to get the indx
    return (c4Diff / m_c4SlotHeight);
}


//
//  Called when we are clicked on, to see which item was hit. The return
//  value is the slot index (from the top), which can be added to the current
//  top index to get the actual index. If none was hit (which can happen since
//  we can have a border, we return max card.
//
tCIDLib::TCard4
TCQCIntfMediaListBrowser::c4HitTest(const TPoint& pntTest) const
{
    //
    //  If the point isn't within our display area, then even if we hit
    //  something it isn't useful.
    //
    if (!m_areaDisplay.bContainsPoint(pntTest))
        return kCIDLib::c4MaxCard;

    // Get the current list. If not set, we hit nothing
    const TListPtr cptrCur = m_objaData[m_eDispLevel];
    if (!cptrCur.pobjData())
        return kCIDLib::c4MaxCard;

    // Get the current list's adjusted content area, relative to our display
    TArea areaAdjCont = cptrCur->areaContentsRelTo(m_areaDisplay);

    //
    //  Calc the difference between the adjusted content area and our point.
    //  This is how far the point is from the 0th column.
    //
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4
    (
        (pntTest.i4Y() - areaAdjCont.i4Y())
    );

    // And we can now just divide
    const tCIDLib::TCard4 c4ItemInd(c4Diff / m_c4SlotHeight);

    // If this value is beyond the list end, we missed
    if (c4ItemInd >= cptrCur->c4Count())
        return kCIDLib::c4MaxCard;

    return c4ItemInd;
}


//
//  Helpers to load up a list for each of the types we have to drill down
//  into. We get the id of the thing we need to load up.
//
TCQCIntfMediaListBrowser::TListPtr
TCQCIntfMediaListBrowser::cptrLoadCols(const TSlotInfo& siSel)
{
    // Create a new list to fill in
    TListPtr cptrNew(new TListInfo(EDispLvls::Col, m_strRepoMoniker, siSel.strCookie()));

    // If we have a database, let's load up the list, else leave empty
    if (m_cptrMetaData.pobjData())
    {
        const TMediaDB& mdbData = m_cptrMetaData->mdbData();

        // Find the title set by id
        const TMediaTitleSet* pmtsSrc = mdbData.pmtsById
        (
            m_eCurMediaType, siSel.c2Id(), kCIDLib::True
        );

        // Run through all of this guy's collections and add them
        TString strCookie;
        const tCIDLib::TCard4 c4Count = pmtsSrc->c4ColCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCollect* pmcolCur = mdbData.pmcolById
            (
                m_eCurMediaType, pmtsSrc->c2ColIdAt(c4Index), kCIDLib::False
            );

            //
            //  Format out the cookie for this guy. Collection cookies use a
            //  faux collection id, just hte 1 based child index./
            //
            strCookie = siSel.strCookie();
            strCookie.Append(L',');
            strCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);

            cptrNew->m_colList.objAdd
            (
                TSlotInfo
                (
                    pmcolCur->strArtist()
                    , strCookie
                    , pmcolCur->strName()
                    , pmcolCur->c2Id()
                    , pmcolCur->c4ItemCount()
                )
            );
        }

        // Let the list object do size calculations
        cptrNew->Resize(m_c4SlotHeight, m_areaDisplay);
    }
    return cptrNew;
}

TCQCIntfMediaListBrowser::TListPtr
TCQCIntfMediaListBrowser::cptrLoadItems(const TSlotInfo& siSel)
{
    // Create a new list to fill in
    TListPtr cptrNew(new TListInfo(EDispLvls::Item, m_strRepoMoniker, siSel.strCookie()));

    // If we have a database, let's load up the list, else leave empty
    if (m_cptrMetaData.pobjData())
    {
        const TMediaDB& mdbData = m_cptrMetaData->mdbData();

        // Find the selected collection
        const TMediaCollect* pmcolSrc = mdbData.pmcolById
        (
            m_eCurMediaType, siSel.c2Id(), kCIDLib::True
        );

        // And run through its items and load them up
        TString strCookie;
        const tCIDLib::TCard4 c4Count = pmcolSrc->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaItem& mitemCur = pmcolSrc->mitemAt(mdbData, c4Index);

            //
            //  Format out the cookie for this guy. Item cookies use a
            //  faux id, just the 1 based child index.
            //
            strCookie = siSel.strCookie();
            strCookie.Append(L',');
            strCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);

            cptrNew->m_colList.objAdd
            (
                TSlotInfo
                (
                    mitemCur.strArtist(), strCookie, mitemCur.strName(), mitemCur.c2Id()
                )
            );
        }

        // Let the list object do size calculations
        cptrNew->Resize(m_c4SlotHeight, m_areaDisplay);
    }
    return cptrNew;
}

TCQCIntfMediaListBrowser::TListPtr
TCQCIntfMediaListBrowser::cptrLoadTitles(const  TSlotInfo&              siSel
                                        , const tCQCMedia::ESortOrders  eSortOrder)
{
    // Create a new list to fill in
    TListPtr cptrNew
    (
        new TListInfo
        (
            EDispLvls::Title
            , m_strRepoMoniker
            , siSel.strCookie()
        )
    );

    // If we have a database, let's load up the list, else leave empty
    if (m_cptrMetaData.pobjData())
    {

        const TMediaDB& mdbData = m_cptrMetaData->mdbData();

        if (m_eCurMediaType == tCQCMedia::EMediaTypes::Music)
        {
            //
            //  In this case we are browsing artists, so get the selected artist and ask
            //  for a list of titles associated with that artist.
            //
            const TArtistMap* pamapSel = mdbData.pcolFindArtistMap
            (
                m_eCurMediaType, siSel.strName()
            );

            // If we got a list, then let's load it
            if (pamapSel)
            {
                const TArtistMap::TSetList& colTitles = pamapSel->colSets();
                const tCIDLib::TCard4 c4Count = colTitles.c4ElemCount();

                TString strCookie;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TMediaTitleSet& mtsCur = *colTitles[c4Index];

                    // Build up the cookie for this guy
                    strCookie = siSel.strCookie();
                    strCookie.Append(L',');
                    strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);

                    cptrNew->m_colList.objAdd
                    (
                        TSlotInfo
                        (
                            mtsCur.strArtist()
                            , strCookie
                            , mtsCur.strName()
                            , mtsCur.c2Id()
                            , mtsCur.c4ColCount()
                        )
                    );
                }
            }
        }
         else
        {
            //
            //  In this case we are browsing categories, so get the category that was
            //  selected.
            //
            const TMediaCat* pmcatSrc = mdbData.pmcatById
            (
                m_eCurMediaType, siSel.c2Id(), kCIDLib::True
            );

            //
            //  We have to go through all the sets and see if they are in our
            //  category.
            //
            TString strCookie;
            const tCIDLib::TCard4 c4Count = mdbData.c4TitleSetCnt(m_eCurMediaType);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TMediaTitleSet& mtsCur = mdbData.mtsAt(m_eCurMediaType, c4Index);

                if (mdbData.bTitleIsInCat(mtsCur, siSel.c2Id()))
                {
                    // Build up the cookie for this guy
                    strCookie = siSel.strCookie();
                    strCookie.Append(L',');
                    strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);

                    cptrNew->m_colList.objAdd
                    (
                        TSlotInfo
                        (
                            mtsCur.strArtist()
                            , strCookie
                            , mtsCur.strName()
                            , mtsCur.c2Id()
                            , mtsCur.c4ColCount()
                        )
                    );
                }
            }
        }

        // Let the list object do size calculations
        cptrNew->Resize(m_c4SlotHeight, m_areaDisplay);

        // Sort the list by the name
        cptrNew->Sort();
    }
    return cptrNew;
}


//
//  This is a helper, only called in debug mode, so we make it's contents
//  debug only as well. It just sanity checks our level info. Sine we use
//  pointers into the cache for our lists, and code all over assumes that
//  any list at or below the current level is valid, and various bits of
//  code assume that we are at some minimal level or greater, this is called
//  as a kind of fancy assert on entry to various methods.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::
CheckListLevel(const TCQCIntfMediaListBrowser::EDispLvls eMinLevel) const
{
    CIDAssert
    (
        m_eDispLevel >= eMinLevel
        , L"The current display level was below the expected minimum"
    );

    #if CID_DEBUG_ON
    #endif
}



//
//  This guy does any drawing required. We just look at the current level
//  get the list, the top index, and update any of the slots that need to
//  be redrawn (intersect the invalid area.)
//
//  To support scrolling operations, we take an offset which allows for
//  loading of different chunks of content into the top or bottom of a
//  scrollable memory image.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::DrawItems(        TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaRelTo
                                    , const TArea&              areaInvalid
                                    , const TPoint&             pntOffset
                                    , const TListPtr            cptrList
                                    , const tCIDLib::TBoolean   bForceAlpha)
{
    // Set our widget's font for this scope, and turn off bgn fill for text
    TCQCIntfFontJan janFont(&gdevTarget, *this);
    gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

    //
    //  Figure out which item will be the first one visible within this
    //  invalid area. It might only be partially so. And get its area. We'll
    //  move forward from there until we go beyond the invalid
    //  area.
    //
    tCIDLib::TCard4 c4Index = c4FirstVisIndex(areaInvalid, areaRelTo);
    TArea areaCur;
    AreaForIndex(c4Index, areaRelTo, areaCur);

    // Remember if we need to bother with focus
    const tCIDLib::TBoolean bDoFocus(!iwdgRootTemplate().bNoFocus() && bHasFocus());
    const tCIDLib::TCard4  c4FocusIndex = cptrList->m_c4FocusIndex;

    // Loop till we move past the target area or run out of items
    TArea areaAdj;
    const tCIDLib::TCard4 c4Count = cptrList->c4Count();
    while (c4Index < c4Count)
    {
        if (areaCur.i4Y() > areaInvalid.i4Bottom())
            break;

        // If this one intersects the invald area, draw it
        if (areaCur.bIntersects(areaInvalid))
        {
            areaAdj = areaCur;
            areaAdj += pntOffset;
            DrawItemText
            (
                gdevTarget
                , areaAdj
                , cptrList->siAt(c4Index)
                , bForceAlpha
                , bDoFocus && (c4Index == c4FocusIndex)
            );
        }

        // Move the index and area forward (including spacing)
        c4Index++;
        areaCur.Move(tCIDLib::EDirs::Down);
    }
}


//
//  There's a bit of work to support the various text options, so we split
//  this out to handle the drawing of the text for a given slot.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::DrawItemText(         TGraphDrawDev&      gdevTarget
                                        , const TArea&              areaTar
                                        , const TSlotInfo&          siToDraw
                                        , const tCIDLib::TBoolean   bForceAlpha
                                        , const tCIDLib::TBoolean   bDoFocus)
{
    // Clip the output to this area
    TRegionJanitor janClip(&gdevTarget, areaTar, tCIDGraphDev::EClipModes::And);

    // For convenience get some stuff we'll need
    const tCIDLib::EHJustify eHJust = eHJustify();
    const tCIDLib::EVJustify eVJust = tCIDLib::EVJustify::Center;
    const TString& strText = siToDraw.strName();

    // If pressed, we fill the background area
    if (siToDraw.bPressed())
        gdevTarget.Fill(areaTar, rgbExtra3());

    // Pull inwards a bit on left and right
    TArea areaText = areaTar;
    areaText.Deflate(5, 0);

    // If text FX is enabled, do that, else the usual thing
    TCQCIntfView& civUs = civOwner();
    if (eTextFX() != tCIDGraphDev::ETextFX::None)
    {
        TPoint pntOfs = pntOffset();
        if (siToDraw.bPressed())
            pntOfs.Adjust(1, 1);

        civUs.DrawTextFX
        (
            gdevTarget
            , strText
            , areaText
            , eTextFX()
            , bDoFocus ? rgbExtra() : rgbFgn()
            , bDoFocus ? rgbExtra2() : rgbFgn2()
            , eHJust
            , eVJust
            , bNoTextWrap()
            , pntOfs
        );
    }
     else
    {
        // If doing shadow text, then draw that
        TArea areaTmp;
        if (bShadow())
        {
            //
            //  The drop amount is scaled by the font height. From 0
            //  to 24, we do one pixel. From there we do (height/12).
            //
            tCIDLib::TCard4 c4Drop = 1;
            if (c4FontHeight() > 24)
                  c4Drop = (c4FontHeight() / 12);
            areaTmp = areaText;
            areaTmp.AdjustOrg(c4Drop, c4Drop);
            if (siToDraw.bPressed())
                areaTmp.AdjustOrg(1, 1);
            gdevTarget.SetTextColor(bDoFocus ? rgbExtra2() : rgbFgn2());

            if (bNoTextWrap())
            {
                if (bForceAlpha)
                    civUs.DrawTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);
                else
                    gdevTarget.DrawString(strText, areaTmp, eHJust, eVJust);
            }
             else
            {
                if (bForceAlpha)
                    civUs.DrawMTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);
                else
                    gdevTarget.DrawMText(strText, areaTmp, eHJust, eVJust);
            }
        }

        // If pressed, we'll adjust the area down and right
        areaTmp = areaText;
        if (siToDraw.bPressed())
            areaTmp.AdjustOrg(1, 1);

        gdevTarget.SetTextColor(bDoFocus ? rgbExtra() : rgbFgn());
        if (bNoTextWrap())
        {
            if (bForceAlpha)
                civUs.DrawTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);
            else
                gdevTarget.DrawString(strText, areaTmp, eHJust, eVJust);
        }
         else
        {
            if (bForceAlpha)
                civUs.DrawMTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);
            else
                gdevTarget.DrawMText(strText, areaTmp, eHJust, eVJust);
        }
    }
}


//
//  Tranlates from the values we take in command parameters, to the
//  internal enumerated type for some commonly used enums. We can't use
//  the standard enum stuff because the value we let the user enter isn't
//  the same as that used by the standard enum translation stuff.
//
tCQCMedia::EMediaTypes
TCQCIntfMediaListBrowser::eXlatMediaType(const  TCQCCmdCfg&     ccfgSrc
                                        , const tCIDLib::TCard4 c4ParmInd) const
{
    const TString& strVal = ccfgSrc.piAt(c4ParmInd).m_strValue;
    const tCQCMedia::EMediaTypes eMType = facCQCMedia().eXlatCmdMediaType(strVal);
    if (eMType == tCQCMedia::EMediaTypes::Count)
        ThrowBadParmVal(ccfgSrc, c4ParmInd);
    return eMType;
}

tCQCMedia::ESortOrders
TCQCIntfMediaListBrowser::eXlatSortOrder(const  TCQCCmdCfg&     ccfgSrc
                                        , const tCIDLib::TCard4 c4ParmInd) const
{
    const TString& strVal = ccfgSrc.piAt(c4ParmInd).m_strValue;
    const tCQCMedia::ESortOrders eOrder = facCQCMedia().eXlatCmdSortOrder(strVal);
    if (eOrder == tCQCMedia::ESortOrders::Count)
        ThrowBadParmVal(ccfgSrc, c4ParmInd);
    return eOrder;
}


//
//  This is called when we get a Back command. We move back to the previous
//  list if we can. In special cases we skip more than one list. If we go
//  back to a collection level and there's only one collection, then we just
//  move on back to the title set level.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::GoBack()
{
    // If on the top level, then nothing to do
    if (m_eDispLevel == EDispLvls::Top)
        return;

    //
    //  Remember the current top list and original level, and undo it's
    //  selection index.
    //
    TListPtr cptrOrg = m_objaData[m_eDispLevel];
    const EDispLvls eOrgLevel = m_eDispLevel;
    cptrOrg->m_c4SelIndex = kCIDLib::c4MaxCard;

    //
    //  See what level we want to go back to, dealing with the special
    //  case mentioned above. Drop the current one first.
    //
    //  We can't actually drop the original list until we exit, because if
    //  we have to scroll, then we need the original list. We already saved
    //  the original info above.
    //
    try
    {
        m_eDispLevel--;

        //
        //  The selection level is really for moving the other direction,
        //  down into the lists. But, because of the way the runtime values
        //  object uses it, when going the other way, it works for us to set
        //  it to the level above what we end up on, if we didn't get up to
        //  the top level yet.
        //
        m_eSelectLevel = m_eDispLevel;
        if (m_eSelectLevel > EDispLvls::Top)
            m_eSelectLevel--;

        if (m_eDispLevel == EDispLvls::Col)
        {
            //
            //  If back to collection level and the title set has just one
            //  collection, move back again.
            //
            if (m_objaData[EDispLvls::Col]->c4Count() == 1)
            {
                m_objaData[m_eDispLevel].DropRef();
                m_eDispLevel--;

                // Same as above...
                m_eSelectLevel = m_eDispLevel;
                m_eSelectLevel--;
            }
        }

        //
        //  If remote mode, just redraw, else do a sliding scroll
        //
        if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
        {
            KeepFocusVisible();
            Redraw();
        }
         else
        {
            SlideListLevel(eOrgLevel, m_eDispLevel);
        }

        // Now we can drop the original list reference
        cptrOrg.DropRef();
    }

    catch(TError& errToCatch)
    {
        // Drop the original list before we return
        cptrOrg.DropRef();

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw errToCatch;
    }

    // And send out an on set command since we changed the list level
    PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::Nested);
}


//
//  Called after moving the list, to insure that the focus index remains
//  visible.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::KeepFocusVisible()
{
    // Get the current list
    TListPtr cptrList = m_objaData[m_eDispLevel];

    // If not set yet, do nothing
    if (!cptrList.pobjData())
        return;

    // Get the area of the focus item for this list
    TArea areaFocus;
    AreaForIndex(cptrList->m_c4FocusIndex, areaFocus);

    //
    //  If it's not fully visible, then move to the closest one
    //  that is visible.
    //
    if (!m_areaDisplay.bContainsArea(areaFocus))
        cptrList->m_c4FocusIndex = c4ClosestVisIndex(cptrList->m_c4FocusIndex);
}



tCIDLib::TVoid
TCQCIntfMediaListBrowser::ItemInvoked(const tCIDLib::TCard4 c4Index)
{
    try
    {
        TArea areaHit;
        AreaForIndex(c4Index, areaHit);

        // Do a faux press/release feedback
        {
            TListPtr cptrList = m_objaData[m_eDispLevel];
            TSlotInfo& siCur = cptrList->siAt(c4Index);

            siCur.bPressed(kCIDLib::True);
            Redraw(areaHit);
            TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs);
            siCur.bPressed(kCIDLib::False);
            Redraw(areaHit);
        }

        //
        //  Handle the selection on the passed target index, If it returns
        //  false, do nothing, mostly likely because the new list is empty.
        //  Shouldn't happen generally but be careful of that since it would
        //  create an empty list area and mayhem could ensue.
        //
        tCIDLib::TCard4 c4RealInd = c4Index;
        if (!bHandleSelection(c4RealInd))
            return;

        // Make sure the selection level is valid now
        CIDAssert
        (
            (m_eSelectLevel >= EDispLvls::Min)
              && (m_eSelectLevel <= EDispLvls::Max)
            , L"Invalid selection level for OnClick"
        );

        //
        //  Any time the list changes we do an OnSet which is just to let
        //  them update any 'what am I browsing in now' type of display info.
        //  So if the select and display levels are different at this point,
        //  a new list was selected. Even if this could somehow cause us to
        //  send an occasional surplus event, it's no biggie.
        //
        if (m_eSelectLevel != m_eDispLevel)
            PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::Nested);

        //
        //  We are getting the selected slot in the originally selected list,
        //  which often is not the list now displayed. The bHandleSelection
        //  method above may have moved us forward more than one level if
        //  a title with one collection was clicked on, or a collection with
        //  just one item.
        //
        //  If so, he will update c4RealInd with the correct index for the
        //  new list we end up on, and bump the select level to the one
        //  we are now really selecting from.
        //
        TListPtr cptrList = m_objaData[m_eSelectLevel];
        const TSlotInfo& siCur = cptrList->siAt(c4RealInd);

        //
        //  Ok, if we are in item mode, that's an easy one, since we know
        //  we can just send a on select item.
        //
        //  Otherwise, if we are in title mode, and the selected title has
        //  only one collection, or in collection mode, do an on select
        //  collection.
        //
        if (m_eSelectLevel == EDispLvls::Item)
        {
            PostOps
            (
                *this
                , kCQCMedia::strEvId_OnSelectItem
                , tCQCKit::EActOrders::Nested
            );
        }
         else if (m_eSelectLevel == EDispLvls::Col)
        {
            PostOps
            (
                *this
                , kCQCMedia::strEvId_OnSelectCol
                , tCQCKit::EActOrders::Nested
            );
        }
         else if (m_eSelectLevel == EDispLvls::Title)
        {
            //
            //  If the child count is 1, then do a collection select. The
            //  RTV gen'er upper will know what to do with this. If it's
            //  more than one, send an OnSelect
            //
            if (siCur.c4ChildCount() == 1)
            {
                PostOps
                (
                    *this
                    , kCQCMedia::strEvId_OnSelectCol
                    , tCQCKit::EActOrders::Nested
                );
            }
             else if (siCur.c4ChildCount() > 1)
            {
                PostOps
                (
                    *this
                    , kCQCKit::strEvId_OnSelect
                    , tCQCKit::EActOrders::Nested
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This is called from the PostInit method, which is called on widgets to give them a chance
//  to do stuff before they are displayed. We have to get the list of artists or categories
//  set up and ready to display the first page initially.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::LoadTopList()
{
    try
    {
        // Create a new list to fill in if we can, else it stays empty

        TListPtr cptrNew
        (
            new TListInfo
            (
                EDispLvls::Top, m_strRepoMoniker, TString::strEmpty()
            )
        );

        // If we have a database, let's load up the list, else leave empty
        if (m_cptrMetaData.pobjData())
        {
            // Choose an initial type, if one isn't set yet
            if (m_eCurMediaType == tCQCMedia::EMediaTypes::Count)
            {
                // Assume the initial value
                m_eCurMediaType = m_eInitMediaType;
                tCQCMedia::EMTFlags eMTFlags = m_cptrMetaData->eMTFlags();

                // If the initial type is not supported, then select one
                if (!facCQCMedia().bTestMediaFlags(m_eCurMediaType, eMTFlags))
                {
                    if (tCIDLib::bAllBitsOn(eMTFlags, tCQCMedia::EMTFlags::Music))
                        m_eCurMediaType = tCQCMedia::EMediaTypes::Music;
                    else if (tCIDLib::bAllBitsOn(eMTFlags, tCQCMedia::EMTFlags::Movie))
                        m_eCurMediaType = tCQCMedia::EMediaTypes::Movie;
                    else
                        m_eCurMediaType = tCQCMedia::EMediaTypes::Pic;
                }
            }

            const TMediaDB& mdbData = m_cptrMetaData->mdbData();

            if (m_eCurMediaType == tCQCMedia::EMediaTypes::Music)
            {
                //
                //  We will set the category on all our list items to the All Movies/Music
                //  category. All our titles are guaranteed to be in those and it let's us work
                //  in a more consitent way if we always have a cookie at each level for the
                //  next level to build on.
                //
                TString strCookie;
                facCQCMedia().FormatCatCookie
                (
                    m_eCurMediaType
                    , (m_eCurMediaType == tCQCMedia::EMediaTypes::Movie) ? kCQCMedia::c2CatId_AllMovies
                                                                         : kCQCMedia::c2CatId_AllMusic
                    , strCookie
                );

                // Get the list of artists, which is already sorted
                const tCQCMedia::TByArtistList& colArtists = mdbData.colByArtistList(m_eCurMediaType);
                const tCIDLib::TCard4 c4Count = colArtists.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TArtistMap* pamapCur = colArtists[c4Index];

                    cptrNew->m_colList.objAdd
                    (
                        TSlotInfo
                        (
                            L"<Artist List>", strCookie, pamapCur->strArtistName(), 0
                        )
                    );
                }
            }
             else
            {
                // Get a sorted list of cats (we knows its for movies in this case)
                TRefVector<const TMediaCat> colCats(tCIDLib::EAdoptOpts::NoAdopt);
                const tCIDLib::TCard4 c4Count = mdbData.c4QueryCatList
                (
                    m_eCurMediaType, colCats, &TMediaCat::eCompByName
                );

                TString strCatCookie;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TMediaCat* pmcatCur = colCats[c4Index];

                    // Format the cookie for this guy
                    facCQCMedia().FormatCatCookie
                    (
                        m_eCurMediaType, pmcatCur->c2Id(), strCatCookie
                    );

                    cptrNew->m_colList.objAdd
                    (
                        TSlotInfo
                        (
                            TString::strEmpty()
                            , strCatCookie
                            , pmcatCur->strName()
                            , pmcatCur->c2Id()
                        )
                    );
                }
            }
        }

        // Point our artist level list at it
        m_objaData[EDispLvls::Top] = cptrNew;

        // Allow it to do size calcs
        cptrNew->Resize(m_c4SlotHeight, m_areaDisplay);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  A helper to set all the runtime stuff to their default values, so we
//  don't have to do it from multiple places.
//
tCIDLib::TVoid TCQCIntfMediaListBrowser::SetDefaults()
{
    m_eCurMediaType = m_eInitMediaType;
    m_eCurSortOrder = tCQCMedia::ESortOrders::Title;
    m_eDispLevel    = EDispLvls::Top;
    m_eInitSortOrder= tCQCMedia::ESortOrders::Title;
    m_eSelectLevel  = EDispLvls::Count;

    // Initialize the list pointers and indices
    EDispLvls eLevel = EDispLvls::Min;
    while (eLevel <= EDispLvls::Max)
        m_objaData[eLevel++].DropRef();

    // Reset our database serial number
    m_strMDBSerialNum.Clear();
}


//
//  A helper that handles scrolling the current list. All we have to do is
//  figure out where to set to top index to, based on the current top index
//  and the type of scroll asked for. Then we just redraw.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::ScrollList(const TString& strScrollOp)
{
    try
    {
        //
        //  First make sure that the list for the current display level is set.
        //  If not, then obviously nothing to do and something is wrong.
        //
        TListPtr cptrCur = m_objaData[m_eDispLevel];
        if (!cptrCur.pobjData())
            return;

        // If our display area is larger than the content, then nothing to do
        const TArea& areaCont = cptrCur->m_areaContent;
        if (m_areaDisplay.c4Height() >= areaCont.c4Height())
            return;

        // Save a copy of the adjusted area so we can tell if we moved
        const TArea areaOrg = cptrCur->m_areaAdjDisp;

        //
        //  Now just move the area based on the command. Clip to the content
        //  area as needed.
        //
        if (strScrollOp == L"Home")
        {
            cptrCur->m_areaAdjDisp.i4Y(areaCont.i4Y());
        }
         else if ((strScrollOp == L"Previous Page")
              ||  (strScrollOp == L"Next Page"))
        {
            const tCIDLib::TBoolean bNext(strScrollOp == L"Next Page");

            if (bNext)
            {
                cptrCur->m_areaAdjDisp.Move(tCIDLib::EDirs::Down);
                if (cptrCur->m_areaAdjDisp.i4Bottom() > areaCont.i4Bottom())
                    cptrCur->m_areaAdjDisp.BottomJustifyIn(areaCont);
            }
             else
            {
                cptrCur->m_areaAdjDisp.Move(tCIDLib::EDirs::Up);
                if (cptrCur->m_areaAdjDisp.i4Y() < areaCont.i4Y())
                    cptrCur->m_areaAdjDisp.i4Y(areaCont.i4Y());

            }
        }
         else if (strScrollOp == L"End")
        {
            cptrCur->m_areaAdjDisp.BottomJustifyIn(areaCont);
        }

        //
        //  If we actually moved, then update. If in remote mode, just
        //  redraw at the new position. Else do a slide.
        //
        if (areaOrg.i4Y() != cptrCur->m_areaAdjDisp.i4Y())
        {
            if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
            {
                KeepFocusVisible();
                Redraw();
            }
             else
            {
                SlideList(areaOrg);
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This handles non-drag oriented sliding of the lists vertically, mostly
//  to support command driven list movement. It's sort of redundant with
//  the level slider below but too much different to want to bother with the
//  complexity of integrating them.
//
//  We have two basic scenarios, one where we are moving from one area to
//  another which are contiguous or overlap, and the other where we move
//  between two discontiguous areas. In the later case we just clear the
//  list content and scroll just the new content in from one side or the
//  other. We don't try to scroll current and new content because of the
//  issues of partial items being scrolled.
//
//  Note that the directions seem backwards here but it's because the
//  adjusted area is moved downwards over the content. But here we are
//  simulating the movement of the content in the other direction.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::SlideList(const TArea& areaOrg)
{
    // Get the top list
    TListPtr cptrCur = m_objaData[m_eDispLevel];

    // Set up the whole area we need to scroll
    TArea areaScroll;

    // Figure out which scenario
    TArea areaFirst;
    TArea areaSec;

    tCIDLib::EDirs eDir;
    if (areaOrg.i4Y() > cptrCur->m_areaAdjDisp.i4Y())
    {
        eDir = tCIDLib::EDirs::Down;
        areaFirst = cptrCur->m_areaAdjDisp;
        areaSec = areaOrg;
    }
     else
    {
        eDir = tCIDLib::EDirs::Up;
        areaFirst = areaOrg;
        areaSec = cptrCur->m_areaAdjDisp;
    }

    //
    //  If the two areas overlap (or just touch) we have a contiguous range,
    //  else we have to do two separate draws.
    //
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TBitmap bmpScroll;
    if (areaFirst.bIntersects(areaSec)
    ||  (areaFirst.i4Bottom() == areaSec.i4Y()))
    {
        //
        //  The scroll area is the intersected width, and the zero origined
        //  version of the combination is the scroll area.
        //
        TArea areaSrc = areaFirst;
        areaSrc |= areaSec;

        areaScroll = areaSrc;
        areaScroll.ZeroOrg();

        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);
        TRegionJanitor janClip(&gdevCont, areaScroll, tCIDGraphDev::EClipModes::And);

        // If a safe bgn, put it into the content buffer
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);
        }

        //
        //  And draw our contiguous range into the scroll buffer. We have
        //  to create a scrolled content area to use as the 'relative to'
        //  area. This will make the bit we care about come out in our
        //  zero origined buffer.
        //
        TArea areaRelTo = cptrCur->m_areaContent;
        areaRelTo -= areaSrc.pntOrg();
        DrawItems
        (
            gdevCont
            , areaRelTo
            , areaScroll
            , TPoint::pntOrigin
            , cptrCur
            , !m_bSafePanBgn
        );
    }
     else
    {
        // The scroll area is just twice the display area
        areaScroll.SetSize
        (
            m_areaDisplay.c4Width(), m_areaDisplay.c4Height() * 2
        );

        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );

        // Copy the two src areas out to their appropriate target places
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);

        TArea areaZDisp(m_areaDisplay);
        areaZDisp.ZeroOrg();


        //
        //  If a safe bgn, put it into the content buffer, but we also
        //  need to draw to the screen the stuff under us, since the
        //  efficient scroll will move the on-screen content, and that
        //  would probably scroll a partial item.
        //
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);

            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
            cptrGraphDev->CopyBits(gdevBgn, areaZDisp, m_areaDisplay);
        }

        TArea areaRelTo;
        if (eDir == tCIDLib::EDirs::Up)
        {
            // Put the second page in the first half and scroll opposite
            areaRelTo = cptrCur->m_areaContent;
            areaRelTo -= areaSec.pntOrg();

            DrawItems
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , TPoint::pntOrigin
                , cptrCur
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Down;
        }
         else
        {
            // Put the first page in the second half and scroll the opposite
            areaRelTo = cptrCur->m_areaContent;
            areaRelTo -= areaFirst.pntOrg();

            DrawItems
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , areaZDisp.pntLL(0, 1)
                , cptrCur
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Up;
        }
    }

    //
    //  We have a special case here since we are doing this scroll. We need
    //  to make sure our slot area is clipped to within all our parent's
    //  areas, since the content of the template we are in may be bigger
    //  than the overlay we are in.
    //
    TGUIRegion grgnClip;
    if (!bFindMostRestrictiveClip(grgnClip, m_areaDisplay))
    {
        //
        //  We are completely clipped. Wierd since we shouldn't have seen
        //  the gesture, but anyway...
        //
        return;
    }

    // Get a device, clip, and do the scroll
    TRegionJanitor janClip
    (
        cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
    );

    TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);
    TCIDEaseScroller escrSlide;
    if (m_bSafePanBgn)
    {
        // Do the light weight slide
        escrSlide.Scroll2
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }
     else
    {
        //
        //  We have to do the heavier scroll, so set up a bitmap with our
        //  bgn fill in it, and provide a double buffer buffer.
        //
        TBitmap bmpBgn = bmpGatherBgn();
        TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

        // Do the double buffering bitmap
        TBitmap bmpBuf
        (
            m_areaDisplay.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

        escrSlide.Scroll1
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevBuf
            , gdevBgn
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }

    // Make sure the focus stays visible
    KeepFocusVisible();

    // And do a final redraw
    Redraw(m_areaDisplay);
}


//
//  Does the standard inertial slide when we move left or right from one
//  one level list to another.
//
//  We get the old and new levels and top offsets. We are always doing two
//  full pages here (well, one or both may not fill the whole display area,
//  but they never will overlap.) When drilling down, the original is left
//  and the new slides in from the right (left scroll.) And vice versa.
//
//  When coming back up to a higher level, the original offset is the offset
//  of the list we were on, and the new is the remembered offset of the
//  previous list we originally came from (and are going back to)
//
//  When drilling downwards, the new offset is always zero since we always
//  start at the top of the newly exposed list.
//
tCIDLib::TVoid
TCQCIntfMediaListBrowser::SlideListLevel(const  EDispLvls   eOrgLevel
                                        , const EDispLvls   eNewLevel)
{
    // Shouldn't happen but just in case
    if (eOrgLevel == eNewLevel)
        return;

    //
    //  If we don't have a scroll safe bgn in the horizontal direction,
    //  we have to force alpha based drawing since we have to composite.
    //
    const tCIDLib::TBoolean bForceAlpha = !bScrollSafeBgn(kCIDLib::False);

    // Make sure this stuff is cleaned up so we can do a redraw at the end
    {
        // The scroll area will be twice the display area, origin based
        TArea areaScroll = m_areaDisplay;
        areaScroll.ScaleSize(2.0, 1.0);
        areaScroll.ZeroOrg();

        // Create the scrollable content buffer
        TBitmap bmpScroll
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);

        //
        //  If we have a safe bgn, we can put our bgn content into the
        //  scrollable content directly. We have to be sure any gradients
        //  align, so we create another area that is the size of the
        //  scrollable content, but as tall as us. we center that over
        //  our scroll area horizontally, and do the fill on that.
        //
        {
            TArea areaFill(areaScroll);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevScroll, areaFill);
        }

        //
        //  Set up the scroll direction and the lower and higher levels,
        //  since we always draw them higher on the left, lower on the
        //  right.
        //
        EDispLvls       eLeftLvl;
        EDispLvls       eRightLvl;
        tCIDLib::EDirs  eDir;
        if (eOrgLevel < eNewLevel)
        {
            // We are drilling down to a more nested level
            eDir = tCIDLib::EDirs::Left;
            eLeftLvl = eOrgLevel;
            eRightLvl = eNewLevel;
        }
         else
        {
            // We are coming back up out of a level
            eDir = tCIDLib::EDirs::Right;
            eLeftLvl = eNewLevel;
            eRightLvl = eOrgLevel;
        }

        // Get the lists as left/right sides
        const TListPtr cptrLeft = m_objaData[eLeftLvl];
        const TListPtr cptrRight = m_objaData[eRightLvl];

        // Draw up to a page's worth on the left
        TArea areaTar = m_areaDisplay;
        areaTar.ZeroOrg();
        TArea areaRelTo = cptrLeft->areaContentsRelTo(areaTar);
        DrawItems
        (
            gdevScroll
            , areaRelTo
            , areaTar
            , TPoint::pntOrigin
            , cptrLeft
            , bForceAlpha
        );

        //
        //  Now do the right side. We pass an offset of the upper right
        //  corner of the target area.
        //
        areaRelTo = cptrRight->areaContentsRelTo(areaTar);
        DrawItems
        (
            gdevScroll
            , areaRelTo
            , areaTar
            , areaTar.pntUR(1, 0)
            , cptrRight
            , bForceAlpha
        );

        //
        //  We have a special case here since we are doing this scroll. We need
        //  to make sure our scroll area is clipped to within all our parent's
        //  areas, since the content of the template we are in may be bigger
        //  than the overlay we are in, and within our display area.
        //
        TGUIRegion grgnClip;
        if (!bFindMostRestrictiveClip(grgnClip, m_areaDisplay))
            return;

        // Get a device an dset the clip area on it
        tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
        TRegionJanitor janClip
        (
            cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::And
        );

        // And finally create a scroller and invoke it
        TCIDEaseScroller escrSlide;

        if (bForceAlpha)
        {
            // We have to provide the bgn info separately here
            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

            // And we need a compositing buffer
            TBitmap bmpBuf
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
            TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

            escrSlide.Scroll1
            (
                *cptrGraphDev
                , m_areaDisplay
                , areaScroll
                , gdevBuf
                , gdevBgn
                , gdevScroll
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
         else
        {
            // We can do the efficient scheme here
            escrSlide.Scroll2
            (
                *cptrGraphDev
                , m_areaDisplay
                , areaScroll
                , gdevScroll
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
    }

    //
    //  Make sure the focus index wasn't made invisible by a size change
    //  while this list was away.
    //
    KeepFocusVisible();

    // Now do a final redraw of our content to end it
    Redraw(m_areaDisplay);
}

