//
// FILE NAME: CQCIntfEng_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2002
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
//  This file defines facility class for this facility. We provide various helper
//  methods and wrappers around remote calls to both make life easier for other
//  code in this facility and to centralize things to facilitate caching in some
//  cases, for templates and images mostly.
//
//  We provide a number of image drawing helpers that reduce redundancy in the
//  various widget classes.
//
//  We maintain a common polling engine that all widgets (and all views when in
//  the RIVA server) will use, so that we maximize efficiency for getting field
//  data.
//
//  We also maintain a list of all widgets, their descriptive names, and their
//  class types. This allows for dynamic creation and also we provide the info
//  to fill in a menu for the designer, so that it can present the available
//  widget types without having to specific knowledge of them all.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TMArtCache;


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfEng
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TFacCQCIntfEng : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TTypeList = TVector<TKeyValuePair>;


        // -------------------------------------------------------------------
        //  Public, static method
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bDesMode()
        {
            return s_bDesMode;
        }

        static tCIDLib::TBoolean bDesMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        static tCIDLib::TBoolean bRemoteMode()
        {
            return s_bRemoteMode;
        }

        static tCIDLib::TBoolean bRemoteMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        static tCIDLib::TBoolean bWebRIVAMode()
        {
            return s_bWebRIVAMode;
        }

        static tCIDLib::TBoolean bWebRIVAMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        static tCIDLib::TBoolean bWidgetHPosComp
        (
            const   TCQCIntfWidget&         iwdg1
            , const TCQCIntfWidget&         iwdg2
        );

        static tCIDLib::TBoolean bWidgetVPosComp
        (
            const   TCQCIntfWidget&         iwdg1
            , const TCQCIntfWidget&         iwdg2
        );

        static tCIDLib::TVoid MakeArtDBKey
        (
            const   TString&                strRepo
            , const TString&                strCookie
            , const tCIDLib::TBoolean       bLarge
            ,       TString&                strKey
        );

        static tCIDLib::TVoid MakeArtPIDKey
        (
            const   TString&                strRepo
            , const TString&                strPerId
            , const tCIDLib::TBoolean       bLarge
            ,       TString&                strKey
        );

        static const TString& strErrText();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIntfEng();

        TFacCQCIntfEng(const TFacCQCIntfEng&) = delete;
        TFacCQCIntfEng(TFacCQCIntfEng&&) = delete;

        ~TFacCQCIntfEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCIntfEng& operator=(const TFacCQCIntfEng&) = delete;
        TFacCQCIntfEng& operator=(TFacCQCIntfEng&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCachedWebImage
        (
            const   TString&                strURL
            ,       TBitmap&                bmpToSet
            ,       TString&                strServerStamp
            ,       tCIDLib::TBoolean&      bExpired
        );

        tCIDLib::TBoolean bFindImage
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bThumb
            ,       TIntfImgCachePtr&       cptrToFill
        );

        tCIDLib::TBoolean bFindLocalArt
        (
            const   TString&                strPIDKey
            ,       TMemBuf&                mbufToFill
            ,       tCIDLib::TCard4&        c4Size
        );

        tCIDLib::TBoolean bFindTarContainer
        (
                    TCQCIntfContainer&      iwdgStart
            , const TString&                strPath
            ,       tCIDLib::TBoolean&      bIsAncestor
            ,       TCQCIntfContainer*&     piwdgTar
        );

        tCIDLib::TBoolean bLoadLocalArt
        (
            const   TGraphDrawDev&          gdevCompat
            , const TString&                strRepoMon
            , const TString&                strCookie
            , const TSize&                  szTar
            , const tCIDLib::TBoolean       bLarge
            ,       TString&                strDBKey
            ,       TBitmap&                bmpToSet
            ,       TSize&                  szOrg
        );

        tCIDLib::TBoolean bIsValidWdgArea
        (
            const   TArea&                  areaParent
            , const TArea&                  areaToCheck
            ,       TArea&                  areaFixed
        )   const;

        tCIDLib::TBoolean bReadField
        (
                    TCQCIntfView&           civCaller
            , const TString&                strMoniker
            , const TString&                strField
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bRefreshImgCache
        (
                    TGraphDrawDev&          gdevCompat
            , const TCQCSecToken&           sectUser
        );

        const TIntfImgCachePtr& cptrEmptyImg() const;

        const TIntfImgCachePtr& cptrEmptyThumbImg() const;

        TIntfImgCachePtr cptrGetImage
        (
            const   TString&                strName
            , const TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
        );

        TIntfImgCachePtr cptrGetThumb
        (
            const   TString&                strName
            , const TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
        );

        TTypeList::TCursor cursTypes() const;

        tCIDLib::TVoid CacheWebImage
        (
            const   TString&                strURL
            , const tCIDLib::TCard4         c4Bytes
            ,       THeapBuf&&              mbufData
            , const TGraphDrawDev&          gdevCompat
            ,       tCIDLib::TCard4&        c4NewSerialNum
        );

        tCIDLib::TVoid DrawImage
        (
                    TGraphDrawDev&          gdevTar
            , const tCIDLib::TBoolean       bForceAlpha
            , const TIntfImgCachePtr&       cptrToDraw
            , const TArea&                  areaDrawIn
            , const TArea&                  areaInvalid
            ,       TArea&                  areaSrc
            , const tCIDGraphDev::EPlacement ePlacement
            , const tCIDLib::TInt4          i4HOffset
            , const tCIDLib::TInt4          i4VOffset
            , const tCIDLib::TCard1         c1ConstOpacity
            , const tCIDLib::TBoolean       bPressed = kCIDLib::False
        );

        tCIDLib::TVoid DrawImage
        (
                    TGraphDrawDev&          gdevTar
            , const tCIDLib::TBoolean       bForceAlpha
            , const TIntfImgCachePtr&       cptrToDraw
            , const TArea&                  areaDrawIn
            , const TArea&                  areaInvalid
            , const tCIDGraphDev::EPlacement ePlacement
            , const tCIDLib::TInt4          i4HOffset
            , const tCIDLib::TInt4          i4VOffset
            , const tCIDLib::TCard1         c1ConstOpacity
            , const tCIDLib::TBoolean       bPressed = kCIDLib::False
        );

        tCIDLib::TVoid DrawImage
        (
                    TGraphDrawDev&          gdevTar
            , const tCIDLib::TBoolean       bForceAlpha
            , const TIntfImgCachePtr&       cptrToDraw
            , const TArea&                  areaDrawIn
            , const TArea&                  areaInvalid
            , const tCIDGraphDev::EPlacement ePlacement
            , const tCIDLib::TCard1         c1ConstOpacity
            , const tCIDLib::TBoolean       bPressed = kCIDLib::False
        );

        tCIDLib::TVoid DrawImage
        (
                    TGraphDrawDev&          gdevTar
            , const tCIDLib::TBoolean       bForceAlpha
            , const TIntfImgCachePtr&       cptrToDraw
            , const TArea&                  areaDrawIn
            , const TArea&                  areaInvalid
            , const tCIDLib::TInt4          i4HOffset
            , const tCIDLib::TInt4          i4VOffset
            , const tCIDLib::TCard1         c1ConstOpacity
            , const tCIDLib::TBoolean       bPressed = kCIDLib::False
        );

        tCIDLib::TVoid DropImgRef
        (
                    TIntfImgCachePtr&       cptrToReset
        );

        tCQCIntfEng::EImgQRes eGetImageData
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bThumb
            , const tCQCIntfEng::EImgTypes  eType
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TMemBuf&                mbufToFill
            ,       tCIDLib::TCard4&        c4Size
        );

        tCIDLib::TVoid FillNewMenu
        (
                    TSubMenu&               menuTarget
            , const tCIDLib::TCard4         c4FirstId
        );

        TCQCIntfWidget* piwdgFindByNamePath
        (
            const   TString&                strPath
            ,       TCQCIntfContainer&      iwdgStart
        );

        TCQCIntfWidget* piwdgMakeInstanceOf
        (
            const   TString&                strUserTypeName
        )   const;

        TCQCPollEngine& polleThis();

        tCIDLib::TVoid QueryTemplate
        (
            const   TString&                strName
            ,       TDataSrvClient&         dsclInit
            ,       TCQCIntfTemplate&       iwdgToFill
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TBoolean       bCacheEnabled
        );

        tCIDLib::TVoid QueryTemplate
        (
            const   TString&                strName
            ,       TCQCIntfTemplate&       iwdgToFill
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TBoolean       bCacheEnabled
            , const tCIDLib::TCard4         c4WaitFor = 8000
        );

        tCIDLib::TVoid RefreshWebImage
        (
            const   TString&                strURL
        );

        TString strGenWidgetId
        (
            const   TCQCIntfContainer&      iwdgCont
            , const TString&                strBaseName
        );

        const TString& strTitle() const;

        tCIDLib::TVoid ScaleTemplate
        (
                    TGraphDrawDev&          gdevTmp
            ,       TCQCIntfTemplate&       iwdgToScale
            , const tCIDLib::TFloat8        f8XScale
            , const tCIDLib::TFloat8        f8YScale
        );

        tCIDLib::TVoid StdBgnFill
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaFill
            , const tCQCIntfEng::EBgnStyles eStyle
            , const TRGBClr&                rgbBgn1
            , const TRGBClr&                rgnBgn2
        );

        tCIDLib::TVoid StartEngine
        (
                    TCQCPollEngine* const   ppolleToUse
        );

        tCIDLib::TVoid StopEngine();

        tCIDCtrls::TWndId widNext();

        tCIDLib::TVoid WriteField
        (
                    TCQCIntfView&           civCaller
            , const TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        TIntfImgCachePtr cptrNewCacheItem
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ImageSz
            , const tCIDLib::TBoolean       bThumb
        );

        tCIDLib::TVoid FaultInTypeList() const;

        tCIDLib::TVoid FlushTmplCache();


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_c4EmptyImgSz
        //      The number of byte sin m_mbufEmptyImg.
        //
        //  m_colTypes
        //      This is a list of the widget types. Its a list of key/value
        //      pairs, where the key is the class name and the value is the
        //      human consumable widget type name. It is faulted in upon
        //      first use, so it is mutable. This is used to do things like
        //      proivde the list of widget types/names to the designer app
        //      so he can present a menu to the user. FaultInTypeList() is
        //      the method that faults this list in.
        //
        //  m_cptrEmptyImg
        //  m_cptrEmptyThumbImg
        //      In addition to setting up m_mbufEmptyImg to use as the data for any images
        //      we can't download, we set force in content for the /System/Blank image so
        //      that it is always available. It's a real image in the repo, but we know what
        //      it is and so we can just use the empty image info. We put refs to these into
        //      the image cache as well, so we don't ever need to really download it if it
        //      is used explicitly.
        //
        //  m_imgMissingArt
        //  m_imgPLArt
        //      The user can provide a cover art image in the image repo for us to use
        //      for playlists or for missing art. We fault these in upon first use, and
        //      will go back to the repo periodically if we haven't already successfully
        //      found any images.
        //
        //  m_mbufEmptyImg
        //      We create an empty image (1x1, alpha transparent) and flatten it out
        //      to this buffer. Any time we fail to be able to download an image, we
        //      set the content to this. Later, it'll be checked again when it's cache
        //      time runs out. If it's available then, then the real stuff can be gotten.
        //
        //  m_mtxSync
        //      A mutex used to synchronize access to the caches. This is more
        //      an issue in the remote viewer server, which runs multiple instances
        //      of views in a non-GUI based environment.
        //
        //  m_pollThis
        //      This is our polling engine that all display windows in the
        //      current process will use as their underlying data cache and
        //      polling engine. The containing application provides this since it
        //      will often have other code that uses one and it's not efficient to
        //      have more than one going. It should not have any filtering of fields
        //      set on it since we can potentially need to access any fields.
        //
        //  m_strTitle
        //      A title string that we set up during ctor and use as the
        //      title for any interface viewer windows we create. This title
        //      is used in any error popups done by the viewer window.
        //
        //  m_widNext
        //      For assigning default widget ids when we are in viewing time. At that
        //      time we need every widget to have an ID so we assigne one that is only
        //      used in the viewing engine, and use this as a running id value.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4EmptyImgSz;
        mutable TTypeList   m_colTypes;
        TIntfImgCachePtr    m_cptrEmptyImg;
        TIntfImgCachePtr    m_cptrEmptyThumbImg;
        THeapBuf            m_mbufEmptyImg;
        TBitmapImage        m_imgMissingArt;
        TBitmapImage        m_imgPLArt;
        TMutex              m_mtxSync;
        TCQCPollEngine*     m_ppolleThis;
        TString             m_strTitle;
        tCIDCtrls::TWndId   m_widNext;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  s_bDesMode
        //      This facility is used by both the runtime viewer engine and
        //      the designer. There are various places where we need to
        //      do different things accordingly. So the hosting application
        //      will set this to tell us what mode we are in. Everyone in
        //      this facility checks here. Actually it defaults to false
        //      so only the designer has to proactively set it.
        //
        //  s_bRemoteMode
        //      The remote viewer engine set this, and it lets us handle some
        //      special case issues for remote viewing. Some of it is done
        //      internally here, and some widgets will look at this flag and turn
        //      off some special functionality. It's also used in licensing, since
        //      we have different limits on IV and RIVA clients.
        //
        //  s_bSimulatorMode
        //      We support a simulator mode, to allow for portable sales demos. It
        //      is set via a command line switch on the interface viewer and
        //      designer, who will call our SetSimMode() method.
        //
        //  s_bWebRIVAMode
        //      For the short term, we have to distinguish (when s_bRemoteMode is
        //      true) between the older RIVA clients and the new WebRIVA client. Later
        //      we can get rid of this, but for a while we have to support them
        //      both and the latter can do things the older ones don't understand.
        //
        //  s_pcolArtCache
        //      This is our cover art cache. We fault this one in upon first
        //      use.
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean    s_bDesMode;
        static tCIDLib::TBoolean    s_bRemoteMode;
        static tCIDLib::TBoolean    s_bSimulatorMode;
        static tCIDLib::TBoolean    s_bWebRIVAMode;
        static TMArtCache*          s_pcolArtCache;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIntfEng,TFacility)
};

#pragma CIDLIB_POPPACK

