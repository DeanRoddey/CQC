//
// FILE NAME: WebRIVACmp.cpp
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
//  This program just parses the XML file and spits out appropriate code to the
//  target projects.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "WebRIVACmp.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
    ,   tCIDLib::TVoid*     pData
);



// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"WebRIVACmp", eMainThreadFunc))


// ---------------------------------------------------------------------------
//  Local static data
// ---------------------------------------------------------------------------
static const TString strEmbeddedDTDText
(
    L"<?xml encoding='$NativeWideChar$'?>\n"

    L"<!ENTITY %  ConstTypes  ' Boolean"
    L"                        | Int1"
    L"                        | Int2"
    L"                        | Int4"
    L"                        | Int8"
    L"                        | Card1"
    L"                        | Card2"
    L"                        | Card4"
    L"                        | Card8"
    L"                        | Float8"
    L"                        | String'>\n"

    L"<!ENTITY %  MemTypes    ' AlphaColor"
    L"                        | Area"
    L"                        | Boolean"
    L"                        | Card1"
    L"                        | Card2"
    L"                        | Card4"
    L"                        | Card8"
    L"                        | Color"
    L"                        | Enum"
    L"                        | Int1"
    L"                        | Int2"
    L"                        | Int4"
    L"                        | Int8"
    L"                        | Float8"
    L"                        | Point"
    L"                        | Size"
    L"                        | String"

    // Raw JSON content passed through as is, passed as text
    L"                        | Passthrough"

    // A Card1 but we need to map from our byte to a 0.0 to 1.0
    L"                        | Opacity'>\n"


    L"<!ELEMENT  Constant EMPTY>\n"
    L"<!ATTLIST  Constant\n"
    L"           Name NMTOKEN #REQUIRED\n"
    L"           Type (%ConstTypes;) #REQUIRED\n"
    L"           Value CDATA #REQUIRED>\n"
    L"<!ELEMENT  Constants (Constant+)>\n"

    L"<!ELEMENT  EnumVal EMPTY>\n"
    L"<!ATTLIST  EnumVal\n"
    L"           Name NMTOKEN #REQUIRED\n"
    L"           Value CDATA #IMPLIED>\n"
    L"<!ELEMENT  Enum (EnumVal+)>\n"
    L"<!ATTLIST  Enum\n"
    L"           Name NMTOKEN #REQUIRED>\n"
    L"<!ELEMENT  Enums (Enum+)>\n"

    L"<!ELEMENT  StructMem EMPTY>\n"
    L"<!ATTLIST  StructMem\n"
    L"           Name NMTOKEN #REQUIRED\n"
    L"           AName NMTOKEN #REQUIRED\n"
    L"           Type (%MemTypes;) #REQUIRED\n"
    L"           EnumType CDATA #IMPLIED>\n"
    L"<!ELEMENT  Structure (StructMem)*>\n"
    L"<!ATTLIST  Structure\n"
    L"           Dir (CtoS | StoC | Both) 'StoC'\n"
    L"           Type NMTOKEN #REQUIRED>\n"
    L"<!ELEMENT  Structures (Structure+)>\n"

    L"<!ELEMENT  WebRIVA (Constants, Enums, Structures)>\n"
);

TOutConsole strmOut;


// ----------------------------------------------------------------------------
//  Functions for local use
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    try
    {
        TXMLTreeParser xtprsData;

        //
        //  Add an entity mapping so that the public URN that is used in the
        //  manifest files get mapped to our hard coded internal DTD text.
        //
        xtprsData.AddMapping
        (
            new TMemBufEntitySrc
            (
                L"http://www.charmedquark.com/CQC/WebRIVACmp.DTD"
                , L"urn:charmedquark.com:XML-WebRIVA.Dtd"
                , strEmbeddedDTDText
            )
        );


        //
        //  Ok, we have a file, so lets try to use the XML tree parser to get
        //  a representation of the file into memory. Tell it that we only
        //  want to see non-ignorable chars and tag information. That will
        //  save us a lot of filtering out comments and whitspace.
        //
        const tCIDLib::TBoolean bOk = xtprsData.bParseRootEntity
        (
            L".\\WebRIVA.xml"
            , tCIDXML::EParseOpts::Validate
            , tCIDXML::EParseFlags::TagsNText
        );

        if (!bOk)
        {
            //
            //  Format the first error in the error list, into a string that
            //  we can pass as a replacement parameter to the real error we
            //  log.
            //
            const TXMLTreeParser::TErrInfo& erriFirst
                                            = xtprsData.colErrors()[0];

            strmOut << L"[" << erriFirst.strSystemId() << L"/"
                    << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Ok, we now have the document with the validated system config data in it.
        //  So lets go through it and get the data out. We start by getting the root
        //  element node from the document.
        //
        const TXMLTreeElement& xtnodeRoot = xtprsData.xtdocThis().xtnodeRoot();

        // Create a C++ output generator and invoke it with the info
        {
            TCppGenerator genCpp;
            genCpp.GenerateOutput(xtnodeRoot);
        }

        // Create a Typescript output generate and invoke it
        {
            TTSGenerator genTypeScript;
            genTypeScript.GenerateOutput(xtnodeRoot);
        }
    }

    catch(TError& errToCatch)
    {
        strmOut << errToCatch << kCIDLib::EndLn;
    }

    return tCIDLib::EExitCodes::Normal;
}
