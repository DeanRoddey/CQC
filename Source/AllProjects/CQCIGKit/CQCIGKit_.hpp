//
// FILE NAME: CQCIGKit_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/18/2004
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
//  This is the internal facility header
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include    "CQCIGKit.hpp"
#include    "CQCIGKit_ResourceIds.hpp"


// ---------------------------------------------------------------------------
//  Bring in other headers that we only need internally
// ---------------------------------------------------------------------------
#include    "CIDRegX.hpp"
#include    "CIDComm.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDPNG.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDGraphDev.hpp"
#include    "CIDWUtils.hpp"
#include    "CIDDBase.hpp"
#include    "CIDMacroEng.hpp"
#include    "CQCRPortClient.hpp"
#include    "CQCIR.hpp"
#include    "CIDUPnP.hpp"
#include    "CQCMedia.hpp"
#include    "CQCIntfEng.hpp"


// ---------------------------------------------------------------------------
//  Include any internal only headers of our own
// ---------------------------------------------------------------------------
#include    "CQCIGKit_WaitCQCDlg_.hpp"


// ---------------------------------------------------------------------------
//  Internal constants
// ---------------------------------------------------------------------------
namespace kCQCIGKit_
{
    // -----------------------------------------------------------------------
    //  Even if we can import a package file, at some point we stop
    //  accepting packages that were created by older CQC versions, because
    //  the file info inside them are no longer understandable. This is the
    //  minimal CQC verion whose packages we'll accept.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard8   c8MinCQCPackVer = 0x0002000200000005;
}
