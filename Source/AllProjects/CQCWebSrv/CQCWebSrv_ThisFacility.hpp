//
// FILE NAME: CQCWebSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This is the header for the facility class for the gateway server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWebSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCWebSrv : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCWebSrv();

        TFacCQCWebSrv(const TFacCQCWebSrv&) = delete;
        TFacCQCWebSrv(TFacCQCWebSrv&&) = delete;

        ~TFacCQCWebSrv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCWebSrv& operator=(const TFacCQCWebSrv&) = delete;
        TFacCQCWebSrv& operator=(TFacCQCWebSrv&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCQCSrvFW::EStateRes eProcessParms
        (
                    tCIDLib::TKVPList::TCursor& cursParms
        )   final;

        tCIDLib::TVoid PostDeregTerm() final;

        tCIDLib::TVoid PostUnbindTerm() final;

        tCIDLib::TVoid PreRegInit() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ippnHTTP
        //  m_ippnHTTPS
        //      THe ports we are configured for for regular and secure HTTP. We default
        //      them but they can be override on the command line.
        //
        //  m_strCertInfo
        //      Our certificate loading location, if any. If not, it's empty.
        //
        //  m_strHTMLDir
        //      The local directory that we should consider the top of the HTML content
        //      space. Comes from the command line and we have to get it.
        // -------------------------------------------------------------------
        tCIDLib::TIPPortNum     m_ippnHTTP;
        tCIDLib::TIPPortNum     m_ippnHTTPS;
        TString                 m_strCertInfo;
        TString                 m_strHTMLDir;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCWebSrv,TCQCSrvCore)
};

#pragma CIDLIB_POPPACK

