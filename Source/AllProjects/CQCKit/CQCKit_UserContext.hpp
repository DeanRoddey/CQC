//
// FILE NAME: CQCKit_UserContext.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/01/2011
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
//  This file defines a class holds the user context info that is required by
//  many helper methods and therefore has to be passed down from the app
//  that provided the login to the stuff that needs it. Since we have apps
//  (XML GW and RIVA) that can have multiple user sessions per process, we
//  can't just store it away in a common area for everyone to access.
//
//  This derives from a very simple base class in the CIDLib layer, so that
//  CIDLib frameworks can support passthrough of client code user context
//  info by just deriving from that class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCUserCtx
//  PREFIX: cuctx
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCUserCtx : public TCIDUserCtx
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCUserCtx();

        TCQCUserCtx
        (
            const   TString&                strUserName
            , const tCQCKit::EUserRoles     eUserRole
            , const TCQCSecToken&           sectUser
            , const TString&                strDefTemplate
        );

        TCQCUserCtx
        (
            const   TString&                strUserName
            , const tCQCKit::EUserRoles     eUserRole
            , const TString&                strDefTemplate
        );

        TCQCUserCtx(const TCQCUserCtx&) = default;
        TCQCUserCtx(TCQCUserCtx&&) = default;

        ~TCQCUserCtx();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCUserCtx& operator=(const TCQCUserCtx&) = default;
        TCQCUserCtx& operator=(TCQCUserCtx&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCQCKit::EUserRoles eUserRole() const;

        tCIDLib::TVoid LoadEnvRTVsFromHost();

        const TCQCSecToken& sectUser() const;

        const TString& strEnvRTVAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strDefTemplate() const;

        tCIDLib::TVoid Set
        (
            const   TCQCUserAccount&        uaccSrc
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetEnvRTVAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_objaEnvRTVs
        //      The environmental runtime values. They'll be empty unless
        //      set by the app provides them. For the apps that just get
        //      them from the host environment (most of them) we provide a
        //      helper to load them.
        //
        //  m_eUserRole
        //      The caller has to provide us with the user role for the
        //      user context this action is being invoked in.
        //
        //  m_sectUser
        //      The caller has to provide us with the security token of the
        //      user account that this action is being invoked in the context
        //      of.
        //
        //  m_strDefTemplate
        //      The default template for this user, if any.
        // -------------------------------------------------------------------
        tCIDLib::TStrArray  m_objaEnvRTVs;
        tCQCKit::EUserRoles m_eUserRole;
        TCQCSecToken        m_sectUser;
        TString             m_strDefTemplate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCUserCtx,TCIDUserCtx)
};

#pragma CIDLIB_POPPACK


