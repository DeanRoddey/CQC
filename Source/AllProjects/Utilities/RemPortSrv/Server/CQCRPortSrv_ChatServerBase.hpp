// ----------------------------------------------------------------------------
//  FILE: CQCRPortSrv_ChatServerBase.hpp
//  DATE: Tue, Jan 19 17:39:56 2021 -0500
//    ID: 8DC119F469C5036D-D3DE9312730302EE
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class  TRemChatServerBase : public TOrbServerBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        static constexpr tCIDLib::TCard4 c4PollSeconds = 5;
        static const TString strInstanceId;


        // --------------------------------------------------------------------
        // Constructors and destructor
        // --------------------------------------------------------------------
        TRemChatServerBase(const TRemChatServerBase&) = delete;
        TRemChatServerBase(TRemChatServerBase&&) = delete;
        ~TRemChatServerBase();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TRemChatServerBase& operator=(const TRemChatServerBase&) = delete;
        TRemChatServerBase& operator=(TRemChatServerBase&&) = delete;

        // --------------------------------------------------------------------
        // Public, pure virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TBoolean bExchange
        (
            const TString& strSendMsg
            , TString& strRecMsg
        ) = 0;

    protected :
        // --------------------------------------------------------------------
        // Hidden constructors
        // --------------------------------------------------------------------
        TRemChatServerBase();
        TRemChatServerBase(const TOrbObjId& ooidThis);

        // --------------------------------------------------------------------
        // Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Dispatch
        (
            const  TString&      strMethodName
            ,      TOrbCmd&      orbcToDispatch
        )   override;

    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TRemChatServerBase,TOrbServerBase)
};

#pragma CIDLIB_POPPACK

