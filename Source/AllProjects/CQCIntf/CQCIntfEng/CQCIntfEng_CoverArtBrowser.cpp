//
// FILE NAME: CQCIntfEng_CoverArtBrowser.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2005
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
//  This file implements the cover art browser widget
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

// The widget types we'll use in our layout, so we have to import them
#include    "CQCIntfEng_SpecialImg.hpp"
#include    "CQCIntfEng_MediaImage.hpp"
#include    "CQCIntfEng_BooleanImg.hpp"
#include    "CQCIntfEng_MItemBrowser.hpp"
#include    "CQCIntfEng_StaticImg.hpp"
#include    "CQCIntfEng_StaticText.hpp"

#include    "CQCIntfEng_CoverArtBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfCvrArtBrowser, TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_CvrArtBrowser
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 1 -
        //      Reset to version 1 for 2.0 since we have to a manual conversion
        //      during install anyway.
        //
        //  Version 2 -
        //      Add support for the backing images (drawn behind the cover art.)
        //      So now we have to stream in and out the image list mixin.
        //
        //  Version 3 -
        //      The user can now indicate whehter he wants to use large images or
        //      thumbs in the CAB, so we neededa new flag.
        //
        //  Version 4 -
        //      Action contexts were moved to the actual events. So, for previous
        //      versions, we have to set the context for any that need to have
        //      anything other than the standard context.
        //
        //  Version 5 -
        //      We removed the display effect enum, since we now always just do a
        //      standard inertial type slide in effect.
        //
        //  Version 6 -
        //      We moved to using a template to provide the formatting of the
        //      display areas. So we got rid of the width/height values and added
        //      a template path. We also got rid of the show options flag since
        //      that no longer makes any sense. And we got rid of the use of the
        //      image list mixin since it's not longer useful.
        //
        //  Version 7 -
        //      Conversion of template paths to 5.x format
        //
        //  Version 8 -
        //      Sort order is a configurable parameter but it wasn't being persisted.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 8;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::TakesFocus
        );


        // -----------------------------------------------------------------------
        //  Max stack depth, then we start throwing out old ones
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MaxStackDepth = 16;


        // -----------------------------------------------------------------------
        //  The default CAB layout template
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszDefLayout = L"/System/Layouts/DefaultCABLayout";


        // -----------------------------------------------------------------------
        //  Some widget ids we look for in our layout template
        // -----------------------------------------------------------------------
        const TString   strWID_Focus(L"FocusImage");
        const TString   strWID_Pressed(L"PressedImage");
        const TString   strWID_CoverArt(L"CoverArt");


        // -----------------------------------------------------------------------
        //  The img list mixin keys for the images we support
        // -----------------------------------------------------------------------
        const TString   strMissingArt(L"Missing Art");
        const TString   strPlayListArt(L"Playlist Art");
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCABRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class, to expose our fairly
//  large number of runtime values.
// ---------------------------------------------------------------------------
class TCQCIntfCABRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfCABRTV
        (
            const   tCIDLib::TBoolean       bTopLevel
            , const tCIDLib::TCard4         c4SelIndex
            , const tCIDLib::TCard4         c4FirstIndex
            , const tCIDLib::TCard4         c4TitleCnt
            , const tCIDLib::TCard4         c4ColCnt
            , const TString&                strCatCookie
            , const TString&                strCatText
            , const TString&                strRepoMoniker
            , const tCQCMedia::EMediaTypes  eMediaType
            , const TString&                strColCookie
            , const TString&                strTitleCookie
            , const TMediaTitleSet* const   pmtsSel
            , const TMediaCollect* const    pmcolSel
            , const TFacCQCMedia::TMDBPtr   cptrMediaDB
            , const TCQCUserCtx&            cuctxToUse
        );

        ~TCQCIntfCABRTV();


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
        //  Private data members
        //
        //  m_bTopLevel
        //      Indicates if the current list is top level, i.e. displaying
        //      titles, else its displaying collections of a title set.
        //
        //  m_c4FirstIndex
        //      The index of the first item displayed (the one in the upper
        //      left.) It will be c4MaxCard if the list is empty.
        //
        //  m_c4SelIndex
        //      The of the selected item within its list. Can be c4MaxCard
        //      if none. This is either the selected title set or collection,
        //      depending on whether m_bTopLevel is set or not.
        //
        //  m_c4TitleCnt
        //  m_c4ColCnt
        //      For a top level list, the title count is the number of titles
        //      in that list and, if it's a selection driven event the col
        //      count is the number of collections in the selected title, else
        //      zero. For a collection level list, the title count is 1 and
        //      the number of collections in the open title is the collection
        //      cnt.
        //
        //  m_eMediaType
        //      The media type currently displayed in the browser, so that
        //      the target widgets can respond accordingly.
        //
        //  m_pmcolSel
        //  m_pmtsSel
        //      The selected collection and title set. Both of them may not
        //      be set, such as in a scroll event at top level. If the
        //      collection is set, then the title always will be, but not
        //      vice versal.
        //
        //  m_strCatCookie
        //  m_strCatText
        //      The category cookie and text for the current category. If
        //      we are in collection mode, the text can be the title text of
        //      the title we are viewing. The cat cookie is always the cat
        //      cookie. If in collection mode, it's the name of the title
        //      set we are in (a 'faux category' of sorts.)
        //
        //  m_strRepoMoniker
        //      The repository driver that we are associated with.
        //
        //  m_strTitleCookie
        //      The artist that was set at the title set level, either the
        //      title clicked on, or the title set that the collection that
        //      was clicked on belongs to.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bTopLevel;
        tCIDLib::TCard4         m_c4ColCnt;
        tCIDLib::TCard4         m_c4FirstIndex;
        tCIDLib::TCard4         m_c4SelIndex;
        tCIDLib::TCard4         m_c4TitleCnt;
        TFacCQCMedia::TMDBPtr   m_cptrMediaDB;
        tCQCMedia::ELocTypes    m_eLocType;
        tCQCMedia::EMediaTypes  m_eMediaType;
        const TMediaCollect*    m_pmcolSel;
        const TMediaTitleSet*   m_pmtsSel;
        TString                 m_strCatCookie;
        TString                 m_strCatText;
        TString                 m_strColCookie;
        TString                 m_strRepoMoniker;
        TString                 m_strTitleCookie;
};


// ---------------------------------------------------------------------------
//  TCQCIntfCABRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCABRTV::
TCQCIntfCABRTV( const   tCIDLib::TBoolean       bTopLevel
                , const tCIDLib::TCard4         c4SelIndex
                , const tCIDLib::TCard4         c4FirstIndex
                , const tCIDLib::TCard4         c4TitleCnt
                , const tCIDLib::TCard4         c4ColCnt
                , const TString&                strCatCookie
                , const TString&                strCatText
                , const TString&                strRepoMoniker
                , const tCQCMedia::EMediaTypes  eMediaType
                , const TString&                strColCookie
                , const TString&                strTitleCookie
                , const TMediaTitleSet* const   pmtsSel
                , const TMediaCollect* const    pmcolSel
                , const TFacCQCMedia::TMDBPtr   cptrMediaDB
                , const TCQCUserCtx&            cuctxToUse) :

    TCQCCmdRTVSrc(cuctxToUse)
    , m_bTopLevel(bTopLevel)
    , m_c4ColCnt(c4ColCnt)
    , m_c4FirstIndex(c4FirstIndex)
    , m_c4SelIndex(c4SelIndex)
    , m_c4TitleCnt(c4TitleCnt)
    , m_cptrMediaDB(cptrMediaDB)
    , m_eMediaType(eMediaType)
    , m_pmcolSel(pmcolSel)
    , m_pmtsSel(pmtsSel)
    , m_strCatCookie(strCatCookie)
    , m_strCatText(strCatText)
    , m_strColCookie(strColCookie)
    , m_strRepoMoniker(strRepoMoniker)
    , m_strTitleCookie(strTitleCookie)
{
}

TCQCIntfCABRTV::~TCQCIntfCABRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCABRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfCABRTV::bRTValue(const TString& strId,  TString& strToFill) const
{
    // Call our parent first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // See if it's one of ours
    if (strId == kCQCMedia::strRTVId_Artist)
    {
        if (m_pmcolSel)
            strToFill = m_pmcolSel->strArtist();
        else if (m_pmtsSel)
            strToFill = m_pmcolSel->strArtist();
    }
     else if (strId == kCQCMedia::strRTVId_CategoryCookie)
    {
        strToFill = m_strCatCookie;
    }
     else if (strId == kCQCMedia::strRTVId_CategoryName)
    {
        // This might be a faux category if in collection or search mode
        strToFill = m_strCatText;
    }
     else if (strId == kCQCMedia::strRTVId_ColArtist)
    {
        if (m_pmcolSel)
            strToFill = m_pmcolSel->strArtist();
    }
     else if (strId == kCQCMedia::strRTVId_ColCookie)
    {
        strToFill = m_strColCookie;
    }
     else if (strId == kCQCMedia::strRTVId_ColCount)
    {
        strToFill.SetFormatted(m_c4ColCnt);
    }
     else if (strId == kCQCKit::strRTVId_FirstIndex)
    {
        strToFill.SetFormatted(m_c4FirstIndex + 1);
    }
     else if (strId == kCQCMedia::strRTVId_LocType)
    {
        // Only have this if we have collection level info
        strToFill.Clear();
        if (m_pmcolSel)
        {
            switch(m_pmcolSel->eLocType())
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
                    break;
            };
        }
    }
     else if (strId == kCQCMedia::strRTVId_MediaFmt)
    {
        // Only available if we have collection level ifno
        if (m_pmcolSel)
            strToFill = m_pmcolSel->strMediaFormat();
        else
            strToFill.Clear();
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
     else if (strId == kCQCMedia::strRTVId_TitleArtist)
    {
        if (m_pmtsSel)
            strToFill = m_pmtsSel->strArtist();
    }
     else if (strId == kCQCMedia::strRTVId_TitleCookie)
    {
        strToFill = m_strTitleCookie;
    }
     else if (strId == kCQCMedia::strRTVId_TitleCount)
    {
        // If top level, the number of title sets, else the number of cols
        if (m_bTopLevel)
            strToFill.SetFormatted(m_c4TitleCnt);
        else
            strToFill.SetFormatted(m_c4ColCnt);
    }
     else if (strId == kCQCMedia::strRTVId_TitleName)
    {
        if (m_pmtsSel)
            strToFill = m_pmtsSel->strName();
    }
     else if (strId == kCQCMedia::strRTVId_TitleNumber)
    {
        if (m_c4SelIndex == kCIDLib::c4MaxCard)
            strToFill.Clear();
        else
            strToFill.SetFormatted(m_c4SelIndex);
    }
     else if (strId == kCQCMedia::strRTVId_TopLevel)
    {
        strToFill = m_bTopLevel ? L"True" : L"False";
    }
     else if ((strId == kCQCMedia::strRTVId_ItemNameList)
          ||  (strId == kCQCMedia::strRTVId_ItemCookieList)
          ||  (strId == kCQCMedia::strRTVId_ItemArtistList)
          ||  (strId == kCQCMedia::strRTVId_LocInfo))
    {
        if ((strId == kCQCMedia::strRTVId_LocInfo)
        &&  m_pmcolSel
        &&  (m_pmcolSel->eLocType() != tCQCMedia::ELocTypes::FileItem))
        {
            // It's collection level location info so a single value
            strToFill = m_pmcolSel->strLocInfo();
        }
         else if ((m_c4SelIndex != kCIDLib::c4MaxCard)
              &&  m_pmtsSel
              &&  m_pmcolSel
              &&  (!m_bTopLevel || (m_pmtsSel->c4ColCount() == 1)))
        {
            //
            //  These only are valid if we are in collection mode, or if
            //  we are in title mode and there's only one collection in
            //  the current set. And of course if we actually have a
            //  selected item.
            //
            const tCIDLib::TCard4 c4Count = m_pmcolSel->c4ItemCount();
            const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

            TString strItemCookie;
            strToFill.Clear();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TMediaItem& mitemCur(m_pmcolSel->mitemAt(mdbInfo, c4Index));

                // Build up the item cookie
                strItemCookie = m_strColCookie;
                strItemCookie.Append(kCIDLib::chComma);
                strItemCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);

                strToFill.Append(L"\"");
                if (strId == kCQCMedia::strRTVId_ItemNameList)
                    strToFill.Append(mitemCur.strName());
                else if (strId == kCQCMedia::strRTVId_ItemCookieList)
                    strToFill.Append(strItemCookie);
                else if (strId == kCQCMedia::strRTVId_ItemArtistList)
                    strToFill.Append(mitemCur.strArtist());
                else
                    strToFill.Append(mitemCur.strLocInfo());
                strToFill.Append(L"\" ");
            }
        }
         else
        {
            // Not valid in our current state so clear it
            strToFill.Clear();
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCvrArtBrowser::TListItem
//  PREFIX: li
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser::TListItem: Public, static methods
// ---------------------------------------------------------------------------

// These we just pass through to the title sets the items contain
tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByArtist(const  TListItem&      li1
                        , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByArtist(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByBitDepth( const   TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByBitDepth(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByBitRate(  const   TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByBitRate(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByChannels( const   TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByChannels(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByDateAdded(const   TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByDateAdded(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompById(const  TListItem&      li1
                    , const TListItem&      li2)
{
    return TMediaTitleSet::eCompById(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompBySampleRate(const  TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompBySampleRate(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompBySeqNum(const  TListItem&      li1
                        , const TListItem&      li2)
{
    return TMediaTitleSet::eCompBySeqNum(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByTitle(const   TListItem&      li1
                        , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByTitle(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompBySortTitle(const   TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompBySortTitle(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByUserRating(const  TListItem&      li1
                            , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByUserRating(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

tCIDLib::ESortComps TCQCIntfCvrArtBrowser::
TListItem::eCompByYear( const   TListItem&      li1
                        , const TListItem&      li2)
{
    return TMediaTitleSet::eCompByYear(*li1.m_pmtsThis, *li2.m_pmtsThis);
}

// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser::TListItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCvrArtBrowser::TListItem::
TListItem(  const   TMediaTitleSet* const   pmtsThis
            , const TMediaCollect* const    pmcolThis
            , const TString&                strCookie) :

    m_bImgLoaded(kCIDLib::False)
    , m_bPressed(kCIDLib::False)
    , m_pmcolThis(pmcolThis)
    , m_pmtsThis(pmtsThis)
    , m_strCookie(strCookie)
{
}

TCQCIntfCvrArtBrowser::TListItem::
TListItem(const TMediaTitleSet* const pmtsThis, const TString& strCookie) :

    m_bImgLoaded(kCIDLib::False)
    , m_bPressed(kCIDLib::False)
    , m_pmcolThis(nullptr)
    , m_pmtsThis(pmtsThis)
    , m_strCookie(strCookie)
{
}

TCQCIntfCvrArtBrowser::TListItem::~TListItem()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser::TListItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If we have no collection pointer, then we are top level. If we have a
//  collection pointer then the title set is just the container of this
//  collection.
//
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::TListItem::bIsTopLevel() const
{
    return (m_pmcolThis == nullptr);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCvrArtBrowser::TItemList
//  PREFIX: il
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser::TItemList: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCvrArtBrowser::TItemList::
TItemList(  const   TString&                strContentDescr
            , const tCIDLib::TCard4         c4ParentSel
            , const tCQCMedia::EMediaTypes  eMediaType
            , const EListTypes              eListType
            , const tCQCMedia::ESortOrders  eSortOrder
            , const TString&                strCookie
            , const TMediaTitleSet* const   pmtsCont) :

    m_c4ColumnCnt(0)
    , m_c4FocusIndex(0)
    , m_c4NextArtIndex(0)
    , m_c4ParentSel(c4ParentSel)
    , m_colItems(tCIDLib::EAdoptOpts::Adopt)
    , m_eListType(eListType)
    , m_eMediaType(eMediaType)
    , m_eSortOrder(eSortOrder)
    , m_pmtsCont(pmtsCont)
    , m_strCookie(strCookie)
    , m_strDescr(strContentDescr)
{
}

TCQCIntfCvrArtBrowser::TItemList::~TItemList()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser::TItemList: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return our content area, adjusted for scroll position
TArea
TCQCIntfCvrArtBrowser::TItemList::areaAdjusted(const TArea& areaRelTo) const
{
    TArea areaRet = m_areaContent;

    // Left/center it in the relative to area
    areaRet.LeftJustifyIn(areaRelTo, kCIDLib::True);

    // Do the scroll offset adjustment and return it
    areaRet -= m_areaAdjDisp.pntOrg();
    return areaRet;
}


// If we are not showing collections, then we are top level
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::TItemList::bIsTopLevel() const
{
    return m_eListType != EListTypes::TitleCols;
}


//
//  We just pass this through to our collection, passing along a sorting
//  function provided by the list item class.
//
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::TItemList::bResort(const tCQCMedia::ESortOrders eNewOrder)
{
    // Don't bother unless the order has changed
    if (eNewOrder != m_eSortOrder)
    {
        // Sort our list by the currently set criteria
        m_eSortOrder = eNewOrder;
        switch(eNewOrder)
        {
            case tCQCMedia::ESortOrders::Artist :
                m_colItems.Sort(TListItem::eCompByArtist);
                break;

            case tCQCMedia::ESortOrders::BitDepth :
                m_colItems.Sort(TListItem::eCompByBitDepth);
                break;

            case tCQCMedia::ESortOrders::BitRate :
                m_colItems.Sort(TListItem::eCompByBitRate);
                break;

            case tCQCMedia::ESortOrders::Channels :
                m_colItems.Sort(TListItem::eCompByChannels);
                break;

            case tCQCMedia::ESortOrders::DateAdded :
                m_colItems.Sort(TListItem::eCompByDateAdded);
                break;

            case tCQCMedia::ESortOrders::DateAddedDesc :
                m_colItems.Sort(tCIDLib::TRevCompWrap(TListItem::eCompByDateAdded));
                break;

            case tCQCMedia::ESortOrders::SampleRate :
                m_colItems.Sort(TListItem::eCompBySampleRate);
                break;

            case tCQCMedia::ESortOrders::SeqNum :
                m_colItems.Sort(TListItem::eCompBySeqNum);
                break;

            case tCQCMedia::ESortOrders::SeqNumDesc :
                m_colItems.Sort(tCIDLib::TRevCompWrap(TListItem::eCompBySeqNum));
                break;

            case tCQCMedia::ESortOrders::Title :
                m_colItems.Sort(TListItem::eCompBySortTitle);
                break;

            case tCQCMedia::ESortOrders::UserRating :
                m_colItems.Sort(TListItem::eCompByUserRating);
                break;

            case tCQCMedia::ESortOrders::UserRatingDesc :
                m_colItems.Sort(tCIDLib::TRevCompWrap(TListItem::eCompByUserRating));
                break;

            case tCQCMedia::ESortOrders::Year :
                m_colItems.Sort(TListItem::eCompByYear);
                break;

            case tCQCMedia::ESortOrders::YearDesc :
                m_colItems.Sort(tCIDLib::TRevCompWrap(TListItem::eCompByYear));
                break;

            default :
                break;
        }

        // Go back to the start of the list
        m_areaAdjDisp.i4X(0);
    }
    return kCIDLib::False;
}


//
//  We set our scroll offset to put the indicated column number on the
//  left, or as close as we can get to it.
//
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::TItemList::
bScrollToCol(const  tCIDLib::TCard4 c4ColNum
            , const tCIDLib::TCard4 c4ColWidth
            , const tCIDLib::TCard4 c4Spacing
            , const tCIDLib::TCard4 c4RowCnt)
{
    if (m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  Set up an area as wide as the display area and with an origin
    //  for the indicated column. The height doesn't really matter for
    //  our purposes.
    //
    TArea areaNew
    (
        tCIDLib::TInt4(c4ColNum * (c4ColWidth + c4Spacing))
        , 0
        , m_areaAdjDisp.c4Width()
        , 10
    );

    //
    //  Clip it to our actual content area. We only have to worry about
    //  going too far to the right.
    //
    if (areaNew.i4Right() > m_areaContent.i4Right())
        areaNew.RightJustifyIn(m_areaContent);

    //
    //  If the new origin is not the same as the current scroll offset,
    //  the we moved.
    //
    if (m_areaAdjDisp.i4X() != areaNew.i4X())
    {
        m_areaAdjDisp.i4X(areaNew.i4X());
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Find the first column that reports it has an item that starts with the passed
//  character. We'll actually return the closest one we got.
//
tCIDLib::TCard4 TCQCIntfCvrArtBrowser::TItemList::
c4FirstCharColumn(  const   tCIDLib::TCh    chFind
                    , const tCIDLib::TCard4 c4RowCount) const
{
    tCIDLib::TCard4 c4Ret = kCIDLib::c4MaxCard;

    // Only valid for top level lists
    if (bIsTopLevel())
    {
        const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0;c4Index < c4Count; c4Index++)
        {
            //
            //  Ask the title set for the appropriate character, given
            //  our sort order.
            //
            const tCIDLib::TCh chSortChar
            (
                m_colItems[c4Index]->m_pmtsThis->chSortChar(m_eSortOrder)
            );

            // If greater or equal to what we are looking for, then take it
            if (chSortChar >= chFind)
            {
                c4Ret = c4Index / c4RowCount;
                break;
            }
        }
    }
    return c4Ret;
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::TItemList::
FinishSetup(const   tCIDLib::TCard4         c4RowsHeight
            , const tCIDLib::TCard4         c4RowCount
            , const tCIDLib::TCard4         c4HSpacing
            , const tCQCMedia::ESortOrders  eInitOrder
            , const TArea&                  areaLayout
            , const TArea&                  areaDisplay)
{
    // Sort our list by the desired initial sort order
    bResort(eInitOrder);

    // Call the resize method for intiail sizing
    Resize(c4RowsHeight, c4RowCount, c4HSpacing, areaLayout, areaDisplay);
}


//
//  This will calculate the columns visible in the display area.
//
//  We have two modes. One where we consider any columns that are visible at
//  all. And another where we take a visibility test area, which is generally
//  used to pass in the area of some widget within the layout template, such
//  as the focus indicator, if any. If we get one, we will only include
//  those where this indicator is at least partially visible.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::TItemList::
QueryVisColumns(const   TArea&              areaLayout
                , const tCIDLib::TCard4     c4Spacing
                ,       tCIDLib::TCard4&    c4FirstCol
                ,       tCIDLib::TCard4&    c4LastCol
                , const TArea&              areaVisTest) const
{
    // The whole width we care about is the column width plus spacing
    const tCIDLib::TCard4 c4Width(areaLayout.c4Width() + c4Spacing);

    // Get the first and last visible column
    c4FirstCol = tCIDLib::TCard4(m_areaAdjDisp.i4X()) / c4Width;
    c4LastCol  = tCIDLib::TCard4(m_areaAdjDisp.i4Right()) / c4Width;

    //
    //  If asked to, check for the special case of the tets sub-area within
    //  the items in the first/last columns being visible. If they aren't
    //  move the columns inward.
    //
    if (!areaVisTest.bIsEmpty() && ((c4LastCol - c4FirstCol) > 2))
    {
        //
        //  Set up a test area relative to the top item on the first
        //  column.
        //
        TArea areaItem(areaVisTest);
        areaItem.AdjustOrg
        (
            tCIDLib::TInt4(c4FirstCol * (areaLayout.c4Width() + c4Spacing))
            , 0
        );

        if (!areaItem.bIntersects(m_areaAdjDisp))
            c4FirstCol++;


        areaItem = areaVisTest;
        areaItem.AdjustOrg
        (
            tCIDLib::TInt4(c4LastCol * (areaLayout.c4Width() + c4Spacing))
            , 0
        );

        if (!areaItem.bIntersects(m_areaAdjDisp))
            c4LastCol--;
    }
}



//
//  This will calculate the first and last indices visible in the display
//  area passed.
//
//  We have two modes. See QueryVisColumns() above which implements that
//  logic.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::TItemList::
QueryVisIndices(const   TArea&              areaLayout
                , const tCIDLib::TCard4     c4Spacing
                , const tCIDLib::TCard4     c4RowCount
                ,       tCIDLib::TCard4&    c4FirstInd
                ,       tCIDLib::TCard4&    c4LastInd
                , const TArea&              areaVisTest) const
{
    // If our list is empty, then return zeros
    if (m_colItems.bIsEmpty())
    {
        c4FirstInd = 0;
        c4LastInd = 0;
        return;
    }

    //
    //  Call the column calculating method first to get the first/last col.
    //  It also does the fully/partially displayed logic.
    //
    tCIDLib::TCard4 c4FirstCol, c4LastCol;
    QueryVisColumns(areaLayout, c4Spacing, c4FirstCol, c4LastCol, areaVisTest);

    //
    //  Use those to calculate the first and last indices, note that on the
    //  last one, we want the bottom index on that row, so we add row count
    //  minus one.
    //
    c4FirstInd = c4FirstCol * c4RowCount;
    c4LastInd = (c4LastCol * c4RowCount) + (c4RowCount - 1);

    // Clip the last index back to the actual list size just in case
    if (c4LastInd >= m_colItems.c4ElemCount())
        c4LastInd = m_colItems.c4ElemCount() - 1;

    // It cannot be less than the first index
    CIDAssert
    (
        c4LastInd >= c4FirstInd
        , L"Calculated last visible index is less than the last visible index"
    )
}


//
//  This is called if our CAB is resized. We need to adjust to that. It's
//  also called from the FinishSetup() method above to do the initial
//  calculations.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::TItemList::
Resize( const   tCIDLib::TCard4         c4RowsHeight
        , const tCIDLib::TCard4         c4RowCount
        , const tCIDLib::TCard4         c4HSpacing
        , const TArea&                  areaLayout
        , const TArea&                  areaDisplay)
{
    // Figure out how many columns we have (including any partial last one)
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    m_c4ColumnCnt = c4Count / c4RowCount;
    if (c4Count % c4RowCount)
        m_c4ColumnCnt++;

    //
    //  Calculate our content area, which we keep zero based here. We'll
    //  adjust it relative to whatever the caller wants when they call
    //  areaAdjusted().
    //
    m_areaContent.Set
    (
        0
        , 0
        , (m_c4ColumnCnt * areaLayout.c4Width())
          + ((m_c4ColumnCnt - 1) * c4HSpacing)
        , c4RowsHeight
    );

    //
    //  Set up the adjusted display area. It's the display area at the
    //  origin. However, watch for a shorter than display area content,
    //  and reduce the width to the content width if so.
    //
    //  Since this is also called for a resize at viewing time, we first
    //  see if we are scrolled already. If so, we do a minimal adjustment.
    //  Else it's already at the origin and fine.
    //
    if (areaDisplay.c4Width() >= m_areaContent.c4Width())
    {
        //
        //  We can't be scrolled, so go back to origin and take the content
        //  width as our width.
        //
        m_areaAdjDisp.Set
        (
            0, 0, m_areaContent.c4Width(), areaDisplay.c4Height()
        );
    }
     else
    {
        //
        //  Use the display area size, then see if we need to adjust our
        //  position.
        //
        m_areaAdjDisp.SetSize(areaDisplay.szArea());
        if (m_areaAdjDisp.i4X() < m_areaContent.i4X())
            m_areaAdjDisp.i4X(m_areaContent.i4X());
        if (m_areaAdjDisp.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisp.RightJustifyIn(m_areaContent);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCvrArtBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCvrArtBrowser::TCQCIntfCvrArtBrowser() :

    TCQCIntfWidget
    (
        CQCIntfEng_CvrArtBrowser::eCapFlags
        ,  facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CvrArtBrowser)
    )
    , m_bHavePressedInd(kCIDLib::False)
    , m_bLargeArt(kCIDLib::False)
    , m_bNewLayout(kCIDLib::False)
    , m_bSafePanBgn(kCIDLib::False)
    , m_c4ColCount(1)
    , m_c4FocusIndex(0)
    , m_c4HSpacing(16)
    , m_c4RowCount(1)
    , m_c4RowsHeight(16)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_c4VSpacing(16)
    , m_c4UpdateCnt(0)
    , m_colStack(tCIDLib::EAdoptOpts::Adopt )
    , m_eInitMediaType(tCQCMedia::EMediaTypes::Music)
    , m_eSortOrder(tCQCMedia::ESortOrders::Title)
    , m_strLayoutTmpl(CQCIntfEng_CvrArtBrowser::pszDefLayout)
{
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
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CollectArtist)
        , kCQCMedia::strRTVId_ColArtist
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
        facCQCMedia().strMsg(kMedMsgs::midRTVal_CollectCnt)
        , kCQCMedia::strRTVId_ColCount
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_FirstIndex)
        , kCQCKit::strRTVId_FirstIndex
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemArtistList)
        , kCQCMedia::strRTVId_ItemArtistList
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemNameList)
        , kCQCMedia::strRTVId_ItemNameList
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemCookieList)
        , kCQCMedia::strRTVId_ItemCookieList
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
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleCnt)
        , kCQCMedia::strRTVId_TitleCount
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleName)
        , kCQCMedia::strRTVId_TitleName
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TitleNum)
        , kCQCMedia::strRTVId_TitleNumber
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_TopLevel)
        , kCQCMedia::strRTVId_TopLevel
        , tCQCKit::ECmdPTypes::Boolean
    );

    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
        , kCQCKit::strEvId_OnScroll
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSet)
        , kCQCKit::strEvId_OnSet
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCMedia().strMsg(kMedMsgs::midCmdEv_OnSelectCol)
        , kCQCMedia::strEvId_OnSelectCol
        , tCQCKit::EActCmdCtx::Standard
    );

    // Set the display format for the date added stamp
    m_tmAddedFmt.strDefaultFormat(L"%(Y)/%(M)/%(D)");

    // Add our image keys
    AddNewImgKey(CQCIntfEng_CvrArtBrowser::strMissingArt, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_CvrArtBrowser::strPlayListArt, tCQCIntfEng::EImgReqFlags::Optional);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/CoverArtBrowser");
}

TCQCIntfCvrArtBrowser::
TCQCIntfCvrArtBrowser(const TCQCIntfCvrArtBrowser& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_bHavePressedInd(iwdgSrc.m_bHavePressedInd)
    , m_bLargeArt(iwdgSrc.m_bLargeArt)
    , m_bNewLayout(kCIDLib::False)
    , m_bSafePanBgn(iwdgSrc.m_bSafePanBgn)
    , m_c4FocusIndex(0)
    , m_c4HSpacing(iwdgSrc.m_c4HSpacing)
    , m_c4RowCount(iwdgSrc.m_c4RowCount)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_c4VSpacing(iwdgSrc.m_c4VSpacing)
    , m_c4UpdateCnt(0)
    , m_eInitMediaType(iwdgSrc.m_eInitMediaType)
    , m_eSortOrder(iwdgSrc.m_eSortOrder)
    , m_colStack(tCIDLib::EAdoptOpts::Adopt)
    , m_strInitCat(iwdgSrc.m_strInitCat)
    , m_strLayoutTmpl(iwdgSrc.m_strLayoutTmpl)
    , m_strRepoMoniker(iwdgSrc.m_strRepoMoniker)
    , m_szArt(iwdgSrc.m_szArt)
    , m_tmAddedFmt(iwdgSrc.m_tmAddedFmt)
{
}

TCQCIntfCvrArtBrowser::~TCQCIntfCvrArtBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfCvrArtBrowser&
TCQCIntfCvrArtBrowser::operator=(const TCQCIntfCvrArtBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);

        m_c4HSpacing        = iwdgSrc.m_c4HSpacing;
        m_c4RowCount        = iwdgSrc.m_c4RowCount;
        m_c4VSpacing        = iwdgSrc.m_c4VSpacing;
        m_eInitMediaType    = iwdgSrc.m_eInitMediaType;
        m_eSortOrder        = iwdgSrc.m_eSortOrder;
        m_strInitCat        = iwdgSrc.m_strInitCat;
        m_strLayoutTmpl     = iwdgSrc.m_strLayoutTmpl;
        m_strRepoMoniker    = iwdgSrc.m_strRepoMoniker;
        m_tmAddedFmt        = iwdgSrc.m_tmAddedFmt;

        //
        //  These are meaninless until this guy is put into use, and clear
        //  the stack until we start getting new stuff set up.
        //
        m_areaDisplay.ZeroAll();
        m_areaFocusInd.ZeroAll();
        m_bHavePressedInd   = kCIDLib::False;
        m_bNewLayout        = kCIDLib::False;
        m_bLargeArt         = kCIDLib::False;
        m_bSafePanBgn       = kCIDLib::False;
        m_c4FocusIndex      = 0;
        m_c4SelIndex        = kCIDLib::c4MaxCard;
        m_c4UpdateCnt       = 0;
        m_colStack.RemoveAll();
        m_szArt.Zero();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfCvrArtBrowser::bCanSizeTo() const
{
    // Our contents is driven by size, not the other way around
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfCvrArtBrowser& iwdgOther
    (
        static_cast<const TCQCIntfCvrArtBrowser&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCCmdSrcIntf::bSameCmds(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }

    if ((m_c4HSpacing != iwdgOther.m_c4HSpacing)
    ||  (m_c4VSpacing != iwdgOther.m_c4VSpacing)
    ||  (m_eInitMediaType != iwdgOther.m_eInitMediaType)
    ||  (m_eSortOrder != iwdgOther.m_eSortOrder)
    ||  (m_strRepoMoniker != iwdgOther.m_strRepoMoniker)
    ||  (m_strLayoutTmpl != iwdgOther.m_strLayoutTmpl))
    {
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  If we are already at the boundary in the direction indicated, then we
//  return tru to allow the focus to move off us. Else, we just move it to
//  the next slot in that direction and redraw the old and new ones.
//
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bMoveFocus(const tCQCKit::EScrNavOps eOp)
{
    // Shouldn't happen, but just in case. Just return to move off us
    if (m_colStack.bIsEmpty())
        return kCIDLib::True;

    // Get the current top of stack
    TItemList& ilTop = *m_colStack.pobjPeek();

    //
    //  We have some special cases. Some we just translate into scrolling
    //  operations. The focus will be moved internally if necessary to keep
    //  it on a visible item. The others we move the focus but never scroll.
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
        // Call push/pop with a null item pointer to pop the top if doable
        PushPopStack(nullptr);

        // Indicate we kept the focus internal
        bRet = kCIDLib::False;
    }
     else
    {
        //
        //  Figure out what row/col our current focus index is on, and what
        //  the range of mostly visible columns are visible. This is the info
        //  we need to decide what to do.
        //
        tCIDLib::TCard4 c4Row;
        tCIDLib::TCard4 c4Col;
        IndexToRowCol(m_c4FocusIndex, c4Row, c4Col);

        //
        //  Get the first and last columns with visible focus indicators
        //
        tCIDLib::TCard4 c4FirstCol;
        tCIDLib::TCard4 c4LastCol;
        ilTop.QueryVisColumns
        (
            m_iwdgLayout.areaActual()
            , m_c4HSpacing
            , c4FirstCol
            , c4LastCol
            , m_areaFocusInd
        );

        // These we move the focus around with the visible items
        tCIDLib::TCard4 c4OrgFocus = m_c4FocusIndex;
        switch(eOp)
        {
            case tCQCKit::EScrNavOps::Left :
                // If not on the left-most column, then move left
                bRet = (c4Col <= c4FirstCol);
                if (!bRet)
                    c4Col--;
                break;

            case tCQCKit::EScrNavOps::Right :
                //
                //  If we are are not on the last column, and moving over
                //  another column won't put us beyond the end of the list,
                //  we can move right and stay internal.
                //
                if ((c4Col < c4LastCol)
                &&  ((m_c4FocusIndex + m_c4RowCount) < ilTop.m_colItems.c4ElemCount()))
                {
                    c4Col++;
                    bRet = kCIDLib::False;
                }
                break;

            case tCQCKit::EScrNavOps::Up :
                // If not on the top-most row, then move up a row
                bRet = (c4Row == 0);
                if (!bRet)
                    c4Row--;
                break;

            case tCQCKit::EScrNavOps::Down :
                //
                //  If we are are not on the last row, and moving down a row
                //  wouldn't put us beyond the end of the list, move down.
                //
                if ((c4Row + 1 < m_c4RowCount)
                &&  (m_c4FocusIndex + 1 < ilTop.m_colItems.c4ElemCount()))
                {
                    c4Row++;
                    bRet = kCIDLib::False;
                }
                break;

            default :
                break;
        };

        // Set the new focus index for our movement, if any
        m_c4FocusIndex = (c4Col * m_c4RowCount) + c4Row;

        //
        //  If moving internally and the focus slot changed, then update
        //  the old and new focus areas.
        //
        if (!bRet && (m_c4FocusIndex != c4OrgFocus))
        {
            const TArea& areaAdj = ilTop.areaAdjusted(m_areaDisplay);

            //
            //  Get the old and new areas. They are always contiguous
            //  so combine them and do a single update.
            //
            TArea areaOrg;
            AreaForIndex(areaAdj, c4OrgFocus, areaOrg);
            TArea areaUpdate;
            AreaForIndex(areaAdj, m_c4FocusIndex, areaUpdate);

            areaUpdate |= areaOrg;
            Redraw(areaUpdate);
        }
    }
    return bRet;
}


//
//  We allow horizontal drags and vertical flicks. The flicks move us
//  between title and collection list levels, and horizontal drags move us
//  within the lists.
//
//  We let two fingers propogate, else we are going to process it, even if
//  we ignore it due to having an empty stack.
//
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bPerGestOpts(const   TPoint&
                                    , const tCIDLib::EDirs      eDir
                                    , const tCIDLib::TBoolean   bTwoFingers
                                    ,       tCIDCtrls::EGestOpts& eToSet
                                    ,       tCIDLib::TFloat4&   f4VScale)
{
    // If two fingers, propogate
    if (bTwoFingers)
        return kCIDLib::True;

    // If empty, don't propogate but don't anything
    if (m_colStack.bIsEmpty())
        return kCIDLib::False;

    if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
    {
        // See if we have enough on the stop of stack list to scroll
        TItemList& ilTop = *m_colStack.pobjPeek();

        if (ilTop.m_areaContent.c4Width() > m_areaDisplay.c4Width())
        {
            // If no inertial or remote mode, only allow flicks, else pan
            if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;
        }
    }
     else
    {
        eToSet = tCIDCtrls::EGestOpts::Flick;
    }

    f4VScale = 1.0F;

    // No propogation, not that it matters since we set the options
    return kCIDLib::False;
}


//
//  Handle horizontal drags.
//
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bProcessGestEv(  const   tCIDCtrls::EGestEvs eEv
                                        , const TPoint&             pntStart
                                        , const TPoint&             pntAt
                                        , const TPoint&             pntDelta
                                        , const tCIDLib::TBoolean   bTwoFinger)
{
    // If the stack is empty, do nothing
    if (m_colStack.bIsEmpty())
        return kCIDLib::False;

    // Get the top of stack list
    TItemList& ilTop = *m_colStack.pobjPeek();

    const TArea& areaUs = areaActual();
    tCIDLib::TBoolean bContinue = kCIDLib::True;
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        //
        //  Get a copy of the of the top list's content area as the
        //  starting point for our pan content.
        //
        const TArea& areaContent = ilTop.m_areaContent;
        m_areaPanCont = ilTop.m_areaContent;
        const TArea& areaLayout = m_iwdgLayout.areaActual();
        const tCIDLib::TCard4 c4ColWidth(areaLayout.c4Width() + m_c4HSpacing);

        // Now we set up the pan area.
        {
            TArea areaScreen = TGUIFacility::areaVirtualScreen();

            // Our basic 'span' unit will be the width of the virtual screen
            const tCIDLib::TInt4 i4Span = tCIDLib::TInt4(areaScreen.c4Width());

            //
            //  Set the size to 5 spans, as a worst case scenario. It's unlikely
            //  they'd ever scroll that far.
            //
            m_areaPanCont.c4Width(i4Span * 5);

            //
            //  Adjust it so that the middel of the pan content area is layed over
            //  the current adjusted display area.
            //
            m_areaPanCont.i4X(ilTop.m_areaAdjDisp.i4X() - (i4Span * 2));

            // And then clip to the content area
            if (m_areaPanCont.i4X() < areaContent.i4X())
                m_areaPanCont.i4X(areaContent.i4X());
            if (m_areaPanCont.i4Right() > areaContent.i4Right())
                m_areaPanCont.i4Right(areaContent.i4Right());
        }

        //
        //  Now set up the actual bit of that pan content that we will initially
        //  draw. We will do just enough to handle the worst case actual move
        //  of the finger by the user, which will be three times our display width,
        //  the current page and one before and after. This is as far as they could
        //  possibly move us in the actual drag, before inertia.
        //
        //  Note it's not the display area, since dragging will continue to the
        //  edge of the widget itself.
        //
        const tCIDLib::TCard4 c4InitialWidth(areaUs.c4Width() * 3);

        //
        //  Set up an area of that size, such that the middle page of the three
        //  widths is layed over the current display content. This sill be the
        //  initial 'drawn so far' area, by starting with the display area and
        //  moving it left one display width.
        //
        m_areaPanSoFar = ilTop.m_areaAdjDisp;
        m_areaPanSoFar.c4Width(c4InitialWidth);
        m_areaPanSoFar.AdjustOrg(-tCIDLib::TInt4(m_areaDisplay.c4Width()), 0);

        // Clip to the pan content area if needed
        if (m_areaPanSoFar.i4X() < m_areaPanCont.i4X())
            m_areaPanSoFar.i4X(m_areaPanCont.i4X());
        if (m_areaPanSoFar.i4Right() > m_areaPanCont.i4Right())
            m_areaPanSoFar.i4Right(m_areaPanCont.i4Right());

        //
        //  Figure out the first and last items covered by this. Bump the column
        //  on the last index calculation, so that we are sure to get the last,
        //  partial, column.
        //
        const tCIDLib::TCard4 c4FirstInd
        (
            (m_areaPanSoFar.i4X() / c4ColWidth) * m_c4RowCount
        );

        const tCIDLib::TCard4 c4LastInd
        (
            ((m_areaPanSoFar.i4Right() / c4ColWidth) + 1) * m_c4RowCount
        );

        // Make sure we have all of the images for this range
        const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();
        LoadListArt(gdevCompat, ilTop, c4FirstInd, c4LastInd);

        // Create the bitmap for the pannable content
        m_bmpCont = TBitmap
        (
            m_areaPanCont.szArea(), tCIDImage::EPixFmts::TrueAlpha, tCIDImage::EBitDepths::Eight
        );

        //
        //  Get a version of the initial 'so far' area adjusted relative to the
        //  pan content, so we can do some drawing into it below. The bitmap is
        //  the zero origined pan content area.
        //
        TArea areaInitial(m_areaPanSoFar);
        areaInitial -= m_areaPanCont.pntOrg();

        TGraphMemDev gdevCont(gdevCompat, m_bmpCont);
        if (m_bSafePanBgn)
        {
            //
            //  We have a safe bgn so we can put our background into the contents
            //  itself. At this point we only need to fill in the initial area.
            //  to insure that gradients line up, we need to take an area of our
            //  own height and center it vertically over the initial area, then
            //  fill that.
            //
            TArea areaFill(areaInitial);
            areaFill.c4Height(areaUs.c4Height());
            areaFill.CenterIn(areaInitial);
            DoBgnFill(gdevCont, areaFill);
        }
         else
        {
            // We have to get the underlying content
            m_bmpBgn = bmpGatherBgn();

            //
            //  We need one for double buffering during the pan in this
            //  case, forced to memory. It only needs to be as big as us.
            //
            m_bmpDBuf = TBitmap
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::False
            );
        }

        //
        //  Draw the items now that intersect the initial area. We need to create
        //  a correctly adjusted content area to act as the relative to, so that
        //  the adjusted initial area falls at the right spot within the bitmap.
        //  So we take the content area, zero orignin it, and shift it down by the
        //  left origin of the pan content area.
        //
        TArea areaRelTo(areaContent);
        areaRelTo.ZeroOrg();
        areaRelTo -= m_areaPanCont.pntOrg();

        DrawList
        (
            gdevCont
            , areaRelTo
            , areaInitial
            , TPoint::pntOrigin
            , ilTop
            , !m_bSafePanBgn
        );
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        // Get a ref to the list top's content area
        const TArea& areaContent = ilTop.m_areaContent;

        // Remember the current scroll pos
        const tCIDLib::TInt4 i4OrgPos = ilTop.m_areaAdjDisp.i4X();

        // Apply the delta to the list pos and clip back to the pan cont
        ilTop.m_areaAdjDisp.AdjustOrg(-pntDelta.i4X(), 0);
        if (ilTop.m_areaAdjDisp.i4X() < m_areaPanCont.i4X())
            ilTop.m_areaAdjDisp.i4X(m_areaPanCont.i4X());
        if (ilTop.m_areaAdjDisp.i4Right() > m_areaPanCont.i4Right())
            ilTop.m_areaAdjDisp.RightJustifyIn(m_areaPanCont);

        //
        //  If we actually moved, then do the movement. If not, and we
        //  are getting inertia at this point, cancel any more inertia.
        //
        if (ilTop.m_areaAdjDisp.i4X() != i4OrgPos)
        {
            tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

            // Calc the actual delta for use below
            tCIDLib::TInt4 i4NewDelta
            (
                ilTop.m_areaAdjDisp.i4X() - i4OrgPos
            );

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
            TArea areaBlitSrc = ilTop.m_areaAdjDisp;
            areaBlitSrc -= m_areaPanCont.pntOrg();

            if (m_bSafePanBgn)
            {
                //
                //  In this case we can just blit the contents directly since
                //  background and content are together. And, we can just
                //  scroll the actual window contents and only blit the part
                //  that has become uncovered, which is far more efficient.
                //
                TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpCont);

                // Scroll the display area over
                TArea areaTmpTar;
                cptrDev->ScrollBits
                (
                    m_areaDisplay
                    , m_areaDisplay
                    , i4NewDelta * -1
                    , 0
                    , areaTmpTar
                    , kCIDLib::True
                );

                //
                //  Create a source area from the target update area the scroll gave
                //  us. It needs to be either at the left or right edge of the src
                //  blit area. Vertcally center ift as well, since the value we get
                //  from the scroll is relative to the window, and this is zero
                //  based.
                //
                TArea areaTmpSrc(areaTmpTar);
                if (i4NewDelta < 0)
                    areaTmpSrc.LeftJustifyIn(areaBlitSrc, kCIDLib::True);
                else
                    areaTmpSrc.RightJustifyIn(areaBlitSrc, kCIDLib::True);

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
                //  based version of our display area. We need this to blit
                //  from the background to our double buffer, and from that
                //  to our display area.
                //
                TArea areaZDisp = m_areaDisplay;
                areaZDisp.ZeroOrg();

                // Copy in the background content to the double buffer
                TGraphMemDev gdevDBuf(civOwner().gdevCompat(), m_bmpDBuf);
                {
                    TGraphMemDev gdevBgn(civOwner().gdevCompat(), m_bmpBgn);
                    gdevDBuf.CopyBits(gdevBgn, areaZDisp, areaZDisp);
                }

                // And alpha blit the content over that
                {
                    TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
                    gdevDBuf.AlphaBlit(gdevCont, areaBlitSrc, areaZDisp, 0xFF, kCIDLib::True);
                }

                // And finally copy the result to the target
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

            // Make sure the focus stays visible
            KeepFocusVisible();

            // Do a final draw in the end position
            Redraw();

            // Send a scroll event
            PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
        }
    }

    return bContinue;
}


// Check some command parameters when we are being edited
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bValidateParm(const  TCQCCmd&        cmdSrc
                                    , const TCQCCmdCfg&     ccfgTar
                                    , const tCIDLib::TCard4 c4Index
                                    , const TString&        strValue
                                    ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCMedia::strCmdId_RandomTitleSearch) && (c4Index == 2))
    {
        tCIDLib::TCard4 c4Val = 0;
        if (!strValue.bToCard4(c4Val) || (c4Val < 4) || (c4Val > 128))
        {
            strErrText = facCQCIntfEng().strMsg(kIEngMsgs::midStatus_BadRandSearchCnt);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Called if we are clicked on We see if one of our items was hit. If so
//  we invoke it.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::Clicked(const TPoint& pntAt)
{
    // If the list stack is empty, do nothing
    if (m_colStack.bIsEmpty())
        return;

    tCIDLib::TCard4 c4Row, c4Col;
    const tCIDLib::TCard4 c4HitInd = c4HitTest(pntAt, c4Row, c4Col);

    // If nothing, return
    if (c4HitInd == kCIDLib::c4MaxCard)
        return;

    TArea areaUpdate;
    TItemList& ilTop = *m_colStack.pobjPeek();
    TListItem& liHit = *ilTop.m_colItems[c4HitInd];

    //
    //  Move the focus index to here if it changed. If we have the
    //  focus, then we have to force a redraw of the old item. We'll be
    //  redrawing the new item below anyway when we do the press/release.
    //
    if (m_c4FocusIndex != c4HitInd)
    {
        AreaForIndex(ilTop.areaAdjusted(m_areaDisplay), m_c4FocusIndex, areaUpdate);
        m_c4FocusIndex = c4HitInd;
        if (bHasFocus())
            Redraw(areaUpdate);
    }

    // Do the faux press/release feedback
    AreaForIndex(ilTop.areaAdjusted(m_areaDisplay), c4HitInd, areaUpdate);

    liHit.m_bPressed = kCIDLib::True;
    Redraw(areaUpdate);
    TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs);
    liHit.m_bPressed = kCIDLib::False;
    Redraw(areaUpdate);

    //
    //  We respond differently according to whether we are in title mode
    //  or collection mode. If in title mode, and there is more than one
    //  collection under the current title set, then we swap to collection
    //  mode and show the collections inside this title. If there is just
    //  one collection, we just act like they selected that one collection.
    //
    if (!ilTop.bIsTopLevel() || (liHit.m_pmtsThis->c4ColCount() == 1))
    {
        //
        //  Set the select index to the invoke index.
        //
        //  NOTE that we only set the selection index long enough for it
        //  to be seen by the RTV object creation callback, then we put
        //  it back to it's default c4MaxCard value. So we use a janitor
        //  to make sure it gets set back to that. And we may also force
        //  back to c4MaxCard before we exit, based on what happens below.
        //
        TCardJanitor janTrack(&m_c4SelIndex, c4HitInd);

        PostOps(*this, kCQCMedia::strEvId_OnSelectCol, tCQCKit::EActOrders::NonNested);

        //
        //  Don't do anything else unless you check that we weren't
        //  killed by this action.
        //
    }
     else
    {
        //
        //  We are in title or artist mode, and the title clicked has
        //  more than one collection, so switch to collection mode.
        //
        try
        {
            PushColList(m_c4SelIndex, liHit);
        }

        catch(const TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            civOwner().ShowErr
            (
                civOwner().strTitle()
                , L"An error occured setting collection mode"
                , errToCatch
            );
        }
    }
}



tCIDLib::TVoid
TCQCIntfCvrArtBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfCvrArtBrowser&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfCvrArtBrowser::eDoCmd(  const   TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Back)
    {
        // Just call the push/pop method with a null item pointer to pop
        PushPopStack(nullptr);
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetIsTitleLevel)
    {
        //
        //  We just return a true or false to indicate if we are at top level
        //  or not.
        //
        if (m_colStack.bIsEmpty())
            bResult = kCIDLib::True;
        else
            bResult = m_colStack.pobjPeek()->bIsTopLevel();
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_GetMediaType)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varMType = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        // If we have a page displayed, get it from that, else use default
        tCIDLib::TBoolean bChanged = kCIDLib::False;
        tCQCMedia::EMediaTypes eMType;

        if (m_colStack.bIsEmpty())
            eMType = m_eInitMediaType;
         else
            eMType = m_colStack.pobjPeek()->m_eMediaType;

        switch(eMType)
        {
            case tCQCMedia::EMediaTypes::Movie :
                bChanged = varMType.bSetValue(L"Movie");
                break;

            case tCQCMedia::EMediaTypes::Music :
                bChanged = varMType.bSetValue(L"Music");
                break;

            case tCQCMedia::EMediaTypes::Pic :
                bChanged = varMType.bSetValue(L"Picture");
                break;

            default :
                CIDAssert(kCIDLib::False, L"Unknown CAB media type");
                break;
        }

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMType.strName(), varMType.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetShowText)
    {
        //
        //  This is deprecated. It's still here so it won't cause an error
        //  but it's not exposed anymore.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_GetSortOrder)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varOrder = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        // If we have a page displayed, get it from that, else use default
        tCIDLib::TBoolean bChanged = kCIDLib::False;
        if (m_colStack.bIsEmpty())
        {
            bChanged = varOrder.bSetValue(L"By Title");
        }
         else
        {
            //
            //  Load the text for the sort order, which is supposed to be
            //  correct for the values we use in the CAB sorting commands.
            //
            bChanged = varOrder.bSetValue
            (
                tCQCMedia::strXlatESortOrders(m_colStack.pobjPeek()->m_eSortOrder)
            );
        }

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varOrder.strName(), varOrder.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_RandomTitleSearch)
    {
        const tCQCMedia::ESortOrders eOrder = eXlatSortOrder(ccfgToDo, 0);
        const tCQCMedia::EMediaTypes eMType = eXlatMediaType(ccfgToDo, 1);
        bResult = bPushRandomTitleSearch(eOrder, eMType, ccfgToDo.piAt(2).c4Val());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollList)
    {
        // Call a helper since we have to do this from more than one place
        const TString& strScrollOp = ccfgToDo.piAt(0).m_strValue;
        ScrollList(strScrollOp);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollToChar)
    {
        const TString& strVal = ccfgToDo.piAt(0).m_strValue;

        //
        //  Get the top of stack and ask the list to give us the first
        //  column to move to for this character.
        //
        if (!m_colStack.bIsEmpty() && !strVal.bIsEmpty())
        {
            TItemList& ilTop = *m_colStack.pobjPeek();
            tCIDLib::TCard4 c4ColNum = ilTop.c4FirstCharColumn
            (
                strVal[0], m_c4RowCount
            );

            // If we got a hit, move to there
            if (c4ColNum != kCIDLib::c4MaxCard)
            {
                // Remember the current adjusted display
                const TArea areaOrg = ilTop.m_areaAdjDisp;

                const tCIDLib::TBoolean bMoved = ilTop.bScrollToCol
                (
                    c4ColNum
                    , m_iwdgLayout.areaActual().c4Width()
                    , m_c4HSpacing
                    , m_c4RowCount
                );

                if (bMoved)
                {
                    //
                    //  We have to make sure the art for the newly visible
                    //  items is loaded.
                    //
                    tCIDLib::TCard4 c4FirstInd;
                    tCIDLib::TCard4 c4LastInd;
                    ilTop.QueryVisIndices
                    (
                        m_iwdgLayout.areaActual()
                        , m_c4HSpacing
                        , m_c4RowCount
                        , c4FirstInd
                        , c4LastInd
                        , TArea::areaEmpty
                    );

                    LoadListArt
                    (
                        civOwner().gdevCompat(), ilTop, c4FirstInd, c4LastInd
                    );

                    // Draw if allowed and send a scroll event
                    if (bCanRedraw(strEventId))
                    {
                        // If in remote mode, just redraw, else slide
                        if (TFacCQCIntfEng::bRemoteMode())
                        {
                            KeepFocusVisible();
                            Redraw();
                        }
                         else
                        {
                            SlideList(ilTop, areaOrg);
                        }

                        PostOps
                        (
                            *this
                            , kCQCKit::strEvId_OnScroll
                            , tCQCKit::EActOrders::Nested
                        );
                    }
                }
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SearchByActor)
    {
        const tCQCMedia::ESortOrders eOrder = eXlatSortOrder(ccfgToDo, 2);
        bResult = bPushActorSearch
        (
            ccfgToDo.piAt(0).m_strValue
            , facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue)
            , eOrder
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SearchByArtist)
    {
        const tCQCMedia::ESortOrders eOrder = eXlatSortOrder(ccfgToDo, 2);
        const tCQCMedia::EMediaTypes eMType = eXlatMediaType(ccfgToDo, 3);
        bResult = bPushArtistSearch
        (
            ccfgToDo.piAt(0).m_strValue
            , facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue)
            , eOrder
            , eMType
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SearchByAudioFmt)
    {
        const tCQCMedia::ECompTypes eCompType = eXlatCompType(ccfgToDo, 0);
        const tCIDLib::TCard4 c4BitDepth = ccfgToDo.piAt(1).c4Val();
        const tCIDLib::TCard4 c4SampleRate = ccfgToDo.piAt(2).c4Val();
        const tCQCMedia::ESortOrders eOrder = eXlatSortOrder(ccfgToDo, 3);
        bResult = bPushAudioFmtSearch
        (
            eCompType, c4BitDepth, c4SampleRate, eOrder
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SearchByTitle)
    {
        const tCQCMedia::ESortOrders eOrder = eXlatSortOrder(ccfgToDo, 2);
        const tCQCMedia::EMediaTypes eMType = eXlatMediaType(ccfgToDo, 3);
        bResult = bPushTitleSearch
        (
            ccfgToDo.piAt(0).m_strValue
            , facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue)
            , eOrder
            , eMType
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetCapPattern)
    {
        // This is deprecated, so it does nothing
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetCategory)
    {
        //
        //  If in the preload, just store the incoming and use it later
        //  in the initial load. Else, try to load the passed category. We
        //  use the same sort order that was last set, if any.
        //
        if (bCanRedraw(strEventId))
            PushCatList(ccfgToDo.piAt(0).m_strValue);
        else
            m_strInitCat = ccfgToDo.piAt(0).m_strValue;
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetMediaType)
    {
        //
        //  If this we can't redraw, then just store the initial type.
        //  Else, load the default category with the new media type.
        //
        m_eInitMediaType = eXlatMediaType(ccfgToDo, 0);
        if (bCanRedraw(strEventId))
            LoadDefCategory();
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        // If the new is different from what we have, then deal with it
        tCQCMedia::EMediaTypes eNewMType = eXlatMediaType(ccfgToDo, 1);
        if ((m_strRepoMoniker != ccfgToDo.piAt(0).m_strValue)
        ||  (eNewMType != m_eInitMediaType))
        {
            // See if we have data for this repo
            TFacCQCMedia::TMDBPtr cptrNew;
            TString strDBSerialNum;
            facCQCMedia().eGetMediaDB
            (
                ccfgToDo.piAt(0).m_strValue, strDBSerialNum, cptrNew
            );

            // If we got one, then load the default category
            if (cptrNew.pobjData())
            {
                m_cptrMediaDB    = cptrNew;
                m_eInitMediaType = eNewMType;
                m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;

                // Delete all of our current stack contents
                m_colStack.RemoveAll();

                // If we can redraw, then load up the default category
                if (bCanRedraw(strEventId))
                    LoadDefCategory();
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetSortOrder)
    {
        //
        //  Store the new sort order away as the new default. And, if we
        //  can redraw in this event, and the stack is not empty, and the
        //  top of stack is a top level list, then resort it.
        //
        m_eSortOrder = eXlatSortOrder(ccfgToDo, 0);
        if (bCanRedraw(strEventId)
        &&  !m_colStack.bIsEmpty()
        &&   m_colStack.pobjPeek()->bIsTopLevel())
        {
            m_colStack.pobjPeek()->bResort(m_eSortOrder);
            Redraw();
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetLayout)
    {
        // Store the new layout template away and set the new layout flag
        piwdgParent()->ExpandTmplPath(ccfgToDo.piAt(0).m_strValue, m_strLayoutTmpl);
        m_bNewLayout = kCIDLib::True;
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ShowText)
    {
        //
        //  This is deprecated now. It's still here to avoid errors but
        //  it doesn't do anything and isn't exposed anymore.
        //
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
//  When inertial movement starts, this is called between each round. It gives us
//  a chance to update our data to get ready for new stuff that has to be displayed.
//  We have a fixed time, so we do as much as we can within that time, until we get
//  far enough ahead.
//
//  When the drag starts, a bitmap three times our width is created, to handle the
//  actual drag part in either direction. But now we can use it however want. So
//  we move the current contents to the edge and draw more contents in. We try to
//  keep the three page's worth full.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::GestInertiaIdle( const   tCIDLib::TEncodedTime   enctEnd
                                        , const tCIDLib::TCard4         c4Millis
                                        , const tCIDLib::EDirs          eDir)
{
    // Get the top of stack list
    TItemList& ilTop = *m_colStack.pobjPeek();

    //
    //  Get the full content area we can possibly move within, and the current
    //  adjusted display area for this list. But we need the display area adjusted
    //  to be relative to the content area
    //
    const TArea& areaUs = areaActual();
    const TArea& areaContent = ilTop.m_areaContent;
    TArea& areaAdjDisplay = ilTop.m_areaAdjDisp;
    const TArea& areaLayout = m_iwdgLayout.areaActual();

    //
    //  If we have have already drawn all the way to the edge in the direction of
    //  inertia, then we are done. Our pan content and so far areas are relative
    //  to the zero origin of the bitmap, so going left if we hit zero we are done.
    //
    //  Else, see how close the display area is to how far we've drawn. If it's
    //  within a certain distance, we will draw out another one. Don't allow a
    //  very short layout to make us never draw ahead any.
    //
    //  We try to stay at least four columns ahead of the current page.
    //
    const tCIDLib::TCard4 c4ColWidth(areaLayout.c4Width() + m_c4HSpacing);
    tCIDLib::TInt4  i4MoveBy;
    tCIDLib::TInt4  i4Span;
    if (areaActual().c4Width() < c4ColWidth * 4)
        i4Span = c4ColWidth * 4;
    else
        i4Span = areaUs.c4Width();

    //
    //  Try to limit how many do on any given round, but don't do fewer than we
    //  could. This will limit us to about 4 per round, unless they have a very
    //  tall one that has more than four rows, which is not very likely.
    //
    if (m_c4RowCount == 1)
        i4MoveBy = c4ColWidth * 4;
    else if (m_c4RowCount == 2)
        i4MoveBy = c4ColWidth * 2;
    else
        i4MoveBy = c4ColWidth;

    TArea   areaUpdate;
    TArea   areaNewSoFar(m_areaPanSoFar);
    if (eDir == tCIDLib::EDirs::Left)
    {
        if (areaNewSoFar.i4Right() >= m_areaPanCont.i4Right())
            return;

        // As long as we have more than our span width, don't do anything
        if (areaNewSoFar.i4Right() - areaAdjDisplay.i4Right() > i4Span)
            return;

        // Size up to the next even column boundary
        tCIDLib::TInt4 i4NewX = areaNewSoFar.i4X();
        i4NewX += i4MoveBy;
        i4NewX -= (i4NewX % i4MoveBy);
        areaNewSoFar.i4X(i4NewX);

        // If it goes too far. clip it back
        if (areaNewSoFar.i4Right() > m_areaPanCont.i4Right())
            areaNewSoFar.i4Right(m_areaPanCont.i4Right());

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4X(m_areaPanSoFar.i4Right(), kCIDLib::True);
    }
     else if (eDir == tCIDLib::EDirs::Right)
    {
        if (!areaNewSoFar.i4X())
            return;

        if (areaAdjDisplay.i4X() - areaNewSoFar.i4X() > i4Span)
            return;

        tCIDLib::TInt4 i4NewX = areaNewSoFar.i4X();
        i4NewX -= i4MoveBy;
        i4NewX -= (i4NewX % i4MoveBy);
        areaNewSoFar.i4X(i4NewX, kCIDLib::True);

        // If it goes below the origin, clip it back
        if (areaNewSoFar.i4Left() < 0)
            areaNewSoFar.i4Left(0, kCIDLib::True);

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4Right(m_areaPanSoFar.i4X() - 1);
    }

    // If by some chance, it ended up empty, then do nothing
    if (areaUpdate.bIsEmpty())
        return;

    //
    //  Make sure that the art images for the stuff we are going to redraw.
    //  We do any that intersect the update area.
    //
    const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();
    {
        const tCIDLib::TCard4 c4FirstInd
        (
            (areaUpdate.i4X() / c4ColWidth)* m_c4RowCount
        );
        const tCIDLib::TCard4 c4LastInd
        (
            ((areaUpdate.i4Right() / c4ColWidth) + 1)
            * m_c4RowCount
        );

        LoadListArt(gdevCompat, ilTop, c4FirstInd, c4LastInd);
    }

    //
    //  If we are about out of time, then return now without drawing anything and
    //  without updating the so far area. This will cause us to try again next
    //  time. The art will already be all there, so we'll get to the drawing
    //  for sure.
    //
    if (TTime::enctNow() + (kCIDLib::enctOneMilliSec * 2) > enctEnd)
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

    TGraphMemDev gdevCont(gdevCompat, m_bmpCont);

    //
    //  Reset the remaining bit before we draw into it. For the safe bgn scenario
    //  we need to do a background fill (making sure gradients align.)
    //
    if (m_bSafePanBgn)
    {
        TArea areaFill(areaUpdate);
        areaFill.c4Height(areaUs.c4Height());
        areaFill.CenterIn(areaUpdate);
        DoBgnFill(gdevCont, areaFill);
    }

    //
    //  We have to clip the output to the actual update area. Otherwise, we could
    //  allow content to be drawn over twice. If that content was drawn with alpha,
    //  it would just add up.
    //
    TRegionJanitor janClip(&gdevCont, areaUpdate, tCIDGraphDev::EClipModes::And);

    DrawList
    (
        gdevCont
        , areaRelTo
        , areaUpdate
        , TPoint::pntOrigin
        , ilTop
        , !m_bSafePanBgn
    );
}


//
//  We just call our parent, and then calculate where our slots are going to
//  go, based on the loaded width/spacing values.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    //
    //  We have to download our template that acts as our layout guide and
    //  process it to remove anything that isn't legal. If we can't find one
    //  this guy will create an empty one.
    //
    LoadTemplate(dsclInit, colErrs);

    //
    //  Do initial size calculations AFTER loading the layout. Pass a bogus old
    //  area so that it'll do something.
    //
    DoSizeCalcs(areaActual(), TArea::areaEmpty);

    //
    //  Remember if we have a pan safe bgn (for our most common horizontal
    //  drag direction.) The code that handles the list push/pop and does
    //  verticals will handle its own check.
    //
    m_bSafePanBgn = bScrollSafeBgn(kCIDLib::False);

    // Reset the update counter, which is used by the active update callback
    m_c4UpdateCnt = 0;
}


//
//  Called in the focus oriented scenarios, when we get an invoke command
//  for the item with the focus. We just act like it was clicked.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::Invoke()
{
    if (bHasFocus() && !m_colStack.bIsEmpty())
    {
        TItemList& ilTop = *m_colStack.pobjPeek();

        // Be careful that we've not whacked the focus index somehow
        if (m_c4FocusIndex >= ilTop.m_colItems.c4ElemCount())
            return;

        TListItem& liInvoke = *ilTop.m_colItems[m_c4FocusIndex];
        if (!ilTop.bIsTopLevel() || (liInvoke.m_pmtsThis->c4ColCount() == 1))
        {
            //
            //  We are in collection mode, or we are on a title that has just
            //  one collection. So we need to send a selection event. We have
            //  to temporarily force the focus index into the selection
            //  index long enough to get the ops posted, since the selection
            //  index is what indicates which item was clicked.
            //
            TCardJanitor janTrack(&m_c4SelIndex, m_c4FocusIndex);

            //
            //  We post a selection collection event no matter what, since
            //  if they clicked a single collection set, that's the same as
            //  clicking the first (only) collection in the set.
            //
            PostOps(*this, kCQCMedia::strEvId_OnSelectCol, tCQCKit::EActOrders::NonNested);

            //
            //  Don't do anything else unless you check that we weren't
            //  killed by this action.
            //
        }
         else
        {
            //
            //  We are in title or artist mode, and the top level selection
            //  clicked on has more than one collection, so switch to
            //  collection mode.
            //
            try
            {
                PushColList(m_c4FocusIndex, liInvoke);
            }

            catch(const TError& errToCatch)
            {
                if (facCQCIntfEng().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                civOwner().ShowErr
                (
                    civOwner().strTitle()
                    , L"An error occured invoking collection mode"
                    , errToCatch
                );
            }
        }
    }
}


//
//  We return a simple derivative of the RTV source since we have some RTVs
//  of our own.
//
TCQCCmdRTVSrc*
TCQCIntfCvrArtBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    //
    //  Based on our mode, get the collection and title cookie. If we don't
    //  have a selected item, then leave them blank.
    //
    tCIDLib::TBoolean       bTopLevel = kCIDLib::True;
    tCIDLib::TCard4         c4FirstIndex = 0;
    tCIDLib::TCard4         c4ColCnt = 0;
    tCIDLib::TCard4         c4TitleCnt = 0;
    tCQCMedia::EMediaTypes  eMediaType = tCQCMedia::EMediaTypes::Count;
    TString                 strCatName;
    TString                 strCatCookie;
    TString                 strColCookie;
    TString                 strTitleCookie;
    const TMediaCollect*    pmcolSel = 0;
    const TMediaTitleSet*   pmtsSel = 0;

    // At design time the stack will be empty
    if (!m_colStack.bIsEmpty())
    {
        // Get the top of stack
        const TItemList& ilTop = *m_colStack.pobjPeek();

        switch(ilTop.m_eListType)
        {
            case EListTypes::Cat :
                bTopLevel = kCIDLib::True;
                strCatCookie = ilTop.m_strCookie;
                break;

            case EListTypes::Random :
            case EListTypes::TitleSearch :
                bTopLevel = kCIDLib::True;
                break;

            case EListTypes::TitleCols :
                bTopLevel = kCIDLib::False;
                break;

            default :
                break;
        };

        eMediaType = ilTop.m_eMediaType;
        strCatName = ilTop.m_strDescr;

        //
        //  Some stuff is different depending on whether we are reacting
        //  to a selection or not.
        //
        if (m_c4SelIndex == kCIDLib::c4MaxCard)
        {
            // No selection
            if (bTopLevel)
            {
                c4TitleCnt = ilTop.m_colItems.c4ElemCount();
                c4ColCnt = 0;

                // Cookie of the category being viewed
                strCatCookie = ilTop.m_strCookie;
            }
             else
            {
                // Provide the (previously) selected title we are viewing
                pmtsSel = ilTop.m_pmtsCont;

                c4TitleCnt = 1;
                c4ColCnt = ilTop.m_colItems.c4ElemCount();

                // Cookie of the title that's open
                strTitleCookie = ilTop.m_strCookie;
            }
        }
         else
        {
            //
            //  Something was selected, so either a title set or collection
            //  was selected. Note that the selection index is for the
            //  thing that we selected, i.e. that got us here, so it's
            //
            //
            if (bTopLevel)
            {
                pmtsSel = ilTop.m_colItems[m_c4SelIndex]->m_pmtsThis;
                strTitleCookie = ilTop.m_colItems[m_c4SelIndex]->m_strCookie;

                c4TitleCnt = ilTop.m_colItems.c4ElemCount();
                c4ColCnt = pmtsSel->c4ColCount();

                //
                //  If only one collection, then get it it's generally the
                //  case that clicking on a title with a single collection
                //  just invokes the collection.
                //
                if (c4ColCnt == 1)
                {
                    pmcolSel = &pmtsSel->mcolAt(m_cptrMediaDB->mdbData(), 0);
                    strColCookie = strTitleCookie;
                    strColCookie.Append(L",1");
                }
            }
             else
            {
                pmcolSel = ilTop.m_colItems[m_c4SelIndex]->m_pmcolThis;
                strColCookie = ilTop.m_colItems[m_c4SelIndex]->m_strCookie;

                // We have the title info as well
                pmtsSel = ilTop.m_pmtsCont;
                strTitleCookie = ilTop.m_strCookie;

                c4TitleCnt = 1;
                c4ColCnt = ilTop.m_colItems.c4ElemCount();
            }
        }

        // Get the first mostly visible index
        tCIDLib::TCard4 c4DummyLast;
        ilTop.QueryVisIndices
        (
            m_iwdgLayout.areaActual()
            , m_c4HSpacing
            , m_c4RowCount
            , c4FirstIndex
            , c4DummyLast
            , TArea::areaEmpty
        );
    }

    // And gen up the item, passing him the info
    return new TCQCIntfCABRTV
    (
        bTopLevel
        , m_c4SelIndex
        , c4FirstIndex
        , c4TitleCnt
        , c4ColCnt
        , strCatCookie
        , strCatName
        , m_strRepoMoniker
        , eMediaType
        , strColCookie
        , strTitleCookie
        , pmtsSel
        , pmcolSel
        , m_cptrMediaDB
        , cuctxToUse
    );
}


//
//  Called for post init loading. We load a default initial category. If it
//  goes awry, we clean up the stack of anything that might have gotten
//  paritally loaded, and leave us empty.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::PostInit()
{
    //  We only need to do this stuff if not in designer mode.
    if (TFacCQCIntfEng::bDesMode())
        return;

    try
    {
        // Get the media database for our repo if available
        TString strDBSerialNum;
        facCQCMedia().eGetMediaDB
        (
            m_strRepoMoniker, strDBSerialNum, m_cptrMediaDB
        );

        //
        //  Preload the missing art and playlist art images, and scale them to the
        //  size we need. We only do this if there's a cover art widget.
        //
        if (!m_szArt.bAllZero())
        {
            const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();

            TImgInfo* pimgiLoad = pimgiForKey(CQCIntfEng_CvrArtBrowser::strMissingArt);
            if (pimgiLoad && !pimgiLoad->m_strImageName.bIsEmpty())
            {
                // Set up an area for the original image
                const TSize szImg = pimgiLoad->m_cptrImg->szImage();
                TArea areaImg(TPoint::pntOrigin, szImg);

                // Create an AR retaining scaled size
                TSize szNew = szImg;
                szNew.ScaleAR(m_szArt);
                TArea areaNew(TPoint::pntOrigin, szNew);

                m_bmpArtMissing = TBitmap(gdevCompat, szNew);
                {
                    TGraphMemDev gdevScale(gdevCompat, m_bmpArtMissing);
                    gdevScale.DrawBitmap
                    (
                        pimgiLoad->m_cptrImg->bmpImage(), areaImg, areaNew
                    );
                }
            }
             else
            {
                m_bmpArtMissing = TBitmap(facCQCIntfEng(), kCQCIntfEng::ridPNG_Empty);
            }

            pimgiLoad = pimgiForKey(CQCIntfEng_CvrArtBrowser::strPlayListArt);
            if (pimgiLoad && !pimgiLoad->m_strImageName.bIsEmpty())
            {
                // Set up an area for the original image
                const TSize szImg = pimgiLoad->m_cptrImg->szImage();
                TArea areaImg(TPoint::pntOrigin, szImg);

                // Create an AR retaining scaled size
                TSize szNew = szImg;
                szNew.ScaleAR(m_szArt);
                TArea areaNew(TPoint::pntOrigin, szNew);

                m_bmpArtPlayList = TBitmap(gdevCompat, szNew);
                {
                    TGraphMemDev gdevScale(gdevCompat, m_bmpArtPlayList);
                    gdevScale.DrawBitmap
                    (
                        pimgiLoad->m_cptrImg->bmpImage(), areaImg, areaNew
                    );
                }
            }
             else
            {
                m_bmpArtMissing = TBitmap(facCQCIntfEng(), kCQCIntfEng::ridPNG_Empty);
            }
        }

        // If we got one, then load the default category
        if (m_cptrMediaDB.pobjData())
            LoadDefCategory();
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bLogFailures() & !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::ProcessFlick(const   tCIDLib::EDirs  eDir
                                    , const TPoint&         pntStart)
{
    if (eDir == tCIDLib::EDirs::Up)
    {
        // Call push/pop with a null item pointer to pop the top
        PushPopStack(nullptr);
    }
     else if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
    {
        // Just translate into a scroll command
        const tCIDLib::TCh* pszTar = 0;
        if (eDir == tCIDLib::EDirs::Right)
            pszTar = L"Previous Page";
        else if (eDir == tCIDLib::EDirs::Left)
            pszTar = L"Next Page";

        if (pszTar)
            ScrollList(pszTar);
    }
}



// This is called to gather up supported commands for the cmd editing dialog
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Pop the back stack and go to the previous page
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Back, facCQCKit().strMsg(kKitMsgs::midCmd_Back), 0
            )
        );
    }

    // Return whether we are currently displaying title set level info
    {
        TCQCCmd cmdGetIsTitleLevel
        (
            kCQCIntfEng::strCmdId_GetIsTitleLevel
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_GetIsTitleLevel)
            , 0
        );
        cmdGetIsTitleLevel.eType(tCQCKit::ECmdTypes::Conditional);
        colCmds.objAdd(cmdGetIsTitleLevel);
    }

    // Get the current media type
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_GetMediaType
            , facCQCMedia().strMsg(kMedMsgs::midCmd_GetMediaType)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // Get the current sort order
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_GetSortOrder
            , facCQCMedia().strMsg(kMedMsgs::midCmd_GetSortOrder)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // Find random titles
    {
        TCQCCmd cmdRandomSearch
        (
            kCQCMedia::strCmdId_RandomTitleSearch
            , facCQCMedia().strMsg(kMedMsgs::midCmd_RandomTitleSearch)
            , 3
        );

        cmdRandomSearch.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                , facCQCMedia().strSortOpts()
            )
        );

        cmdRandomSearch.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCMedia().strMsg(kMedMsgs::midCmdP_MediaType)
                , facCQCMedia().strMTypeOpts()
            )
        );

        cmdRandomSearch.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_MaxCount)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );

        cmdRandomSearch.eType(tCQCKit::ECmdTypes::Both);
        colCmds.objAdd(cmdRandomSearch);
    }


    // Scroll the list
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
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

        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ScrollToChar
                , facCQCKit().strMsg(kKitMsgs::midCmd_ScrollToChar)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_FirstChar)
            )
        );
    }

    // Various types of searches
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Search for music titles that have particular audio formats
        {
            TCQCCmd cmdSearchByAudFmt
            (
                kCQCMedia::strCmdId_SearchByAudioFmt
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SearchByAudioFmt)
                , 4
            );

            cmdSearchByAudFmt.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_CompType)
                    , facCQCMedia().strCompTypeOpts()
                )
            );

            cmdSearchByAudFmt.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_BitDepth)
                    , tCQCKit::ECmdPTypes::Unsigned
                )
            );

            cmdSearchByAudFmt.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_SampleRate)
                    , tCQCKit::ECmdPTypes::Unsigned
                )
            );

            cmdSearchByAudFmt.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                    , facCQCMedia().strSortOpts()
                )
            );
            cmdSearchByAudFmt.eType(tCQCKit::ECmdTypes::Both);
            colCmds.objAdd(cmdSearchByAudFmt);
        }

        // Do a by actor search of titles
        {
            TCQCCmd cmdSearchByActor
            (
                kCQCMedia::strCmdId_SearchByActor
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SearchByActor)
                , 4
            );

            cmdSearchByActor.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                    , tCQCKit::ECmdPTypes::Text
                )
            );

            cmdSearchByActor.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_IsRegEx)
                    , tCQCKit::ECmdPTypes::Boolean
                )
            );

            cmdSearchByActor.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                    , facCQCMedia().strSortOpts()
                )
            );
            cmdSearchByActor.eType(tCQCKit::ECmdTypes::Both);
            colCmds.objAdd(cmdSearchByActor);
        }

        // Do a by artist search of titles
        {
            TCQCCmd cmdSearchByArtist
            (
                kCQCMedia::strCmdId_SearchByArtist
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SearchByArtist)
                , 4
            );

            cmdSearchByArtist.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                    , tCQCKit::ECmdPTypes::Text
                )
            );

            cmdSearchByArtist.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_IsRegEx)
                    , tCQCKit::ECmdPTypes::Boolean
                )
            );

            cmdSearchByArtist.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                    , facCQCMedia().strSortOpts()
                )
            );

            cmdSearchByArtist.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_MediaType)
                    , facCQCMedia().strMTypeOpts()
                )
            );

            cmdSearchByArtist.eType(tCQCKit::ECmdTypes::Both);
            colCmds.objAdd(cmdSearchByArtist);
        }

        // Search for titles whose names match a pattern
        {
            TCQCCmd cmdSearchByTitle
            (
                kCQCMedia::strCmdId_SearchByTitle
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SearchByTitle)
                , 4
            );

            cmdSearchByTitle.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                    , tCQCKit::ECmdPTypes::Text
                )
            );

            cmdSearchByTitle.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdP_IsRegEx)
                    , tCQCKit::ECmdPTypes::Boolean
                )
            );

            cmdSearchByTitle.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                    , facCQCMedia().strSortOpts()
                )
            );

            cmdSearchByTitle.cmdpAddParm
            (
                TCQCCmdParm
                (
                    facCQCMedia().strMsg(kMedMsgs::midCmdP_MediaType)
                    , facCQCMedia().strMTypeOpts()
                )
            );

            cmdSearchByTitle.eType(tCQCKit::ECmdTypes::Both);
            colCmds.objAdd(cmdSearchByTitle);
        }
    }

    //
    //  These are not allowed in IV events but are allowed in preloads,
    //  though we just store the info for later initial loading, and don't
    //  try to actually update if it's in a preload.
    //
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        // Set a new category to alod
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCMedia::strCmdId_SetCategory
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SetCategory)
                , tCQCKit::ECmdPTypes::Text
                , facCQCMedia().strMsg(kMedMsgs::midCmdP_CategoryCookie)
            )
        );

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

        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCMedia::strCmdId_SetSortOrder
                , facCQCMedia().strMsg(kMedMsgs::midCmd_SetSortOrder)
                , facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleSortOrder)
                , facCQCMedia().strSortOpts()
            )
        );
    }

    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        //
        //  Set a new layout to use. It's always just stored away for later use
        //  the next time a new list is loaded.
        //
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetLayout
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetLayout)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
            )
        );
    }

    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Pop the back stack until we get back to title set level
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_UpLevel
                , facCQCKit().strMsg(kKitMsgs::midCmd_UpLevel)
                , 0
            )
        );
    }
}


//
//  We override these next two to allow for moniker replacement, mostly for
//  template package import/export. We have a repository moniker that we need
//  to account for.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}



tCIDLib::TVoid TCQCIntfCvrArtBrowser::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");

    // Do our mixins. Our actions are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Browser Actions"
    );
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    // And do our attributes
    colAttrs.objPlace
    (
        L"Browser Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // We have to construct these and add, since we have to set two things
    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_Media_Repo
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strRepoMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));

    adatTmp.Set
    (
        L"Layout Template"
        , kCQCIntfEng::strAttr_Brws_Layout
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strLayoutTmpl);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_Template);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));


    TString strLim;
    tCQCMedia::FormatEMediaTypes
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Init Media Type"
        , kCQCIntfEng::strAttr_Media_Type
        , strLim
        , tCIDMData::EAttrTypes::String
    ).SetString(tCQCMedia::strXlatEMediaTypes(m_eInitMediaType));

    tCQCMedia::FormatESortOrders
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    colAttrs.objPlace
    (
        L"Init Sort Order"
        , kCQCIntfEng::strAttr_SortOrder
        , strLim
        , tCIDMData::EAttrTypes::String
    ).SetString(tCQCMedia::strXlatESortOrders(m_eSortOrder));
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfCvrArtBrowser::szDefaultSize() const
{
    return TSize(240, 192);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    // And do our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Brws_Layout)
        m_strLayoutTmpl = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Type)
        m_eInitMediaType = tCQCMedia::eXlatEMediaTypes(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Repo)
        m_strRepoMoniker = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_SortOrder)
        m_eSortOrder = tCQCMedia::eXlatESortOrders(adatNew.strValue());
}


tCIDLib::TVoid TCQCIntfCvrArtBrowser::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_RandomTitleSearch)
    {
        ccfgToSet.piAt(2).m_strValue = L"64";
    }
     else if ((ccfgToSet.strCmdId() == kCQCKit::strCmdId_ScrollList)
          ||  (ccfgToSet.strCmdId() == kCQCKit::strCmdId_ScrollToChar)
          ||  (ccfgToSet.strCmdId() == kCQCKit::strCmdId_ShowText))
    {
        // Nothing to do here
    }
     else if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_SetCategory)
    {
        // Default it to the category cookie source value
        ccfgToSet.SetParmAtAsRTV(0, kCQCMedia::strRTVId_CategoryCookie);
    }
     else
    {
        // Not one of ours, pass long to our parent
        TParent::SetDefParms(ccfgToSet);
    }
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Allows us to pass judgement on a template selected by the user during
//  configuration. Our attributes tab will call this.
//
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::
bIsValidTemplate(const  TCQCIntfTemplate&   iwdgTest
                ,       TString&            strReason) const
{
    const tCIDLib::TCard4 c4Count = iwdgTest.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget& iwdgCur = iwdgTest.iwdgAt(c4Index);

        if (!iwdgCur.bIsA(TCQCIntfStaticBooleanImg::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfStaticImg::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfStaticText::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfMediaRepoText::clsThis()))
        {
            strReason = L"The layout template contains unsupported widget types.";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


// Get or set the art slot horizontal spacing
tCIDLib::TCard4 TCQCIntfCvrArtBrowser::c4HSpacing() const
{
    return m_c4HSpacing;
}

tCIDLib::TCard4 TCQCIntfCvrArtBrowser::c4HSpacing(const tCIDLib::TCard4 c4ToSet)
{
    m_c4HSpacing = c4ToSet;

    // Update our area calcs, pass a bogus old area so it'll do something
    DoSizeCalcs(areaActual(), TArea::areaEmpty);
    return m_c4HSpacing;
}


// Get or set the art slot vertical spacing
tCIDLib::TCard4 TCQCIntfCvrArtBrowser::c4VSpacing() const
{
    return m_c4VSpacing;
}

tCIDLib::TCard4 TCQCIntfCvrArtBrowser::c4VSpacing(const tCIDLib::TCard4 c4ToSet)
{
    m_c4VSpacing = c4ToSet;

    // Update our area calcs, pass a bogus old area so it'll do something
    DoSizeCalcs(areaActual(), TArea::areaEmpty);
    return m_c4VSpacing;
}


// Get/set the initial media type
tCQCMedia::EMediaTypes TCQCIntfCvrArtBrowser::eInitMediaType() const
{
    return m_eInitMediaType;
}

tCQCMedia::EMediaTypes
TCQCIntfCvrArtBrowser::eInitMediaType(const tCQCMedia::EMediaTypes eToSet)
{
    m_eInitMediaType = eToSet;
    return m_eInitMediaType;
}


// Get or set the pattern template to use
const TString& TCQCIntfCvrArtBrowser::strLayoutTmpl() const
{
    return m_strLayoutTmpl;
}

const TString& TCQCIntfCvrArtBrowser::strLayoutTmpl(const TString& strToSet)
{
    // Clear the new layout flag
    m_bNewLayout = kCIDLib::False;

    // Load the template and do our size calcs
    TDataSrvClient dsclLoad;
    tCQCIntfEng::TErrList colErrs;
    LoadTemplate(dsclLoad, colErrs);

    // Update our area calcs, pass a bogus old area so it'll do something
    DoSizeCalcs(areaActual(), TArea::areaEmpty);

    m_strLayoutTmpl = strToSet;
    return m_strLayoutTmpl;
}


// Get or set the moniker of the repository driver we are associated with
const TString& TCQCIntfCvrArtBrowser::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfCvrArtBrowser::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  WE don't actually have to do any active updating. But, what we do is use this
//  to preload art, so that as little of it as possible has to be done during the
//  gesture drag/inertia.
//
//  This won't get called when we are in a gesture, so we don't have to worry about
//  conficting with the ongoing gesture. We only do it every so many times through
//  so as not to be too abusive.
//
//  The item list class has a counter we use to just run up incrementally through
//  the images.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::ActiveUpdate(const   tCIDLib::TBoolean
                                    , const TGUIRegion&         grgnClip
                                    , const tCIDLib::TBoolean   )
{
    m_c4UpdateCnt++;
    if (m_c4UpdateCnt % 3)
        return;

    // If the stack isn't empty, then we have a list we can check
    if (!m_colStack.bIsEmpty())
    {
        // Get the top of stack
        TItemList& ilTop = *m_colStack.pobjPeek();

        // Loop forward till we hit the end or find one not loaded yet
        const tCIDLib::TCard4 c4Count = ilTop.m_colItems.c4ElemCount();
        while (ilTop.m_c4NextArtIndex < c4Count)
        {
            TListItem& liCur = *ilTop.m_colItems[ilTop.m_c4NextArtIndex++];

            //
            //  If already loaded, skip it. This could happen if they do a jump to
            //  first character type thing, which will cause non-contiguous loading
            //  of art within the list.
            //
            if (liCur.m_bImgLoaded)
                continue;

            // Try to load this one and break out
            TString strKey;
            LoadItemArt(civOwner().gdevCompat(), liCur, strKey);
            break;
        }
    }
}


// Re-calc our slot position based on the new area
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    // Call our parent first
    TParent::AreaChanged(areaNew, areaOld);

    // Re-do our size calculations
    DoSizeCalcs(areaNew, areaOld);
}


//
//  If we are getting focus, then we show the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we have the focus.
//
//  This can sometimes happen before we get a page loaded if the CAB is
//  the widget that would get focus initially on load. So we have to
//  watch for an empty stack.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::GotFocus()
{
    if (!m_colStack.bIsEmpty())
    {
        // Just force a redraw of the slot with the focus
        TItemList& ilTop = *m_colStack.pobjPeek();
        TArea areaUpdate;
        AreaForIndex(ilTop.areaAdjusted(m_areaDisplay), m_c4FocusIndex, areaUpdate);
        Redraw(areaUpdate);
    }
}


//
//  If we are losing the focus, we remove the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we don't have the focus.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::LostFocus()
{
    if (!m_colStack.bIsEmpty())
    {
        // Just force a redraw of the slot with the focus
        TItemList& ilTop = *m_colStack.pobjPeek();
        TArea areaUpdate;
        AreaForIndex(ilTop.areaAdjusted(m_areaDisplay), m_c4FocusIndex, areaUpdate);
        Redraw(areaUpdate);
    }
}


tCIDLib::TVoid TCQCIntfCvrArtBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CvrArtBrowser::c2FmtVersion))
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

    //
    //  Do any mixins.
    //
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    //
    //  If V2 or later, read in the image list stuff. However, if less
    //  then V6 we need to remove all the old images and add the ones
    //  that we now support.
    //
    if (c2FmtVersion > 1)
    {
        MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

        if (c2FmtVersion < 6)
        {
            DeleteAllImgKeys();
            AddNewImgKey
            (
                CQCIntfEng_CvrArtBrowser::strMissingArt
                , tCQCIntfEng::EImgReqFlags::Optional
            );
            AddNewImgKey
            (
                CQCIntfEng_CvrArtBrowser::strPlayListArt
                , tCQCIntfEng::EImgReqFlags::Optional
            );
        }
    }

    //
    //  And do our stuff. Many things have been removed, now that we are
    //  getting all of the formatting from a pattern template, so we just
    //  toss them.
    //
    if (c2FmtVersion < 6)
    {
        tCIDLib::TBoolean bDummy;
        strmToReadFrom  >> bDummy;
    }

    strmToReadFrom  >> m_c4HSpacing
                    >> m_c4VSpacing;

    //
    //  If before V6, read in the old width and height. We need them for
    //  upgrading them to the new template pattern.
    //
    if (c2FmtVersion < 6)
    {
        tCIDLib::TCard4 c4Dummy;
        strmToReadFrom  >> c4Dummy
                        >> c4Dummy;
    }

    strmToReadFrom  >> m_strRepoMoniker;

    // Eat the old show flag if less than V6
    if (c2FmtVersion < 6)
    {
        tCIDLib::TInt4 i4Eat;
        strmToReadFrom >> i4Eat;
    }

    // Eat the old display f/x enum if less than V5
    if (c2FmtVersion < 5)
    {
        tCIDLib::TInt4 i4Eat;
        strmToReadFrom >> i4Eat;
    }

    strmToReadFrom  >> m_eInitMediaType;

    // If V3 to V6, read in the large image flag and toss it.
    if ((c2FmtVersion > 2) && (c2FmtVersion < 6))
    {
        tCIDLib::TBoolean bDummy;
        strmToReadFrom >> bDummy;
    }

    //
    //  If less than V6, then set up the default layout template, else
    //  read it in.
    //
    if (c2FmtVersion < 6)
        m_strLayoutTmpl = CQCIntfEng_CvrArtBrowser::pszDefLayout;
    else
        strmToReadFrom >> m_strLayoutTmpl;

    if (c2FmtVersion < 8)
        m_eSortOrder = tCQCMedia::ESortOrders::Title;
    else
        strmToReadFrom >> m_eSortOrder;

    // Convert the path to 5.x format if needed
    if (c2FmtVersion < 7)
        facCQCKit().Make50Path(m_strLayoutTmpl);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear the runtime stuff
    m_bSafePanBgn = kCIDLib::False;
    m_c4FocusIndex = 0;
    m_c4SelIndex = kCIDLib::c4MaxCard;

    // If pre-V4 then initial the context on a few events
    if (c2FmtVersion < 4)
    {
        SetEventContext(kCQCKit::strEvId_OnScroll, tCQCKit::EActCmdCtx::NoContChange);
        SetEventContext(kCQCKit::strEvId_OnSet, tCQCKit::EActCmdCtx::NoContChange);
    }

    //
    //  We added a new event, so we have to force it into any previously
    //  stored CAB objects that haven't had it added yet. In this case,
    //  since we already have other events, we only want to add this one,
    //  and it could already be there from the ctor, so we check first
    //  and only added it if not there.
    //
    if (!pcolOpsForEvent(kCQCKit::strEvId_OnScroll))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
            , kCQCKit::strEvId_OnScroll
            , tCQCKit::EActCmdCtx::NoContChange
        );
    }
}


tCIDLib::TVoid TCQCIntfCvrArtBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our marker and version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CvrArtBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // And do our stuff and end marker
    strmToWriteTo   << m_c4HSpacing
                    << m_c4VSpacing
                    << m_strRepoMoniker
                    << m_eInitMediaType

                    // V6 stuff
                    << m_strLayoutTmpl

                    // V8 Stuff, V7 was just for a conversion step
                    << m_eSortOrder

                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is only called during design, which is the only time a widget's
//  size will ever change. We use it to adjust our display areas.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    // Call our parent first
    TParent::SizeChanged(szNew, szOld);

    // Update our size calculations
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    DoSizeCalcs(areaNew, areaOld);
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::StyleFlagChanged(const   tCIDLib::TCard4 c4OldFlags
                                        , const tCIDLib::TCard4 c4NewFlags)
{
    const tCIDLib::TCard4 c4Old(c4OldFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
    const tCIDLib::TCard4 c4New(c4NewFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
}


tCIDLib::TVoid
TCQCIntfCvrArtBrowser::Update(          TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    //
    //  Let our parent provide the standard background fill and border
    //  stuff.
    //
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // If not in designer mode, draw our content
    if (!TFacCQCIntfEng::bDesMode())
    {
        //
        //  If the stack is empty, or the layout template has no children,
        //  then there is nothing to do.
        //
        if (!m_colStack.bIsEmpty() && m_iwdgLayout.c4ChildCount())
        {
            //  Add our display area to the clip region
            TRegionJanitor janClip(&gdevTarget, m_areaDisplay, tCIDGraphDev::EClipModes::And);

            //
            //  The target area to update is the intersection of our display
            //  area and the invalid area.
            //
            TArea areaUpdate(areaInvalid);
            areaUpdate &= m_areaDisplay;

            if (!areaUpdate.bIsEmpty())
            {
                DrawList
                (
                    gdevTarget
                    , m_colStack.pobjPeek()->areaAdjusted(m_areaDisplay)
                    , areaUpdate
                    , TPoint::pntOrigin
                    , *m_colStack.pobjPeek()
                    , kCIDLib::False
                );
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfCvrArtBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Calculate the area for a given list slot index, relative to an area. The
//  relative area is generally the scroll adjusted position, either relative
//  to our own widget area or relative to the zero origin of a memory into
//  into which scrollable content is being draw.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::AreaForIndex(const   TArea&          areaRelTo
                                    , const tCIDLib::TCard4 c4Index
                                    ,       TArea&          areaToFill)
{
    // Figure out the column it would be in
    const tCIDLib::TCard4 c4Col = c4Index / m_c4RowCount;

    // And the row will be the modulus
    const tCIDLib::TCard4 c4Row = c4Index % m_c4RowCount;

    // And move over and down the appropriate amounts from the origin
    const TSize szLayout = m_iwdgLayout.areaActual().szArea();
    areaToFill.Set(areaRelTo.pntOrg(), szLayout);

    areaToFill.AdjustOrg
    (
        tCIDLib::TInt4(c4Col * (szLayout.c4Width() + m_c4HSpacing))
        , tCIDLib::TInt4(c4Row * (szLayout.c4Height() + m_c4VSpacing))
    );
}



//
//  We have a special case issue when doing our pans or scrolls that we
//  need to capture any content under the items. That includes any popup
//  layers under us, and any widgets under us in our own layer, and
//  including our own bgn fill if any but not the buttons themselves. So it's
//  sort of a special case.
//
//  We use a special method on the parent container to draw up to, but
//  not including us. Then we draw our background stuff over the top of
//  that. We return a bitmap with that content.
//
//  NOTE: This cannot be called from the regular update, since the big
//  background bitmap is already in use in that case. This is only to
//  be called from the scrolling method.
//
//  NOTE: The bitmap returned is the content within our display area, not
//  our whole widget area, unless they both happen to be the same. We only
//  scroll within the display area so that's all that matters.
//
TBitmap TCQCIntfCvrArtBrowser::bmpGatherBgn()
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

    //
    //  Now let's draw our bgn contents on top of that. Center an area
    //  of our full size over the display area and fill over that. to
    //  insure alignment of gradients within the display are bit we move.
    //
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
    if (!bIsTransparent())
    {
        TArea areaFill(areaActual());
        areaFill.CenterIn(m_areaDisplay);
        DoBgnFill(gdevBgn, areaFill);
    }

    // Create a bitmap the size of our display area
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::False
    );

    //
    //  And copy the bgn contents over into our return bitmap. WE create a
    //  zero origined version of the display area as the target.
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
//  This is called when the user sends a command to do a search by actor.
//  This is only relevant for movie media type, else we ignore it. We will
//  search for all title sets that have a lead actor that matches the
//  passed actor (or reg ex pattern) and load up a new list with that set
//  of titles, in the indicated sort order.
//
tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::
bPushActorSearch(const  TString&                strActorName
                , const tCIDLib::TBoolean       bRegEx
                , const tCQCMedia::ESortOrders  eSortOrder)
{
    // Everything here is movies
    const tCQCMedia::EMediaTypes eMType(tCQCMedia::EMediaTypes::Movie);

    // If we have not gotten media data, can't do this
    if (!m_cptrMediaDB.pobjData())
        return kCIDLib::False;

    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Make sure our database supports movies
    if (!tCIDLib::bAnyBitsOn(m_cptrMediaDB->eMTFlags(), tCQCMedia::EMTFlags::Movie))
        return kCIDLib::False;

    // Set up the regular expression if there is one
    TRegEx regxActor;
    if (bRegEx)
        regxActor.SetExpression(strActorName);

    // Provisionally create a list
    TString strListDescr(L"Actors matching '");
    strListDescr.Append(strActorName);
    strListDescr.Append(L"'");
    TItemList* pilNew = new TItemList
    (
        strListDescr
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::TitleSearch
        , tCQCMedia::ESortOrders::Count
        , TString::strEmpty()
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    tCIDLib::TCard4 c4Count = mdbInfo.c4TitleSetCnt(eMType);
    TString strName;
    TString strCookie;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current title set
        const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);

        // And now let's go through the collections of this title set
        tCIDLib::TBoolean bAdded = kCIDLib::False;
        const tCIDLib::TCard4 c4ColCnt = mtsCur.c4ColCount();
        for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
        {
            const TMediaCollect& mcolCur = mtsCur.mcolAt(mdbInfo, c4ColInd);

            // Check the lead actor
            if (bRegEx)
            {
                if (regxActor.bFullyMatches(mcolCur.strLeadActor()))
                    bAdded = kCIDLib::True;
            }
             else
            {
                if (mcolCur.strLeadActor().bCompareI(strActorName))
                    bAdded = kCIDLib::True;
            }

            if (!bAdded)
            {
                // Not the lead actor, so check the cast
                TStringTokenizer stokCast(&mcolCur.strCast(), L",");
                while (stokCast.bGetNextToken(strName) && !bAdded)
                {
                    strName.StripWhitespace();
                    if (bRegEx)
                    {
                        if (regxActor.bFullyMatches(strName))
                            bAdded = kCIDLib::True;
                    }
                     else
                    {
                        if (strName.bCompareI(strActorName))
                            bAdded = kCIDLib::True;
                    }
                }
            }

            if (bAdded)
            {
                //
                //  It's a keeper. Set up the cookie relative to the
                //  the all movies category, since we know that it
                //  is available there.
                //
                strCookie = mdbInfo.strAllCatCookieFor(eMType);
                strCookie.Append(kCIDLib::chComma);
                strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
                pilNew->m_colItems.Add
                (
                    new TListItem(&mtsCur, strCookie)
                );
            }
        }
    }

    // If no matches, give up now
    if (pilNew->m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  If the current top of stack is a random type, then pop it off first, since
    //  we just want to treat them as temporary really. Don't pop, just trash the
    //  top and push the new one.
    //
    if (!m_colStack.bIsEmpty()
    &&  (m_colStack.pobjPeek()->m_eListType == EListTypes::Random))
    {
        m_colStack.TrashTop();
    }

    // And now push it onto the stack
    PushPopStack(janList.pobjOrphan(), eSortOrder);

    // And indicate we pushed a new list
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::
bPushArtistSearch(  const   TString&                strArtistName
                    , const tCIDLib::TBoolean       bRegEx
                    , const tCQCMedia::ESortOrders  eSortOrder
                    , const tCQCMedia::EMediaTypes  eMType)
{
    // If we have not gotten media data, can't do this
    if (!m_cptrMediaDB.pobjData())
        return kCIDLib::False;

    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Provisionally create a list
    TString strListDescr(L"Artists matching '");
    strListDescr.Append(strArtistName);
    strListDescr.Append(L"'");
    TItemList* pilNew = new TItemList
    (
        strListDescr
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::TitleSearch
        , tCQCMedia::ESortOrders::Count
        , TString::strEmpty()
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    tCIDLib::TCard4 c4Count = mdbInfo.c4TitleSetCnt(eMType);
    TString strName;
    TString strCookie;

    tCIDLib::TBoolean bAdded = kCIDLib::False;
    if (bRegEx)
    {
        // Set up the regular expression
        TRegEx regxArtist(strArtistName);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // Get the current title set
            const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);
            if (regxArtist.bFullyMatches(mtsCur.strArtist()))
            {
                strCookie = mdbInfo.strAllCatCookieFor(eMType);
                strCookie.Append(kCIDLib::chComma);
                strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
                pilNew->m_colItems.Add
                (
                    new TListItem(&mtsCur, strCookie)
                );
            }
        }
    }
     else
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);
            if (mtsCur.strArtist().bCompareI(strArtistName))
            {
                strCookie = mdbInfo.strAllCatCookieFor(eMType);
                strCookie.Append(kCIDLib::chComma);
                strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
                pilNew->m_colItems.Add
                (
                    new TListItem(&mtsCur, strCookie)
                );
            }
        }
    }

    // If no matches, give up now
    if (pilNew->m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  If the current top of stack is a random type, then pop it off first, since
    //  we just want to treat them as temporary really. Don't pop, just trash the
    //  top and push the new one.
    //
    if (!m_colStack.bIsEmpty()
    &&  (m_colStack.pobjPeek()->m_eListType == EListTypes::Random))
    {
        m_colStack.TrashTop();
    }

    // And now push it onto the stack
    PushPopStack(janList.pobjOrphan(), eSortOrder);

    // And indicate we pushed a new list
    return kCIDLib::True;
}


//
//  When the user sends us an audio format search, we find all of the
//  title sets that match the criteria.
//
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::
bPushAudioFmtSearch(const   tCQCMedia::ECompTypes   eCompType
                    , const tCIDLib::TCard4         c4BitDepth
                    , const tCIDLib::TCard4         c4SampleRate
                    , const tCQCMedia::ESortOrders  eSortOrder)
{
    // Everything here is movies
    const tCQCMedia::EMediaTypes eMType(tCQCMedia::EMediaTypes::Music);

    // If we have not gotten media data, can't do this
    if (!m_cptrMediaDB.pobjData())
        return kCIDLib::False;

    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Make sure our database supports music
    if (!tCIDLib::bAnyBitsOn(m_cptrMediaDB->eMTFlags(), tCQCMedia::EMTFlags::Music))
        return kCIDLib::False;

    //
    //  The bits of the comparison type are set up to work correctly as
    //  a mask if we just add one to the value. So we get:
    //
    //  Bit 0 on means equality is included
    //  Bit 1 on means less than is included
    //  Bit 2 on means greater than is included
    //
    //  So create the mask we are to compare against
    //
    const tCIDLib::TCard4 c4CompMask(tCIDLib::TCard4(eCompType) + 1);

    // Provisionally create a list
    TString strListDescr(L"Audio format matches ");
    strListDescr.Append(L"(BitDepth ");
    strListDescr.Append(tCQCMedia::strXlatECompTypes(eCompType));
    strListDescr.AppendFormatted(c4BitDepth);
    strListDescr.Append(L" AND (SampleRate ");
    strListDescr.Append(tCQCMedia::strXlatECompTypes(eCompType));
    strListDescr.AppendFormatted(c4SampleRate);
    strListDescr.Append(L")");

    TItemList* pilNew = new TItemList
    (
        strListDescr
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::TitleSearch
        , tCQCMedia::ESortOrders::Count
        , TString::strEmpty()
        , 0
    );
    TJanitor<TItemList> janList(pilNew);


    tCIDLib::TCard4 c4Count = mdbInfo.c4TitleSetCnt(eMType);
    TString strName;
    TString strCookie;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current title set
        const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);

        tCIDLib::TCard4 c4DepthMask = 0;
        tCIDLib::TCard4 c4RateMask = 0;

        //
        //  Set up masks for the relationships between the current item
        //  values and the passed values.
        //
        if (mtsCur.c4BitDepth() == c4BitDepth)
            c4DepthMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Eq) + 1);
        else if (mtsCur.c4BitDepth() < c4BitDepth)
            c4DepthMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Lt) + 1);
        else
            c4DepthMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Gt) + 1);

        if (mtsCur.c4SampleRate() == c4SampleRate)
            c4RateMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Eq) + 1);
        else if (mtsCur.c4SampleRate() < c4SampleRate)
            c4RateMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Lt) + 1);
        else
            c4RateMask |= (tCIDLib::c4EnumOrd(tCQCMedia::ECompTypes::Gt) + 1);

        // If common bits are on in both, then we keep it
        if ((c4DepthMask & c4CompMask) && (c4RateMask & c4CompMask))
        {
            // Set up a cookie relative to the All XXX category
            strCookie = mdbInfo.strAllCatCookieFor(eMType);
            strCookie.Append(kCIDLib::chComma);
            strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
            pilNew->m_colItems.Add(new TListItem(&mtsCur, strCookie));
        }
    }

    // If no matches, give up now
    if (pilNew->m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  If the current top of stack is a random type, then pop it off first, since
    //  we just want to treat them as temporary really. Don't pop, just trash the
    //  top and push the new one.
    //
    if (!m_colStack.bIsEmpty()
    &&  (m_colStack.pobjPeek()->m_eListType == EListTypes::Random))
    {
        m_colStack.TrashTop();
    }

    // And now push it onto the stack
    PushPopStack(janList.pobjOrphan(), eSortOrder);

    // And indicate we pushed a new list
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfCvrArtBrowser::bPushRandomTitleSearch(  const   tCQCMedia::ESortOrders  eSortOrder
                                                , const tCQCMedia::EMediaTypes  eMType
                                                , const tCIDLib::TCard4         c4MaxCount)
{
    // If we have not gotten media data, can't do this
    if (!m_cptrMediaDB.pobjData())
        return kCIDLib::False;

    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Get the count of titles for the media type. If less than 2, then do nothing
    const tCIDLib::TCard4 c4TitleCnt = mdbInfo.c4TitleSetCnt(eMType);
    if (c4TitleCnt < 2)
        return kCIDLib::False;

    // If the requested max count is less than 2, then do nothing
    if (c4MaxCount < 2)
        return kCIDLib::False;

    // Provisionally create a list
    TString strListDescr(L"Random ");
    strListDescr.Append(tCQCMedia::strXlatEMediaTypes(eMType));
    strListDescr.Append(L" Titles");

    TItemList* pilNew = new TItemList
    (
        strListDescr
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::Random
        , tCQCMedia::ESortOrders::Count
        , TString::strEmpty()
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    //
    //  We will generate random numbers and use those to index the titles list
    //  for the current media type. We will do either 128 or half of the available
    //  titles or the max count, whichever is less.
    //
    tCIDLib::TCard4 c4Count = 128;
    if ((c4TitleCnt / 2) < 128)
        c4Count = (c4TitleCnt / 2);
    if (c4MaxCount < c4Count)
        c4Count = c4MaxCount;

    // We need a flag array to keep up with which ones we've already added
    tCIDLib::TBoolArray fcolAlready(c4TitleCnt, kCIDLib::False);
    TRandomNum randSearch;
    TString strCookie;
    randSearch.Seed(TTime::c4Millis());

    //
    //  Loop till we have the needed count. At the most we are only getting half
    //  of the available titles, so we should never get caught here looping forever.
    //  Hitting so far unhit ones should be easy. But, just in case, we use a check
    //  for that. But because we will often hit the same one twice and not take it,
    //  we can't just loop for c4Count times or we'll come up short most of the time.
    //
    tCIDLib::TCard4 c4LoopCnt = 0;
    tCIDLib::TCard4 c4Done = 0;
    while ((c4Done < c4Count) && (c4LoopCnt < c4Count * 2))
    {
        const tCIDLib::TCard4 c4TitleInd = randSearch.c4GetNextNum() % c4TitleCnt;
        if (!fcolAlready[c4TitleInd])
        {
            fcolAlready[c4TitleInd] = kCIDLib::True;

            const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4TitleInd);

            strCookie = mdbInfo.strAllCatCookieFor(eMType);
            strCookie.Append(kCIDLib::chComma);
            strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
            pilNew->m_colItems.Add(new TListItem(&mtsCur, strCookie));

            c4Done++;
        }
        c4LoopCnt++;
    }

    // If no matches, give up now
    if (pilNew->m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  If the current top of stack is a random type, then pop it off first, since
    //  we just want to treat them as temporary really. Don't pop, just trash the
    //  top and push the new one.
    //
    if (!m_colStack.bIsEmpty()
    &&  (m_colStack.pobjPeek()->m_eListType == EListTypes::Random))
    {
        m_colStack.TrashTop();
    }

    // And now push it onto the stack, orphaning from the janitor
    PushPopStack(janList.pobjOrphan(), eSortOrder);

    // And indicate we pushed a new list
    return kCIDLib::True;
}


//
//  When the user sends us an by title name search, we find all of the
//  title sets that match the criteria.
//
tCIDLib::TBoolean TCQCIntfCvrArtBrowser::
bPushTitleSearch(const  TString&                strMatch
                , const tCIDLib::TBoolean       bRegEx
                , const tCQCMedia::ESortOrders  eSortOrder
                , const tCQCMedia::EMediaTypes  eMType)
{
    // If we have not gotten media data, can't do this
    if (!m_cptrMediaDB.pobjData())
        return kCIDLib::False;

    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Provisionally create a list
    TString strListDescr(L"Title matches '");
    strListDescr.Append(strMatch);
    strListDescr.Append(L"'");

    TItemList* pilNew = new TItemList
    (
        strListDescr
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::TitleSearch
        , tCQCMedia::ESortOrders::Count
        , TString::strEmpty()
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    tCIDLib::TCard4 c4Count = mdbInfo.c4TitleSetCnt(eMType);
    TString strName;
    TString strCookie;

    tCIDLib::TBoolean bAdded = kCIDLib::False;
    if (bRegEx)
    {
        // Set up the regular expression
        TRegEx regxMatch(strMatch);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // Get the current title set
            const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);
            if (regxMatch.bFullyMatches(mtsCur.strName()))
            {
                strCookie = mdbInfo.strAllCatCookieFor(eMType);
                strCookie.Append(kCIDLib::chComma);
                strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
                pilNew->m_colItems.Add
                (
                    new TListItem(&mtsCur, strCookie)
                );
            }
        }
    }
     else
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);
            if (mtsCur.strName().bCompareI(strMatch))
            {
                strCookie = mdbInfo.strAllCatCookieFor(eMType);
                strCookie.Append(kCIDLib::chComma);
                strCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
                pilNew->m_colItems.Add
                (
                    new TListItem(&mtsCur, strCookie)
                );
            }
        }
    }

    // If no matches, give up now
    if (pilNew->m_colItems.bIsEmpty())
        return kCIDLib::False;

    //
    //  If the current top of stack is a random type, then pop it off first, since
    //  we just want to treat them as temporary really. Don't pop, just trash the
    //  top and push the new one.
    //
    if (!m_colStack.bIsEmpty()
    &&  (m_colStack.pobjPeek()->m_eListType == EListTypes::Random))
    {
        m_colStack.TrashTop();
    }

    // And now push it onto the stack, orphaning from the janitor
    PushPopStack(janList.pobjOrphan(), eSortOrder);

    // And indicate we pushed a new list
    return kCIDLib::True;
}


//
//  Calculate the first visible column, given a visible area and a drawing
//  area relative to that (where the relative area X origin is at or to the
//  left of the visible area, i.e. scrolled.
//
//  This one is ok for use during drawing of scrollable content into zero
//  origined memory images, unlike the ones on the item list class which
//  always just work in terms of the currently adjusted display area.
//
tCIDLib::TCard4 TCQCIntfCvrArtBrowser::
c4FirstVisColumn(const  TArea&              areaVis
                , const TArea&              areaRelTo
                ,       tCIDLib::TCard4&    c4Index) const
{
    // See how far we are scrolled from the content origin
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4(areaVis.i4X() - areaRelTo.i4X());

    //
    //  And we can just divide to get the index, and again by the number of
    //  slots per column to get the column.
    //
    const tCIDLib::TCard4 c4Ret
    (
        c4Diff / (m_iwdgLayout.areaRelative().c4Width() + m_c4HSpacing)
    );
    c4Index = (c4Ret * m_c4RowCount);
    return c4Ret;
}


//
//  Tests a point to see if it falls in one of our slots. Note that we don't
//  just test for slots but for valid ones. If we have fewer images than
//  slots (on a partial last page) then some of them aren't used.
//
//  If we have as many as or more than, then all the slots will always be
//  valid since we never scroll beyond the point where the last name is in
//  the last slot.
//
tCIDLib::TCard4
TCQCIntfCvrArtBrowser::c4HitTest(const  TPoint&             pntTest
                                ,       tCIDLib::TCard4&    c4Row
                                ,       tCIDLib::TCard4&    c4Col) const
{
    //
    //  If the stack is empty (and it can be when loading a new template
    //  during initalization), then we can just return a no hit value now.
    //
    if (m_colStack.bIsEmpty())
        return kCIDLib::c4MaxCard;

    //
    //  Set up an adjusted content area for the stack top, relative to our
    //  our own area. Hit testing is only ever done relative to the actual
    //  display area.
    //
    const TItemList* pilTop = m_colStack.pobjPeek();
    TArea areaCont = pilTop->areaAdjusted(m_areaDisplay);

    // If not within this area, then can't have hit anything
    if (!areaCont.bContainsPoint(pntTest))
        return kCIDLib::c4MaxCard;

    // See what column we hit
    const TArea& areaLO = m_iwdgLayout.areaActual();
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4(pntTest.i4X() - areaCont.i4X());
    c4Col = c4Diff / (areaLO.c4Width() + m_c4HSpacing);

    // See what row
    c4Diff = tCIDLib::TInt4(pntTest.i4Y() - areaCont.i4Y());
    c4Row = c4Diff / (areaLO.c4Height() + m_c4VSpacing);

    //
    //  It's possible this is in the unused last partial column, so multiply
    //  back out and see if it's beyond our list count.
    //
    const tCIDLib::TCard4 c4Index = (c4Col * m_c4RowCount) + c4Row;
    if (c4Index >= pilTop->m_colItems.c4ElemCount())
        return kCIDLib::c4MaxCard;

    return c4Index;
}


//
//  Upon init or if we size changes, we have to recalculate some stuff that
//  we want to keep around for efficiency.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::DoSizeCalcs(const TArea& areaNew, const TArea& areaOld)
{
    TArea areaDraw = areaNew;

    // Move in one if we have a border
    if (bHasBorder())
        areaDraw.Deflate(1);

    // If rounding, provide some more margin
    if (c4Rounding())
        areaDraw.Deflate(2);

    const TArea& areaLayout = m_iwdgLayout.areaActual();

    // Set up an area of one configured item size
    TArea areaItem(areaDraw);
    areaItem.SetSize(areaLayout.szArea());

    // Just in case, if the layout is empty, set it to some default size
    if (areaItem.bIsEmpty())
        areaItem.SetSize(92, 64);

    //
    //  Figure out how many rows we can display, given our height. We use
    //  the configured item height and spacing. Since we don't need the
    //  final spacer, we have to be careful how we do this.
    //
    TArea areaTmp = areaItem;
    areaTmp.AdjustSize(m_c4HSpacing, m_c4VSpacing);
    m_c4RowsHeight = 0;
    m_c4RowCount = 0;
    do
    {
        m_c4RowCount += 1;
        m_c4RowsHeight += areaLayout.c4Height() + m_c4VSpacing;

        areaTmp.Move(tCIDLib::EDirs::Down);
    }   while (areaTmp.i4Bottom() <= areaDraw.i4Bottom());

    // Remove the last spacing value
    m_c4RowsHeight -= m_c4VSpacing;

    //
    //  Calculate how many columns will fit across our width. This is not
    //  an exact thing since we are draggable and we could have a partial
    //  fit. We include any partial one, and just need a basic count for
    //  setting up scroll areas and such.
    //
    areaTmp.SetOrg(areaDraw.pntOrg());
    m_c4ColCount = 1;
    while (areaTmp.i4Right() <= areaDraw.i4Right())
    {
        areaTmp.Move(tCIDLib::EDirs::Right);
        m_c4ColCount += 1;
    }

    //
    //  Set up our display area, which is the area within us that we will
    //  display content and scroll. Center it vertically, so that as we are
    //  dynamically resized and the number of rows change, the content will
    //  stay in the center. Left justify it horizontally.
    //
    m_areaDisplay = areaDraw;
    m_areaDisplay.c4Height(m_c4RowsHeight);
    m_areaDisplay.LeftJustifyIn(areaDraw, kCIDLib::True);

    //
    //  Now go through our stack of lists and let them update their content
    //  and adjusted display areas.
    //
    //  Also, for the top one, we need to make sure we have the cover art
    //  required to display the new content. We may have been near the end
    //  and a size up uncovered more stuff.
    //
    //  It's ok to do this here since the fact that our stack is not empty
    //  means this isn't some initial loading pass that's setting size
    //  before we are ready to go.
    //
    TListStack::TNCCursor cursStack(&m_colStack);
    if (cursStack.bReset())
    {
        do
        {
            cursStack.objWCur().Resize
            (
                m_c4RowsHeight
                , m_c4RowCount
                , m_c4HSpacing
                , m_iwdgLayout.areaActual()
                , m_areaDisplay
            );
        }   while(cursStack.bNext());

        //
        //  Make sure the top one has sufficient cover art, so get the
        //  first and last indices of items that are at all visible
        //  and load the art for them if not already.
        //
        TItemList& ilTop = *m_colStack.pobjPeek();
        tCIDLib::TCard4 c4FirstInd;
        tCIDLib::TCard4 c4LastInd;
        ilTop.QueryVisIndices
        (
            areaLayout
            , m_c4HSpacing
            , m_c4RowCount
            , c4FirstInd
            , c4LastInd
            , TArea::areaEmpty
        );

        tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();
        LoadListArt(*cptrDev.pobjData(), ilTop, c4FirstInd, c4LastInd);
    }

    // Make sure the current list focus stays visible
    KeepFocusVisible();
}


//
//  Draws a single list item at the indicated position.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::DrawListItem(        TGraphDrawDev&      gdevTarget
                                    ,       TListItem&          liDraw
                                    , const TArea&              areaAt
                                    , const TArea&              areaInvalid
                                    , const tCIDLib::TBoolean   bAlpha
                                    , const tCIDLib::TBoolean   bDoFocus)
{
    // Loop through the layout widgets and draw the info
    TArea areaCur;
    TArea areaSrc;
    TArea areaTar;
    const tCIDLib::TCard4 c4Count = m_iwdgLayout.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = m_iwdgLayout.iwdgAt(c4Index);
        areaCur = iwdgCur.areaRelative();
        areaCur.AdjustOrg(areaAt.pntOrg());

        //
        //  If this doesn't intersect both areas, ignore it. It's more likely
        //  not to intersect the invalid area, so we do that first.
        //
        if (!areaCur.bIntersects(areaInvalid) || !areaCur.bIntersects(areaAt))
            continue;

        // Save the device state around each round
        TGraphicDevJanitor janDev(&gdevTarget);

        if (iwdgCur.clsIsA() == TCQCIntfStaticBooleanImg::clsThis())
        {
            //
            //  These are used to show things like pressed state or focus
            //  state. They have to have specific names to indicate what
            //  role they play. Any that didn't have these names were removed
            //  during the load.
            //
            //  So we have to look at the name and figure out which of their
            //  images to display.
            //
            TCQCIntfStaticBooleanImg& iwdgImg(static_cast<TCQCIntfStaticBooleanImg&>(iwdgCur));

            // Pressed takes precedences over focus, so check that first
            const tCIDLib::TCh* pszKey = kCQCKit::pszFld_False;
            if (iwdgImg.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_Pressed)
            {
                if (liDraw.m_bPressed)
                    pszKey = kCQCKit::pszFld_True;
            }
             else if (iwdgImg.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_Focus)
            {
                // If told to do focus on this one, then draw the true
                if (bDoFocus)
                    pszKey = kCQCKit::pszFld_True;
            }

            // If we got a key, look it up. If we find it, display the image
            if (pszKey)
            {
                MCQCIntfImgListIntf::TImgInfo* pimgiCur = iwdgImg.pimgiForKeyEO(pszKey);
                if (pimgiCur)
                {
                    facCQCIntfEng().DrawImage
                    (
                        gdevTarget
                        , bAlpha
                        , pimgiCur->m_cptrImg
                        , areaCur
                        , areaInvalid
                        , pimgiCur->m_i4HOffset
                        , pimgiCur->m_i4VOffset
                        , pimgiCur->m_c1Opacity
                    );
                }
            }
        }
         else if (iwdgCur.clsIsA() == TCQCIntfMediaRepoText::clsThis())
        {
            DrawRepoText
            (
                gdevTarget
                , static_cast<TCQCIntfMediaRepoText&>(iwdgCur)
                , liDraw
                , c4Index
                , areaCur
                , bAlpha
            );
        }
         else if (iwdgCur.clsIsA() == TCQCIntfMediaRepoImg::clsThis())
        {
            // If this guy's image has been loaded, let's draw it
            if (liDraw.m_bImgLoaded)
            {
                //
                //  For the old RIVA clients, for the time being, leave the old code
                //  in place. For IV and new WebRIVA we can do the same code and better
                //
                if (TFacCQCIntfEng::bRemoteMode() && !TFacCQCIntfEng::bWebRIVAMode())
                {
                    areaSrc.Set(TPoint::pntOrigin, liDraw.m_szOrgImg);
                    if (liDraw.m_bmpImg.bHasAlpha())
                        gdevTarget.AlphaBlit(liDraw.m_bmpImg, areaSrc, areaCur, 0xFF, kCIDLib::True);
                    else
                        gdevTarget.DrawBitmap(liDraw.m_bmpImg, areaSrc, areaCur);
                }
                 else
                {
                    // The source is the area of the image
                    TSize szCurImg = liDraw.m_bmpImg.szBitmap();
                    areaSrc.Set(TPoint::pntOrigin, szCurImg);

                    // Scale the target area so that fits the available height
                    const tCIDLib::TFloat8 f8AR
                    (
                        tCIDLib::TFloat8(m_szArt.c4Height()) / tCIDLib::TFloat8(m_szArt.c4Width())
                    );
                    const TSize szNew
                    (
                        szCurImg.c4Width(), tCIDLib::TCard4(szCurImg.c4Height() * f8AR)
                    );
                    TArea areaBlit(areaCur.pntOrg(), szNew);

                    // Horizontally center it at the top of the current area
                    areaBlit.TopJustifyIn(areaCur, kCIDLib::True);

                    if (bAlpha || liDraw.m_bmpImg.bHasAlpha())
                        gdevTarget.DrawBitmapAlpha(liDraw.m_bmpImg, areaSrc, areaBlit);
                    else
                        gdevTarget.DrawBitmap(liDraw.m_bmpImg, areaSrc, areaBlit);
                }
            }
        }
         else if (iwdgCur.clsIsA() == TCQCIntfStaticImg::clsThis())
        {
            TCQCIntfStaticImg& iwdgImg(static_cast<TCQCIntfStaticImg&>(iwdgCur));
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , bAlpha
                , iwdgImg.cptrImage()
                , areaCur
                , areaInvalid
                , iwdgImg.ePlacement()
                , 0
                , 0
                , iwdgImg.c1Opacity()
            );
        }
         else if (iwdgCur.clsIsA() == TCQCIntfStaticText::clsThis())
        {
            //
            //  Nothing special here, just pass it to the text drawing
            //  helper class.
            //
            TCQCIntfStaticText& iwdgText(static_cast<TCQCIntfStaticText&>(iwdgCur));
            DrawText
            (
                gdevTarget
                , iwdgText
                , static_cast<MCQCIntfFontIntf&>(iwdgText)
                , iwdgText.strCaption()
                , areaCur
                , bAlpha
            );
        }
    }
}


//
//  This is the top top level content drawing method. We get a few areas
//  that control what we draw.
//
//  araRelTo    For regular updates this is our area. When building up
//              scrollable content, it's the zero origined memory image
//              to draw into.
//  areaInvalid We redraw slots that intersect this. For drawing zero
//              origined scrollable content, this is always the full
//              size.
//
//  To allow for drawing different pages into different sides of a scroll
//  buffer, we take an offset point.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::DrawList(        TGraphDrawDev&      gdevTarget
                                , const TArea&              areaRelTo
                                , const TArea&              areaInvalid
                                , const TPoint&             pntOffset
                                ,       TItemList&          ilDraw
                                , const tCIDLib::TBoolean   bForceAlpha)
{
    // Set up for drawing as required
    gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

    const TArea& areaLayout = m_iwdgLayout.areaActual();

    //
    //  We will ignore the do alpha flag if in remote mode, so to avoid doing it
    //  on every time through the loop, we set up a local flag.
    //
    const tCIDLib::TBoolean bAlpha = bForceAlpha && !TFacCQCIntfEng::bRemoteMode();

    //
    //  Figure out what the first (possibly partial) visible column is
    //  and the list index of the item at the top of that column.
    //
    tCIDLib::TCard4 c4Index;
    const tCIDLib::TCard4 c4FirstCol = c4FirstVisColumn(areaInvalid, areaRelTo, c4Index);

    // Build an area for this first (upper left) visible index
    TArea areaCur;
    AreaForIndex(areaRelTo, c4Index, areaCur);

    //
    //  Create an adjusted invalid area to pass into the individal item
    //  drawing method. Else, if adjusted, the invalid area won't be
    //  right for him.
    //
    TArea areaAdjInvalid = areaInvalid;
    areaAdjInvalid += pntOffset;

    //
    //  We have to tell the focus item it is the focus item, and we don't
    //  bother unless focus is enabled and we have focus and we have a focus
    //  indicator, so remember that much up front.
    //
    const tCIDLib::TBoolean bDoFocus
    (
        !iwdgRootTemplate().bNoFocus()
        && bHasFocus()
        && !m_areaFocusInd.bIsEmpty()
    );

    //
    //  Now we just iterate until our current area is beyond the target
    //  area in the X direction, or we run out of list items to draw.
    //
    const tCIDLib::TInt4 i4OrgY = areaCur.i4Y();
    const tCIDLib::TCard4 c4Count = ilDraw.m_colItems.c4ElemCount();
    TArea areaAdjusted;
    while ((areaCur.i4X() < areaInvalid.i4Right()) && (c4Index < c4Count))
    {
        tCIDLib::TCard4 c4RowInd = 0;
        while ((c4RowInd < m_c4RowCount) && (c4Index < c4Count))
        {
            // if this guy intersects the invalid area, draw it
            if (areaCur.bIntersects(areaInvalid))
            {
                TListItem& liCur = *ilDraw.m_colItems[c4Index];

                // Create the adjusted target area
                areaAdjusted = areaCur;
                areaAdjusted += pntOffset;

                // If pressed and we have no pressed indicator, simulate
                if (liCur.m_bPressed && !m_bHavePressedInd)
                    areaAdjusted.AdjustAndShrink(1, 1);

                DrawListItem
                (
                    gdevTarget
                    , liCur
                    , areaAdjusted
                    , areaAdjInvalid
                    , bAlpha
                    , bDoFocus && (c4Index == m_c4FocusIndex)
                );
            }

            // Move downwards
            areaCur.AdjustOrg
            (
                0, tCIDLib::TInt4(areaLayout.c4Height() + m_c4VSpacing)
            );

            // Move the index and row index forward on each inner loop
            c4Index++;
            c4RowInd++;
        }

        // Move back to the original Y pos, and right a slot
        areaCur.i4Y(i4OrgY);
        areaCur.AdjustOrg(tCIDLib::TInt4(areaLayout.c4Width() + m_c4HSpacing), 0);
    }
}


//
//  This is called to draw media repo text widget driven text content. We
//  look at the attribute that the repo text widget is set for and format
//  out that value for dislay. Note that we favor collections, since when
//  in collection mode we also have the title set. So always check for the
//  collection first if it's something that could be present in either.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::
DrawRepoText(           TGraphDrawDev&          gdevTarget
                , const TCQCIntfMediaRepoText&  iwdgDraw
                , const TListItem&              liData
                , const tCIDLib::TCard4         // c4Index not used so far
                ,       TArea&                  areaDraw
                , const tCIDLib::TBoolean       bDoAlpha)
{
    // Get the correct text, leave ??? in it if not available
    m_strTmp1.Clear();
    switch(iwdgDraw.eAttr())
    {
        case tCQCMedia::EMTextAttrs::Artist :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strArtist();
            else
                m_strTmp1 = liData.m_pmtsThis->strArtist();
            break;

        case tCQCMedia::EMTextAttrs::AspectRatio :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strAspectRatio();
            break;

        case tCQCMedia::EMTextAttrs::BitDepth :
            if (liData.m_pmtsThis)
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4BitDepth());
            break;

        case tCQCMedia::EMTextAttrs::BitRate :
            if (liData.m_pmtsThis)
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4BitRate());
            break;

        case tCQCMedia::EMTextAttrs::Cast :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strCast();
            break;

        case tCQCMedia::EMTextAttrs::Channels :
            if (liData.m_pmtsThis)
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4Channels());
            break;

        case tCQCMedia::EMTextAttrs::DateAdded :
            break;

        case tCQCMedia::EMTextAttrs::Description :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strDescr();
            break;

        case tCQCMedia::EMTextAttrs::Label :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strLabel();
            break;

        case tCQCMedia::EMTextAttrs::LeadActor :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strLeadActor();
            break;

        case tCQCMedia::EMTextAttrs::MediaFmt :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strMediaFormat();
            break;

        case tCQCMedia::EMTextAttrs::Name :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strName();
            else
                m_strTmp1 = liData.m_pmtsThis->strName();
            break;

        case tCQCMedia::EMTextAttrs::Rating :
            if (liData.m_pmcolThis)
                m_strTmp1 = liData.m_pmcolThis->strRating();
            break;

        case tCQCMedia::EMTextAttrs::RawRunTime :
            if (liData.m_pmcolThis)
                m_strTmp1.SetFormatted(liData.m_pmcolThis->c4Duration());
            break;

        case tCQCMedia::EMTextAttrs::RunTime :
        {
            tCIDLib::TCard4 c4Secs = 0;
            if (liData.m_pmcolThis)
            {
                c4Secs = liData.m_pmcolThis->c4Duration();
            }
             else if (liData.m_pmtsThis)
            {
                // Add up the times of the collections
                const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();
                const tCIDLib::TCard4 c4ColCount = liData.m_pmtsThis->c4ColCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
                    c4Secs += liData.m_pmtsThis->mcolAt(mdbInfo, c4Index).c4Duration();
            }

            const tCIDLib::TCard4 c4Hours = (c4Secs / 3600);
            const tCIDLib::TCard4 c4Mins((c4Secs - (c4Hours * 3600)) / 60);
            if (c4Hours)
            {
                m_strTmp1.SetFormatted(c4Hours);
                m_strTmp1.Append(L" hrs, ");
            }

            m_strTmp1.AppendFormatted(c4Mins);
            m_strTmp1.Append(L" mins");
            break;
        }

        case tCQCMedia::EMTextAttrs::SampleRate :
            if (liData.m_pmtsThis)
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4SampleRate());
            break;

        case tCQCMedia::EMTextAttrs::SeqNum :
            if (liData.m_pmtsThis)
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4SeqNum());
            break;

        case tCQCMedia::EMTextAttrs::Title :
        {
            m_strTmp1.Clear();

            // If we have the title start with it
            if (liData.m_pmtsThis)
                m_strTmp1 = liData.m_pmtsThis->strName();

            // If we have the collection add it
            if (liData.m_pmcolThis)
            {
                //
                //  If we had title text, and the collection name isn't
                //  the same, then add it.
                //
                if (!m_strTmp1.bIsEmpty()
                &&  (m_strTmp1 != liData.m_pmcolThis->strName()))
                {
                    m_strTmp1.Append(L" - ");
                }
                m_strTmp1.Append(liData.m_pmcolThis->strName());
            }
            break;
        }

        case tCQCMedia::EMTextAttrs::UserRating :
            if (liData.m_pmtsThis)
                m_strTmp1.AppendFormatted(liData.m_pmtsThis->c4UserRating());
            break;

        case tCQCMedia::EMTextAttrs::Year :
            if (liData.m_pmcolThis)
                m_strTmp1.SetFormatted(liData.m_pmcolThis->c4Year());
            else
                m_strTmp1.SetFormatted(liData.m_pmtsThis->c4Year());
            break;

        default :
            break;
    };


    // Draw the text using the text settings of the guide widget
    DrawText
    (
        gdevTarget
        , iwdgDraw
        , static_cast<const MCQCIntfFontIntf&>(iwdgDraw)
        , m_strTmp1
        , areaDraw
        , bDoAlpha
    );
}



//
//  This handles the text drawing grunt work and will work for any of the
//  text widgets. The caller has to get the font mixin out for us since
//  we can't access it due to getting the widget via the base widget class.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::DrawText(        TGraphDrawDev&      gdevTarget
                                , const TCQCIntfWidget&     iwdgSrc
                                , const MCQCIntfFontIntf&   miwdgFont
                                , const TString&            strText
                                ,       TArea&              areaTar
                                , const tCIDLib::TBoolean   bDoAlpha)
{
    //
    //  The static text widget does this so we need to in order for our
    //  output to match the layout template's appearance.
    //
    areaTar.Deflate(3, 2);

    // Set the widget's font while in here
    TCQCIntfFontJan janFont(&gdevTarget, miwdgFont);

    TCQCIntfView& civUs = civOwner();
    if (miwdgFont.eTextFX() != tCIDGraphDev::ETextFX::None)
    {
        civUs.DrawTextFX
        (
            gdevTarget
            , strText
            , areaTar
            , miwdgFont.eTextFX()
            , iwdgSrc.rgbFgn()
            , iwdgSrc.rgbFgn2()
            , miwdgFont.eHJustify()
            , miwdgFont.eVJustify()
            , miwdgFont.bNoTextWrap()
            , miwdgFont.pntOffset()
        );
    }
     else
    {
        //
        //  Now draw our text. If the shadow style is enabled, then draw
        //  first a drop version in fgn3, then draw the regular one in
        //  fgn1 in the normal position.
        //
        if (iwdgSrc.bShadow())
        {
            tCIDLib::TCard4 c4Drop = 1;
            if (miwdgFont.c4FontHeight() > 24)
                  c4Drop = 2;
            TArea areaDrop(areaTar);
            areaDrop.AdjustOrg(c4Drop, c4Drop);
            gdevTarget.SetTextColor(iwdgSrc.rgbFgn2());
            if (miwdgFont.bNoTextWrap())
            {
                if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
                {
                    gdevTarget.DrawString
                    (
                        strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                    );
                }
                 else
                {
                    civUs.DrawTextAlpha
                    (
                        gdevTarget
                        , strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                    );
                }
            }
             else
            {
                if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
                {
                    gdevTarget.DrawMText
                    (
                        strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                        , kCIDLib::True
                    );
                }
                 else
                {
                    civUs.DrawMTextAlpha
                    (
                        gdevTarget
                        , strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                        , kCIDLib::True
                    );
                }
            }
        }

        gdevTarget.SetTextColor(iwdgSrc.rgbFgn());
        if (miwdgFont.bNoTextWrap())
        {
            if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
            {
                gdevTarget.DrawString
                (
                    strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                );
            }
             else
            {
                civUs.DrawTextAlpha
                (
                    gdevTarget
                    , strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                );
            }
        }
         else
        {
            if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
            {
                gdevTarget.DrawMText
                (
                    strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                    , kCIDLib::True
                );
            }
             else
            {
                civUs.DrawMTextAlpha
                (
                    gdevTarget
                    , strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                    , kCIDLib::True
                );
            }
        }
    }
}


//
//  Tranlates from some of the values we take in command parameters, to the
//  internal enumerated type for some commonly used enums. We can't use
//  the standard enum stuff because the value we let the user enter is
//  the readable text, not the enum name. So we do a search for it by
//  the loadable text.
//
tCQCMedia::ECompTypes
TCQCIntfCvrArtBrowser::eXlatCompType(   const   TCQCCmdCfg&     ccfgSrc
                                        , const tCIDLib::TCard4 c4ParmInd) const
{
    const tCQCMedia::ECompTypes eType = facCQCMedia().eXlatCmdCompType
    (
        ccfgSrc.piAt(c4ParmInd).m_strValue
    );

    if (eType == tCQCMedia::ECompTypes::Count)
        ThrowBadParmVal(ccfgSrc, c4ParmInd);
    return eType;
}

tCQCMedia::EMediaTypes
TCQCIntfCvrArtBrowser::eXlatMediaType(  const   TCQCCmdCfg&     ccfgSrc
                                        , const tCIDLib::TCard4 c4ParmInd) const
{
    const tCQCMedia::EMediaTypes eMType = facCQCMedia().eXlatCmdMediaType
    (
        ccfgSrc.piAt(c4ParmInd).m_strValue
    );

    if (eMType == tCQCMedia::EMediaTypes::Count)
        ThrowBadParmVal(ccfgSrc, c4ParmInd);
    return eMType;
}

tCQCMedia::ESortOrders
TCQCIntfCvrArtBrowser::eXlatSortOrder(  const   TCQCCmdCfg&     ccfgSrc
                                        , const tCIDLib::TCard4 c4ParmInd) const
{
    const tCQCMedia::ESortOrders eOrder = facCQCMedia().eXlatCmdSortOrder
    (
        ccfgSrc.piAt(c4ParmInd).m_strValue
    );

    if (eOrder == tCQCMedia::ESortOrders::Count)
        ThrowBadParmVal(ccfgSrc, c4ParmInd);
    return eOrder;
}


//
//  Return the row/column for a given index There's not much code involved,
//  but it's worth keeping it separate for flexibility.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::IndexToRowCol(const  tCIDLib::TCard4     c4Index
                                    ,       tCIDLib::TCard4&    c4Row
                                    ,       tCIDLib::TCard4&    c4Col) const
{
    c4Col = c4Index / m_c4RowCount;
    c4Row = c4Index % m_c4RowCount;
}


//
//  Any time we move the contents around, we need to make sure the focus
//  stays visible. It won't actually matter unless we are doing focus, but
//  we just do it anyway. It has to be done from a number of places so we
//  want to keep it in one place.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::KeepFocusVisible()
{
    // Shouldn't happen, but just in case
    if (m_colStack.bIsEmpty())
        return;

    //
    //  Get the new visible range of indices for the new scroll position.
    //  Pass the focus indicator area so we get only get the ones with
    //  visible focus indicators.
    //
    tCIDLib::TCard4 c4FirstVis;
    tCIDLib::TCard4 c4LastVis;
    m_colStack.pobjPeek()->QueryVisIndices
    (
        m_iwdgLayout.areaActual()
        , m_c4HSpacing
        , m_c4RowCount
        , c4FirstVis
        , c4LastVis
        , m_areaFocusInd
    );

    //
    //  If the focus index is below this range, move it up to this
    //  range. If beyond it, move it dwn into this range.
    //
    if (m_c4FocusIndex < c4FirstVis)
        m_c4FocusIndex = c4FirstVis;
    else if (m_c4FocusIndex > c4LastVis)
        m_c4FocusIndex = c4LastVis;
}


//
//  Given the current scroll position of the top of stack list, figure out
//  the first and last visible indices and call LoadListArt() to make sure
//  that all of those list items have art loaded.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::LoadCurPageArt()
{
    // Shouldn't happen but check anyway
    if (m_colStack.bIsEmpty())
        return;

    TItemList* pilTop = m_colStack.pobjPeek();
    tCIDLib::TCard4 c4StartInd;
    tCIDLib::TCard4 c4EndInd;

    // Indicate we want any items that are visible at all
    pilTop->QueryVisIndices
    (
        m_iwdgLayout.areaActual()
        , m_c4HSpacing
        , m_c4RowCount
        , c4StartInd
        , c4EndInd
        , TArea::areaEmpty
    );

    LoadListArt(gdevCompat(), *pilTop, c4StartInd, c4EndInd);
}


//
//  Loads up the default category for our associated repository. If one was
//  set OnPreload, we use that. Else we figure one out.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::LoadDefCategory()
{
    if (m_strInitCat.bIsEmpty())
    {
        const tCQCMedia::EMTFlags eMTFlags = m_cptrMediaDB->eMTFlags();

        tCIDLib::TCard2         c2Id;
        tCQCMedia::EMediaTypes  eMType;
        TString                 strName;
        TString                 strCookie;
        const tCIDLib::TBoolean bRes = facCQCMedia().bDefCategory
        (
            m_eInitMediaType, eMTFlags, strName, m_strInitCat, eMType, c2Id
        );
    }

    // If we got the category, then let's try to load it
    if (!m_strInitCat.bIsEmpty())
    {
        // Load up a category based list
        PushCatList(m_strInitCat);

        // We can clear it now, just to be extra safe
        m_strInitCat.Clear();
    }
}


// Handles loading the art for a single list item
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::LoadItemArt( const   TGraphDrawDev&      gdevCompat
                                    ,       TListItem&          liCur
                                    ,       TString&            strKey)
{
    // REmember we've tried to load this one now
    liCur.m_bImgLoaded = kCIDLib::True;

    const tCIDLib::TBoolean bRes = facCQCIntfEng().bLoadLocalArt
    (
        gdevCompat
        , m_strRepoMoniker
        , liCur.m_strCookie
        , m_szArt
        , m_bLargeArt
        , strKey
        , liCur.m_bmpImg
        , liCur.m_szOrgImg
    );

    if (!bRes)
    {
        if (liCur.m_pmtsThis->bIsPlayList())
            liCur.m_bmpImg = m_bmpArtPlayList;
        else
            liCur.m_bmpImg = m_bmpArtMissing;
    }
}


//
//  This is called to do bulk image loading for a range of items in a
//  list. Any that aren't already loaded we try to load up and cache
//  within the list item objects. We load large or small depending on
//  what we are configured to use.
//
//  We work through a cover art cache provided by our facility class,
//  since that makes us, generally, a lot more efficient.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::LoadListArt( const   TGraphDrawDev&  gdevCompat
                                    ,       TItemList&      ilTar
                                    , const tCIDLib::TCard4 c4StartInd
                                    , const tCIDLib::TCard4 c4EndInd)
{
    // If no cover art widget was found, do nothing
    if (m_szArt.bAllZero())
        return;

    // If the list is empty, then nothing to do
    const tCIDLib::TCard4 c4ListMax = ilTar.m_colItems.c4ElemCount();
    if (!c4ListMax)
        return;

    // The end index may be beyond the actual list end
    tCIDLib::TCard4 c4RealEnd = c4EndInd;
    if (c4RealEnd >= c4ListMax)
        c4RealEnd = c4ListMax - 1;

    TString strKey;
    for (tCIDLib::TCard4 c4Index = c4StartInd; c4Index <= c4RealEnd; c4Index++)
    {
        TListItem& liCur = *ilTar.m_colItems[c4Index];

        // If we haven't set this guy's image yet
        if (!liCur.m_bImgLoaded)
            LoadItemArt(gdevCompat, liCur, strKey);
    }
}


//
//  Called during initialization to get our template loaded, and cleaned up
//  if it has stuff we can't use. If we can't downlaod it, we create a default
//  empty one, which will basically have us displaying nothing.
//
//  If we load any boolean or static image widgets, though it's a little
//  kludgey, we initialize their image (or image list) mixins, so that it
//  will load the images referenced by those widgets. Otherwise we'd have to
//  do a lot of work to manage them ourself.
//
//  It's a litte kludgey because these are not real widgets, but the mixins
//  by definition don't have much knowledge of things they are mixed into. So
//  just trying to load the images won't do anything bad.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::LoadTemplate(TDataSrvClient&             dsclToUse
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    try
    {
        TCQCIntfView& civUs = civOwner();

        // Expand the path out first in case it's relative and query the template
        TString strToLoad;
        piwdgParent()->ExpandTmplPath(m_strLayoutTmpl, strToLoad);
        facCQCIntfEng().QueryTemplate
        (
            strToLoad
            , dsclToUse
            , m_iwdgLayout
            , civUs.cuctxToUse()
            , civUs.bEnableTmplCaching()
        );

        // Initialize the focus area to empty
        m_bHavePressedInd = kCIDLib::False;
        m_areaFocusInd.ZeroAll();

        // And assume no cover art widget until we get one
        m_szArt.Zero();

        // Let's go through and remove anything that isn't kosher
        tCIDLib::TCard4 c4Count = m_iwdgLayout.c4ChildCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            TCQCIntfWidget& iwdgCur = m_iwdgLayout.iwdgAt(c4Index);

            if ((iwdgCur.clsIsA() == TCQCIntfStaticBooleanImg::clsThis())
            &&  ((iwdgCur.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_Pressed)
            ||   (iwdgCur.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_Focus)))
            {
                //
                //  These have to be named with specific names, to indicate
                //  their function. Any that don't have those names are
                //  removed. These that we keep we have to get them to load
                //  their image list.
                //
                TCQCIntfStaticBooleanImg& iwdgImg
                (
                    static_cast<TCQCIntfStaticBooleanImg&>(iwdgCur)
                );
                iwdgImg.InitImgList(civUs, dsclToUse, colErrs, *this);
                c4Index++;

                //
                //  If it's the focus area, then add it's area to the focus
                //  indicator area.
                //
                if (iwdgCur.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_Focus)
                {
                    if (m_areaFocusInd.bIsEmpty())
                        m_areaFocusInd = iwdgImg.areaActual();
                    else
                        m_areaFocusInd |= iwdgImg.areaActual();
                }
                 else
                {
                    // Remember if we have a pressed index
                    m_bHavePressedInd = kCIDLib::True;
                }
            }
             else if ((iwdgCur.clsIsA() == TCQCIntfMediaRepoImg::clsThis())
                  &&  (iwdgCur.strWidgetId() == CQCIntfEng_CvrArtBrowser::strWID_CoverArt))
            {
                // Remember if it's using large or small art
                TCQCIntfMediaRepoImg& iwdgImg
                (
                    static_cast<TCQCIntfMediaRepoImg&>(iwdgCur)
                );

                tCIDLib::TBoolean bUseLrg(iwdgImg.eAttr() == tCQCMedia::EMImgAttrs::CoverArtLrg);

                //
                //  If the art is not the same size as before, or we changed the
                //  large/small image setting, then flush the list stack.
                //
                TSize szNew = iwdgCur.areaActual().szArea();
                if ((szNew != m_szArt) || (bUseLrg != m_bLargeArt))
                    m_colStack.RemoveAll();

                m_bLargeArt = bUseLrg;

                m_szArt = szNew;
                c4Index++;
            }
             else if (iwdgCur.clsIsA() == TCQCIntfStaticImg::clsThis())
            {
                // These we need to get to load their image
                TCQCIntfStaticImg& iwdgImg
                (
                    static_cast<TCQCIntfStaticImg&>(iwdgCur)
                );
                iwdgImg.InitImage(civUs, dsclToUse, colErrs, *this);
                c4Index++;
            }
             else if ((iwdgCur.clsIsA() == TCQCIntfStaticText::clsThis())
                  ||  (iwdgCur.clsIsA() == TCQCIntfMediaRepoText::clsThis()))
            {
                //
                //  Nothing special, just keep these. We assume the repo image
                //  is the cover art.
                //
                c4Index++;
            }
             else
            {
                // Remove this one and reduce the count
                m_iwdgLayout.RemoveWidget(c4Index);
                c4Count--;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Make sure it's emptied out
        m_iwdgLayout.ResetTemplate(&civOwner());
        m_szArt.Zero();
    }
}


//
//  This is called to push a category list onto the stack. We make the
//  stack active and display it.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::PushCatList(const TString& strCatCookie)
{
    // If we haven't gotten our database yet, then see if we can now
    if (!m_cptrMediaDB.pobjData())
    {
        TString strDBSerialNum;
        facCQCMedia().eGetMediaDB
        (
            m_strRepoMoniker, strDBSerialNum, m_cptrMediaDB
        );

        // If stil not, then nothing we can do
        if (!m_cptrMediaDB.pobjData())
            return;
    }

    // Get a convenient ref to the media database
    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    // Parse the cookie to get the media type and id
    tCIDLib::TCard2 c2CatId;
    const tCQCMedia::EMediaTypes eMType
    (
        facCQCMedia().eParseCatCookie(strCatCookie, c2CatId)
    );

    // Look up the category
    const TMediaCat& mcatPush = *mdbInfo.pmcatById(eMType, c2CatId, kCIDLib::True);

    //
    //  Create a new item list and put a janitor on it until we get it
    //  stored away. We pass a bogus sort order initially, so that when
    //  we are done loading we can make it sort according to our current
    //  sort order. If it were the same order as we pass in, it would ignore
    //  the request, thinking it was already in that order.
    //
    TItemList* pilNew = new TItemList
    (
        mcatPush.strName()
        , kCIDLib::c4MaxCard
        , eMType
        , EListTypes::Cat
        , tCQCMedia::ESortOrders::Count
        , strCatCookie
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    //
    //  Iterate all of the titles of this category and find all of the
    //  ones that have the target category in their list of categories.
    //  For each one, add it to the list.
    //
    TString strTitleCookie;
    tCIDLib::TCard4 c4Count = mdbInfo.c4TitleSetCnt(eMType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaTitleSet& mtsCur = mdbInfo.mtsAt(eMType, c4Index);
        if (mdbInfo.bTitleIsInCat(mtsCur, c2CatId))
        {
            strTitleCookie = strCatCookie;
            strTitleCookie.Append(kCIDLib::chComma);
            strTitleCookie.AppendFormatted(mtsCur.c2Id(), tCIDLib::ERadices::Hex);
            pilNew->m_colItems.Add(new TListItem(&mtsCur, strTitleCookie));
        }
    }

    // And add it to the stack
    PushPopStack(janList.pobjOrphan(), m_eSortOrder);
}


//
//  This is called when a title set is clicked on and it has more than
//  one collection. We have to push a new list with these collections.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::PushColList( const   tCIDLib::TCard4 c4SelIndex
                                    ,       TListItem&      liSel)
{
    // Get a convenient ref to the media database
    const TMediaDB& mdbInfo = m_cptrMediaDB->mdbData();

    TString strTitle(L"Contents of '");
    strTitle.Append(liSel.m_pmtsThis->strName());
    strTitle.Append(L"'");
    TItemList* pilNew = new TItemList
    (
        strTitle
        , kCIDLib::c4MaxCard
        , liSel.m_pmtsThis->eType()
        , EListTypes::TitleCols
        , tCQCMedia::ESortOrders::Count
        , liSel.m_strCookie
        , 0
    );
    TJanitor<TItemList> janList(pilNew);

    // Load up the collections
    TString strColCookie;
    const tCIDLib::TCard4 c4Count = liSel.m_pmtsThis->c4ColCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCollect& mcolCur = liSel.m_pmtsThis->mcolAt(mdbInfo, c4Index);

        strColCookie = liSel.m_strCookie;
        strColCookie.Append(kCIDLib::chComma);
        strColCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);
        pilNew->m_colItems.Add
        (
            new TListItem(liSel.m_pmtsThis, &mcolCur, strColCookie)
        );
    }

    // And now push it onto the stack
    PushPopStack(janList.pobjOrphan(), m_eSortOrder);

    // If we didn't get killed by some action, then post an onselect
    if (!bIsDead())
        PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::Nested);
}


//
//  This method 'pushes' a new list onto the stack or pops the current top
//  one off, handling the scrolling in/out of the current top of stack. If
//  we get a new item, we assume a push. If the new item is null, we assume
//  a pop. The only real difference is push pushes the new top in from
//  the top and pop pushes the current top upwards out of view. And of course
//  if pushing we then add the new list to the stack.
//
//  If the stack is empty we don't do any slide on the first page.
//
//  NOTE that our m_bSafePanBgn doesn't apply here because we are scrolling
//  in the vertical direction. So we have to check separately here.
//
//  NOTE also that the above and other reasons mean we kind of have to
//  duplicate a lot of code between this and the general list scroller that
//  we use for command driven horizontal scrolling. Unfortunate, but they are
//  different enough that it would be messy to try to integrate them.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::PushPopStack(        TItemList* const        pilToPush
                                    , const tCQCMedia::ESortOrders  eSortOrder)
{
    //
    //  We need to slide it in. Put it into a janitor until we are sure
    //  we can keep it. If we are popping, this is fine. The pointer will
    //  be null and the janitor won't do anything.
    //
    TJanitor<TItemList> janList(pilToPush);

    //
    //  If pushing, make sure it's not gotten crazy deep. If so, ignore it. The
    //  new item is in the janitor and will get cleaned up.
    //
    if (pilToPush && m_colStack.bIsFull(128))
        return;

    //
    //  If pushing, and a new layout has been set, try to set it. It will clear
    //  the list whether it successfully loads the template or not. If it fails,
    //  we log and continue with what we have.
    //
    if (pilToPush)
    {
        if (m_bNewLayout)
        {
            try
            {
                strLayoutTmpl(m_strLayoutTmpl);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        //
        //  Finish the setup of the list info. This lets it do some calculations it
        //  needs to do and to sort itself by the current sort order. It couldn't do
        //  this stuff until the item list is filled in and until we've loaded any
        //  new layout.
        //
        pilToPush->FinishSetup
        (
            m_c4RowsHeight
            , m_c4RowCount
            , m_c4HSpacing
            , eSortOrder
            , m_iwdgLayout.areaActual()
            , m_areaDisplay
        );

        //
        //  Do an initial load of art, to make sure we'vegot enough to satisfy
        //  the initial display plus some leeway.
        //
        const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();

        tCIDLib::TCard4 c4LastInd = pilToPush->m_colItems.c4ElemCount();;
        if (c4LastInd > 64)
            c4LastInd = 64;
        LoadListArt(gdevCompat, *pilToPush, 0, c4LastInd - 1);

        //
        //  Set the next art index on the list to whatever we got up to in this
        //  initiall load. Our active update method will start loading from there.
        //
        pilToPush->m_c4NextArtIndex = c4LastInd;
    }

    //
    //  If stack is empty, then either just push the new one or do nothing,
    //  then redraw and send events.
    //
    if (m_colStack.bIsEmpty())
    {
        if (pilToPush)
            m_colStack.Push(janList.pobjOrphan());
        Redraw();

        // We still need to send set/scroll events
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::Nested);
        PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
        return;
    }

    // If there's not at least two items and they ask to pop, ignore it
    if (!pilToPush && (m_colStack.c4ElemCount() < 2))
        return;

    // Get a device content for multiple uses below
    tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();
    TGraphDrawDev& gdevTarget = *cptrDev.pobjData();

    //
    //  Create an image for the scrollable content. It is twice the size
    //  of the display area. We have to force this one into memory
    //
    TSize szDbl = m_areaDisplay.szArea();
    szDbl.Scale(1.0, 2.0);
    TBitmap bmpItems
    (
        szDbl, tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight
    );
    TGraphMemDev gdevItems(civOwner().gdevCompat(), bmpItems);

    // The zero based scrollable area
    TArea areaScroll(TPoint::pntOrigin, szDbl);

    //
    //  In this case we are scrolling vertically, so our m_bSafePanBgn
    //  isn't relevant. We also have the issue of insuring that we get
    //  any gradient aligned properly. So see if we are safe vertically.
    //  If so, then we can put the bgn into the scrolled content.
    //
    const tCIDLib::TBoolean bSafeBgn = bScrollSafeBgn(kCIDLib::True);

    if (bSafeBgn)
    {
        //
        //  Get our area, then set the vertical values to the doubled
        //  area. Then center it on the scroll area. We can do a fill on
        //  this and insure that it's aligned since we are filling from
        //  the actual widget edge, which may be out beyond the display
        //  area edge.
        //
        TArea areaFill(areaActual());
        areaFill.i4Y(m_areaDisplay.i4Y());
        areaFill.c4Height(szDbl.c4Height());

        areaFill.CenterIn(areaScroll);
        DoBgnFill(gdevItems, areaFill);
    }

    //
    //  Save the current focus index for the current list before we draw
    //  the new stuff below. So now we can zero the focus index and it'll
    //  make the new list come up with it drawn in as it slides in.
    //
    m_colStack.pobjPeek()->m_c4FocusIndex = m_c4FocusIndex;
    m_c4FocusIndex = 0;

    //
    //  If pushing, draw the current visible contents into the bottom,
    //  since we will be pushing it downwards. If popping, draw it into the
    //  top, since we will be pushing it upwards up off the CAB.
    //
    TArea areaTar(m_areaDisplay);
    areaTar.ZeroOrg();
    {
        TItemList* pilTop = m_colStack.pobjPeek();
        TArea areaRel(pilTop->areaAdjusted(areaTar));

        DrawList
        (
            gdevItems
            , areaRel
            , areaTar
            , pilToPush ? areaTar.pntLL(0, 1) : TPoint::pntOrigin
            , *pilTop
            , !bSafeBgn
        );
    }

    //
    //  If popping, we can now delete the top level. And, as a precaution
    //  we will check the new top to make sure we have the required art.
    //  We may have been resized while it was on the stack, and might now
    //  be larger than we were and therefore new items will show up.
    //
    if (!pilToPush)
    {
        m_colStack.TrashTop();

        //
        //  Make sure the top one has sufficient cover art. Indicate we want
        //  the indices for any that are visible at all.
        //
        TItemList& ilTop = *m_colStack.pobjPeek();
        tCIDLib::TCard4 c4FirstInd;
        tCIDLib::TCard4 c4LastInd;
        ilTop.QueryVisIndices
        (
            m_iwdgLayout.areaActual()
            , m_c4HSpacing
            , m_c4RowCount
            , c4FirstInd
            , c4LastInd
            , TArea::areaEmpty
        );
        LoadListArt(gdevTarget, ilTop, c4FirstInd, c4LastInd);

        // Bet back the previous focus index
        m_c4FocusIndex = m_colStack.pobjPeek()->m_c4FocusIndex;

        //
        //  Just in case, make sure the focus index is still visible on this
        //  list. We may have been resized while it was away.
        //
        KeepFocusVisible();
    }

    //
    //  If pushing, draw the new top into the top, since we'll be pushing
    //  down the old one and replacing with the new one. If popping, draw
    //  it into the bottom since we will be pulling it upwards, to push
    //  the old stack up off the stack.
    //
    {
        TItemList* pilTar;
        if (pilToPush)
            pilTar = pilToPush;
        else
            pilTar = m_colStack.pobjPeek();

        TArea areaRel(pilTar->areaAdjusted(areaTar));
        DrawList
        (
            gdevItems
            , areaRel
            , areaTar
            , pilToPush ? TPoint::pntOrigin : areaTar.pntLL(0, 1)
            , *pilTar
            , !bSafeBgn
        );
    }

    //
    //  We have a special case here since we are doing this scroll. We need
    //  to make sure our slot area is clipped to within all our parent's
    //  areas, since the content of the template we are in may be bigger
    //  than the overlay we are in.
    //
    TGUIRegion grnClip;
    if (!bFindMostRestrictiveClip(grnClip))
        return;

    // If pushing we scroll downwards, else up
    tCIDLib::EDirs eDir;
    if (pilToPush)
        eDir = tCIDLib::EDirs::Down;
    else
        eDir = tCIDLib::EDirs::Up;

    // If not in remote mode, slide the list in
    if (!TFacCQCIntfEng::bRemoteMode())
    {
        TRegionJanitor janClip(&gdevTarget, grnClip, tCIDGraphDev::EClipModes::And);
        TCIDEaseScroller escrSlide;
        if (bSafeBgn)
        {
            escrSlide.Scroll2
            (
                gdevTarget
                , m_areaDisplay
                , areaScroll
                , gdevItems
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
         else
        {
            //
            //  In this mode we have to provide a compositing buffer and
            //  the background content under us that has to be composited
            //  in each time.
            //
            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

            TBitmap bmpBuf
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
            TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

            escrSlide.Scroll1
            (
                gdevTarget
                , m_areaDisplay
                , areaScroll
                , gdevBuf
                , gdevBgn
                , gdevItems
                , kCQCIntfEng_::c4EaseScrollMSs
                , eDir
                , kCQCIntfEng_::c4EaseOrder
            );
        }
    }

    //
    //  If pushing, we can put it on our stack, orhpaning it out of the
    //  janitor.
    //
    if (pilToPush)
        m_colStack.Push(janList.pobjOrphan());

    // And do a final redraw
    Redraw(m_areaDisplay);

    // Send set and scroll events
    PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::Nested);
    PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
}


//
//  A helper that handles non-inertial scrolling, which we have to do when
//  we receive scroll commands.
//
tCIDLib::TVoid TCQCIntfCvrArtBrowser::ScrollList(const TString& strScrollOp)
{
    // Shouldn't be called if stack is empty, but check anyway
    if (m_colStack.bIsEmpty())
        return;

    // See if top list has enough to scroll. If not, do nothing
    TItemList* pilTop = m_colStack.pobjPeek();

    if (pilTop->m_areaContent.c4Width() <= m_areaDisplay.c4Width())
        return;

    // Remember some info for below.
    const TArea areaOrg = pilTop->m_areaAdjDisp;
    const tCIDLib::TInt4 i4Span = tCIDLib::TInt4(m_areaDisplay.c4Width());;

    if (strScrollOp == L"Home")
        pilTop->m_areaAdjDisp.i4X(0);
    else if (strScrollOp == L"Previous Page")
        pilTop->m_areaAdjDisp.AdjustOrg(-(i4Span - (i4Span >> 2)), 0);
    else if (strScrollOp == L"Next Page")
        pilTop->m_areaAdjDisp.AdjustOrg(i4Span - (i4Span >> 2), 0);
    else if (strScrollOp == L"End")
        pilTop->m_areaAdjDisp.RightJustifyIn(pilTop->m_areaContent);

    // Clip it back to the content area
    if (pilTop->m_areaAdjDisp.i4X() < pilTop->m_areaContent.i4X())
        pilTop->m_areaAdjDisp.i4X(pilTop->m_areaContent.i4X());
    if (pilTop->m_areaAdjDisp.i4Right() > pilTop->m_areaContent.i4Right())
        pilTop->m_areaAdjDisp.RightJustifyIn(pilTop->m_areaContent);

    //
    //  Redraw if the scroll offset changed. We may also need to load some
    //  cover art, so see what indices are going to be visible and load
    //  any art that isn't yet loaded in that range.
    //
    if (areaOrg != pilTop->m_areaAdjDisp)
    {
        LoadCurPageArt();

        if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
        {
            // Make sure the focus stays visible
            KeepFocusVisible();
            Redraw();
        }
         else
        {
            SlideList(*pilTop, areaOrg);
        }

        // Send set and scroll events
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::Nested);
        PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::Nested);
    }
}



//
//  When we need to move the contents in a non-drag way, generally due to
//  a scroll command from another widget, this is called. The adjusted area
//  of the list has already been changed, and we get the original, which
//  gives us what we need.
//
//  We are either dealing with a contiguous range, which we just load up
//  and scroll, or discontiguous in which case we just scroll in the new
//  contents.
//
tCIDLib::TVoid
TCQCIntfCvrArtBrowser::SlideList(TItemList& ilTop, const TArea& areaOrg)
{
    // Set up the whole area we need to scroll
    TArea areaScroll;

    // Figure out which scenario
    TArea areaFirst;
    TArea areaSec;

    TArea areaInvalid;
    tCIDLib::EDirs eDir;
    if (areaOrg.i4X() > ilTop.m_areaAdjDisp.i4X())
    {
        eDir = tCIDLib::EDirs::Right;
        areaFirst = ilTop.m_areaAdjDisp;
        areaSec = areaOrg;
    }
     else
    {
        eDir = tCIDLib::EDirs::Left;
        areaFirst = areaOrg;
        areaSec = ilTop.m_areaAdjDisp;
    }

    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TBitmap bmpScroll;
    if (areaFirst.bIntersects(areaSec)
    ||  (areaFirst.i4Right() == areaSec.i4X()))
    {
        //
        //  The areas overlap or touch, so we can do a contiguous scroll
        //  from one positon to the other.
        //
        //  The scroll area is the intersected width, and the zero origined
        //  version of the combination is the scroll area.
        //
        TArea areaSrc = areaFirst;
        areaSrc |= areaSec;
        areaScroll = areaSrc;
        areaScroll.ZeroOrg();

        // Create the scroll bitmap
        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);

        // If a safe bgn, put it into the content buffer
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);
        }

        //
        //  And draw our contiguous range into the scroll buffer. We have
        //  to create a scrolled content area to use as the 'relative to'
        //  area. This will make the bit we care about come out in our
        //  zero origined buffer.
        //
        TArea areaRelTo = ilTop.m_areaContent;
        areaRelTo -= areaSrc.pntOrg();
        DrawList
        (
            gdevCont
            , areaRelTo
            , areaScroll
            , TPoint::pntOrigin
            , ilTop
            , !m_bSafePanBgn
        );
    }
     else
    {
        //
        //  In this case, we only want to scroll in the new contents. This
        //  one is more complex. The scroll size is twice our width.
        //
        areaScroll.SetSize
        (
            m_areaDisplay.c4Width() * 2, m_areaDisplay.c4Height()
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
        //  have to redraw our on-screen content (without our buttons),
        //  because the efficient scroll will scroll the current contents
        //  and we don't want that since it would probably scroll a partial
        //  one at one end.
        //
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);

            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
            cptrGraphDev->CopyBits(gdevBgn, areaZDisp, m_areaDisplay);
        }

        //
        //  NOTE that we are flipping the directions in this case.
        //
        TArea areaRelTo;
        if (eDir == tCIDLib::EDirs::Left)
        {
            // Put the second page in the first half and scroll right
            areaRelTo = ilTop.m_areaContent;
            areaRelTo -= areaSec.pntOrg();

            DrawList
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , TPoint::pntOrigin
                , ilTop
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Right;
        }
         else
        {
            //
            //  Put the first page in the second half and scroll the opposite
            //  of our initially selected direction.
            //
            areaRelTo = ilTop.m_areaContent;
            areaRelTo -= areaFirst.pntOrg();

            DrawList
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , areaZDisp.pntUR(1, 0)
                , ilTop
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Left;
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

    // Clip, and do the scroll
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


