//
// FILE NAME: CQCMedia_StdRepoDrvEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/14/2012
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
//  This is the header for the CQCMedia_StdRepoDrvEng.cpp file. This file
//  implements a class that all standard repository drivers can use to provide
//  pretty much all of the standard functionality required by media repos
//  drivers.
//
//  The driver provides the (repo specific) functionality required to load up
//  the database. Then it can just forward the backdoor driver methods (used
//  by clients to query media info) to this class for processing. Thsi facility
//  provides a standard repository driver bass class that does this forwarding
//  for them, and they can just derive from it.
//
//  If a driver doesn't want the standard, file based, cover art loading,
//  then they have to override c4LoadCoverArt(). If not, we provide a default
//  implementation that does loading from a file, and also handles some
//  nice scaling of art and such.
//
//  Generally drivers us this class, but it's possible that some other programs
//  might want to do as well. There's not that driver specific about it other
//  than the fact that they query methods are in the form of the driver
//  backdoor methods, to make it easy for drivers to pass forward those calls
//  to us.
//
// CAVEATS/GOTCHAS:
//
//  1)  It is assumed that the derived provides any synchronization to this
//      class as required. Generally drivers use this class and they
//      automatically would synchronize access between the driver thread and
//      incoming remote client request threads.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCStdMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCStdMediaRepoEng : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStdMediaRepoEng
        (
            const   TString&                strMoniker
            , const tCQCMedia::EMTFlags     eSupportedTypes
        );

        TCQCStdMediaRepoEng(const TCQCStdMediaRepoEng&) = delete;
        TCQCStdMediaRepoEng(TCQCStdMediaRepoEng&&) = delete;

        ~TCQCStdMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdMediaRepoEng& operator=(const TCQCStdMediaRepoEng&) = delete;
        TCQCStdMediaRepoEng& operator=(TCQCStdMediaRepoEng&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCookieLookup
        (
            const   TString&                strQData
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bQueryAudioFmt
        (
            const   TString&                strCookie
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bQueryCoverArt
        (
            const   TString&                strCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
            , const tCQCMedia::EArtLevels   Level
            , const tCQCMedia::ERArtTypes   eType
        );

        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4LoadArtData
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
            , const tCIDLib::TCard4         c4AtIndex
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        );

        tCIDLib::TVoid CheckType
        (
            const   tCQCMedia::EMediaTypes  eType
            ,       tCIDLib::TCard4         c4Line
        );

        tCQCMedia::EMTFlags eMediaTypes() const;

        const TMediaDB& mdbInfo() const;

        TMediaDB& mdbInfo();

        tCIDLib::TVoid QueryCatList
        (
                    tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TVoid QueryColDetails
        (
            const   TString&                strColCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid QueryColItems
        (
            const   TString&                strColCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid QueryColLoc
        (
            const   TString&                strColCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid QueryDefCatInfo
        (
                    tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TVoid QueryItemDetails
        (
            const   TString&                strItemCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid QueryItemList
        (
            const   TString&                strColCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid QueryMediaFormat
        (
            const   TString&                strCookie
            ,       TString&                strToFill
        );

        tCIDLib::TVoid QueryRandomCatItems
        (
                    tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TVoid QuerySingleFld
        (
            const   TString&                strCookie
            , const TString&                strToGet
            ,       TString&                strToFill
        );

        tCIDLib::TVoid QueryTitleDetails
        (
            const   TString&                strTitleCookie
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid ResetMediaDB();

        tCIDLib::TVoid ResetMediaDB
        (
                    TMediaDB&               mdbToLoad
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eTypes
        //      The derived class passes us the list of support media types
        //      that his repository provides. This allows us to do some
        //      validation on requests.
        //
        //  m_mdbInfo
        //      The in-memory database that we manage on behalf of the derived
        //      class. He accesses it directly in plenty of cases during the
        //      load. But after that, he mostly just uses the above methods
        //      to query data from it.
        //
        //  m_mbufTmp
        //  m_mbufTmp2
        //      Temp bufs, mostly used for loading images. The callbacks are
        //      synchronized by the driver, so it's safe to use this to help
        //      make things more efficient.
        //
        //  m_strMissingXXX
        //      Strings that we preload and return as the text of things that
        //      no longer exist but are referenced. This is possible for
        //      collections and items since they are referenced indirectly.
        //
        //  m_strMoniker
        //      The driver who creates us gives us the moniker so we can use
        //      it error messages.
        // -------------------------------------------------------------------
        tCQCMedia::EMTFlags m_eTypes;
        TMediaDB            m_mdbInfo;
        THeapBuf            m_mbufTmp;
        THeapBuf            m_mbufTmp2;
        TString             m_strMissingCol;
        TString             m_strMissingItem;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdMediaRepoEng,TObject)
};

#pragma CIDLIB_POPPACK


