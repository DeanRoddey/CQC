//
// FILE NAME: WebRIVACmp_TSGen.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/04/2017
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
//  the Typescript side.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TTSGenerator
//  PREFIX: gen
// ---------------------------------------------------------------------------
class TTSGenerator
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TTSGenerator();

        TTSGenerator(const TTSGenerator&) = delete;

        ~TTSGenerator();


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
        tWebRIVACmp::EConstTypes eFormatConst
        (
                    TTextOutStream&         strmTar
            , const TXMLTreeElement&        xtnodeSrc
        );

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



        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_strmTS
        //      Our output .ts file that we spit the types, constants and methods out
        //      to.
        // --------------------------------------------------------------------
        TTextFileOutStream  m_strmTS;
};
