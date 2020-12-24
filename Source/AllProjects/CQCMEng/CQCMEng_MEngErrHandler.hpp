//
// FILE NAME: CQCMEng_MEngErrHandler.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2001
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
//  This is the header for the CQCMEng_MEngErrHandler.cpp file, which
//  implements a derivative of the CIDLib Macro Engine's error handler
//  interface. This one will be used by all CQC users of the macro engine.
//  It just logs to the standard log server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCMEngErrHandler
// PREFIX: meeh
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCMEngErrHandler : public TObject, public MMEngErrHandler
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMEngErrHandler();

        TCQCMEngErrHandler(const TCQCMEngErrHandler&) = delete;
        TCQCMEngErrHandler(TCQCMEngErrHandler&&) = delete;

        ~TCQCMEngErrHandler();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMEngErrHandler& operator=(const TCQCMEngErrHandler&) = delete;
        TCQCMEngErrHandler& operator=(TCQCMEngErrHandler&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Exception
        (
            const   TError&                 errThrown
            ,       TCIDMacroEngine&        meSource
        )   final;

        tCIDLib::TVoid MacroException
        (
            const   TMEngExceptVal&         mecvThrown
            ,       TCIDMacroEngine&        meSource
        )   final;

        tCIDLib::TVoid UnknownException
        (
                    TCIDMacroEngine&        meSource
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DumpCallStack
        (
                    TCIDMacroEngine&        meSource
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCMEngErrHandler,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCPrsErrHandler
// PREFIX: meeh
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCPrsErrHandler

    : public TObject, public MMEngPrsErrHandler
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPrsErrHandler();

        TCQCPrsErrHandler(const TCQCPrsErrHandler&) = delete;
        TCQCPrsErrHandler(TCQCPrsErrHandler&&) = delete;

        ~TCQCPrsErrHandler();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPrsErrHandler& operator=(const TCQCPrsErrHandler&) = delete;
        TCQCPrsErrHandler& operator=(TCQCPrsErrHandler&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseEvent
        (
            const   tCIDMacroEng::EPrsEvs   eEvent
            , const TString&                strText
            , const tCIDLib::TCard4         c4LineNum
            , const tCIDLib::TCard4         c4ColNum
            , const TString&                strClassPath
        )   final;

        tCIDLib::TVoid ParseException
        (
            const   TError&                 errCaught
            , const tCIDLib::TCard4         c4LineNum
            , const tCIDLib::TCard4         c4ColNum
            , const TString&                strClassPath
        )   final;

        tCIDLib::TVoid ParseException
        (
            const   tCIDLib::TCard4         c4LineNum
            , const tCIDLib::TCard4         c4ColNum
            , const TString&                strClassPath
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPrsErrHandler,TObject)
};


#pragma CIDLIB_POPPACK


