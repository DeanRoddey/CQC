//
// FILE NAME: CQCNedia_MediaInfoClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2003
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
//  This is the header for the CQCMedia_MediaInfoClass.cpp, which implements
//  CML level classes for accessing media data out of CQC media repository
//  drivers.
//
//  The media info class is just a wrapper around the remote interface to the repo
//  driver to do the backdoor calls required, and as of 4.4.902 we also make calls
//  to the local client service to get locally cached data.
//
//  We don't need any data in the value objects, since we just create CQC Server
//  or client service proxies as needed per call. So we don't have our own value
//  class in this case. We just need an info class and use the generic value class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCMediaInfoInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCMediaInfoInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strLocTypePath();
        static const TString& strMediaTypePath();
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMediaInfoInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCMediaInfoInfo(const TCQCMediaInfoInfo&) = delete;

        ~TCQCMediaInfoInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMediaInfoInfo& operator=(const TCQCMediaInfoInfo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        )   override;

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const override;



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TError&                 errCaught
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      We store the ids of some nested types that we create, to make
        //      it quicker to deal with them.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_c2TypeId_StrList
        //      We create vector of strings nested class since we need one
        //      for some of our parameters.
        //
        //  m_c4ErrXXX
        //      We store away the individual values for our error enum, so
        //      that we can easily us them in throws.
        //
        //  m_pmeciXXX
        //      The info objects for nested classes we create. We don't own
        //      them, the engine does. We just keep them around so that we
        //      can use them easily.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2EnumId_CookieTypes;
        tCIDLib::TCard2         m_c2EnumId_Errors;
        tCIDLib::TCard2         m_c2EnumId_LocTypes;
        tCIDLib::TCard2         m_c2EnumId_MediaTypes;
        tCIDLib::TCard2         m_c2EnumId_SortOrders;
        tCIDLib::TCard2         m_c2MethId_DefCtor;
        tCIDLib::TCard2         m_c2MethId_GetAllCatCookie;
        tCIDLib::TCard2         m_c2MethId_GetAudioFmt;
        tCIDLib::TCard2         m_c2MethId_GetCatList;
        tCIDLib::TCard2         m_c2MethId_GetColDetails;
        tCIDLib::TCard2         m_c2MethId_GetColDetails2;
        tCIDLib::TCard2         m_c2MethId_GetCookieType;
        tCIDLib::TCard2         m_c2MethId_GetCoverArt;
        tCIDLib::TCard2         m_c2MethId_GetCoverArtEx;
        tCIDLib::TCard2         m_c2MethId_GetCurRendArt;
        tCIDLib::TCard2         m_c2MethId_GetItemDetails;
        tCIDLib::TCard2         m_c2MethId_GetItemList;
        tCIDLib::TCard2         m_c2MethId_GetTitleDetails;
        tCIDLib::TCard2         m_c2MethId_GetTitleDetails2;
        tCIDLib::TCard4         m_c4ErrAccess;
        tCIDLib::TCard4         m_c4ErrCookieFmt;
        TMEngVectorInfo*        m_pmeciCardList;
        TMEngEnumInfo*          m_pmeciCookieTypes;
        TMEngEnumInfo*          m_pmeciErrors;
        TMEngEnumInfo*          m_pmeciLocTypes;
        TMEngEnumInfo*          m_pmeciMediaTypes;
        TMEngEnumInfo*          m_pmeciSortOrders;
        TMEngVectorInfo*        m_pmeciStrList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCMediaInfoInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


