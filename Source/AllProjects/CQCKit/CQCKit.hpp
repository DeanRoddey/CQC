//
// FILE NAME: CQCKit.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/04/2000
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
//  This is the main header of the CQC toolkit. Clients wishing to develop
//  drivers or clients only have to include this one file. It will define and
//  sub-include anything else needed, in the correct order.
//
// CAVEATS/GOTCHAS:
//
//  1)  We desparately need to reduce the number of headers that this facility
//      imposes on downstream clients. So unusual levels of effort are taken
//      on this front. Headers that are actually used by client code, but
//      only a few, are actually brought in privately and those client
//      facilities include them directly. This is against the rules per se,
//      but the build times for CQC are way too large and we have to do
//      anything possible to reduce them.
//
//      It's very difficult to break any of the classes defined here out
//      into a separate facility in a way that would be worth the effort in
//      terms of reduction of build times.
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDMData.hpp"
#include    "CIDComm.hpp"
#include    "CIDSock.hpp"
#include    "CIDOrb.hpp"

//
//  We have to bring in the low level CIDCtrls types header, since some of our
//  interfaces reference the extended keys values. But we can't bring in the
//  main CIDCtrls header, since this is the fundamental CQC facility and is used
//  in back end ustff all the time.
//
#if         !defined(CIDKERNEL_TYPE_CTRLS)
#include    "CIDKernel_Type_Ctrls.hpp"
#endif


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCKIT)
#define CQCKITEXPORT    CID_DLLEXPORT
#else
#define CQCKITEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Some forward refs
// ---------------------------------------------------------------------------
class TRegEx;
class TCQCDriverObjCfg;
class TCQCServerBase;
class TFldIOPacket;
class TXMLTreeElement;
class TXMLTreeParser;
class TCQCUserAccount;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCKit_ErrorIds.hpp"
#include    "CQCKit_MessageIds.hpp"
#include    "CQCKit_Shared.hpp"
#include    "CQCKit_Type.hpp"
#include    "CQCKit_Constant.hpp"

#include    "CQCKit_CmdIntf.hpp"

#include    "CQCKit_Expression.hpp"
#include    "CQCKit_FieldDef.hpp"
#include    "CQCKit_FldLimits.hpp"
#include    "CQCKit_FldValues.hpp"
#include    "CQCKit_FldStorage.hpp"
#include    "CQCKit_ActVariable.hpp"

#include    "CQCKit_ConnCfg.hpp"
#include    "CQCKit_IPConnCfg.hpp"
#include    "CQCKit_OtherConnCfg.hpp"
#include    "CQCKit_SerialConnCfg.hpp"
#include    "CQCKit_UPnPConnCfg.hpp"

#include    "CQCKit_SecurityChallenge.hpp"
#include    "CQCKit_SecurityToken.hpp"
#include    "CQCKit_UserContext.hpp"


namespace tCQCKit
{
    // -----------------------------------------------------------------------
    //  A fundamental list of device categories. This is used by the driver
    //  configuration.
    // -----------------------------------------------------------------------
    using TDevClassList = TFundVector<tCQCKit::EDevClasses>;


    // -----------------------------------------------------------------------
    //  A safe bit set of device categories, which is used a good bit, and a
    //  fundamental vector of them.
    // -----------------------------------------------------------------------
    using TDevCatList = TFundVector<tCQCKit::EDevCats>;
    using TDevCatSet  = TEnumBitset<tCQCKit::EDevCats, tCQCKit::EDevCats::Count>;


    // -----------------------------------------------------------------------
    //  Some commonly used collection types
    // -----------------------------------------------------------------------
    using TFldTypeList  = TFundVector<tCQCKit::EFldTypes>;
    using TFldSTypeList = TFundVector<tCQCKit::EFldSTypes>;
    using TFldDefList   = TVector<TCQCFldDef>;
    using TDrvObjCfgCol = TCollection<TCQCDriverObjCfg>;
}

#include    "CQCKit_DrvStats.hpp"
#include    "CQCKit_FldIOPacket.hpp"
#include    "CQCKit_FieldFilter.hpp"
#include    "CQCKit_ModuleInfo.hpp"
#include    "CQCKit_DriverCfg.hpp"
#include    "CQCKit_DriverObjCfg.hpp"
#include    "CQCKit_Event.hpp"
#include    "CQCKit_VarDrvCfg.hpp"

#include    "CQCKit_UserAccount.hpp"

#include    "CQCKit_Package.hpp"
#include    "CQCKit_DevClass.hpp"

#include    "CQCKit_AppCtrlClientProxy.hpp"
#include    "CQCKit_ClSrvClientProxy.hpp"
#include    "CQCKit_IVCtrlClientProxy.hpp"
#include    "CQCKit_PowerStateMon.hpp"
#include    "CQCKit_CQCSrvAdminClientProxy.hpp"
#include    "CQCKit_FldInfoCache.hpp"
#include    "CQCKit_SecurityClientProxy.hpp"

#include    "CQCKit_StdCmdSrc.hpp"
#include    "CQCKit_KeyMaps.hpp"
#include    "CQCKit_InstClientProxy.hpp"

namespace tCQCKit
{
    // -----------------------------------------------------------------------
    //  We create counted pointer objects for some of the client proxies
    //  that we provide in CQCKit. The actually headers are internal and the
    //  small number of client facilities that need these can include those
    //  headers as required, so that we can reduce the compile overhead of
    //  this fundamental facility.
    // -----------------------------------------------------------------------
    using TIVCtrlSrvProxy   = TCntPtr<TIntfCtrlClientProxy>;
    using TInstSrvProxy     = TCntPtr<TCQCInstClientProxy>;
    using TCQCSrvProxy      = TCntPtr<TCQCSrvAdminClientProxy>;
    using TSecuritySrvProxy = TCntPtr<TCQCSecureClientProxy>;


    // -----------------------------------------------------------------------
    //  A ref vector of command source objects
    // -----------------------------------------------------------------------
    using TCmdSrcList       = TRefVector<MCQCCmdSrcIntf>;


    // -----------------------------------------------------------------------
    //  A list of email accounts
    // -----------------------------------------------------------------------
    using TEMailAcctList    = TVector<TCQCEMailAccount>;


    // -----------------------------------------------------------------------
    //  A ref vector of field values. We commonly have a need to keep a list
    //  of field values, and they are always polymorphic basically.
    // -----------------------------------------------------------------------
    using TFldValList       = TRefVector<TCQCFldValue>;


    // -----------------------------------------------------------------------
    //  A counted pointer to an event object. We use this during handling of
    //  trigged events so as to avoid undue replication of the event data.
    // -----------------------------------------------------------------------
    using TCQCEvPtr         = TCntPtr<const TCQCEvent>;


    // -----------------------------------------------------------------------
    //  A list of driver configuration objects
    // -----------------------------------------------------------------------
    using TDrvCfgList       = TVector<TCQCDriverObjCfg>;


    // -----------------------------------------------------------------------
    //  A list of action variables, keyed by the variable name. Mostly used
    //  by the standard variable action targets.
    // -----------------------------------------------------------------------
    using TVarList          = TRefKeyedHashSet<TCQCActVar,TString,TStringKeyOps>;


    // -----------------------------------------------------------------------
    //  A list of user accounts
    // -----------------------------------------------------------------------
    using TUserAcctList     = TVector<TCQCUserAccount>;
}

#include    "CQCKit_SecurityData.hpp"
#include    "CQCKit_EvMonCfg.hpp"
#include    "CQCKit_SchEvent.hpp"
#include    "CQCKit_TrgEvent.hpp"
#include    "CQCKit_VarsTarget.hpp"
#include    "CQCKit_ActEngine.hpp"
#include    "CQCKit_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCKITEXPORT TFacCQCKit& facCQCKit();


