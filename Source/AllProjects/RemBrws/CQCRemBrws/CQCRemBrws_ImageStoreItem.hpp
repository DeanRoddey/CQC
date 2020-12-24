//
// FILE NAME: CQCRemBrws_ImageStoreItem.hpp
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
//  The data server has an image repository that lets the user store images there for
//  use in user drawn interfaces or web content via our own web server (and possibly other
//  features in future.)
//
//  THIS CLASS is now legacy and is just kept around in order to convert old data. It
//  has been stripped down to just the data members and ability to read in the data.
//  As of 5.3.903 we moved to a consistent storage format for data server resources,
//  based on our new TChunkedFile format.
//
//  There's no need for them to be public anymore either, so they are no longer
//  exported.
//
// CAVEATS/GOTCHAS:
//
//  1.  In the pre-1.3.4 versions of CQC, the repository format was bitmap based instead
//      of PNG, so it was considerably different. That's so long ago that we have dropped
//      the conversion code. That would require an intermediate version upgrade. It's
//      not really likely anyone would even try that at this point.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreBase
//  PREFIX: isi
// ---------------------------------------------------------------------------
class TCQCImgStoreBase : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCImgStoreBase();

        TCQCImgStoreBase(const TCQCImgStoreBase&) = delete;
        TCQCImgStoreBase(TCQCImgStoreBase&&) = delete;

        ~TCQCImgStoreBase();

        TCQCImgStoreBase& operator=(const TCQCImgStoreBase&) = delete;
        TCQCImgStoreBase& operator=(TCQCImgStoreBase&&) = delete;

        // -------------------------------------------------------------------
        //  Just public methods since this class is only used for conversion now
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4SerialNum;
        tCIDImage::EBitDepths   m_eOrgDepth;
        tCIDImage::EPixFmts     m_eOrgFmt;
        tCIDLib::TCard4         m_c4ImageSz;
        tCIDLib::TCard4         m_c4ThumbSz;
        THeapBuf                m_mbufThumb;
        TString                 m_strName;
        TSize                   m_szOrgImg;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;



    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCImgStoreBase,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreItem
//  PREFIX: isi
// ---------------------------------------------------------------------------
class TCQCImgStoreItem : public TCQCImgStoreBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCImgStoreItem();

        TCQCImgStoreItem(const TCQCImgStoreItem&) = delete;
        TCQCImgStoreItem(TCQCImgStoreItem&&) = delete;

        ~TCQCImgStoreItem();

        TCQCImgStoreItem& operator=(const TCQCImgStoreItem&) = delete;
        TCQCImgStoreItem& operator=(TCQCImgStoreItem&&) = delete;


        // -------------------------------------------------------------------
        //  Just public class members now since this class is no longer used but
        //  for conversion.
        // -------------------------------------------------------------------
        THeapBuf    m_mbufImage;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   final;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCImgStoreItem,TCQCImgStoreBase)
};

#pragma CIDLIB_POPPACK


