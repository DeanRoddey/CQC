//
// FILE NAME: CQCIntfEng_ImgIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/17/2003
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
//  A number of widget types share the characteristic of displaying a user uploaded
//  image, so they implement this interface so that they can be configured via the
//  image config tab, and this mixin also helps manage the images at viewing time.
//
//  Because some widgets need to minimize overhead, they don't want to re-scale the
//  image every time. So, this guy keeps a pre-scaled version of the image as well.
//  Initially it just sets this separate at the original. If the containing widget
//  never asks to have it scaled, then we just then naturally use the original.
//
//  If they ask us to scale it, we create a separate cache item entry (but not in
//  the cache, it's just so that the drawing methods that assume a cache entry can
//  still be used) and we then return that since we've updated the pre-scaled
//  pointer.
//
//  For the general attribute querying methods, other than size, it doesn't really
//  matter which one we use.
//
// CAVEATS/GOTCHAS:
//
//  1)  The bitmaps have user data set on them, which goes to the RIVA clients, so
//      we have to be sure to copy that over if we create the pre-scaled copy, since
//      that is now the image we'll end up drawing, and hence that's the one from
//      which the user data key will get sent.
//
//  2)  For now we assume that placement type can only be changed at design time,
//      when there is no pre-scaling in use, so we don't have to worry about
//      pre-scaling being enabled if it is changed.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfImgIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfImgIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCIntfImgIntf() = delete;

        MCQCIntfImgIntf
        (
            const   tCIDLib::TBoolean       bDefaultUse
        );

        MCQCIntfImgIntf
        (
            const   MCQCIntfImgIntf&        miwdgSrc
        );

        MCQCIntfImgIntf(MCQCIntfImgIntf&&) = delete;

        virtual ~MCQCIntfImgIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MCQCIntfImgIntf& operator=
        (
            const   MCQCIntfImgIntf&        miwdgSrc
        );

        MCQCIntfImgIntf& operator=
        (
                    MCQCIntfImgIntf&&       miwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bImgHitTrans() const;

        tCIDLib::TBoolean bImgHitTrans
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSameImg
        (
            const   MCQCIntfImgIntf&        miwdgSrc
        )   const;

        tCIDLib::TBoolean bSrcAlpha() const;

        tCIDLib::TBoolean bTransparentImg() const;

        tCIDLib::TBoolean bUseImage() const;

        tCIDLib::TBoolean bUseImage
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TBitmap& bmpPreImage() const;

        const TBitmap& bmpPreMask() const;

        tCIDLib::TCard1 c1Opacity() const;

        tCIDLib::TCard1 c1Opacity
        (
             const  tCIDLib::TCard1         c1ToSet
        );

        TIntfImgCachePtr cptrImage() const;

        tCIDLib::TVoid DrawImgOn
        (
                    TGraphDrawDev&          gdevTar
            , const tCIDLib::TBoolean       bForceAlpha
            , const TArea&                  areaDrawIn
            , const TArea&                  areaInvalid
            , const tCIDLib::TInt4          i4HOffset
            , const tCIDLib::TInt4          i4VOffset
            , const tCIDLib::TBoolean       bPressed
            , const tCIDLib::TBoolean       bNoOpacity
        );

        tCIDGraphDev::EPlacement ePlacement() const;

        tCIDGraphDev::EPlacement ePlacement
        (
            const   tCIDGraphDev::EPlacement     eToSet
        );

        tCIDLib::TVoid InitImage
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TCQCIntfWidget&         iwdgThis
        );

        tCIDLib::TVoid QueryImgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
            , const tCIDLib::TBoolean       bImageOpt
        )   const;

        tCIDLib::TVoid QueryImgPath
        (
                    tCIDLib::TStrHashSet&   colToFill
            , const tCIDLib::TBoolean       bIncludeSysImgs
        )   const;

        tCIDLib::TVoid ReadInImage
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

        tCIDLib::TVoid ResetImage();

        const TString& strImageName() const;

        const TString& strImageName
        (
            const   TString&                strToSet
        );

        TSize szImgDefault() const;

        const TSize& szImage() const;

        const TSize& szImgPrescaled() const;

        tCIDLib::TVoid SetImgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetNewImage
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
            , const TString&                strToSet
            , const tCIDLib::TBoolean       bForce
        );

        tCIDLib::TVoid SetImgPrescale
        (
            const   TCQCIntfView&           civOwner
            , const tCIDLib::TBoolean       bState
            , const TSize&                  szPrescale
        );

        tCIDLib::TVoid UpdateImage
        (
            const   TCQCIntfView&           civOwner
            ,       TDataSrvClient&         dsclInit
        );

        tCIDLib::TVoid ValidateImage
        (
                    tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
            , const TCQCIntfWidget&         iwdgThis
        )   const;

        tCIDLib::TVoid WriteOutImage
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasScalingPlacement() const;

        tCIDLib::TVoid DoPrescale
        (
            const   TCQCIntfView&           civOwner
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDefaultUse
        //      The default use image state to set if we are reset. The derived
        //      class must provide this to us during ctor. We don't persist it.
        //
        //  m_bHitTrans
        //      Indicates whether we should be transparent to hit testing or
        //      not. Defaults to not if the user doesn't explicitly change it.
        //
        //  m_bPrescale
        //      If the derived class asks us to pre-scale, we set this and store
        //      the prescaled size to use. If this is not set, any update of the
        //      original image is just taked as is as the pre-scaled version. If
        //      this is set, we pre-scale the original one.
        //
        //  m_bUseImage
        //      Some users of this mixin make the image usage an option, so
        //      we have a flag to keep up with whether it's used. The config
        //      tab for us allows them to tell us whether to show the check
        //      box for this or not.
        //
        //  m_bmpPreImage
        //  m_bmpPreMask
        //      The pre-scaled image and mask. These are initially just pointed at
        //      the cache item's images. If the containing widget asks for pre-scaling
        //      then new images are created and stored here.
        //
        //  m_c1Opacity
        //      Indicate any constant alpha blend amount that should be
        //      used. 0 is transparent, 255 is fully opaque.
        //
        //  m_cptrImg
        //      A counted pointer reference to the image cache item that holds our
        //      image. This insures we have a link back to the cached original.
        //
        //  m_ePlacement
        //      If the background style indicates an image, this holds the
        //      placement options for the image.
        //
        //  m_strImageName
        //      If the bgn style indicates that the bgn is a custom image,
        //      then this is the name of the image. This name can be used to
        //      download the image from the data server.
        //
        //  m_szImage
        //  m_szPrescaled
        //      The size of the original image, and of the pre-scaled image. If
        //      the containing widget never asked us to create a pre-scaled version,
        //      then they will be the same.
        //
        //  m_szPreTarget
        //      The target size for pre-scaling. m_szPrescaled is the actual resulting
        //      prescaled size. This is the target size the contining widget wants
        //      the images to be scaled into.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bDefaultUse;
        tCIDLib::TBoolean           m_bHitTrans;
        tCIDLib::TBoolean           m_bPrescale;
        tCIDLib::TBoolean           m_bUseImage;
        TBitmap                     m_bmpPreImage;
        TBitmap                     m_bmpPreMask;
        tCIDLib::TCard1             m_c1Opacity;
        TIntfImgCachePtr            m_cptrImage;
        tCIDGraphDev::EPlacement    m_ePlacement;
        TString                     m_strImageName;
        TSize                       m_szImage;
        TSize                       m_szPrescaled;
        TSize                       m_szPreTarget;
};

#pragma CIDLIB_POPPACK


