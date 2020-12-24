//
// FILE NAME: ZWaveLevi2Sh_CmdClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/28/2014
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
//  This file provides the implementation for the TZWCmdClass class, which is the
//  base class for all fo the pluggable command class handlers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCmdClass,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLevi2Sh_CmdClass
{
    // -----------------------------------------------------------------------
    //  The active ping interval. If we haven't heard anything in this long,
    //  then we'll do an active poll even if polling is not otherwise enabled
    //  for this class.
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    const tCIDLib::TEncodedTime enctPingInt     = kCIDLib::enctOneMinute * 15;
    #else
    const tCIDLib::TEncodedTime enctPingInt     = kCIDLib::enctOneMinute * 30;
    #endif

}



// ---------------------------------------------------------------------------
//   CLASS: MZWLeviSrvFuncs
//  PREFIX: mzwsf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MZWLeviSrvFuncs: Constructors and Destructor
// ---------------------------------------------------------------------------
MZWLeviSrvFuncs::MZWLeviSrvFuncs()
{
}

MZWLeviSrvFuncs::~MZWLeviSrvFuncs()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCmdClass
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCmdClass: Destructor
// ---------------------------------------------------------------------------
TZWCmdClass::~TZWCmdClass()
{
}


// ---------------------------------------------------------------------------
//  TZWCmdClass: Public, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCmdClass::bHandleGenReport(  const   tCIDLib::TCardList&
                                , const TZWaveUnit&
                                ,       MZWLeviSrvFuncs&
                                , const tCIDLib::TCard4)
{
    return kCIDLib::False;
}

// Some don't have any fields, so we just return false if not overridden
tCIDLib::TBoolean TZWCmdClass::bOwnsFld(const tCIDLib::TCard4) const
{
    return kCIDLib::False;
}


// If not overridden, we just say we didn't do anything
tCIDLib::TBoolean TZWCmdClass::bQueryFldVals(TZWaveUnit&, MZWLeviSrvFuncs&)
{
    return kCIDLib::False;
}


//
//  For any of these that aren't overridden, just say we reject the field write.
//
tCQCKit::ECommResults
TZWCmdClass::eWriteBoolFld( const   tCIDLib::TCard4
                            , const tCIDLib::TBoolean
                            ,       TZWaveUnit&
                            ,       MZWLeviSrvFuncs&)
{
    return tCQCKit::ECommResults::ValueRejected;
}

tCQCKit::ECommResults
TZWCmdClass::eWriteCardFld( const   tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            ,       TZWaveUnit&
                            ,       MZWLeviSrvFuncs&)
{
    return tCQCKit::ECommResults::ValueRejected;
}

tCQCKit::ECommResults
TZWCmdClass::eWriteIntFld(  const   tCIDLib::TCard4
                            , const tCIDLib::TInt4
                            ,       TZWaveUnit&
                            ,       MZWLeviSrvFuncs&)
{
    return tCQCKit::ECommResults::ValueRejected;
}

tCQCKit::ECommResults
TZWCmdClass::eWriteFloatFld(const   tCIDLib::TCard4
                            , const tCIDLib::TFloat8
                            ,       TZWaveUnit&
                            ,       MZWLeviSrvFuncs&)
{
    return tCQCKit::ECommResults::ValueRejected;
}

tCQCKit::ECommResults
TZWCmdClass::eWriteStringFld(const  tCIDLib::TCard4
                            , const TString&
                            ,       TZWaveUnit&
                            ,       MZWLeviSrvFuncs&)
{
    return tCQCKit::ECommResults::ValueRejected;
}



// Some classes have no options, so we provide an empty default
tCIDLib::TVoid
TZWCmdClass::LoadOptVals(const  TZWDevClass&
                        ,       tZWaveLevi2Sh::TOptValList&
                        ,       TZWOptionVal&) const
{
}

tCIDLib::TVoid TZWCmdClass::LoadOptList(tZWaveLevi2Sh::TOptList&) const
{
}


// If not overridden, we do nothing. Not all classes create fields
tCIDLib::TVoid
TZWCmdClass::QueryFldDefs(          tCQCKit::TFldDefList&
                            ,       TCQCFldDef&
                            , const TZWaveUnit&
                            ,       TString&
                            ,       TString&) const
{
}


tCIDLib::TVoid
TZWCmdClass::StoreFldIds(const  TZWaveUnit&
                        , const MZWLeviSrvFuncs&
                        ,       TString&
                        ,       TString& )
{
}


// ---------------------------------------------------------------------------
//  TZWCmdClass: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWCmdClass::bAllZeroInst() const
{
    return m_bAllZeroInst;
}


// Return our read/write overwride flags
tCIDLib::TBoolean TZWCmdClass::bCanRead() const
{
    return m_bCanRead;
}

tCIDLib::TBoolean TZWCmdClass::bCanWrite() const
{
    return m_bCanWrite;
}


//
//  The driver will call this periodically to give us a chance to poll if we need
//  to. We just return whether we want to or not. The derived class may not actually
//  respond to the call to do the field value queries, but that's fine. We have
//  discharged our duties.
//
//  Derived class that are never readable can always overwrite m_bCanRead if they want,
//  to make us always say no, which is a bit more efficient. If not, returning true
//  from here will just make the driver do a useless call to it's field reading method.
//
//  Note that, if we return true, we call ResetTimeStamp() to reset the stamps. The
//  derived class will call it again if this causes a poll and new data comes in. But
//  this insures, just in case, that the time stamp always gets reset. Else we could
//  start beating the device every time this is called.
//
tCIDLib::TBoolean TZWCmdClass::bCheckPollReady()
{
    // If we are not readable, then obviously not
    if (!m_bCanRead)
        return kCIDLib::False;

    //
    //  If marked as needing polling and it's been over the polling interval, then
    //  say we want to.
    //
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (m_bPoll)
    {
        if (enctNow > m_enctNextPoll)
        {
            ResetTimeStamp();
            return kCIDLib::True;
        }

        // Not time yet
        return kCIDLib::False;
    }

    // We aren't polling,  but see if it's time for a ping.
    if (enctNow > m_enctNextPing)
    {
        ResetTimeStamp();
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


//
//  If we have the indicated command class and instance id, then return true to
//  say we are this dude.
//
tCIDLib::TBoolean
TZWCmdClass::bIsThisObject( const   tZWaveLevi2Sh::ECClasses    eCheck
                            , const tCIDLib::TCard4             c4InstId) const
{
    return ((eCheck == m_eClass) && (c4InstId == m_c4InstId));
}


// Return our secure flag
tCIDLib::TBoolean TZWCmdClass::bSecure() const
{
    return m_bSecure;
}


//
//  Get/set the wait init vals flag. It's initialized to true. The driver will turn
//  it off for each class once they get their initial data successfully.
//
tCIDLib::TBoolean TZWCmdClass::bWaitInitVals() const
{
    return m_bWaitInitVals;
}

tCIDLib::TBoolean TZWCmdClass::bWaitInitVals(const tCIDLib::TBoolean bToSet)
{
    m_bWaitInitVals = bToSet;
    return m_bWaitInitVals;
}


// Return the general flags value
tCIDLib::TCard4 TZWCmdClass::c4Flags() const
{
    return m_c4Flags;
}



// The unit needs to know our inst id to know what msgs to send to us
tCIDLib::TCard4 TZWCmdClass::c4InstId() const
{
    return m_c4InstId;
}


// Return the Z_Wave level command class id for this class
tCIDLib::TCard4 TZWCmdClass::c4ZWClassId() const
{
    return m_c4ZWClassId;
}



tZWaveLevi2Sh::ECClasses TZWCmdClass::eCmdClass() const
{
    return m_eClass;
}


// Based on our read/write capabilities, return the appropriate field access
tCQCKit::EFldAccess TZWCmdClass::eFldAccess() const
{
    if (m_bCanRead && m_bCanWrite)
        return tCQCKit::EFldAccess::ReadWrite;

    if (m_bCanRead)
        return tCQCKit::EFldAccess::Read;

    if (m_bCanWrite)
        return tCQCKit::EFldAccess::Write;

    return tCQCKit::EFldAccess::None;
}


tZWaveLevi2Sh::EMultiTypes TZWCmdClass::eMultiType() const
{
    return m_eMultiType;
}


//
//  Update our next ping/poll time stamps. For the poll stamp, if a specific value
//  hasn't been set by device info or derived class, then we use 15 minutes.
//
tCIDLib::TVoid TZWCmdClass::ResetTimeStamp()
{
    m_enctNextPing = TTime::enctNow();
    m_enctNextPoll = m_enctNextPing;

    m_enctNextPing += ZWaveLevi2Sh_CmdClass::enctPingInt;
    m_enctNextPoll += (m_c4PollMins ? m_c4PollMins : 10) * kCIDLib::enctOneMinute;
}


const TString& TZWCmdClass::strInstDescr() const
{
    return m_strInstDescr;
}


const TString& TZWCmdClass::strInstName() const
{
    return m_strInstName;
}



// ---------------------------------------------------------------------------
//  TZWCmdClass: Hidden constructors and operators
// ---------------------------------------------------------------------------

TZWCmdClass::TZWCmdClass(const  tZWaveLevi2Sh::ECClasses    eCCmdClass
                        , const TZWDevClass&                zwdcInst
                        , const tCIDLib::TCard4             c4ZWClassId) :

    m_bAllZeroInst(zwdcInst.m_bAllZeroInst)
    , m_bCanRead(zwdcInst.m_bCanRead)
    , m_bCanWrite(zwdcInst.m_bCanWrite)
    , m_bPoll(zwdcInst.m_bPoll)
    , m_bSecure(zwdcInst.m_bSecure)
    , m_bWaitInitVals(kCIDLib::True)
    , m_c4Flags(zwdcInst.m_c4Flags)
    , m_c4InstId(zwdcInst.m_c4InstId)
    , m_c4PollMins(zwdcInst.m_c4PollMins)
    , m_c4SetRespCls(zwdcInst.m_c4SetRespCls)
    , m_c4SetRespCmd(zwdcInst.m_c4SetRespCmd)
    , m_c4ZWClassId(c4ZWClassId)
    , m_enctNextPing(0)
    , m_enctNextPoll(0)
    , m_eClass(eCCmdClass)
    , m_eMultiType(zwdcInst.m_eMultiType)
    , m_strInstDescr(zwdcInst.m_strDescr)
    , m_strInstName(zwdcInst.m_strName)
{
    // Call ResetTimeStamp() to get our time stamps initailized
    ResetTimeStamp();
}


// ---------------------------------------------------------------------------
//  TZWCmdClass: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The command classes commonly have to build instance names when creating fields
//  or looking them up. This is a helper to do that. We start with the unit name.
//
//  If we have an instance name driven by device info, then take that. Else, if
//  we have a default one passed in, take that. Else, if not instance 1, then
//  append our instance id.
//
tCIDLib::TVoid
TZWCmdClass::AppendInstName(const   TZWaveUnit&         unitMe
                            , const tCIDLib::TCh* const pszDefInst
                            ,       TString&            strToFill) const
{
    strToFill.Append(unitMe.strName());

    if (!m_strInstName.bIsEmpty())
    {
        strToFill.Append(kCIDLib::chUnderscore);
        strToFill.Append(m_strInstName);
    }
     else if (*pszDefInst)
    {
        strToFill.Append(kCIDLib::chUnderscore);
        strToFill.Append(pszDefInst);
    }
     else if (m_c4InstId > 1)
    {
        strToFill.Append(kCIDLib::chUnderscore);
        strToFill.AppendFormatted(m_c4InstId);
    }
}



//
//  A helper for derived classes to make sure that a field id has been initialized
//  before using it.
//
tCIDLib::TBoolean
TZWCmdClass::bCheckFldId(const  tCIDLib::TCard4     c4ToCheck
                        , const tCIDLib::TCh* const pszFldName
                        , const TZWaveUnit&         unitMe) const
{
    if (c4ToCheck == kCIDLib::c4MaxCard)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcUnit_FldIdNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , unitMe.strName()
            , TString(pszFldName)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  A general helper for extracting the variable size numbers that many device classes
//  return. It returns the value in either float or int formats. The caller indicates
//  whether it expects an int or float value.
//
tCIDLib::TBoolean
TZWCmdClass::bDecodeValue(  const   tCIDLib::TCard4     ac4Vals[]
                            , const tCIDLib::TCard4     c4ValCnt
                            , const tCIDLib::TBoolean   bExpectInt
                            ,       tCIDLib::TInt4&     i4Result
                            ,       tCIDLib::TFloat8&   f8Result
                            , const tCIDLib::TCh* const pszFld
                            , const MZWLeviSrvFuncs&    mzwsfLevi) const
{
    // The first value is bitmapped so pull the values out
    const tCIDLib::TCard4 c4Precision = (ac4Vals[0] & 0xE0) >> 5;
    const tCIDLib::TCard4 c4Scale = (ac4Vals[0] & 0x18) >> 3;
    const tCIDLib::TCard4 c4Size = (ac4Vals[0] & 7);

    //
    //  Use the size to pull the bytes out and build up the value. It is
    //  two's complement, so we have to be careful about how we treat it,
    //  to insure we get the appropriate signed value.
    //
    if (c4Size == 1)
    {
        tCIDLib::TCard1 c1Val = tCIDLib::TCard1(ac4Vals[1]);
        i4Result = tCIDLib::TInt1(c1Val);
    }
     else if (c4Size == 2)
    {
        tCIDLib::TCard2 c2Val = tCIDLib::TCard1(ac4Vals[1]);
        c2Val <<= 8;
        c2Val |= tCIDLib::TCard1(ac4Vals[2]);
        i4Result = tCIDLib::TInt2(c2Val);
    }
     else if (c4Size == 4)
    {
        tCIDLib::TCard4 c4Val = tCIDLib::TCard1(ac4Vals[1]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[2]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[3]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[4]);
        i4Result = tCIDLib::TInt4(c4Val);
    }
     else
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"%(1) is not a valid value size. Field=%(2)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4Size)
                , TString(pszFld)
            );
        }
        return kCIDLib::False;
    }


    //
    //  Dependng on what the caller wants, fill in the right result. Dividing by
    //  any precision indicated.
    //
    if (bExpectInt)
    {
        if (c4Precision)
            i4Result /= (10 * c4Precision);
    }
     else
    {
        f8Result = tCIDLib::TFloat8(i4Result);
        if (c4Precision)
            f8Result /= (10.0 * c4Precision);
    }

    // It worked
    return kCIDLib::True;
}


//
//  The command classes commonly have to build instance names when creating fields
//  or looking them up. This is a helper to do that. See AppendInstName above, which
//  we call.
//
tCIDLib::TVoid
TZWCmdClass::BuildInstName( const   TZWaveUnit&         unitMe
                            , const tCIDLib::TCh* const pszDefInst
                            ,       TString&            strToFill) const
{
    // Clear the target and call the append version.
    strToFill.Clear();
    AppendInstName(unitMe, pszDefInst, strToFill);
}


//
//  Because we can have multiple instances of command classes, this complicates the
//  setup of the option keys. So we provide this helper to do that. We just prefix
//  the base key name with our instance id followed by a period.
//
tCIDLib::TVoid
TZWCmdClass::BuildOptKey(const  tCIDLib::TCh* const pszKeyBase
                        ,       TString&            strKey) const
{
    strKey.AppendFormatted(m_c4InstId);
    strKey.Append(kCIDLib::chPeriod);
    strKey.Append(pszKeyBase);
}


// Expose the response class/cmd values to the derived class
tCIDLib::TCard4 TZWCmdClass::c4SetRespCls() const
{
    return m_c4SetRespCls;
}

tCIDLib::TCard4 TZWCmdClass::c4SetRespCmd() const
{
    return m_c4SetRespCmd;
}


//
//  Generally these are purely driven by the device info files. However, some classes
//  know that they cannot be read or written, and can overwrite these during ctor, after
//  they call our ctor (which defaults these from the device info.) This can make things
//  more efficient since the driver won't bother even giving them a chance to do some
//  things.
//
tCIDLib::TVoid
TZWCmdClass::SetAccess( const   tCIDLib::TBoolean   bCanRead
                        , const tCIDLib::TBoolean   bCanWrite)
{
    m_bCanRead = bCanRead;
    m_bCanWrite = bCanWrite;
}

