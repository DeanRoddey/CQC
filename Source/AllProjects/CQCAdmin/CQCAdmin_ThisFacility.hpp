//
// FILE NAME: CQCAdmin_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This is the header for the facility class for the admin client. This program
//  has quite a bit of functionality that must be coordinated, and most of
//  that coordination comes from the facility object.
//
// CAVEATS/GOTCHAS:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAdmin
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCAdmin : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCAdmin();

        TFacCQCAdmin(const TFacCQCAdmin&) = delete;
        TFacCQCAdmin(TFacCQCAdmin&&) = delete;

        ~TFacCQCAdmin();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCAdmin& operator=(const TFacCQCAdmin&) = delete;
        TFacCQCAdmin& operator=(TFacCQCAdmin&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::EUserRoles eUserRole() const;

        const TCQCSecToken& sectUser() const;

        const TString& strUserName() const;

        const TCQCPollEngine& polleToUse() const;

        TCQCPollEngine& polleToUse();

        const TMainFrameWnd& wndMain() const
        {
            return m_wndMain;
        }

        TMainFrameWnd& wndMain()
        {
            return m_wndMain;
        }


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoLogon();

        tCIDLib::TBoolean bParseParams
        (
                    tCIDLib::TBoolean&      bNoState
        );

        tCIDLib::TVoid InitObjectStore
        (
                    TCQCCAppState&          fstInit
            , const tCIDLib::TBoolean       bNoState
        );

        tCIDLib::TVoid RegisterCOMFactories();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cuctxToUse
        //      The user context that we set up when the user logs in. We
        //      have to pass this to various parties along the way.
        //
        //  m_polleToUse
        //      A polling engine we create and keep around. We pass it to various
        //      facilities that we use that need a polling engine, so that we only
        //      have one for maximum efficiency.
        //
        //  m_wndMain
        //      The main frame window of the client.
        //
        //  m_uaccLogin
        //      The user account info for the user who logged in. There is
        //      nothing unsafe in here. Those fields are not returned from
        //      the security server. We want it for things like logon time
        //      limits and the default template.
        // -------------------------------------------------------------------
        TCQCUserCtx         m_cuctxToUse;
        TCQCPollEngine      m_polleToUse;
        TMainFrameWnd       m_wndMain;
        TCQCUserAccount     m_uaccLogin;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCAdmin,TGUIFacility)
};

#pragma CIDLIB_POPPACK

