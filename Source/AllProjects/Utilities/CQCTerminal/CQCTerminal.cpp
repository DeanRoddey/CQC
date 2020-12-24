//
// FILE NAME: CQCTerminal.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/18/2018
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
//  This is a simple utility program that implements a basic telnet terminal type
//  program. Our customers often need such a thing and don't need anything fancy,
//  just something to spelunk such protocols. So we provide this one. It's a command
//  line program that will work for either stream sockets or serial ports. It is
//  invoked like this:
//
//      CQCTerminal /Socket addr port
//      CQCTerminal /Serial port
//
//  So they tell us if it's a socket or a comm port and provide the appropriate
//  info.
//
//  We can also accept /LEnd=[CR, LF, CRLF, LFCR] to let them tell us what type of
//  line ends to expect.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Bring in our main header
// ----------------------------------------------------------------------------
#include    "CQCTerminal.hpp"



// ----------------------------------------------------------------------------
//  Global data
// ----------------------------------------------------------------------------
TFacCQCTerminal facCQCTerminal;



// ----------------------------------------------------------------------------
//  Do the magic main module code. We are setting up our main thread to run on the
//  facility object in the usual way.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCTerminalThread"
        , TMemberFunc<TFacCQCTerminal>(&facCQCTerminal, &TFacCQCTerminal::eMainThread)
    )
)

