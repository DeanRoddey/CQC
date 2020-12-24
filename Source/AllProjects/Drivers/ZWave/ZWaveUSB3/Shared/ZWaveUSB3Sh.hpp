//
// FILE NAME: ZWaveUSB3Sh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This is the public facility header. It defines and includes stuff that
//  is publically visible
//
//  This library provides a few different major types of functionality:
//
//  1.  The 'unit info' class which contains the configuration (user provided/discovered)
//      information. We have one of these per known unit. Some of it is loaded from
//      'device info' files once we have identified a unit (via its manufacturer ids)
//      or the user has forced us to set a particular device type.
//  2.  A base unit handler class and derivatives for each specific (or generic) type
//      of unit we can handle. The unit info class creates one of these at runtime for
//      its unit. Where possible we have generic handlers that can be parameterized (via
//      extra data in the device info files), but we can have very specific ones if we
//      need to.
//  3.  A set of name spaces that implement helper methods for particular Z-Wave command
//      classes. These are basically plumbing type classes, not ones that are reflected
//      by driver fields. So security, naming, association, that sort of stuff. Mostly
//      these are used by the unit info class since it handles all the housekeeping of
//      managing the information ABOUT the unit. These are CC helpers.
//  4.  A set of classes that represent Z-Wave command classes that do need to create
//      driver fields. Since any given unit might mix two or more command classes, sometimes
//      a good many more, we don't want every handler class to have to deal with the
//      details over and over again. Not just msg I/O formats, but creating and managing
//      the fields, field writes to out msgs, and pulling info out of incoming msgs.
//      So any given handler can create an instance (or more if it has multiple end points)
//      of any of these helpers they need and delegate stuff to them as appropriate. These
//      take parameters as required to enforce any options that CC may support, to meet
//      the needs of the handler object. These are CC implementations.
//  5.  These CC impls are typically split into two types, one of which just accepts a
//      value as a notification and stores it internally, and another that creates a field
//      and can interact with the unit if the unit is of that type. In many cases a
//      particular field can be read/written via one CC but received async notifications
//      via another. The owning unit handler has to create both types and keep the two
//      in sync when it sees a value change from either one.
//  6.  There is a base class for these CC impl classes to hold some common stuff like
//      its end point (if it is representing one), the command class it represents, some
//      runtime housekeeping stuff, and so forth.
//
//  There is one big special case to #4. Because we want to use generic handlers as much
//  as possible, and those by definition cannot deal with details of the specific unit
//  it is talking to. So, for a small set of very common CCs, the base handler class (#2)
//  will watch for those CCs and provide support for any fields that CC requires. This way,
//  even generic handlers can support these common ones. These are:
//
//  1. Battery level. A lot of units have batteries and expose the battery level
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_ZWAVEUSB3SH)
#define ZWUSB3SHEXPORT  CID_DLLEXPORT
#else
#define ZWUSB3SHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the underlying stuff we need
// ---------------------------------------------------------------------------
#include "CIDMath.hpp"
#include "CIDXML.hpp"
#include "CIDCrypto.hpp"
#include "CIDMData.hpp"

#include "CQCKit.hpp"
#include "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kZWaveUSB3Sh
{
    // -----------------------------------------------------------------------
    //  The maximum number of units that the Z-Wave networks supports
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxUnits  = 232;


    // -----------------------------------------------------------------------
    //  The minimum payload size for the simplest command class message.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MinCCPayload = 5;


    // -----------------------------------------------------------------------
    //  How many seconds we will allow a nonce to be valid for. For the ones we create
    //  for other nodes we are fairly forgiving. For the ones we store for subsequent
    //  use to send out msgs, we are conservative to make sure he won't have timed them
    //  out before we use them.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4InNonceValidSecs = 25;
    const tCIDLib::TCard4   c4OutNonceValidSecs = 10;


    // -----------------------------------------------------------------------
    //  The maximum number of byte counters supported by the outgoing message
    //  class.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4OutMsgCounters = 4;


    // -----------------------------------------------------------------------
    //  The bytes in a security nonce
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4NonceBytes = 8;
}


// ---------------------------------------------------------------------------
//  Some forward references
// ---------------------------------------------------------------------------
class TZWUnitInfo;


// ---------------------------------------------------------------------------
//  Bring in our own public headers.
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_ErrorIds.hpp"
#include    "ZWaveUSB3Sh_MessageIds.hpp"
#include    "ZWaveUSB3Sh_Shared.hpp"

namespace tZWaveUSB3Sh
{
    typedef TFundVector<tZWaveUSB3Sh::ECClasses>    TClassList;

    // The states outgoing msgs have to go through, though not always all of them
    enum EOMStates
    {
        EOMState_Working
        , EOMState_WaitEncrypt
        , EOMState_ReadyToSend
    };

    // Returned from the eHandleCCMsg() method of the CC impl class
    enum ECCMsgRes
    {
        ECCMsgRes_Unhandled
        , ECCMsgRes_Handled
        , ECCMsgRes_Value
    };

};

#include    "ZWaveUSB3Sh_DrvXData.hpp"
#include    "ZWaveUSB3Sh_InMsg.hpp"
#include    "ZWaveUSB3Sh_CtrlIntf.hpp"
#include    "ZWaveUSB3Sh_CCImpl.hpp"
#include    "ZWaveUSB3Sh_DevInfo.hpp"

namespace tZWaveUSB3Sh
{
    typedef TRefVector<TZWCCImpl>       TCCImplList;
}

#include    "ZWaveUSB3Sh_UnitInfo.hpp"
#include    "ZWaveUSB3Sh_Unit.hpp"
#include    "ZWaveUSB3Sh_OutMsg.hpp"

// Command class helper namespaces
#include    "ZWaveUSB3Sh_CCHelp_Assoc.hpp"
#include    "ZWaveUSB3Sh_CCHelp_Config.hpp"
#include    "ZWaveUSB3Sh_CCHelp_MultiCh.hpp"
#include    "ZWaveUSB3Sh_CCHelp_Naming.hpp"
#include    "ZWaveUSB3Sh_CCHelp_Thermo.hpp"
#include    "ZWaveUSB3Sh_CCHelp_Wakeup.hpp"


namespace tZWaveUSB3Sh
{
    typedef TVector<TZWUnitInfo>        TUnitInfoList;
    typedef TVector<TZWUSB3EndPntInfo>  TEPInfoList;
}

#include    "ZWaveUSB3Sh_DrvCfg.hpp"
#include    "ZWaveUSB3Sh_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern ZWUSB3SHEXPORT TFacZWaveUSB3Sh& facZWaveUSB3Sh();
