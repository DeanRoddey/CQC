//
// FILE NAME: IRClient.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This is the main header file for the RedRat client side driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDCtrls.hpp"

#include    "CQCIR.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TIRClientWnd;



// ---------------------------------------------------------------------------
//  Facility constants namespace
// ---------------------------------------------------------------------------
namespace kIRClient
{
    // -----------------------------------------------------------------------
    //  The base name of our client side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFacName  = L"IRClient";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number equal
    //  to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order.
// ---------------------------------------------------------------------------
#include    "IRClient_ErrorIds.hpp"
#include    "IRClient_MessageIds.hpp"
#include    "IRClient_ResourceIds.hpp"
#include    "IRClient_SelModelDlg.hpp"
#include    "IRClient_BlastTrainingDlg.hpp"
#include    "IRClient_RecTrainingDlg.hpp"
#include    "IRClient_EditModelDlg.hpp"

#include    "IRClient_BlasterTab.hpp"
#include    "IRClient_ReceiverTab.hpp"
#include    "IRClient_Window.hpp"


// ---------------------------------------------------------------------------
//  Make our facility object access function visible to everyone internally.
// ---------------------------------------------------------------------------
extern TGUIFacility& facIRClient();


