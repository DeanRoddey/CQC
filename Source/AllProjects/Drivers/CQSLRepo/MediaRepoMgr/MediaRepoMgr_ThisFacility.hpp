//
// FILE NAME: TFacMediaRepoMgr_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2005
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
//  This file defines facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TMainFrameWnd;


// ---------------------------------------------------------------------------
//   CLASS: TFacMediaRepoMgr
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacMediaRepoMgr : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacMediaRepoMgr();

        ~TFacMediaRepoMgr();


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDownloadDB
        (
                    tMediaRepoMgr::TRepoMgrProxyPtr& orbcRepo
            ,       TMediaDB&               mdbToFill
            ,       tCQCMedia::EMTFlags&    eMTFlags
            ,       TString&                strDBSerNum
            ,       TString&                strRepoPath
        );

        tCIDLib::TBoolean bExtractImg
        (
                    THTTPClient&            httpcGet
            , const TString&                strURL
            ,       TJPEGImage&             imgToFill
            ,       TMemBuf&                mbufTmp
        );

        tCIDLib::TBoolean bGetHTMLText
        (
                    THTTPClient&            httpcGet
            , const TURL&                   urlSrc
            ,       TString&                strToFill
            ,       tCIDLib::TCard4&        c4TextLen
            ,       TMemBuf&                mbufTmp
        );

        tCIDLib::TBoolean bMakeUniqueId
        (
            const   TString&                strPath
            , const TKrnlRemMedia::EMediaTypes eMType
            ,       TKrnlRemMedia::TCDTOCInfo& TOCData
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bScrapeAAOData
        (
            const   TString&                strAlbum
            ,       TJPEGImage&             imgiToFill
        );

        tCIDLib::TBoolean bScrapeDCSData
        (
            const   tCIDLib::TBoolean       bIsMusic
            , const TString&                strArtist
            , const TString&                strAlbum
            ,       TJPEGImage&             imgiToFill
        );

        tCIDLib::TBoolean bScrapeMSData
        (
            const   TString&                strURL
            ,       TJPEGImage&             imgiToFill
        );

        tCIDLib::TVoid BuildUploadMediaPath
        (
            const   tCQCMedia::EMediaTypes  eMType
            , const tCQCMedia::EDataTypes   eDType
            , const tCIDLib::TCard2         c2Id
            , const tCIDLib::TCard4         c4TrackNum
            , const tCIDLib::TCh* const     pszFileExt
            ,       TPathStr&               pathToFill
        );

        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tMediaRepoMgr::TRepoMgrProxyPtr orbcMakeProxy
        (
            const   TString&                strRepoMoniker
        );

        const TString& strRipDir() const;

        const TString& strUserName() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TFacMediaRepoMgr(const TFacMediaRepoMgr&);
        tCIDLib::TVoid operator=(const TFacMediaRepoMgr&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoLogon();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cuctxToUse
        //      After login, we set up a user context that we have to pass to
        //      anything that requires user credentials.
        //
        //  m_pathRiDir
        //      The path where we store ripped data until we can upload it. We just
        //      pre-build the path (which is fixed, and in our local CQCData dir)
        //      upon startup.
        //
        //  m_pwndMainFrame
        //      A pointer to the main frame window we create.
        //
        //  m_uaccLogin
        //      The user account info for the user who logged in. There is nothing
        //      unsafe in here. Those fields are not returned from the security
        //      server. We want it for things like logon time limits and such.
        // -------------------------------------------------------------------
        TCQCUserCtx         m_cuctxToUse;
        TPathStr            m_pathRipDir;
        TMainFrameWnd*      m_pwndMainFrame;
        TCQCUserAccount     m_uaccLogin;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacMediaRepoMgr,TGUIFacility)
};

#pragma CIDLIB_POPPACK

