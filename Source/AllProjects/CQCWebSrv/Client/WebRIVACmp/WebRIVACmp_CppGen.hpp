//
// FILE NAME: WebRIVACmp_CppGen.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/30/2017
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
//  This class takes the root element of the XML tree and does the generation for
//  the C++ side.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TCppGenerator
//  PREFIX: gen
// ---------------------------------------------------------------------------
class TCppGenerator
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TCppGenerator();

        TCppGenerator(const TCppGenerator&) = delete;

        ~TCppGenerator();


        // --------------------------------------------------------------------
        //  Public, non-static methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid GenerateOutput
        (
            const   TXMLTreeElement&        xtnodeRoot
        );


    private :
        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tWebRIVACmp::EMemTypes eFormatStructMemType
        (
                    TTextOutStream&         strmTar
            , const TXMLTreeElement&        xtnodeSrc
        );

        tCIDLib::TVoid GenCSMethod
        (
            const   TString&                strType
            , const TXMLTreeElement&        xtnodeStruct
        );


        tCIDLib::TVoid GenSCMethod
        (
            const   TString&                strType
            , const TXMLTreeElement&        xtnodeStruct
        );


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_strmCpp
        //  m_strmHpp
        //      Our two output file streams, which are spit out to the WebRIVA facility's
        //      directory.
        // --------------------------------------------------------------------
        TTextFileOutStream  m_strmCpp;
        TTextFileOutStream  m_strmHpp;
};
