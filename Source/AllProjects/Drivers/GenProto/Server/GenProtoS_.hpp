//
// FILE NAME: GenProtoS_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the internal header for the generic protocol server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Include our public header for everyone to see internally
// ---------------------------------------------------------------------------
#include    "GenProtoS.hpp"


// ---------------------------------------------------------------------------
//  Facility private types
// ---------------------------------------------------------------------------
namespace tGenProtoS_
{
}


// ---------------------------------------------------------------------------
//  Facility private constants
// ---------------------------------------------------------------------------
namespace kGenProtoS_
{
    // -----------------------------------------------------------------------
    //  The modulus we use on the key hashes in various internal collections
    //  that hold items from the protocol files.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4Modulus = 29;


    // -----------------------------------------------------------------------
    //  The name of the magic state that indicates acceptance of a message.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszAcceptState = L"<Accept>";
}


// ---------------------------------------------------------------------------
//  And our own internal-only headers, which we have a good number of because
//  we have lots of classes for the internal representation of all of the data
//  in a protocol description, which aren't needed by the outside world.
// ---------------------------------------------------------------------------
#include    "GenProtoS_ConstExpression_.hpp"
#include    "GenProtoS_ExprBaseFuncs_.hpp"
#include    "GenProtoS_ExprMathFuncs_.hpp"
#include    "GenProtoS_StrFuncs_.hpp"
#include    "GenProtoS_CastExpression_.hpp"
#include    "GenProtoS_RuntimeExpression_.hpp"
#include    "GenProtoS_Tokenizer_.hpp"

