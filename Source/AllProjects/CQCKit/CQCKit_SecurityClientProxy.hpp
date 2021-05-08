// ----------------------------------------------------------------------------
//  FILE: CQCKit_SecurityClientProxy.hpp
//  DATE: Sat, May 01 11:54:22 2021 -0400
//    ID: 1E1696267012659C-EEB55AB4F80BBE55
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class CQCKITEXPORT TCQCSecureClientProxy : public TOrbClientBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        static const TString strBinding;



        // --------------------------------------------------------------------
        // Public Constructors and Destructor
        // --------------------------------------------------------------------
        TCQCSecureClientProxy();
        TCQCSecureClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding);
        TCQCSecureClientProxy(const TCQCSecureClientProxy&) = delete;
        TCQCSecureClientProxy(TCQCSecureClientProxy&&) = delete;
        ~TCQCSecureClientProxy();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQCSecureClientProxy& operator=(const TCQCSecureClientProxy&) = delete;
        TCQCSecureClientProxy& operator=(TCQCSecureClientProxy&&) = delete;

        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid SetObjId
        (
            const   TOrbObjId& ooidToSet
            , const TString& strNSBinding
            , const tCIDLib::TBoolean bCheck = kCIDLib::True
        )   override;

        // --------------------------------------------------------------------
        // Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bCheckToken
        (
            const TCQCSecToken& sectToCheck
        );

        tCIDLib::TBoolean bLoginReq
        (
            const TString& strName
            , COP TCQCSecChallenge& seccToFill
        );

        tCIDLib::TBoolean bGetSToken
        (
            const TCQCSecChallenge& seccResponse
            , COP TCQCSecToken& sectToFill
            , COP TCQCUserAccount& uaccToFill
            , COP tCQCKit::ELoginRes& eResult
        );

        tCIDLib::TBoolean bLoginExists
        (
            const TString& strToCheck
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryEventKey
        (
            COP TMD5Hash& mhashKey
            , COP TMD5Hash& mhashSysId
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQuerySrvCreds
        (
            COP TString& strName
            , COP TString& strPassword
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryWAccount
        (
            const TString& strLogin
            , tCIDLib::TCard4& c4BufSz_mbufPW
            , COP THeapBuf& mbufPW
            , COP tCQCKit::EUserRoles& eRole
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TCard4 c4QueryAccounts
        (
            COP TVector<TCQCUserAccount>& colToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid ChangePassword
        (
            const TString& strLogin
            , const tCIDLib::TBoolean bIsCQCPW
            , const TCQCSecToken& sectUser
            , const tCIDLib::TCard4 c4BufSz_mbufNew
            , const THeapBuf& mbufNew
        );

        tCIDLib::TVoid CreateAccount
        (
            const TCQCUserAccount& uaccNew
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DeleteAccount
        (
            const TString& strLogin
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DecodeSecurityToken
        (
            const TCQCSecToken& sectSrc
            , COP TString& strLogin
            , COP tCQCKit::EUserRoles& eRole
            , COP tCQCKit::EUserTypes& eType
            , COP tCIDLib::TCard8& enctStamp
            , COP TString& strDefTmpl
        );

        tCIDLib::TVoid QueryAccount
        (
            const TString& strLogin
            , const TCQCSecToken& sectUser
            , COP TCQCUserAccount& uaccToFill
        );

        tCIDLib::TCard8 c8QueryVersion();

        tCIDLib::TVoid ResetAccountPassword
        (
            const TString& strLogin
            , const TString& strNewPassword
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid UpdateAccount
        (
            const TCQCUserAccount& uaccNew
            , const TCQCSecToken& sectUser
        );

    protected :
        // --------------------------------------------------------------------
        // Declare our friends
        // --------------------------------------------------------------------
        friend class TFacCIDOrb;

    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCSecureClientProxy,TOrbClientBase)
};

#pragma CIDLIB_POPPACK

inline tCIDLib::TVoid
TCQCSecureClientProxy::SetObjId(const  TOrbObjId& ooidToSet
              , const TString& strNSBinding
              , const tCIDLib::TBoolean bCheck)
{
    TParent::SetObjId(ooidToSet, strNSBinding, bCheck);
}

