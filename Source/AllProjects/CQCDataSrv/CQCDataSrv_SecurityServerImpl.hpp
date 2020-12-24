//
// FILE NAME: CQCDataSrv_SecurityServerImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2002
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
//  This is the header for the CQCDataSrv_SecurityServerImpl.cpp file, which
//  provides an implementation of the data server's remote security interface.
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
//   CLASS: TCQCSecuritySrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCSecuritySrvImpl : public TCQCSecureServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSecuritySrvImpl();

        TCQCSecuritySrvImpl(const TCQCSecuritySrvImpl&) = delete;
        TCQCSecuritySrvImpl(TCQCSecuritySrvImpl&&) = delete;

        ~TCQCSecuritySrvImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSecuritySrvImpl& operator=(const TCQCSecuritySrvImpl&) = delete;
        TCQCSecuritySrvImpl& operator=(TCQCSecuritySrvImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckToken
        (
            const   TCQCSecToken&           sectToCheck
        )   final;

        tCIDLib::TBoolean bGetSToken
        (
            const   TCQCSecChallenge&       seccResponse
            ,       TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            ,       tCQCKit::ELoginRes&     eResult
        )   final;

        tCIDLib::TBoolean bLoginExists
        (
            const   TString&                strName
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bLoginReq
        (
            const   TString&                strName
            ,       TCQCSecChallenge&       seccToFill
        )   final;

        tCIDLib::TBoolean bQueryEventKey
        (
                    TMD5Hash&               mhashKey
            ,       TMD5Hash&               mhashSysId
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQuerySrvCreds
        (
                    TString&                strName
            ,       TString&                strPassword
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryWAccount
        (
            const   TString&                strLogin
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufPW
            ,       tCQCKit::EUserRoles&    eRole
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TCard4 c4QueryAccounts
        (
                    TVector<TCQCUserAccount>& colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TCard8 c8QueryVersion() final;

        tCIDLib::TVoid ChangePassword
        (
            const   TString&                strLogin
            , const tCIDLib::TBoolean       bIsCQCPW
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TCard4         c4DataBytes
            , const THeapBuf&               mbufData
        )   final;

        tCIDLib::TVoid CreateAccount
        (
            const   TCQCUserAccount&        uaccNew
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DeleteAccount
        (
            const   TString&                strLogin
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DecodeSecurityToken
        (
            const   TCQCSecToken&           sectSrc
            ,       TString&                strLoginName
            ,       tCQCKit::EUserRoles&    eRole
            ,       tCQCKit::EUserTypes&    eType
            ,       tCIDLib::TEncodedTime&  enctStamp
            ,       TString&                strDefTmpl
        )   final;

        tCIDLib::TVoid QueryAccount
        (
            const   TString&                strLogin
            , const TCQCSecToken&           sectUser
            ,       TCQCUserAccount&        uaccToFill
        )   final;

        tCIDLib::TVoid ResetAccountPassword
        (
            const   TString&                strLogin
            , const TString&                strNewPassword
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UpdateAccount
        (
            const   TCQCUserAccount&        uaccNew
            , const TCQCSecToken&           sectUser
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCSrvFW::EStateRes eLoadSecurityInfo
        (
                    tCIDLib::TCard4&        c4WaitNext
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TTokenList = TKeyedHashSet<TCQCSrvSecChallenge,TMD5Hash,TMD5KeyOps>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid EncodeSecToken
        (
                    TCQCSecToken&           sectToFill
            , const TCQCUserAccount&        uaccToUse
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colChallenges
        //      This is the list of outstanding login challenges that we are waiting
        //      for the second calls for. We prune it during each login attempt, and
        //      get rid of any that are over their timeout period.
        //
        //  m_crypTokens
        //      Used to encrypted security tokens. The key is the hash of the server
        //      account password.
        //
        //  m_oseRepo
        //      Our repository file to store security info in.
        //
        //  m_mhashEventKey
        //  m_mhashEventSysId
        //      We need a key that is used by the event system to encrypt event triggers.
        //      And we need a unique per-system id , in case multiple CQC systems are on the
        //      same system, to prevent them from reacting to each other's event triggers.
        //      Clients and servers that need to send or receive event triggers will call
        //      to get these. We store these in our private repo.
        //
        //  m_mhashSrvPW
        //      There are some things that we encrypt with the hash of the server account
        //      password, so we go ahead and set it up.
        //
        //  m_mtxSync
        //      Everything happens though us, so we provide all of the sync that's
        //      required, both for internal invocation of macros and field writes,
        //      as well as macros invoked on behalf of incoming clients.
        //
        //  m_secdCurrent
        //      Our current security data. Its maintained fully in memory since there's
        //      not much of it.
        //
        //  m_strSrvPW
        //      We have to provide the server password and user name to the installer.
        //      The name we can get from the user account below. It has the password but
        //      only in hashed form.
        //
        //  m_uaccSrv
        //      Some of the servers require an account to use, which causes an issue
        //      since they have to log in without a user. The username/password is
        //      gotten from the Windows registry.
        // -------------------------------------------------------------------
        TTokenList          m_colChallenges;
        TBlowfishEncrypter  m_crypTokens;
        TCIDObjStore        m_oseRepo;
        TMD5Hash            m_mhashEventKey;
        TMD5Hash            m_mhashEventSysId;
        TMD5Hash            m_mhashSrvPW;
        TMutex              m_mtxSync;
        TCQCSecurityData    m_secdCurrent;
        TString             m_strSrvPW;
        TCQCUserAccount     m_uaccSrv;


        // -------------------------------------------------------------------
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSecuritySrvImpl,TCQCSecureServerBase)
};

#pragma CIDLIB_POPPACK


