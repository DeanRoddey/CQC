//
// FILE NAME: CppDrvTest.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//  This is the main header for the program. It sub-includes all of the other
//  programs headers and the needed headers for underlying facilities.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Include the needed underlying facility headers
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCKit.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCRPortClient.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"
#include    "CQCDriver.hpp"
#include    "CQCSrvDrvTI.hpp"



// ----------------------------------------------------------------------------
//  And include our own headers
// ----------------------------------------------------------------------------
#include    "CppDrvTest_MessageIds.hpp"
#include    "CppDrvTest_ErrorIds.hpp"
#include    "CppDrvTest_ResourceIds.hpp"

#include    "CppDrvTest_Constant.hpp"
#include    "CppDrvTest_QDrvTextDlg.hpp"
#include    "CppDrvTest_SrvDrvTab.hpp"
#include    "CppDrvTest_ClDrvTab.hpp"
#include    "CppDrvTest_TrigMonTab.hpp"
#include    "CppDrvTest_MainFrame.hpp"
#include    "CppDrvTest_ThisFacility.hpp"


// ----------------------------------------------------------------------------
//  Global data from CppDrvTest.cpp
// ----------------------------------------------------------------------------
extern TFacCppDrvTest   facCppDrvTest;
