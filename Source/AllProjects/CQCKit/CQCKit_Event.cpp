//
// FILE NAME: CQCKit_Event.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2005
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
//  This file implements the representation of event triggers that are broadcast
//  to report changes system/device status of various types.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCEvent,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TDrvEvInfo
//  PREFIX: devi
//
//  This is justa simple class that we use for creating a table to deal with
//  the standard driver events. A static array of these is faulted in, in the
//  order of the tCQCKit::EStdDrvEvs enumeration.
// ---------------------------------------------------------------------------
class TDrvEvInfo
{
    public :
        TDrvEvInfo();

        ~TDrvEvInfo();


        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCh* const pszClass
            , const tCIDLib::TCh* const pszSrcType
            , const tCIDLib::TCh* const pszInfoBlk
            , const tCIDLib::TCh* const pszVal1
            , const tCIDLib::TCh* const pszVal2
            , const tCIDLib::TCh* const pszVal3
            , const tCIDLib::TCh* const pszVal4
        );

        TString     m_strClass;
        TString     m_strSrcType;
        TString     m_strInfoBlk;
        TString     m_strVal1;
        TString     m_strVal2;
        TString     m_strVal3;
        TString     m_strVal4;
};

TDrvEvInfo::TDrvEvInfo()
{
}

TDrvEvInfo::~TDrvEvInfo()
{
}

tCIDLib::TVoid
TDrvEvInfo::Set(const   tCIDLib::TCh* const pszClass
                , const tCIDLib::TCh* const pszSrcType
                , const tCIDLib::TCh* const pszInfoBlk
                , const tCIDLib::TCh* const pszVal1
                , const tCIDLib::TCh* const pszVal2
                , const tCIDLib::TCh* const pszVal3
                , const tCIDLib::TCh* const pszVal4)
{
    m_strClass   = pszClass;
    m_strSrcType = pszSrcType;
    m_strInfoBlk = pszInfoBlk;
    m_strVal1    = pszVal1;
    m_strVal2    = pszVal2;
    m_strVal3    = pszVal3;
    m_strVal4    = pszVal4;
}



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_Event
    {
        // -----------------------------------------------------------------------
        //  Our on the wire format version
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2  c2OTWVersion = 1;


        // -----------------------------------------------------------------------
        //  Our event text format version. Not really used but could be useful in
        //  the future
        // -----------------------------------------------------------------------
        const tCIDLib::TCh* const    pszEvFmtVer = L"1";


        // -----------------------------------------------------------------------
        //  An object array of driver event info objects. It will be lazily
        //  faulted in upon the first creation of an event object.
        // -----------------------------------------------------------------------
        TAtomicFlag                                 atomInfoLoaded;
        TObjArray<TDrvEvInfo, tCQCKit::EStdDrvEvs>  colDrvEvs(tCQCKit::EStdDrvEvs::Count);
    }
}





// ---------------------------------------------------------------------------
//  CLASS: TCQCEvent
// PREFIX: cev
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEvent: Public, static methods
//
//  Make sure these stay in sync with the contents of colDrvEvs above.
// ---------------------------------------------------------------------------
const TString TCQCEvent::strHdr_Std(L"cqsl.header");
const TString TCQCEvent::strPath_ActData(L"/cqsl.actinfo/evdata");
const TString TCQCEvent::strPath_ActType(L"/cqsl.actinfo/evtype");
const TString TCQCEvent::strPath_Class(L"/cqsl.header/class");
const TString TCQCEvent::strPath_FldValue(L"/cqsl.fldval/val");
const TString TCQCEvent::strPath_LoadName(L"/cqsl.loadinfo/name");
const TString TCQCEvent::strPath_LoadNum(L"/cqsl.loadinfo/loadnum");
const TString TCQCEvent::strPath_LoadState(L"/cqsl.loadinfo/state");
const TString TCQCEvent::strPath_LockId(L"/cqsl.lockInfo/lockid");
const TString TCQCEvent::strPath_LockState(L"/cqsl.lockInfo/state");
const TString TCQCEvent::strPath_LockType(L"/cqsl.lockInfo/type");
const TString TCQCEvent::strPath_LockCode(L"/cqsl.lockInfo/code");
const TString TCQCEvent::strPath_MotionName(L"/cqsl.motioninfo/name");
const TString TCQCEvent::strPath_MotionNum(L"/cqsl.motioninfo/sensornum");
const TString TCQCEvent::strPath_MotionType(L"/cqsl.motioninfo/type");
const TString TCQCEvent::strPath_PresArea(L"/cqsl.presenceinfo/area");
const TString TCQCEvent::strPath_PresIdInfo(L"/cqsl.presenceinfo/idinfo");
const TString TCQCEvent::strPath_PresType(L"/cqsl.presenceinfo/type");
const TString TCQCEvent::strPath_Source(L"/cqsl.header/source");
const TString TCQCEvent::strPath_SysID(L"/cqsl.header/sysid");
const TString TCQCEvent::strPath_Version(L"/cqsl.header/v");
const TString TCQCEvent::strPath_ZoneName(L"/cqsl.zoneinfo/name");
const TString TCQCEvent::strPath_ZoneNum(L"/cqsl.zoneinfo/zonenum");
const TString TCQCEvent::strPath_ZoneStatus(L"/cqsl.zoneinfo/state");
const TString TCQCEvent::strSrc_DevPref(L"cqsl.dev:");
const TString TCQCEvent::strSrc_FldPref(L"cqsl.field:");

const TString TCQCEvent::strVal_LockState_Locked(L"locked");
const TString TCQCEvent::strVal_LockState_Unlocked(L"unlocked");

const TString TCQCEvent::strVal_LockType_Manual(L"manual");
const TString TCQCEvent::strVal_LockType_Other(L"other");
const TString TCQCEvent::strVal_LockType_Pad(L"pad");
const TString TCQCEvent::strVal_LockType_Remote(L"remote");


// ---------------------------------------------------------------------------
//  TCQCEvent: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEvent::TCQCEvent() :

    m_c4LineNum(0)
    , m_colBlocks(tCIDLib::EMTStates::Unsafe, kCIDLib::False, kCIDLib::False)
{
    if (!CQCKit_Event::atomInfoLoaded)
        LoadInfo();
}

TCQCEvent::TCQCEvent(const TCQCEvent& cevSrc) :

    m_c4LineNum(0)
    , m_colBlocks(cevSrc.m_colBlocks)
{
}

TCQCEvent::TCQCEvent(TCQCEvent&& cevSrc) :

    TCQCEvent()
{
    *this = tCIDLib::ForceMove(cevSrc);
}

TCQCEvent::~TCQCEvent()
{
}


// ---------------------------------------------------------------------------
//  TCQCEvent: Public operators
// ---------------------------------------------------------------------------
TCQCEvent& TCQCEvent::operator=(const TCQCEvent& cevSrc)
{
    if (this != &cevSrc)
    {
        m_c4LineNum = 0;
        m_colBlocks = cevSrc.m_colBlocks;
    }
    return *this;
}

TCQCEvent& TCQCEvent::operator=(TCQCEvent&& cevSrc)
{
    if (this != &cevSrc)
    {
        tCIDLib::Swap(m_c4LineNum, cevSrc.m_c4LineNum);
        m_colBlocks = tCIDLib::ForceMove(cevSrc.m_colBlocks);
    }
    return *this;
}


//
//  Equality is sort of messy for us. We just iterate both trees and compare
//  node names and values. They must be in the same exact order to be equal.
//
//  The two ids are not used here. They are used for filtering when coming in
//  over the network.
//
tCIDLib::TBoolean TCQCEvent::operator==(const TCQCEvent& cevSrc) const
{
    TBlockCol::TConstCursor<TString> cursUs(&m_colBlocks);
    TBlockCol::TConstCursor<TString> cursSrc(&cevSrc.m_colBlocks);

    while (cursUs && cursSrc)
    {
        if (cursUs.eCurType() != cursSrc.eCurType())
            return kCIDLib::False;

        //
        //  In both terminal and non-terminal we care about the name of the node. For
        //  the former it's the name of the block. For the later, the key of the value.
        //
        if (cursUs.strCurName() != cursSrc.strCurName())
            return kCIDLib::False;

        // If a terminal, then the actual node data is the value
        if (cursUs.eCurType() == tCIDLib::ETreeNodes::Terminal)
        {
            if (*cursUs != *cursSrc)
                return kCIDLib::False;
        }

        ++cursUs;
        ++cursSrc;
    }

    // Both cursors must be invalid by now since had to get to the end of both
    return !cursUs.bIsValid() && !cursSrc.bIsValid();
}


// ---------------------------------------------------------------------------
//  TCQCEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add a new block under the passed parent block. If it's a dup, we'll
//  throw.
//
tCIDLib::TVoid TCQCEvent::AddBlock( const   TString&    strParent
                                    , const TString&    strName)
{
    try
    {
        m_colBlocks.pnodeAddNonTerminal(strParent, strName);
    }

    catch(TError& errToCatch)
    {
        // If a dump node, then throw our error, else rethrow original
        if (errToCatch.bCheckEvent(facCIDLib().strName(), kCIDErrs::errcCol_DupTreeNode))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvTrg_AlreadyExists
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , strParent
                , strName
            );
        }

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Add a new value under the passed parent block. If it's a dup, we'll
//  throw. They pass the initial string to set on this value.
//
tCIDLib::TVoid TCQCEvent::AddValue( const   TString&    strParent
                                    , const TString&    strName
                                    , const TString&    strValue)
{
    try
    {
        m_colBlocks.objAddTerminal(strParent, strName, strValue);
    }

    catch(TError& errToCatch)
    {
        // If a dump node, then throw our error, else rethrow original
        if (errToCatch.bCheckEvent(facCIDLib().strName(), kCIDErrs::errcCol_DupTreeNode))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvTrg_AlreadyExists
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , strParent
                , strName
            );
        }

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  These test this event to see if it matches various criteria
//
tCIDLib::TBoolean TCQCEvent::bIsDrvEv(const tCQCKit::EStdDrvEvs eToCheck) const
{
    // Fault in info if not already
    if (!CQCKit_Event::atomInfoLoaded)
        LoadInfo();

    // Get the class value. If not one, then obviously we fail
    const TString* pstrVal = m_colBlocks.pobjAt(strPath_Class);
    if (!pstrVal)
        return kCIDLib::False;

    //
    //  Use this to convert to the enum value. We just look for one that
    //  has the indicated class. If we don't find it, the value will end
    //  up on the ::Count value, which won't match anything passed.
    //
    tCQCKit::EStdDrvEvs eEvent = tCIDLib::eForEachE<tCQCKit::EStdDrvEvs>
    (
        [pstrVal](const auto eCur)
        { return (CQCKit_Event::colDrvEvs[eCur].m_strClass != *pstrVal); }
    );

    // And now just return whether it matches the passed event value
    return (eEvent == eToCheck);
}


tCIDLib::TBoolean
TCQCEvent::bIsDrvEvForDev(  const   tCQCKit::EStdDrvEvs eToCheck
                            , const TString&            strDev)   const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    //
    //  Get the source. If it is a device type, we just grab that. If it's
    //  a field type, we have to pull out just the device part.
    //
    TString strSrcDev = m_colBlocks.objAt(strPath_Source);

    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];

    if (deviThis.m_strSrcType == strSrc_DevPref)
    {
        strSrcDev.Cut(0, strSrc_DevPref.c4Length());
    }
     else
    {
        strSrcDev.Cut(0, strSrc_FldPref.c4Length());
        TString strTmp;
        strSrcDev.bSplit(strTmp, L'.');
    }

    if (strDev.bCompareI(strSrcDev))
        return kCIDLib::True;
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCEvent::bIsDrvEvForField(const   tCQCKit::EStdDrvEvs eToCheck
                            , const TString&            strFldName) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    // Get the source. If it is a device type, thenit cannot be a match.
    TString strSrc = m_colBlocks.objAt(strPath_Source);
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strSrcType == strSrc_DevPref)
        return kCIDLib::False;

    // Get the field part out and compare
    strSrc.Cut(0, strSrc_FldPref.c4Length());
    if (strFldName.bCompareI(strSrc))
        return kCIDLib::True;
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCEvent::bIsDrvEvVal( const   tCQCKit::EStdDrvEvs eToCheck
                        , const TString&            strFrom
                        , const TString&            strVal1) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    //
    //  Quick sanity check, if this type has no info block or no value 1,
    //  obviously not.
    //
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strInfoBlk.bIsEmpty() || deviThis.m_strVal1.bIsEmpty())
        return kCIDLib::False;

    //
    //  Get the source and break out the part we want to compare. If it
    //  doesn't match the from string, we failed.
    //
    TString strSrcDev = m_colBlocks.objAt(strPath_Source);
    if (deviThis.m_strSrcType == strSrc_DevPref)
    {
        strSrcDev.Cut(0, strSrc_DevPref.c4Length());
    }
     else
    {
        strSrcDev.Cut(0, strSrc_FldPref.c4Length());
    }

    if (!strFrom.bCompareI(strSrcDev))
        return kCIDLib::False;

    // Build the path to value 1 and compare it
    TString strPath(L"/");
    strPath.Append(deviThis.m_strInfoBlk);
    strPath.Append(L'/');
    strPath.Append(deviThis.m_strVal1);

    return (m_colBlocks.objAt(strPath) == strVal1);
}


tCIDLib::TBoolean
TCQCEvent::bIsDrvEvVals(const   tCQCKit::EStdDrvEvs eToCheck
                        , const TString&            strFrom
                        , const TString&            strVal1
                        , const TString&            strVal2) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    //
    //  Quick sanity check, if this type has no info block or no value 2,
    //  obviously not.
    //
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strInfoBlk.bIsEmpty() || deviThis.m_strVal2.bIsEmpty())
        return kCIDLib::False;

    //
    //  Get the source and break out the part we want to compare. If it
    //  doesn't match the from string, we failed.
    //
    TString strSrcDev = m_colBlocks.objAt(strPath_Source);
    if (deviThis.m_strSrcType == strSrc_DevPref)
    {
        strSrcDev.Cut(0, strSrc_DevPref.c4Length());
    }
     else
    {
        strSrcDev.Cut(0, strSrc_FldPref.c4Length());
    }

    if (!strFrom.bCompareI(strSrcDev))
        return kCIDLib::False;

    // Build the path to value 1 and compare it
    TString strPath(L"/");
    strPath.Append(deviThis.m_strInfoBlk);
    strPath.Append(L'/');
    const tCIDLib::TCard4 c4CapInd(strPath.c4Length());
    strPath.Append(deviThis.m_strVal1);

    if (m_colBlocks.objAt(strPath) != strVal1)
        return kCIDLib::False;

    // And check the second value
    strPath.CapAt(c4CapInd);
    strPath.Append(deviThis.m_strVal2);

    if (m_colBlocks.objAt(strPath) != strVal2)
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  Checks for a specific event, with specific values, but not the source
//  that it came from.
//
tCIDLib::TBoolean
TCQCEvent::bIsThisDrvEv(const   tCQCKit::EStdDrvEvs eToCheck
                        , const TString&            strVal1) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    // Quick sanity check, if this type has no info block, then we failed
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strInfoBlk.bIsEmpty())
        return kCIDLib::False;

    // Build the path to value 1 and compare it
    TString strPath(L"/");
    strPath.Append(deviThis.m_strInfoBlk);
    strPath.Append(L'/');
    strPath.Append(deviThis.m_strVal1);
    if (m_colBlocks.objAt(strPath) != strVal1)
        return kCIDLib::False;

    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCEvent::bIsThisDrvEv(const   tCQCKit::EStdDrvEvs eToCheck
                        , const TString&            strVal1
                        , const TString&            strVal2) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    //
    //  Quick sanity check, if this type has no info block or second value,
    //  then we failed.
    //
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strInfoBlk.bIsEmpty() || deviThis.m_strVal2.bIsEmpty())
        return kCIDLib::False;

    // Build the path to value 1 and compare it
    TString strPath(L"/");
    strPath.Append(deviThis.m_strInfoBlk);
    strPath.Append(L'/');
    const tCIDLib::TCard4 c4CapInd(strPath.c4Length());
    strPath.Append(deviThis.m_strVal1);
    if (m_colBlocks.objAt(strPath) != strVal1)
        return kCIDLib::False;

    // Check value two
    strPath.CapAt(c4CapInd);
    strPath.Append(deviThis.m_strVal2);
    if (m_colBlocks.objAt(strPath) != strVal2)
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCEvent::bIsThisDrvEvV2(  const   tCQCKit::EStdDrvEvs eToCheck
                            , const TString&            strVal2) const
{
    //
    //  Call the more basic one and if that fails, it's not of the
    //  indicated type obviously.
    //
    if (!bIsDrvEv(eToCheck))
        return kCIDLib::False;

    //
    //  Quick sanity check, if this type has no info block or second value,
    //  then we failed.
    //
    const TDrvEvInfo& deviThis = CQCKit_Event::colDrvEvs[eToCheck];
    if (deviThis.m_strInfoBlk.bIsEmpty() || deviThis.m_strVal2.bIsEmpty())
        return kCIDLib::False;

    // Build the path to value 2 and compare it
    TString strPath(L"/");
    strPath.Append(deviThis.m_strInfoBlk);
    strPath.Append(L'/');
    strPath.Append(deviThis.m_strVal2);
    if (m_colBlocks.objAt(strPath) != strVal2)
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  A helper method that just checks to see if this event is of the
//  indicated class.
//
tCIDLib::TBoolean TCQCEvent::bIsOfClass(const TString& strClass) const
{
    // Get the class field. If not found, then obviously not equal
    const TString* pstrVal = m_colBlocks.pobjAt(strPath_Class);
    if (!pstrVal)
        return kCIDLib::False;

    // See if it equals the passed value
    return (*pstrVal == strClass);
}


//
//  Checks to see if the event is one that has a field source. If so, it
//  pulls that out and splits it into moniker and field.
//
tCIDLib::TBoolean
TCQCEvent::bQueryFldSrc(TString& strMoniker, TString& strField) const
{
    // Get the source, if none, then shouldn't happen, but obviously not
    const TString* pstrVal = m_colBlocks.pobjAt(strPath_Source);
    if (!pstrVal)
        return kCIDLib::False;

    //
    //  If a field change, then we have both a moniker and field. Get out
    //  the source path and pull off the prefix.
    //
    if (pstrVal->bStartsWith(strSrc_FldPref))
    {
        strMoniker = *pstrVal;
        strMoniker.Cut(0, strSrc_FldPref.c4Length());

        // Now split out the field part
        return strMoniker.bSplit(strField, kCIDLib::chPeriod);
    }
    return kCIDLib::False;
}


//
//  Checks to see if the source is either field or device and if so it
//  pulls out the field or device moniker and returns it.
//
tCIDLib::TBoolean TCQCEvent::bQuerySrc(TString& strToFill) const
{
    // Get the source, if none, then shouldn't happen, but obviously not
    const TString* pstrVal = m_colBlocks.pobjAt(strPath_Source);
    if (!pstrVal)
        return kCIDLib::False;

    if (pstrVal->bStartsWith(strSrc_FldPref))
    {
        strToFill = *pstrVal;
        strToFill.Cut(0, strSrc_FldPref.c4Length());
    }
     else if (pstrVal->bStartsWith(strSrc_DevPref))
    {
        strToFill = *pstrVal;
        strToFill.Cut(0, strSrc_DevPref.c4Length());
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




//  Look up the indicated item and return the value as a boolean. If not
//  convertable to a boolean, an error is thrown.
//
tCIDLib::TBoolean TCQCEvent::bValueAs(const TString& strPath) const
{
    const TString& strVal = strCheckValNode(strPath);

    tCIDLib::TBoolean bRes;
    if (!facCQCKit().bCheckBoolVal(strVal, bRes))
        ThrowCvtErr(strPath, L"boolean");
    return bRes;
}


//
//  Return a boolean that indicates whether the value at the indicated
//  path both exists and equals the passed value. If it doesn't exist or
//  doesn't equal, then the return is False.
//
tCIDLib::TBoolean
TCQCEvent::bValueEquals(const   TString&    strPath
                        , const TString&    strCompVal) const
{
    const TString* pstrVal = m_colBlocks.pobjAt(strPath);
    if (!pstrVal)
        return kCIDLib::False;

    // It exists, return whether it equals the comparison value
    return (*pstrVal == strCompVal);
}


//
//  Just indicates whether the value exists, and optionally returns the
//  value.
//
tCIDLib::TBoolean TCQCEvent::bValueExists(const TString& strPath) const
{
    return m_colBlocks.bNodeExists(strPath);
}

tCIDLib::TBoolean
TCQCEvent::bValueExists(const   TString&    strPath
                        ,       TString&    strValue) const
{
    const TString* pstrVal = m_colBlocks.pobjAt(strPath);
    if (!pstrVal)
        return kCIDLib::False;

    strValue = *pstrVal;
    return kCIDLib::True;
}


//
//  A generic method for building up the standard driver events, which
//  are far and away the most commonly sent events.
//
tCIDLib::TVoid
TCQCEvent::BuildStdDrvEvent(const   tCQCKit::EStdDrvEvs eEvent
                            , const TString&            strMoniker
                            , const TString&            strField
                            , const TString&            strVal1
                            , const TString&            strVal2
                            , const TString&            strVal3
                            , const TString&            strVal4)
{
    // Clear out any current content
    Reset();

    // Get the info for the event to send
    const TDrvEvInfo& deviSend = CQCKit_Event::colDrvEvs[eEvent];

    //
    //  Build up the source path. It's either a device moniker or a field
    //  type source.
    //
    TString strPath(deviSend.m_strSrcType);
    strPath.Append(strMoniker);
    if (deviSend.m_strSrcType == strSrc_FldPref)
    {
        // It's a field type so add the extra field stuff
        strPath.Append(kCIDLib::chPeriod);
        strPath.Append(strField);
    }

    // Now we can make the header
    BuildHeader(deviSend.m_strClass, strPath);

    // Now add our field value block if we have one
    if (!deviSend.m_strInfoBlk.bIsEmpty())
    {
        strPath = L"/";
        AddBlock(strPath, deviSend.m_strInfoBlk);
        AppendLevel(strPath, deviSend.m_strInfoBlk);

        //
        //  And add our values. We have a special case for the field
        //  change where we have to potentially truncate the value
        //  for size reasons and indicate that.
        //
        if (eEvent == tCQCKit::EStdDrvEvs::FldChange)
        {
            const tCIDLib::TCard4 c4Len = strVal1.c4Length();
            if (c4Len > 32)
            {
                AddValue(strPath, deviSend.m_strVal2, L"yes");
                TString strCapped(strVal1);
                strCapped.CapAt(32);
                AddValue(strPath, deviSend.m_strVal1, strCapped);
            }
             else
            {
                AddValue(strPath, deviSend.m_strVal1, strVal1);
            }
        }
         else
        {
            if (!deviSend.m_strVal1.bIsEmpty())
                AddValue(strPath, deviSend.m_strVal1, strVal1);
            if (!deviSend.m_strVal2.bIsEmpty())
                AddValue(strPath, deviSend.m_strVal2, strVal2);
            if (!deviSend.m_strVal3.bIsEmpty())
                AddValue(strPath, deviSend.m_strVal3, strVal3);
            if (!deviSend.m_strVal4.bIsEmpty())
                AddValue(strPath, deviSend.m_strVal4, strVal4);
        }
    }
}


//
//  Look up the indicated item and return the value as a Card4. If not
//  convertable to a boolean, an error is thrown.
//
tCIDLib::TCard4
TCQCEvent::c4ValueAs(const  TString&            strPath
                    , const tCIDLib::ERadices   eRadix) const
{
    const TString& strVal = strCheckValNode(strPath);
    tCIDLib::TBoolean bOk;
    const tCIDLib::TCard4 c4Ret = TRawStr::c4AsBinary(strVal.pszBuffer(), bOk, eRadix);
    if (!bOk)
        ThrowCvtErr(strPath, L"cardinal");
    return c4Ret;
}


//
//  Look up the indicated item and return the value as a Card8. If not
//  convertable to a boolean, an error is thrown.
//
tCIDLib::TCard8
TCQCEvent::c8ValueAs(const  TString&            strPath
                    , const tCIDLib::ERadices   eRadix) const
{
    // Make sure it exists and is a value node
    const TString& strVal = strCheckValNode(strPath);
    tCIDLib::TBoolean bOk;
    const tCIDLib::TCard8 c8Ret = TRawStr::c8AsBinary(strVal.pszBuffer(), bOk, eRadix);
    if (!bOk)
        ThrowCvtErr(strPath, L"cardinal64");
    return c8Ret;
}


//
//  Look up the indicated item and return the value as an int. If not
//  convertable to a boolean, an error is thrown.
//
tCIDLib::TFloat8 TCQCEvent::f8ValueAs(const TString& strPath) const
{
    // Make sure it exists and is a value node
    const TString& strVal = strCheckValNode(strPath);
    tCIDLib::TBoolean bOk;
    const tCIDLib::TFloat8 f8Ret = TRawStr::f8AsBinary(strVal.pszBuffer(), bOk);
    if (!bOk)
        ThrowCvtErr(strPath, L"float");
    return f8Ret;
}


//
//  Look up the indicated item and return the value as a card. If not
//  convertable to a boolean, an error is thrown.
//
tCIDLib::TInt4
TCQCEvent::i4ValueAs(const  TString&            strPath
                    , const tCIDLib::ERadices   eRadix) const
{
    const TString& strVal = strCheckValNode(strPath);
    tCIDLib::TBoolean bOk;
    const tCIDLib::TInt4 i4Ret = TRawStr::i4AsBinary(strVal.pszBuffer(), bOk, eRadix);
    if (!bOk)
        ThrowCvtErr(strPath, L"integer");
    return i4Ret;
}



//
//  Parses our contents out from a text stream that provides the source text
//  of a formatted event object (produced by streaming one of these to a text
//  output stream, or possibly building one in a string and putting a text
//  stream over that.) If it's not valid, we will throw.
//
tCIDLib::TVoid TCQCEvent::ParseFrom(TTextInStream& strmSrc)
{
    // Reset us back to emptpy
    Reset();

    // Reset the line counter for the new parse pass
    m_c4LineNum = 0;

    //
    //  Kick off the recursive parse. We pass the root node of the tree,
    //  which always exists, as the initial parent. If anything fails, we
    //  throw out of here.
    //
    TString     strTmp1;
    TString     strTmp2;
    TString     strStart(L"/");
    ParseBlock(strmSrc, strTmp1, strTmp2, strStart);
}


// Just resets us back to an empty event
tCIDLib::TVoid TCQCEvent::Reset()
{
    // Currently we just need to flush the collection
    m_colBlocks.RemoveAll();
}


// Just a convenience to return the very commonly accessed class value
const TString& TCQCEvent::strClass() const
{
    return strCheckValNode(strPath_Class);
}


// Just a convenience to return the very commonly accessed source value
const TString& TCQCEvent::strSource() const
{
    return strCheckValNode(strPath_Source);
}


//
//  Look up the indicated item and return the raw string value.
//
const TString& TCQCEvent::strValue(const TString& strPath) const
{
    return strCheckValNode(strPath);
}


//
//  Updates the value of the indicated terminal mode
//
tCIDLib::TVoid TCQCEvent::SetValue( const   TString&    strPath
                                    , const TString&    strValue)
{
    try
    {
        m_colBlocks.RefreshTerminal(strPath, strValue);
    }

    catch(TError& errToCatch)
    {
        // If it was a non-terminal, then throw our error, else rethrow
        if (errToCatch.bCheckEvent( facCIDLib().strName()
                                    , kCIDErrs::errcCol_MustBeTerminal))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvTrg_NotValueNode
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
                , strPath
            );
        }

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TCQCEvent: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Formats the information out in the standard format.
//
tCIDLib::TVoid TCQCEvent::FormatTo(TTextOutStream& strmTar) const
{
    // Do the recursive format
    const TBlockNode* const pnodeRoot = m_colBlocks.pnodeRoot();
    FormatBlock(strmTar, *pnodeRoot);
    strmTar.Flush();
}


tCIDLib::TVoid TCQCEvent::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_Event::c2OTWVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // We get a frame marker between it and the tree data, just for safety
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Stream in the tree
    StreamInBasicTree(m_colBlocks, strmToReadFrom);
}


tCIDLib::TVoid TCQCEvent::StreamTo(TBinOutStream& strmToWriteTo)  const
{
    // First we get housekeeping stuff and the unique id hash
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_Event::c2OTWVersion
                    << tCIDLib::EStreamMarkers::Frame;

    // Stream out the tree collection
    StreamOutBasicTree(m_colBlocks, strmToWriteTo);
    strmToWriteTo.Flush();
}



// ---------------------------------------------------------------------------
//  TCQCEvent: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCEvent::AppendLevel(TString& strAddTo, const TString& strToAdd) const
{
    if (strToAdd.c4Length())
    {
        if ((strAddTo.chLast() == kCIDLib::chTreeSepChar)
        &&  (strToAdd.chFirst() == kCIDLib::chTreeSepChar))
        {
            strAddTo.DeleteLast();
        }
         else if ((strAddTo.chLast() != kCIDLib::chTreeSepChar)
              &&  (strToAdd.chFirst() != kCIDLib::chTreeSepChar))
        {
            if (!strAddTo.bIsEmpty())
                strAddTo.Append(kCIDLib::chTreeSepChar);
        }
    }
    strAddTo.Append(strToAdd);
}


//
//  Builds a standard header block. Assumes the caller wants to build a new
//  event so it resets.
//
tCIDLib::TVoid
TCQCEvent::BuildHeader( const   TString&    strClass
                        , const TString&    strSource)
{
    // Clear current contents
    Reset();

    // Start the standard header block
    TString strPath(L"/");
    AddBlock(strPath, strHdr_Std);
    AppendLevel(strPath, strHdr_Std);

    // Add the standard housekeeping type fields
    AddValue(strPath, L"v", CQCKit_Event::pszEvFmtVer);

    // Put in the class/source values
    AddValue(strPath, L"class", strClass);
    AddValue(strPath, L"source", strSource);
}


//
//  Looks for the next non-empty line. Returns false if end of stream. If
//  the passed bEOSOk flag is not set, then we throw if EOS.
//
tCIDLib::TBoolean
TCQCEvent::bGetNextLine(        TTextInStream&      strmSrc
                        ,       TString&            strToFill
                        , const tCIDLib::TBoolean   bEOSOk)
{
    while(!strmSrc.bEndOfStream())
    {
        strmSrc >> strToFill;
        m_c4LineNum++;
        if (!strToFill.bIsEmpty())
            return kCIDLib::True;
    }

    // If EOS is not ok, then throw
    if (!bEOSOk)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvP_UnexpectedEOS
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4LineNum)
        );
    }
    return kCIDLib::False;
}


//
//  Looks for next block or key, or EOS. It will return an enum value that
//  indicates what it found, or throw if EOS and it's not legal here.
//
TCQCEvent::ETokenTypes
TCQCEvent::eFindNext(       TTextInStream&      strmSrc
                    ,       TString&            strName
                    ,       TString&            strValue
                    , const tCIDLib::TBoolean   bEOSOk)
{
    //
    //  If we get a false, then it has to be EOS and EOS was legal, since
    //  it didn't throw, so return EOS.
    //
    if (!bGetNextLine(strmSrc, strName, bEOSOk))
        return ETokenTypes::EOS;

    //
    //  If it has an equal sign in it, then it has to be a key=value line,
    //  so we are done.
    //
    tCIDLib::TCard4 c4Pos;
    if (strName.bFirstOccurrence(kCIDLib::chEquals, c4Pos))
    {
        //
        //  If the equals is at the end, then no value. Else, copy the value
        //  out and cap the name at the equal sign.
        //
        if (c4Pos + 1 == strName.c4Length())
            strValue.Clear();
        else
            strName.CopyOutSubStr(strValue, c4Pos + 1);

        strName.CapAt(c4Pos);
        strName.StripWhitespace();
        return ETokenTypes::Key;
    }

    //
    //  Got to be a block start or end. So strip the whitspace and see if
    //  it's a closing brace. If so, then end of block. Else it's got to
    //  be a start of block, so the  next line should be opening brace.
    //  Here we don't use bEOSOk, since even if it was ok not to find a
    //  block, now we found one so there has to be a brace.
    //
    strName.StripWhitespace();
    if (strName == L"}")
        return ETokenTypes::EOB;

    bGetNextLine(strmSrc, strValue, kCIDLib::False);
    strValue.StripWhitespace();
    if (strValue != L"{")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvP_ExpectedBrace
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4LineNum)
        );
    }
    return ETokenTypes::Block;
}


//
//  A recursive method to handle formatting out all of the body blocks, which
//  can be nested. This is for text, hence 'format'. StreamBlock is for binary
//  streaming.
//
//  Note that the indent janitors below only do anything if indentation isn't
//  suppressed in the passed stream. So the caller controls whether we 'pretty
//  print' or just dump it out raw (for on the wire transmission.)
//
tCIDLib::TVoid TCQCEvent::FormatBlock(          TTextOutStream& strmTar
                                        , const TBlockNode&     nodePar) const
{
    //
    //  Iterate the children of this parent node, If we hit a new scope,
    //  we just recurse.
    //
    const TBasicTreeCol<TString>::TNode* pnodeChild = nodePar.pnodeFirstChild();
    if (pnodeChild)
    {
        while (pnodeChild)
        {
            if (pnodeChild->eType() == tCIDLib::ETreeNodes::NonTerminal)
            {
                const TBlockNode& nodeChild = *static_cast<const TBlockNode*>(pnodeChild);

                // Open a new block, indent, and recurse
                strmTar << kCIDLib::NewLn << nodeChild.strName() << L"\n{";
                {
                    TStreamIndentJan janIndent(&strmTar, 4);
                    FormatBlock(strmTar, nodeChild);
                }
                strmTar << L"\n}";
            }
             else
            {
                // We have to downcast the node to our terminal node type
                const TKeyNode& nodeKey = *static_cast<const TKeyNode*>(pnodeChild);

                strmTar << kCIDLib::NewLn
                        << nodeKey.strName() << L'=' << nodeKey.objData();
            }
            pnodeChild = pnodeChild->pnodeNext();
        }
    }
}


//
//  This is used to lazily fault in some info. The ctors call this if the
//  the static flag indicating info is loaded is false.
//
//  MAKE SURE that these stay in sync with any of our public static
//  strings that provide quick paths to important info in the event
//  tree.
//
tCIDLib::TVoid TCQCEvent::LoadInfo() const
{
    // Lock and double check. If still not set, then load the info
    TBaseLock lockInit;
    if (!CQCKit_Event::atomInfoLoaded)
    {
        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::FldChange].Set
        (
            L"cqsl.fldchange"
            , L"cqsl.field:"
            , L"cqsl.fldval"
            , L"val"
            , L"trunc"
            , kCIDLib::pszEmptyZStr
            , kCIDLib::pszEmptyZStr
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::LoadChange].Set
        (
            L"cqsl.loadchange"
            , L"cqsl.field:"
            , L"cqsl.loadinfo"
            , L"state"
            , L"loadnum"
            , L"name"
            , kCIDLib::pszEmptyZStr
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::LockStatus].Set
        (
            L"cqsl.lockStatus"
            , L"cqsl.dev:"
            , L"cqsl.lockinfo"
            , L"state"
            , L"lockid"
            , L"code"
            , L"type"
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::Motion].Set
        (
            L"cqsl.motion"
            , L"cqsl.field:"
            , L"cqsl.motioninfo"
            , L"type"
            , L"sensornum"
            , L"name"
            , kCIDLib::pszEmptyZStr
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::Presence].Set
        (
            L"cqsl.presence"
            , L"cqsl.dev:"
            , L"cqsl.presenceinfo"
            , L"type"
            , L"idinfo"
            , L"area"
            , kCIDLib::pszEmptyZStr
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::UserAction].Set
        (
            L"cqsl.useract"
            , L"cqsl.dev:"
            , L"cqsl.actinfo"
            , L"evtype"
            , L"evdata"
            , kCIDLib::pszEmptyZStr
            , kCIDLib::pszEmptyZStr
        );

        CQCKit_Event::colDrvEvs[tCQCKit::EStdDrvEvs::ZoneAlarm].Set
        (
            L"cqsl.zonealarm"
            , L"cqsl.field:"
            , L"cqsl.zoneinfo"
            , L"state"
            , L"zonenum"
            , L"name"
            , kCIDLib::pszEmptyZStr
        );

        // And mark it loaded last of all
        CQCKit_Event::atomInfoLoaded.Set();
    }
}



//
//  A recursive method to handle parsing in all of the body blocks.
//
tCIDLib::TVoid
TCQCEvent::ParseBlock(  TTextInStream&  strmSrc
                        , TString&      strTmp1
                        , TString&      strTmp2
                        , TString&      strCur)
{
    //
    //  Remember the length of the path coming in, so that we can keep just
    //  capping it back to there instead of having each level have to have
    //  a local path to build up the new child paths.
    //
    const tCIDLib::TCard4 c4OrgLen = strCur.c4Length();

    // Remember if we are at the root
    const tCIDLib::TBoolean bAtRoot = (strCur == L"/");
    while (kCIDLib::True)
    {
        //
        //  Get the next token. If it comes back EOS, then we hit the end
        //  nad it was a valid place to hit the end, so break out.
        //
        ETokenTypes eType = eFindNext(strmSrc, strTmp1, strTmp2, bAtRoot);
        if (eType == ETokenTypes::EOS)
            break;

        if (eType == ETokenTypes::Block)
        {
            //
            //  Add a new non-terminal node under the parent, with the block
            //  name. We obviously want to do this BEFORE we extend the
            //  path.
            //
            m_colBlocks.pnodeAddNonTerminal(strCur, strTmp1);

            // Now extend the path to the new level and recurse
            AppendLevel(strCur, strTmp1);
            ParseBlock(strmSrc, strTmp1, strTmp2, strCur);

            //
            //  Cap the path back to the original incoming length, for the
            //  next round.
            //
            strCur.CapAt(c4OrgLen);
        }
         else if (eType == ETokenTypes::EOB)
        {
            // We are done with this block
            break;
        }
         else
        {
            // It's just a key/value at this level, so add a terminal node
            m_colBlocks.objAddTerminal(strCur, strTmp1, strTmp2);
        }
    }

}


//
//  Sees if the passed path exists, is a value node, and returns the
//  value node. For the not found and not the right type errors, we
//  intecept them and throw our own error.
//
const TString& TCQCEvent::strCheckValNode(const TString& strPath) const
{
    const TString* pstrVal = 0;
    try
    {
        pstrVal = m_colBlocks.pobjAt(strPath);
    }

    catch(TError& errToCatch)
    {
        // If it was a non-terminal, then throw our error, else rethrow
        if (errToCatch.bCheckEvent( facCIDLib().strName()
                                    , kCIDErrs::errcCol_MustBeTerminal))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvTrg_NotValueNode
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
                , strPath
            );
        }
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // If not found, then throw our not found error
    if (!pstrVal)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvTrg_ValNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strPath
        );
    }
    return *pstrVal;
}


tCIDLib::TVoid
TCQCEvent::ThrowCvtErr( const   TString&            strPath
                        , const tCIDLib::TCh* const pszType) const
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcEvTrg_CantCvtVal
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Format
        , strPath
        , TString(pszType)
    );
}

