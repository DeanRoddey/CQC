//
// FILE NAME: CQCIR.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This is the main header of the IR blaster helper facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCIR)
#define CQCIREXPORT    CID_DLLEXPORT
#else
#define CQCIREXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our needed underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TCQCRTClassLoader;
class TCQCMEngErrHandler;
class TMEngFixedBaseFileResolver;
class TCIDMacroEngine;
class TOrbAutoRebind;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCIR_ErrorIds.hpp"
#include    "CQCIR_MessageIds.hpp"
#include    "CQCIR_Type.hpp"
#include    "CQCIR_Constant.hpp"

#include    "CQCIR_BlasterCmd.hpp"
#include    "CQCIR_BlasterDevModel.hpp"
#include    "CQCIR_BlasterDevModelInfo.hpp"
#include    "CQCIR_BlasterCfg.hpp"
#include    "CQCIR_BlasterPersistData.hpp"

#include    "CQCIR_ReceiverCfg.hpp"
#include    "CQCIR_RepoClientProxy.hpp"


// ---------------------------------------------------------------------------
//  We have to slip this one in here, instead of in the types header, because
//  the facility class needs to see it.
// ---------------------------------------------------------------------------
namespace tCQCIR
{
    typedef TCntPtr<TIRRepoClientProxy>     TRepoSrvProxy;
}

#include    "CQCIR_ActionsThread.hpp"
#include    "CQCIR_DriverBase.hpp"
#include    "CQCIR_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCIREXPORT TFacCQCIR& facCQCIR();

