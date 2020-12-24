//
// FILE NAME: CQCRPortClient_PortFactory.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This is the header for the CIDRPortClient_PortFactory.cpp file, which
//  implements the pluggable com port factory for remote ports.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCRemSrvPortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------
class CQCRPORTCEXPORT TCQCRemSrvPortFactory : public TComPortFactory
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCRemSrvPortFactory
        (
            const   TIPEndPoint&            ipepSrv
        );

        ~TCQCRemSrvPortFactory();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryPorts
        (
                    TCollection<TString>&   colToFill
            , const tCIDLib::TBoolean       bAvailOnly
        )   const;

        tCIDLib::TBoolean bTestCfg
        (
            const   TCommPortCfg&           cpcfgTest
            ,       TString&                strReason
        )   const;

        tCIDLib::TBoolean bValidatePath
        (
            const   TString&                strPath
        )   const;

        [[nodiscard]] TCommPortBase* pcommMakeNew
        (
            const   TString&                strPath
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TIPEndPoint& ipepServer() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCQCRemSrvPortFactory(const TCQCRemSrvPortFactory&);
        tCIDLib::TVoid operator=(const TCQCRemSrvPortFactory&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
            const   TString&                strPath
            ,       tCIDLib::TCard4&        c4PortNum
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ipepServer
        //      The end point of the server we are set up to talk to. We need
        //      this in order to set up a client proxy.
        // -------------------------------------------------------------------
        TIPEndPoint     m_ipepServer;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCRemSrvPortFactory,TComPortFactory)
};

#pragma CIDLIB_POPPACK


