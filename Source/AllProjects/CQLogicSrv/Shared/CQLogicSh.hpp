//
// FILE NAME: CQLogicSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQLOGICSH)
#define CQLOGICSHEXPORT  CID_DLLEXPORT
#else
#define CQLOGICSHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CIDLib.hpp"
#include "CIDMath.hpp"
#include "CIDOrbUC.hpp"

#include "CQCKit.hpp"
#include "CQCPollEng.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TCQLSrvFldType;


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kCQLogicSh
{
    // -----------------------------------------------------------------------
    //  Constants that both sides need to see
    //
    //  We bumped the maximum source fields from 8 to 16 for 4.8.917.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MaxFields = 92;
    constexpr tCIDLib::TCard4   c4MaxSrcFields = 16;


    // -----------------------------------------------------------------------
    //  The number of samples in a graph field, and the maximum number of
    //  hours of collection that will be done (effectively 8 hours since there
    //  are 60 samples and the user indicates 1, 2, 3, etc... minutes between
    //  each sample.) So each value is another hour. We also have the number
    //  of seconds between each sub-sample.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4GraphSampleCnt = 60;
    constexpr tCIDLib::TCard4   c4MaxGraphCol = 8;
    constexpr tCIDLib::TCard4   c4SubSamplSecs = 10;


    // -----------------------------------------------------------------------
    //  The sample value we store if the field was in error or not available
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TFloat4  f4SampleErr(kCIDLib::f4MaxFloat * -1);
}


// ---------------------------------------------------------------------------
//  Public facility types
// ---------------------------------------------------------------------------
namespace tCQLogicSh
{
    // -----------------------------------------------------------------------
    //  The result of an evaluation of a field. It tells the server side driver
    //  what to do in response. If a new value, it needs to store that new
    //  value in the actual field. Else it may need to put it into error state
    //  if any of the source fields can't be evaluated.
    //
    //  If none of the source fields have changed since the last time, it just
    //  retursn no change.
    //
    //  The order is important!
    // -----------------------------------------------------------------------
    enum class EEvalRes
    {
        Error
        , NewValue
        , NoChange
    };


    // -----------------------------------------------------------------------
    // The return from our facility class' graph samples update query.
    // -----------------------------------------------------------------------
    enum class EGraphQRes
    {
        Error
        , NoNewSamples
        , NewSamples
    };


    // -----------------------------------------------------------------------
    //  Used by the min/max/average field type. It indicates which of those
    //  ways it will derive its value.
    // -----------------------------------------------------------------------
    enum class EMMATypes
    {
        Minimum
        , Maximum
        , Average
    };


    // -----------------------------------------------------------------------
    //  Each of our virtual fields (other than the passive ones) needs to
    //  track a list of fields that they use to derive their composite final
    //  value. This is stored at the base field types class. It's the usual
    //  moniker.field type indicator of the name of the source field.
    // -----------------------------------------------------------------------
    using TSrcList = tCIDLib::TStrList;


    // -----------------------------------------------------------------------
    //  Though it's a server side driver thing, it just makes sense to have
    //  the base field type class maintain a list of poll info objects for
    //  each of the source fields that feed the virtual field's value, so that
    //  all the details can be hidden.
    // -----------------------------------------------------------------------
    using TInfoList = TVector<TCQCFldPollInfo>;


    // -----------------------------------------------------------------------
    //  And we need a by ref list of field types, since we need to store a
    //  polymorphic list of field type objects as the main component of our
    //  configuration data.
    // -----------------------------------------------------------------------
    using TFldList = TRefVector<TCQLSrvFldType>;


    // -----------------------------------------------------------------------
    //  We need this for the client proxy interface
    // -----------------------------------------------------------------------
    using TSampleList = TFundVector<tCIDLib::TFloat4>;
}


// ---------------------------------------------------------------------------
//  Provide streaming support for some enums. This coudl be done via IDL< but it's
//  the only one we really have that needs streaming.
// ---------------------------------------------------------------------------
EnumBinStreamMacros(tCQLogicSh::EMMATypes)



// ---------------------------------------------------------------------------
//  Bring in our own headers
// ---------------------------------------------------------------------------
#include    "CQLogicSh_ErrorIds.hpp"
#include    "CQLogicSh_FldTypes.hpp"
#include    "CQLogicSh_BoolFld.hpp"
#include    "CQLogicSh_ElapsedTmFld.hpp"
#include    "CQLogicSh_GraphFld.hpp"
#include    "CQLogicSh_MathFld.hpp"
#include    "CQLogicSh_OnCounterFld.hpp"
#include    "CQLogicSh_RunAvgFld.hpp"
#include    "CQLogicSh_Config.hpp"

#include    "CQLogicSh_LSIntfClientProxy.hpp"


namespace tCQLogicSh
{
    // -----------------------------------------------------------------------
    //  A counted pointer to our client proxy. We have a helper method on the
    //  facility class that will return a proxy in this form.
    // -----------------------------------------------------------------------
    using TLogicSrvProxy = TCntPtr<TLogicSrvClientProxy>;
}

#include    "CQLogicSh_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern CQLOGICSHEXPORT TFacCQLogicSh& facCQLogicSh();

