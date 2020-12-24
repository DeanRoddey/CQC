//
// FILE NAME: GC100Ser_PortFactory.hpp
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
//  up new ports. Any app that wants to support GC ports must register one
//  of these with the CIDComm facility, and provide us with config on which
//  GCs are to be available.
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
//  CLASS: TGC100PortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------
class GC100SEREXPORT TGC100PortFactory : public TComPortFactory
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGC100PortFactory
        (
            const   TGC100CfgList&          gcclSrc
        );

        TGC100PortFactory(const TGC100PortFactory&) = delete;
        TGC100PortFactory(TGC100PortFactory&&) = delete;

        ~TGC100PortFactory();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGC100PortFactory& operator=(const TGC100PortFactory&) = delete;
        TGC100PortFactory& operator=(TGC100PortFactory&&) = delete;


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
        )   const final;

        [[nodiscard]] TCommPortBase* pcommMakeNew
        (
            const   TString&                strPath
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] const TIPEndPoint& ipepServer() const;

        tCIDLib::TVoid UpdateConfig
        (
            const   TGC100CfgList&          gcclToSet
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
        //  m_gcclCfg
        //      The list of GCs that we can make available. It's a list of
        //      TGC100Cfg objects.
        // -------------------------------------------------------------------
        TGC100CfgList   m_gcclCfg;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGC100PortFactory,TComPortFactory)
};

#pragma CIDLIB_POPPACK



