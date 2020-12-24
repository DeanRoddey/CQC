//
// FILE NAME: CQCKit_ImageStoreItemOld.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/27/2003
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
//  This is the pre-1.3.4 format for the CQC image repository. As of 1.3.4
//  it is all PNG based, which is far more efficient and flexible. This class
//  is kept around so that the installer can do a conversion during the
//  upgrade. This header is not included in our facility header. Only the
//  installer will bring it in directly.
//
//  We only need to keep enough of the functionality here to read in the
//  data for conversion. Anthing not related to that has been removed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


namespace tCQCKit
{
    // -----------------------------------------------------------------------
    //  The image types that the interface repository can store
    // -----------------------------------------------------------------------
    enum class EImgTypes
    {
        None
        , Bitmap

        , Count
        , Min           = None
        , Max           = Bitmap
    };
}

EnumBinStreamMacros(tCQCKit::EImgTypes)


// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreBaseOld
//  PREFIX: isi
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCImgStoreBaseOld : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCImgStoreBaseOld();

        TCQCImgStoreBaseOld
        (
            const   TCQCImgStoreBaseOld&    isiToCopy
        );

        ~TCQCImgStoreBaseOld();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCImgStoreBaseOld& operator=
        (
            const   TCQCImgStoreBaseOld&    isiToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTransparent() const;

        tCIDLib::TCard4 c4ImageSz() const;

        tCIDLib::TCard4 c4ThumbSz() const;

        tCQCKit::EImgTypes eType() const;

        const THeapBuf& mbufThumb() const;

        const TRGBClr& rgbTransClr() const;

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ImageSz
        (
            const   tCIDLib::TCard4         c4ImageSz
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTransparent
        //      Indicates if the supports a transparency color not. Even if it
        //      does, it can be ignored, but if it doesn't then we don't want
        //      the client trying to display it that way.
        //
        //  m_szImageSz
        //      The size of the image buffer.
        //
        //  m_c4ThumbSz
        //      The size of the data in the thumb buffer.
        //
        //  m_eType
        //      The image type, so that we know how to interpret it.
        //
        //  m_mbufThumb
        //      The data for the thumb image.
        //
        //  m_rgbTransClr
        //      The transparency color for this image, if any.
        //
        //  m_strName
        //      The image name. This comes from the file name. The data server
        //      doesn't need it, but it's need to stream header info to and
        //      from the client.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bTransparent;
        tCIDLib::TCard4     m_c4ImageSz;
        tCIDLib::TCard4     m_c4ThumbSz;
        tCQCKit::EImgTypes  m_eType;
        THeapBuf            m_mbufThumb;
        TRGBClr             m_rgbTransClr;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCImgStoreBaseOld,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreItemOld
//  PREFIX: isi
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCImgStoreItemOld : public TCQCImgStoreBaseOld
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCImgStoreItemOld();

        TCQCImgStoreItemOld
        (
            const   TCQCImgStoreItemOld&    isiToCopy
        );

        ~TCQCImgStoreItemOld();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCImgStoreItemOld& operator=
        (
            const   TCQCImgStoreItemOld&    isiToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const THeapBuf& mbufImage() const;

        THeapBuf& mbufImage();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mbufImage
        //      The data for the image. The size indicator is stored in the
        //      base class, so that it can be streamed in without having to
        //      get the image data.
        // -------------------------------------------------------------------
        THeapBuf    m_mbufImage;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCImgStoreItemOld,TCQCImgStoreBaseOld)
};

#pragma CIDLIB_POPPACK


