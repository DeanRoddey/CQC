//
// FILE NAME: CQCIntfEng_ImgListIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2003
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
//  A number of widget types share the characteristic of displaying one of
//  a number of user uploaded images, according to the value of the widget
//  (usually the value of an associated field.) They mix in this interface
//  and tell us to display a particular image via a key value which each
//  image is associated with.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfImgListIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  We need a small class to hold info about each image. We have a
        //  key string for each image, the image name, and a ref to the image
        //  in the cache.
        // -------------------------------------------------------------------
        class CQCINTFENGEXPORT TImgInfo
        {
            public :
                // -----------------------------------------------------------
                //  Public, static methods
                // -----------------------------------------------------------
                static const TString& strKey(const TImgInfo& imgiSrc)
                {
                    return imgiSrc.m_strKey;
                }


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TImgInfo();

                TImgInfo
                (
                    const   TString&                strKey
                    , const TString&                strImageName
                    , const tCQCIntfEng::EImgReqFlags eReqFlag
                    , const tCIDLib::TCard1         c1Opacity
                    , const tCIDLib::TBoolean       bEnabled
                );

                TImgInfo
                (
                    const   TString&                strKey
                    , const tCQCIntfEng::EImgReqFlags eReqFlag
                    , const tCIDLib::TCard1         c1Opacity
                    , const tCIDLib::TBoolean       bEnabled
                );

                TImgInfo
                (
                    const   TImgInfo&               imgiToCopy
                );

                ~TImgInfo();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                tCIDLib::TVoid operator=
                (
                    const   TImgInfo&               imgiSrc
                );

                tCIDLib::TBoolean operator==
                (
                    const   TImgInfo&               imgiSrc
                )   const;

                tCIDLib::TBoolean operator!=
                (
                    const   TImgInfo&               imgiSrc
                )   const;


                // -----------------------------------------------------------
                //  Public, non-virutal methods
                // -----------------------------------------------------------
                tCIDLib::TVoid Reset();


                // -----------------------------------------------------------
                //  Public data members
                // -----------------------------------------------------------
                tCIDLib::TBoolean           m_bEnabled;
                tCIDLib::TCard1             m_c1Opacity;
                TIntfImgCachePtr            m_cptrImg;
                tCQCIntfEng::EImgReqFlags   m_eReqFlag;
                tCIDLib::TInt4              m_i4HOffset;
                tCIDLib::TInt4              m_i4VOffset;
                TString                     m_strImageName;
                TString                     m_strKey;
        };

        using TImgList      = TVector<TImgInfo>;
        using TImgCursor    = TImgList::TCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCIntfImgListIntf();

        MCQCIntfImgListIntf
        (
            const   MCQCIntfImgListIntf&    miwdgToCopy
        );

        virtual ~MCQCIntfImgListIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   MCQCIntfImgListIntf&    miwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TArea areaImgBounds() const;

        tCIDLib::TVoid AddNewImgKey
        (
            const   TString&                strKey
            , const tCQCIntfEng::EImgReqFlags eReqFlag
            , const tCIDLib::TCard1         c1Opacity = 255
            , const tCIDLib::TBoolean       bEnabled = kCIDLib::False
            , const tCIDLib::TCh* const     pszImagePath = 0
        );

        tCIDLib::TBoolean bAnyActiveKeys() const;

        tCIDLib::TBoolean bDeleteImgKey
        (
            const   TString&                strKey
        );

        tCIDLib::TBoolean bImgEnabled
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TBoolean bImgTransparent
        (
            const   TString&                strKey
        );

        tCIDLib::TBoolean bSameImgList
        (
            const   MCQCIntfImgListIntf&    miwdgSrc
        )   const;

        const TBitmap& bmpImage
        (
            const   TString&                strKey
        )   const;

        const TBitmap& bmpMask
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TCard4 c4ImgKeyCount() const;

        TImgCursor cursImgList() const;

        tCIDLib::TVoid DeleteAllImgKeys();

        tCIDLib::TVoid DisableAllImgs();

        tCIDLib::TVoid EnableImgKey
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bNewState
        );

        const TImgInfo& imgiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TImgInfo& imgiAt
        (
            const   tCIDLib::TCard4         c4At
        );

        TImgInfo& imgiForKey
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid InitImgList
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TCQCIntfWidget&         iwdgThis
        );

        tCIDLib::TVoid LoadNewImgKeys
        (
            const   TCollection<TString>&   colNewKeys
        );

        TImgInfo* pimgiFirstKey();

        TImgInfo* pimgiForKey
        (
            const   TString&                strKey
        );

        TImgInfo* pimgiForKeyEO
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid QueryImgListAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid QueryImgPaths
        (
                    tCIDLib::TStrHashSet&   colToFill
            , const tCIDLib::TBoolean       bIncludeSysImgs
        )   const;

        tCIDLib::TVoid ReadInImgList
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid ReplaceImage
        (
            const   TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        TSize szImgDefault() const;

        TSize szLargestImg() const;

        tCIDLib::TVoid ScaleImgList
        (
            const   tCIDLib::TFloat8        f8XScale
            , const tCIDLib::TFloat8        f8YScale
        );

        tCIDLib::TVoid SetImgListAttr
        (
            const   TCQCIntfView&           civOwner
            ,       TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetImgKey
        (
            const   TString&                strKey
            , const TString&                strImage
            , const tCQCIntfEng::EImgReqFlags eReqFlag
            , const tCIDLib::TCard1         c1Opacity
            , const tCIDLib::TBoolean       bEnabled
        );

        tCIDLib::TVoid SetImgOpacity
        (
            const   TString&                strKey
            , const tCIDLib::TCard1         c1Opacity
        );

        tCIDLib::TBoolean bTakeMatchingImgKeys
        (
            const   MCQCIntfImgListIntf&    miwdgSrc
        );

        tCIDLib::TVoid UpdateAllImgs
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
        );

        tCIDLib::TVoid UpdateImg
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            , const TString&                strKey
            , const TString&                strToSet
        );

        tCIDLib::TVoid UpdateImg
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            , const TString&                strKey
            , const TString&                strToSet
            , const tCIDLib::TCard1         c1Opacity
        );

        tCIDLib::TVoid ValidateImgList
        (
                    tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
            , const TCQCIntfWidget&         iwdgThis
        )   const;

        tCIDLib::TVoid WriteOutImgList
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadImgInfo
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            ,       TImgInfo&               imgiToLoad
        );

        TImgInfo* pimgiFindByKey
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        const TImgInfo* pimgiFindByKey
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colList
        //      Our list of image info objects.
        // -------------------------------------------------------------------
        TImgList    m_colList;
};

#pragma CIDLIB_POPPACK


