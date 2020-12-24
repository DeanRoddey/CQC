//
// FILE NAME: CQCMEng_MEngErrHandler.cpp
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
//  This file implements a derivative of the CIDLib Macro Engine's error
//  handler interface. We just turn around and log the events, which makes
//  them go to whatever logger is installed on the process we are running
//  in.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCMEngErrHandler,TObject)
RTTIDecls(TCQCPrsErrHandler,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCMEngErrHandler
// PREFIX: meeh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCMEngErrHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMEngErrHandler::TCQCMEngErrHandler()
{
}

TCQCMEngErrHandler::~TCQCMEngErrHandler()
{
}


// ---------------------------------------------------------------------------
//  TCQCMEngErrHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCMEngErrHandler::Exception(  const   TError&             errThrown
                                ,       TCIDMacroEngine&    meSource)
{
    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kKitMsgs::midMacro_RunCppExcept
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    // Log the original error
    TModule::LogEventObj(errThrown);
    DumpCallStack(meSource);
}


tCIDLib::TVoid
TCQCMEngErrHandler::MacroException( const   TMEngExceptVal&     mecvThrown
                                    ,       TCIDMacroEngine&    )
{
    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kKitMsgs::midMacro_RunMacroExcept
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    // Log an initial error, then the actual error logged
    facCQCKit().LogMsg
    (
        mecvThrown.strSrcClassPath()
        , mecvThrown.c4LineNum()
        , mecvThrown.strErrorName()
        , mecvThrown.strErrorText()
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::Format
    );
}


tCIDLib::TVoid
TCQCMEngErrHandler::UnknownException(TCIDMacroEngine& meSource)
{
    // And log a follow up error about where it was in the macro
    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kKitMsgs::midMacro_RunUnknownExcept
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    DumpCallStack(meSource);
}


// ---------------------------------------------------------------------------
//  TCQCMEngErrHandler: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCMEngErrHandler::DumpCallStack(TCIDMacroEngine& meSource)
{
    //
    //  Log a stack dump. Note that, in the case of a recursive macro method,
    //  we will limit the number of stack frames we'll do. It'll provide enough
    //  info to see the recursion pattern.
    //
    tCIDLib::TCard4      c4Count = 0;
    TTextStringOutStream strmLog(4096UL);
    tCIDLib::TCard4      c4FInd = kCIDLib::c4MaxCard;
    while (meSource.bFormatNextCallFrame(strmLog, c4FInd))
    {
        strmLog << L"\n   ";
        c4Count++;
        if (c4Count > 16)
        {
            strmLog << L"Ending dump due to deep stack nesting\n";
            break;
        }
    }

    strmLog.Flush();
    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , strmLog.strData()
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCPrsErrHandler
// PREFIX: meres
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPrsErrHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPrsErrHandler::TCQCPrsErrHandler()
{
}

TCQCPrsErrHandler::~TCQCPrsErrHandler()
{
}


// ---------------------------------------------------------------------------
//  TCQCPrsErrHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCPrsErrHandler::ParseEvent(  const   tCIDMacroEng::EPrsEvs   eEvent
                                , const TString&                strText
                                , const tCIDLib::TCard4         c4LineNum
                                , const tCIDLib::TCard4
                                , const TString&                strClassPath)
{
    tCIDLib::ESeverities eSev;
    switch(eEvent)
    {
        case tCIDMacroEng::EPrsEvs::InfoMsg :
            eSev = tCIDLib::ESeverities::Info ;
            break;

        case tCIDMacroEng::EPrsEvs::WarningMsg :
            eSev = tCIDLib::ESeverities::Warn ;
            break;

        case tCIDMacroEng::EPrsEvs::ErrorMsg :
        case tCIDMacroEng::EPrsEvs::CIDLibExcept :
        case tCIDMacroEng::EPrsEvs::UnknownExcept :
        default :
            eSev = tCIDLib::ESeverities::Failed;
            break;
    };

    facCQCKit().LogMsg
    (
        strClassPath
        , c4LineNum
        , kKitMsgs::midMacro_ParseMacroEvent
        , strText
        , eSev
        , tCIDLib::EErrClasses::Format
    );
}


tCIDLib::TVoid
TCQCPrsErrHandler::ParseException(  const   TError&         errCaught
                                    , const tCIDLib::TCard4 c4LineNum
                                    , const tCIDLib::TCard4 c4ColNum
                                    , const TString&        strClassPath)
{
    // Log an initial error, then the actual error logged
    TString strTmp(strClassPath);
    strTmp.Append(L"  Line=");
    strTmp.AppendFormatted(c4LineNum);
    strTmp.Append(L", Col=");
    strTmp.AppendFormatted(c4ColNum);
    strTmp.Append(L")");
    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kKitMsgs::midMacro_ParseCppExcept
        , strTmp
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::Format
    );

    TModule::LogEventObj(errCaught);
}


tCIDLib::TVoid
TCQCPrsErrHandler::ParseException(  const   tCIDLib::TCard4 c4LineNum
                                    , const tCIDLib::TCard4 c4ColNum
                                    , const TString&        strClassPath)
{
    TString strTmp(strClassPath);
    strTmp.Append(L"  Line=");
    strTmp.AppendFormatted(c4LineNum);
    strTmp.Append(L", Col=");
    strTmp.AppendFormatted(c4ColNum);
    strTmp.Append(L")");

    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kKitMsgs::midMacro_ParseUnkExcept
        , strTmp
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::Format
    );
}


