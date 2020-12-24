//
// FILE NAME: CQCTerminal.hpp
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
//  This is the main header of this program. It brings in the underlying stuff we
//  need and our own headers for internal use.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include    "CIDEncode.hpp"
#include    "CIDComm.hpp"
#include    "CIDSock.hpp"
#include    "CIDORB.hpp"
#include    "CQCKit.hpp"
#include    "CQCRPortClient.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"



// ----------------------------------------------------------------------------
//  Facility types
// ----------------------------------------------------------------------------
namespace tCQCTerminal
{
    // Indicates the type of connection
    enum class EConnTypes
    {
        Unknown
        , Serial
        , Socket
    };

    // The types of line ends we support
    enum class ELineEnds
    {
        CR
        , CRLF
        , LF
        , LFCR
    };
}


// ---------------------------------------------------------------------------
//  Bring in our own headers
// ---------------------------------------------------------------------------
#include    "CQCTerminal_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object to any other modules. It's declared in the main
//  cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCTerminal  facCQCTerminal;
