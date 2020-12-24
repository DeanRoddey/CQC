//
// FILE NAME: JAPwrSer_PortFactory.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This is the header for factory class. This guy is responsible for genning
//  up new ports. Any app that wants to support JAP ports must register one
//  of these with the CIDComm facility, and provide us with config on which
//  JAPs are to be available.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCommPortCfg;


// ---------------------------------------------------------------------------
//  CLASS: TJAPwrPortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------
class JAPWRSEREXPORT TJAPwrPortFactory : public TComPortFactory
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJAPwrPortFactory
        (
            const   TJAPwrCfgList&          japlSrc
        );

        TJAPwrPortFactory(const TJAPwrPortFactory&) = delete;
        TJAPwrPortFactory(TJAPwrPortFactory&&) = delete;

        ~TJAPwrPortFactory();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TJAPwrPortFactory& operator=(const TJAPwrPortFactory&) = delete;
        TJAPwrPortFactory& operator=(TJAPwrPortFactory&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryPorts
        (
                    tCIDLib::TStrCollect&   colToFill
            , const tCIDLib::TBoolean       bAvailOnly
        )   const final;

        tCIDLib::TBoolean bTestCfg
        (
            const   TCommPortCfg&           cpcfgTest
            ,       TString&                strReason
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bValidatePath
        (
            const   TString&                strPath
        )   const;

        [[nodiscard]] TCommPortBase* pcommMakeNew
        (
            const   TString&                strPath
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TIPEndPoint& ipepServer() const;

        tCIDLib::TVoid UpdateConfig
        (
            const   TJAPwrCfgList&          japlToSet
        );


    private :
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
        //  m_japlCfg
        //      The list of JAPs that we can make available. It's a list of TJAPwrCfg
        //      objects.
        // -------------------------------------------------------------------
        TJAPwrCfgList   m_japlCfg;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJAPwrPortFactory,TComPortFactory)
};

#pragma CIDLIB_POPPACK



