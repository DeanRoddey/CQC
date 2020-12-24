//
// FILE NAME: CQCProtoDev.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2003
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
//  This is the main header for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our needed underlying headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDXML.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "GenProtoS.hpp"
#include    "CQCRPortClient.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "CQCProtoDev_Type.hpp"
#include    "CQCProtoDev_ErrorIds.hpp"
#include    "CQCProtoDev_MessageIds.hpp"
#include    "CQCProtoDev_ResourceIds.hpp"
#include    "CQCProtoDev_Constant.hpp"

#include    "CQCProtoDev_MainFrame.hpp"
#include    "CQCProtoDev_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object to other files of the program
// ---------------------------------------------------------------------------
extern TFacCQCProtoDev facCQCProtoDev;


