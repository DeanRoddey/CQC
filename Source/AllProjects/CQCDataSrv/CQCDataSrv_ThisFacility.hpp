//
// FILE NAME: CQCDataSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/23/2002
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
//  We are based on the standard CQC server core framework. So we just give him
//  some information about this server, and otherwise we are just handling callbacks
//  from him for initialization and termination.
//
//  Otherwise, all the work is done by the server side interface objects we create
//  and install, which are servicing incoming calls from clients. We only need to
//  override a handful of server core callbacks, since this is a very conventional
//  CQC server.
//
//  The individual server side interface objects provide any synchronziation that
//  they need so we don't need any at all.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDataSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCDataSrv : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCDataSrv();

        TFacCQCDataSrv(const TFacCQCDataSrv&) = delete;
        TFacCQCDataSrv(TFacCQCDataSrv&&) = delete;

        ~TFacCQCDataSrv() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCDataSrv& operator=(const TFacCQCDataSrv&) = delete;
        TFacCQCDataSrv& operator=(TFacCQCDataSrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bCheckSTok
        (
            const   TCQCSecToken&           sectUser
        );

        [[nodiscard]] tCIDLib::TBoolean bCheckMinRole
        (
            const   TCQCSecToken&           sectUser
            , const tCQCKit::EUserRoles     eMinRole
        );

        [[nodiscard]] tCIDLib::TBoolean bCheckAccess
        (
            const   TCQCSecToken&           sectUser
            , const TString&                strAccount
            , const tCQCKit::EUserRoles     eMinRoleIfNot
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DeregisterSrvObjs() final;

        tCQCSrvFW::EStateRes eLoadConfig
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        )   final;

        tCIDLib::TVoid MakeDirs() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   final;

        tCIDLib::TVoid RegisterSrvObjs() final;

        tCIDLib::TVoid UnbindSrvObjs
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadManifests();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_porbsAccImpl
        //  m_porbsInstImpl
        //  m_porbsIRProtoImpl
        //  m_porbsSecurityImpl
        //  m_porbsSysCfg
        //      We keep a pointer to our server objects, which we register
        //      with the ORB and advertise in the name server. We don't own
        //      these, the ORB does. But its convenient to have a typed pointer
        //      to them, to avoid lots of getting it from the ORB and having
        //      to downcast it. We have other objects, such as the admin
        //      object, but those we just give away to the ORB and leave it
        //      at that, since we never use those ourself directly.
        // -------------------------------------------------------------------
        TDataSrvAccImpl*        m_porbsAccImpl;
        TCQCInstSrvImpl*        m_porbsInstImpl;
        TCQCIRSrvImpl*          m_porbsIRImpl;
        TCQCSecuritySrvImpl*    m_porbsSecurityImpl;
        TCQCSysCfgSrvImpl*      m_porbsSysCfg;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCDataSrv,TFacility)
};


