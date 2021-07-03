//
// FILE NAME: CQCKit_TrgEvent.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2005
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
//  This file implements the triggered event class.
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
RTTIDecls(TCQCTEvFilter,TCQCStdCmdSrc)
RTTIDecls(TEventRTVs,TCQCCmdRTVSrc)
RTTIDecls(TCQCTrgEvent,TCQCStdCmdSrc)
RTTIDecls(TCQCTrgEventEx,TCQCTrgEvent)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_TrgEvent
    {
        // -----------------------------------------------------------------------
        //  Our triggered event persistent format version
        //
        //  Version 1 -
        //      Initial version (stayed at 1 for 1.7 since we threw away the old
        //      format and manually converted during the upgrade.)
        //
        //  Version 2 -
        //      Added the m_bPaused member to allow for pausing of events.
        //
        //  Version 3 -
        //      Added the m_bLoggable member to allow for selective logging of
        //      events as the are invoked.
        //
        //  Version 4 -
        //      Action command context info was moved into the action events
        //      so we need to update ours after streaming it in because ours is
        //      not the standard context.
        //
        //  Version 5 -
        //      Added the 'serialized' flag.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2     c2TEvFmtVersion = 5;


        // -----------------------------------------------------------------------
        //  Our triggered event filter persistent format version
        //
        //  Version 1 -
        //      Initial version
        //
        //  Version 2 -
        //      We added the ability for the field name or comp value to be
        //      regular expressions, which requires two new boolean flags to
        //      indicate if they are literal values or reg ex'es.
        //
        //  Version 3 -
        //      Some inconsistency crept in wrt to devices being referenced in
        //      the comp value and fields in the field value. We want to use the
        //      first (field) value for either device or field, so we update those
        //      during loading to the new correct format if less than V3.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2     c2TEvFFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  Some strings we look for. Most of them are provided for more general
        //  use by the facility class.
        // -----------------------------------------------------------------------
        const   TString     strSecure(L"secure");
        const   TString     strViolated(L"violated");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCTrgEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TKeyedCQCTrgEvent: Constructors and destructor
// ---------------------------------------------------------------------------
TKeyedCQCTrgEvent::TKeyedCQCTrgEvent() :

    m_c4SerialNum(1)
{
}

TKeyedCQCTrgEvent::TKeyedCQCTrgEvent(const  TString&        strPath
                                    , const TCQCTrgEvent&   csrcBase) :
    TCQCTrgEvent(csrcBase)
    , m_c4SerialNum(1)
    , m_strPath(strPath)
{
}

TKeyedCQCTrgEvent::~TKeyedCQCTrgEvent()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TKeyedCQCTrgEvent::c4SerialNum() const
{
    return m_c4SerialNum;
}


tCIDLib::TCard4 TKeyedCQCTrgEvent::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


const TString& TKeyedCQCTrgEvent::strPath() const
{
    return m_strPath;
}

const TString& TKeyedCQCTrgEvent::strPath(const TString& strToSet)
{
    m_strPath = strToSet;
    return m_strPath;
}


tCIDLib::TVoid TKeyedCQCTrgEvent::SetConfig(const TCQCTrgEvent& csrcBase)
{
    // Just set it on our base class
    TCQCTrgEvent::operator=(csrcBase);
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCTEvFilter
// PREFIX: cevf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Public, static methods
// ---------------------------------------------------------------------------

//
//  This method is a very useful one and avoids a lot of redundant code in
//  some of the dialogs that allow configuration of filters and validation
//  of filters. We tell the caller for the indicated filter type whether
//  the two values are required or not and whethre they can be regular
//  expressions or not.
//
tCIDLib::TVoid
TCQCTEvFilter::QueryFilterTypeInfo( const   tCQCKit::ETEvFilters eEvType
                                    ,       tCIDLib::TBoolean&   bFldUsed
                                    ,       tCIDLib::TBoolean&   bCompUsed
                                    ,       tCIDLib::TBoolean&   bFldReq
                                    ,       tCIDLib::TBoolean&   bCompReq
                                    ,       tCIDLib::TBoolean&   bFldRegEx
                                    ,       tCIDLib::TBoolean&   bCompRegEx)
{
    //
    //  Since there are lots of overlap, we do them in separate groups. First
    //  do the values used stuff.
    //
    bCompUsed = kCIDLib::False;
    bFldUsed = kCIDLib::False;
    switch(eEvType)
    {
        // These use just the comp value
        case tCQCKit::ETEvFilters::IsFromSource :
        case tCQCKit::ETEvFilters::IsLoadChange :
        case tCQCKit::ETEvFilters::IsLockStatus :
        case tCQCKit::ETEvFilters::IsInTimeRange :
        case tCQCKit::ETEvFilters::IsOfClass :
        case tCQCKit::ETEvFilters::IsPresenceEv :
        case tCQCKit::ETEvFilters::IsZoneAlarm :
            bCompUsed = kCIDLib::True;
            break;

        // These use just the field/device
        case tCQCKit::ETEvFilters::IsDevReady :
        case tCQCKit::ETEvFilters::IsFieldChangeFor :
        case tCQCKit::ETEvFilters::IsMotionEvFor :
        case tCQCKit::ETEvFilters::IsMotionEvFrom :
        case tCQCKit::ETEvFilters::IsUserActionFrom :
            bFldUsed = kCIDLib::True;
            break;

        // These use both
        case tCQCKit::ETEvFilters::Arbitrary :
        case tCQCKit::ETEvFilters::FldValEquals :
        case tCQCKit::ETEvFilters::IsLoadChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFrom :
        case tCQCKit::ETEvFilters::IsLockStatusFrom :
        case tCQCKit::ETEvFilters::IsNewFldValFor :
        case tCQCKit::ETEvFilters::IsPresenceEvFrom :
        case tCQCKit::ETEvFilters::IsPresenceEvInArea :
        case tCQCKit::ETEvFilters::IsThisUserAction :
        case tCQCKit::ETEvFilters::IsLockStatusCode :
        case tCQCKit::ETEvFilters::IsUserActionFor :
        case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
        case tCQCKit::ETEvFilters::IsZoneAlarmFor :
            bCompUsed = kCIDLib::True;
            bFldUsed = kCIDLib::True;
            break;

        default :
            // Neither used, keep the defaults
            break;
    };


    //
    //  Indicate which ones cannot be empty values, i.e. we have to have
    //  something or it can't possible be good. In almost all cases,
    //  if it's used, it's required. So set that and only change it for
    //  the exceptions. The reason mostly is because the incoming value
    //  could be an empty value in some cases, so we have to allow an empty
    //  comparison value if that's what they want to match again.
    //
    //  In others, if the value isn't set we treat it as a match.
    //
    bCompReq = bCompUsed;
    bFldReq = bFldUsed;
    if ((eEvType == tCQCKit::ETEvFilters::Arbitrary)
    ||  (eEvType == tCQCKit::ETEvFilters::FldValEquals)
    ||  (eEvType == tCQCKit::ETEvFilters::IsLoadChange)
    ||  (eEvType == tCQCKit::ETEvFilters::IsLoadChangeFor)
    ||  (eEvType == tCQCKit::ETEvFilters::IsLoadChangeFrom)
    ||  (eEvType == tCQCKit::ETEvFilters::IsLockStatus)
    ||  (eEvType == tCQCKit::ETEvFilters::IsLockStatusFrom)
    ||  (eEvType == tCQCKit::ETEvFilters::IsNewFldValFor)
    ||  (eEvType == tCQCKit::ETEvFilters::IsPresenceEv)
    ||  (eEvType == tCQCKit::ETEvFilters::IsPresenceEvFrom)
    ||  (eEvType == tCQCKit::ETEvFilters::IsZoneAlarm)
    ||  (eEvType == tCQCKit::ETEvFilters::IsZoneAlarmFrom)
    ||  (eEvType == tCQCKit::ETEvFilters::IsZoneAlarmFor))
    {
        bCompReq = kCIDLib::False;
    }

    // And do the 'can be regex' values
    bCompRegEx = kCIDLib::False;
    bFldRegEx = kCIDLib::False;
    switch(eEvType)
    {
        // These allow it on the comp value
        case tCQCKit::ETEvFilters::Arbitrary :
        case tCQCKit::ETEvFilters::FldValEquals :
        case tCQCKit::ETEvFilters::IsFromSource :
        case tCQCKit::ETEvFilters::IsLockStatus :
        case tCQCKit::ETEvFilters::IsOfClass :
        case tCQCKit::ETEvFilters::IsPresenceEv :
        case tCQCKit::ETEvFilters::IsZoneAlarm :
            bCompRegEx = kCIDLib::True;
            break;

        // These allow it on the device/field
        case tCQCKit::ETEvFilters::IsFieldChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFrom :
        case tCQCKit::ETEvFilters::IsMotionEvFor :
        case tCQCKit::ETEvFilters::IsMotionEvFrom :
        case tCQCKit::ETEvFilters::IsUserActionFrom :
            bFldRegEx = kCIDLib::True;
            break;

        // These allow it on both
        case tCQCKit::ETEvFilters::IsNewFldValFor :
        case tCQCKit::ETEvFilters::IsLockStatusFrom :
        case tCQCKit::ETEvFilters::IsLockStatusCode :
        case tCQCKit::ETEvFilters::IsThisUserAction :
        case tCQCKit::ETEvFilters::IsPresenceEvFrom :
        case tCQCKit::ETEvFilters::IsPresenceEvInArea :
        case tCQCKit::ETEvFilters::IsUserActionFor :
        case tCQCKit::ETEvFilters::IsZoneAlarmFor :
        case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
            bCompRegEx = kCIDLib::True;
            bFldRegEx = kCIDLib::True;
            break;

        default :
            // Neither, so keep the deafults
            break;
    };

    // Sanity check that we don't turn on regex for something not used
    if (bCompRegEx && !bCompUsed)
    {
        CIDAssert2(L"Conflicting used/regex for comp value")
    }

    if (bFldRegEx && !bFldUsed)
    {
        CIDAssert2(L"Conflicting used/regex for field value");
    }
}


//
//  This one is a helper for the filter editing dialog, to let it display
//  more targeted prompt information when the user selects a particualr type
//  of filter. The comp prefix we mostly don't set, which will let the caller
//  leave it at the default value in the dialog definition.
//
tCIDLib::TVoid TCQCTEvFilter::
QueryFilterValPrefs(const   tCQCKit::ETEvFilters        eEvType
                    ,       TString&                    strFldPref
                    ,       TCQCTEvFilter::EFldTypes&   eFldType
                    ,       TString&                    strCompPref)
{
    switch(eEvType)
    {
        // These want devices
        case tCQCKit::ETEvFilters::IsDevReady :
        case tCQCKit::ETEvFilters::IsLoadChangeFrom :
        case tCQCKit::ETEvFilters::IsLockStatusFrom :
        case tCQCKit::ETEvFilters::IsMotionEvFrom :
        case tCQCKit::ETEvFilters::IsPresenceEvFrom :
        case tCQCKit::ETEvFilters::IsPresenceEvInArea :
        case tCQCKit::ETEvFilters::IsUserActionFor :
        case tCQCKit::ETEvFilters::IsUserActionFrom :
        case tCQCKit::ETEvFilters::IsLockStatusCode :
        case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
            strFldPref = facCQCKit().strMsg(kKitMsgs::midEvF_DevPref);
            eFldType = EFldTypes::Device;
            break;

        // These want fields
        case tCQCKit::ETEvFilters::FldValEquals :
        case tCQCKit::ETEvFilters::IsFieldChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFor :
        case tCQCKit::ETEvFilters::IsMotionEvFor :
        case tCQCKit::ETEvFilters::IsNewFldValFor :
        case tCQCKit::ETEvFilters::IsZoneAlarmFor :
            strFldPref = facCQCKit().strMsg(kKitMsgs::midEvF_FldPref);
            eFldType = EFldTypes::Field;
            break;

        // Special case ones
        case tCQCKit::ETEvFilters::Arbitrary :
            strFldPref = facCQCKit().strMsg(kKitMsgs::midEvF_PathPref);
            eFldType = EFldTypes::Alt;
            break;

        case tCQCKit::ETEvFilters::IsThisUserAction :
            strFldPref = facCQCKit().strMsg(kKitMsgs::midEvF_TypePref);
            eFldType = EFldTypes::Alt;
            break;

        default :
            // If not handled explicitly, then not used
            strFldPref = facCQCKit().strMsg(kKitMsgs::midEvF_NAPref);
            eFldType = EFldTypes::Unused;
            break;
    };


    switch(eEvType)
    {
        case tCQCKit::ETEvFilters::IsFromSource :
            strCompPref = L"Source:";
            break;

        case tCQCKit::ETEvFilters::IsInTimeRange :
            strCompPref = L"Range:";
            break;

        case tCQCKit::ETEvFilters::IsOfClass :
            strCompPref = L"Class:";
            break;

        case tCQCKit::ETEvFilters::IsLockStatus :
        case tCQCKit::ETEvFilters::IsLockStatusFrom :
            strCompPref = L"Lock Name: ";
            break;

        case tCQCKit::ETEvFilters::IsLockStatusCode :
            strCompPref = L"Code: ";
            break;

        case tCQCKit::ETEvFilters::IsPresenceEv :
        case tCQCKit::ETEvFilters::IsPresenceEvFrom :
        case tCQCKit::ETEvFilters::IsUserActionFor :
            strCompPref = L"Type:";
            break;

        case tCQCKit::ETEvFilters::IsPresenceEvInArea :
            strCompPref = L"Area:";
            break;

        case tCQCKit::ETEvFilters::IsThisUserAction :
            strCompPref = facCQCKit().strMsg(kKitMsgs::midEvF_DataPref);
            break;

        case tCQCKit::ETEvFilters::IsLoadChange :
        case tCQCKit::ETEvFilters::IsLoadChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFrom :
        case tCQCKit::ETEvFilters::IsZoneAlarm :
        case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
        case tCQCKit::ETEvFilters::IsZoneAlarmFor :
            strCompPref = L"State:";
            break;

        // These don't use the comp value, set to unused
        case tCQCKit::ETEvFilters::IsDevReady :
        case tCQCKit::ETEvFilters::IsFieldChangeFor :
        case tCQCKit::ETEvFilters::IsMotionEvFor :
        case tCQCKit::ETEvFilters::IsMotionEvFrom :
        case tCQCKit::ETEvFilters::IsUserActionFrom :
        case tCQCKit::ETEvFilters::IsZoneSecured :
        case tCQCKit::ETEvFilters::IsZoneViolated :
            strCompPref = facCQCKit().strMsg(kKitMsgs::midEvF_NAPref);
            break;

        default :
            strCompPref = facCQCKit().strMsg(kKitMsgs::midEvF_CompPref);
            break;
    };
}


// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTEvFilter::TCQCTEvFilter() :

    m_bCompValRegEx(kCIDLib::False)
    , m_bFldRegEx(kCIDLib::False)
    , m_bNegate(kCIDLib::False)
    , m_eType(tCQCKit::ETEvFilters::Unused)
    , m_pregxComp(0)
    , m_pregxFld(0)
{
}

TCQCTEvFilter::TCQCTEvFilter(const TCQCTEvFilter& cevfSrc) :

    m_bCompValRegEx(cevfSrc.m_bCompValRegEx)
    , m_bFldRegEx(cevfSrc.m_bFldRegEx)
    , m_bNegate(cevfSrc.m_bNegate)
    , m_eType(cevfSrc.m_eType)
    , m_pregxComp(0)
    , m_pregxFld(0)
    , m_strCompVal(cevfSrc.m_strCompVal)
    , m_strEvFld(cevfSrc.m_strEvFld)
{
    // Do any common setup
    DoSetup();
}

TCQCTEvFilter::~TCQCTEvFilter()
{
    // Clean up any reg expressions that got created
    delete m_pregxComp;
    m_pregxComp = 0;

    delete m_pregxFld;
    m_pregxFld = 0;
}


// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Public operators
// ---------------------------------------------------------------------------
TCQCTEvFilter& TCQCTEvFilter::operator=(const TCQCTEvFilter& cevfSrc)
{
    if (this != &cevfSrc)
    {
        m_bCompValRegEx = cevfSrc.m_bCompValRegEx;
        m_bFldRegEx     = cevfSrc.m_bFldRegEx;
        m_bNegate       = cevfSrc.m_bNegate;
        m_eType         = cevfSrc.m_eType;
        m_strCompVal    = cevfSrc.m_strCompVal;
        m_strEvFld      = cevfSrc.m_strEvFld;

        // Do any common setup
        DoSetup();
    }
    return *this;
}

tCIDLib::TBoolean TCQCTEvFilter::operator==(const TCQCTEvFilter& cevfSrc) const
{
    if (&cevfSrc != this)
    {
        //
        //  We don't have to compare the regular expression objects since they are
        //  just set via the comp value members (if the regex flags are set.) So we
        //  just have to compare the reg ex flags and the comp values.
        //
        if ((m_eType != cevfSrc.m_eType)
        ||  (m_bCompValRegEx != cevfSrc.m_bCompValRegEx)
        ||  (m_bFldRegEx != cevfSrc.m_bFldRegEx)
        ||  (m_bNegate != cevfSrc.m_bNegate)
        ||  (m_strCompVal != cevfSrc.m_strCompVal)
        ||  (m_strEvFld != cevfSrc.m_strEvFld))
        {
            return kCIDLib::False;
        }
    }

    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCTEvFilter::operator!=(const TCQCTEvFilter& cevfSrc) const
{
    return !operator==(cevfSrc);
}



// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Public non-virtual methods
// ---------------------------------------------------------------------------

//
//  Checks if the passed info would be valid if set. Regular expressions are
//  not allowed in some cases.
//
tCIDLib::TBoolean
TCQCTEvFilter::bCheckValidRegX( const   tCQCKit::ETEvFilters    eType
                                , const tCIDLib::TBoolean       bCompValRegEx
                                , const tCIDLib::TBoolean       bFldRegEx) const
{
    // Query the info for this filter
    tCIDLib::TBoolean   bFldUsed;
    tCIDLib::TBoolean   bCompUsed;
    tCIDLib::TBoolean   bFldReq;
    tCIDLib::TBoolean   bCompReq;
    tCIDLib::TBoolean   bFldRegExOK;
    tCIDLib::TBoolean   bCompRegExOK;

    QueryFilterTypeInfo
    (
        eType
        , bFldUsed
        , bCompUsed
        , bFldReq
        , bCompReq
        , bFldRegExOK
        , bCompRegExOK
    );

    // If the passed flags conflict with the allowed flags, we failed
    if (bFldRegEx && !bFldRegExOK)
        return kCIDLib::False;

    if (bCompValRegEx && !bCompRegExOK)
        return kCIDLib::False;
    return kCIDLib::True;
}


// Return the comp value reg ex/literal value flag
tCIDLib::TBoolean TCQCTEvFilter::bCompValRegEx() const
{
    return m_bCompValRegEx;
}


// Tests the passed event against our filter criteria
tCIDLib::TBoolean
TCQCTEvFilter::bEvaluate(const  TCQCEvent&              evToTest
                        , const tCIDLib::TBoolean       bIsNight
                        , const tCIDLib::TEncodedTime   enctNow
                        , const tCIDLib::TCard4         c4Hour
                        , const tCIDLib::TCard4         c4Minute) const
{
    // According to the event type, evaluate it
    tCIDLib::TBoolean bRet = kCIDLib::False;

    //
    //  For efficiency pull out the source (without the prefix). We use
    //  that all over below.
    //
    evToTest.bQuerySrc(m_strEvSrc);

    switch(m_eType)
    {
        case tCQCKit::ETEvFilters::Unused :
            // A no op. Shouldn't see it, but just return false
            break;

        case tCQCKit::ETEvFilters::Arbitrary :
            // If our comp value is empty, then we will accept anything
            bRet = bDoComp(kCIDLib::True, evToTest.strValue(m_strEvFld), kCIDLib::True);
            break;

        case tCQCKit::ETEvFilters::IsDevReady :
        {
            // Do quick test mode here
            tCQCKit::TCQCSrvProxy orbcAdmin
            (
                facCQCKit().orbcCQCSrvAdminProxy(m_strEvFld, 0, kCIDLib::True)
            );
            tCIDLib::TCard4         c4ThreadId;
            tCQCKit::EDrvStates     eState;
            tCQCKit::EVerboseLvls   eVerbose;
            orbcAdmin->QueryDriverState(m_strEvFld, eState, eVerbose, c4ThreadId);
            bRet = (eState == tCQCKit::EDrvStates::Connected);
            break;
        }

        case tCQCKit::ETEvFilters::FldValEquals :
        {
            // Break the src value into moniker/field names
            TString strMon;
            TString strFld;
            if (!facCQCKit().bParseFldName(m_strEvFld, strMon, strFld))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFld_BadFldNameFmt
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , m_strEvFld
                );
            }

            // Get a proxy for the CQCServer that owns this moniker, in quick mode
            tCQCKit::TCQCSrvProxy orbcSrv
            (
                facCQCKit().orbcCQCSrvAdminProxy(strMon, 0, kCIDLib::True)
            );

            // Try to get the value for this field
            tCIDLib::TCard4 c4SerNum = 0;
            TString strVal;
            tCQCKit::EFldTypes eType;
            orbcSrv->bReadFieldByName(c4SerNum, strMon, strFld, strVal, eType);

            bRet = bDoComp(kCIDLib::True, strVal);
            break;
        }

        case tCQCKit::ETEvFilters::IsFieldChange :
        {
            // Just check if it's a field change
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange);
            break;
        }

        case tCQCKit::ETEvFilters::IsFieldChangeFor :
        {
            //
            //  See if it is a field change first. If so, then see if the
            //  source field matches our field value.
            //
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange);
            if (bRet)
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);
            break;
        }

        case tCQCKit::ETEvFilters::IsFromSource :
            // In this case, we want the whole source string with prefix
            bRet = bDoComp(kCIDLib::True, evToTest.strSource());
            break;

        case tCQCKit::ETEvFilters::IsInTimeRange :
        {
            //
            //  The comp value is in the form "start,end" where each are hours
            //  in 24 hour format.
            //
            TStringTokenizer stokVals(&m_strCompVal, L", ");
            TString strStart;
            TString strEnd;
            bRet = stokVals.bGetNextToken(strStart) && stokVals.bGetNextToken(strEnd);
            if (bRet)
            {
                tCIDLib::TCard4 c4End;
                tCIDLib::TCard4 c4Start;

                c4Start = TRawStr::c4AsBinary(strStart.pszBuffer(), bRet, tCIDLib::ERadices::Dec);
                if (bRet)
                    c4End = TRawStr::c4AsBinary(strEnd.pszBuffer(), bRet, tCIDLib::ERadices::Dec);

                if (bRet)
                {
                    if (c4Start <= c4End)
                        bRet = ((c4Hour >= c4Start) && (c4Hour < c4End));
                    else
                        bRet= ((c4Hour >= c4Start) || (c4Hour < c4End));
                }
            }
            break;
        }

        case tCQCKit::ETEvFilters::IsLoadChange :
        case tCQCKit::ETEvFilters::IsLoadChangeFor :
        case tCQCKit::ETEvFilters::IsLoadChangeFrom :
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::LoadChange);
            if (bRet)
            {
                if (m_eType == tCQCKit::ETEvFilters::IsLoadChangeFrom)
                    bRet = bDoMonikerComp(kCIDLib::False, m_strEvSrc);
                else if (m_eType == tCQCKit::ETEvFilters::IsLoadChangeFor)
                    bRet = bDoComp(kCIDLib::False, m_strEvSrc);
            }

            // If that worked test the lock id against the comp value
            if (bRet)
            {
                // If the comp value is empty, we'll match anything
                bRet = bDoComp
                (
                    kCIDLib::True
                    , evToTest.strValue(TCQCEvent::strPath_LoadState)
                    , kCIDLib::True
                );
            }
            break;

        case tCQCKit::ETEvFilters::IsLockStatus :
        case tCQCKit::ETEvFilters::IsLockStatusFrom :
            // Check if it's a lock status event
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::LockStatus);

            // If that worked, and this is the from version, check the source device
            if (bRet && (m_eType == tCQCKit::ETEvFilters::IsLockStatusFrom))
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);

            // If that worked test the lock id against the comp value
            if (bRet)
            {
                // If the comp value is empty, we'll match anything
                bRet = bDoComp
                (
                    kCIDLib::True
                    , evToTest.strValue(TCQCEvent::strPath_LockId)
                    , kCIDLib::True
                );
            }
            break;

        case tCQCKit::ETEvFilters::IsLockStatusCode :
            // See if it's a lock status first
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::LockStatus);

            // If so, check its from the indicated source
            if (bRet)
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);

            // If that worked, see if any provided comp value matches
            if (bRet)
            {
                // Can't be empty, and has to match the indicated lock code
                bRet = bDoComp
                (
                    kCIDLib::True, evToTest.strValue(TCQCEvent::strPath_LockCode)
                );
            }
            break;

        case tCQCKit::ETEvFilters::IsLoadChangeOn :
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , facCQCKit().strBoolOffOn(kCIDLib::True)
            );
            break;

        case tCQCKit::ETEvFilters::IsLoadChangeOff :
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , facCQCKit().strBoolOffOn(kCIDLib::False)
            );
            break;


        case tCQCKit::ETEvFilters::IsMotionEv :
        case tCQCKit::ETEvFilters::IsMotionEvFor :
        case tCQCKit::ETEvFilters::IsMotionEvFrom :
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::Motion);
            if (bRet)
            {
                if (m_eType == tCQCKit::ETEvFilters::IsMotionEvFrom)
                    bRet = bDoMonikerComp(kCIDLib::False, m_strEvSrc);
                else if (m_eType == tCQCKit::ETEvFilters::IsMotionEvFor)
                    bRet = bDoComp(kCIDLib::False, m_strEvSrc);
            }
            break;

        case tCQCKit::ETEvFilters::IsMotionStartEv :
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::Motion
                , facCQCKit().strBoolStartEnd(kCIDLib::True)
            );
            break;

        case tCQCKit::ETEvFilters::IsMotionEndEv :
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::Motion
                , facCQCKit().strBoolStartEnd(kCIDLib::False)
            );
            break;

        case tCQCKit::ETEvFilters::IsNewFldValFor :
        {
            // See if it's a field change
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange);

            if (bRet)
            {
                // If so, check the field
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);

                if (bRet)
                {
                    // That matched so check the value
                    bRet = bDoComp
                    (
                        kCIDLib::True, evToTest.strValue(TCQCEvent::strPath_FldValue)
                    );
                }
            }
            break;
        }

        case tCQCKit::ETEvFilters::IsNight :
            bRet = bIsNight;
            break;

        case tCQCKit::ETEvFilters::IsOfClass :
            bRet = bDoComp(kCIDLib::True, evToTest.strClass());
            break;

        case tCQCKit::ETEvFilters::IsPresenceEv :
        case tCQCKit::ETEvFilters::IsPresenceEvFrom :
            // Check if it's a presence event
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::Presence);

            // If that worked, and this is the from version, check the source device
            if (bRet && (m_eType == tCQCKit::ETEvFilters::IsPresenceEvFrom))
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);

            // If that worked test the presence type against the comp value
            if (bRet)
            {
                // If the comp avlue is empty, we'll match anything
                bRet = bDoComp
                (
                    kCIDLib::True
                    , evToTest.strValue(TCQCEvent::strPath_PresType)
                    , kCIDLib::True
                );
            }
            break;

        case tCQCKit::ETEvFilters::IsPresenceEvInArea :
            // Check if it's a presence event
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::Presence);

            // If that worked, check the source device
            if (bRet)
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);

            // If that worked test the area against the comp value
            if (bRet)
            {
                // It can't be empty, it has to match the area
                bRet = bDoComp
                (
                    kCIDLib::True, evToTest.strValue(TCQCEvent::strPath_PresArea)
                );
            }
            break;

        case tCQCKit::ETEvFilters::IsThisUserAction :
            //
            //  See if it's a user action. If so, then see if the action
            //  type and data match our criteria.
            //
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);
            if (bRet)
            {
                bRet = bDoComp
                (
                    kCIDLib::False, evToTest.strValue(TCQCEvent::strPath_ActType)
                );

                if (bRet)
                {
                    bRet = bDoComp
                    (
                        kCIDLib::True, evToTest.strValue(TCQCEvent::strPath_ActData)
                    );
                }
            }
            break;

        case tCQCKit::ETEvFilters::IsUserAction :
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);
            break;

        case tCQCKit::ETEvFilters::IsUserActionFrom :
        {
            //
            //  See if it's a user action. If so, then see if the src device
            //  matches our filter.
            //
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);
            if (bRet)
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);
            break;
        }

        case tCQCKit::ETEvFilters::IsUserActionFor :
            //
            //  See if it's a user action. If so, then see if the source
            //  device and action type match our criteria.
            //
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);
            if (bRet)
            {
                bRet = bDoComp(kCIDLib::False, m_strEvSrc);
                if (bRet)
                {
                    bRet = bDoComp
                    (
                        kCIDLib::True, evToTest.strValue(TCQCEvent::strPath_ActType)
                    );
                }
            }
            break;

        case tCQCKit::ETEvFilters::IsWeekDay :
        {
            // If it's not saturday or sunday, set the result true
            TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
            tCIDLib::EWeekDays eWDay = tmNow.eDayOfWeek();
            bRet = (eWDay != tCIDLib::EWeekDays::Saturday)
                   && (eWDay != tCIDLib::EWeekDays::Sunday);
            break;
        }

        case tCQCKit::ETEvFilters::IsZoneAlarm :
        case tCQCKit::ETEvFilters::IsZoneAlarmFor :
        case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
            bRet = evToTest.bIsDrvEv(tCQCKit::EStdDrvEvs::ZoneAlarm);
            if (bRet)
            {
                if (m_eType == tCQCKit::ETEvFilters::IsZoneAlarmFrom)
                    bRet = bDoMonikerComp(kCIDLib::False, m_strEvSrc);
                else if (m_eType == tCQCKit::ETEvFilters::IsZoneAlarmFor)
                    bRet = bDoComp(kCIDLib::False, m_strEvSrc);

                //
                //  If that worked, tests the comp value. It's option and
                //  can indicate violated, notready, etc...
                //
                if (bRet)
                {
                    bRet = bDoComp
                    (
                        kCIDLib::True
                        , evToTest.strValue(TCQCEvent::strPath_ZoneStatus)
                        , kCIDLib::True
                    );
                }
            }
            break;

        case tCQCKit::ETEvFilters::IsZoneViolated :
            // Check to see if it's violated
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::ZoneAlarm, CQCKit_TrgEvent::strViolated
            );
            break;

        case tCQCKit::ETEvFilters::IsZoneSecured :
            // Check to see if it's secure
            bRet = evToTest.bIsThisDrvEv
            (
                tCQCKit::EStdDrvEvs::ZoneAlarm, CQCKit_TrgEvent::strSecure
            );
            break;

        default :
            // <TBD> throw here if in debug mode
            #if CID_DEBUG_ON
            #endif
            break;
    };

    // If negated, then flip it
    if (m_bNegate)
        bRet = !bRet;

    return bRet;
}


// Return the field/device reg ex/literal value flag
tCIDLib::TBoolean TCQCTEvFilter::bFldRegEx() const
{
    return m_bFldRegEx;
}


// Get the negation flag
tCIDLib::TBoolean TCQCTEvFilter::bNegate() const
{
    return m_bNegate;
}


// Let them see the type, to know if it's enabled or not
tCQCKit::ETEvFilters TCQCTEvFilter::eType() const
{
    return m_eType;
}


// Get the two string values
const TString& TCQCTEvFilter::strCompVal() const
{
    return m_strCompVal;
}

const TString& TCQCTEvFilter::strEvFld() const
{
    return m_strEvFld;
}


//
//  Do the standard sort of search and replce on our field and comparison
//  values.
//
tCIDLib::TVoid
TCQCTEvFilter::SearchNReplace(  const   TString&            strFind
                                , const TString&            strRepStr
                                , const tCIDLib::TBoolean   bRegEx
                                , const tCIDLib::TBoolean   bFullMatch
                                ,       TRegEx&             regxFind)
{
    facCQCKit().bDoSearchNReplace
    (
        strFind, strRepStr, m_strCompVal, bRegEx, bFullMatch, regxFind
    );

    facCQCKit().bDoSearchNReplace
    (
        strFind, strRepStr, m_strEvFld, bRegEx, bFullMatch, regxFind
    );
}


// Reset this guy back to unused
tCIDLib::TVoid TCQCTEvFilter::Reset()
{
    m_bCompValRegEx = kCIDLib::False;
    m_bFldRegEx = kCIDLib::False;
    m_bNegate   = kCIDLib::False;
    m_eType     = tCQCKit::ETEvFilters::Unused;
    m_strCompVal.Clear();
    m_strEvFld.Clear();

    // Do any common init
    DoSetup();
}


// Set up this filter, which can enable or disable it according to eType
tCIDLib::TVoid
TCQCTEvFilter::Set( const   tCIDLib::TBoolean       bNegate
                    , const tCQCKit::ETEvFilters    eType
                    , const TString&                strEvFld
                    , const TString&                strCompVal
                    , const tCIDLib::TBoolean       bCompValRegEx
                    , const tCIDLib::TBoolean       bFldRegEx)
{
    // Query the info for this filter
    tCIDLib::TBoolean   bFldUsed;
    tCIDLib::TBoolean   bCompUsed;
    tCIDLib::TBoolean   bFldReq;
    tCIDLib::TBoolean   bCompReq;
    tCIDLib::TBoolean   bFldRegExOK;
    tCIDLib::TBoolean   bCompRegExOK;

    QueryFilterTypeInfo
    (
        eType
        , bFldUsed
        , bCompUsed
        , bFldReq
        , bCompReq
        , bFldRegExOK
        , bCompRegExOK
    );

    // Make sure the settings passed are valid
    if ((bFldRegEx && !bFldRegExOK)
    ||  (bCompValRegEx && !bCompRegExOK)
    ||  (strEvFld.bIsEmpty() && bFldReq)
    ||  (!strEvFld.bIsEmpty() && !bFldUsed)
    ||  (strCompVal.bIsEmpty() && bCompReq)
    ||  (!strCompVal.bIsEmpty() && !bCompUsed))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvF_BadFilterValues
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    m_bCompValRegEx = bCompValRegEx;
    m_bFldRegEx = bFldRegEx;
    m_bNegate = bNegate;
    m_eType = eType;
    m_strCompVal = strCompVal;
    m_strEvFld = strEvFld;

    // Do any common setup
    DoSetup();
}


tCIDLib::TVoid TCQCTEvFilter::ToXML(TTextOutStream& strmTarget) const
{
    // Format out our stuff
    strmTarget  << L"\n<EvFilt Neg='" << (m_bNegate ? L"Yes" : L"No")
                << L" Type='" << m_eType << L"' Comp='";
    facCIDXML().EscapeFor(m_strCompVal, strmTarget, tCIDXML::EEscTypes::Attribute, TString::strEmpty());
    strmTarget  << L"' EvFld='" << m_strEvFld << L"'/>";
}


// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCTEvFilter::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the version to make sure we understand it
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_TrgEvent::c2TEvFFmtVersion))
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

    // Read in our stuff
    strmToReadFrom  >> m_bNegate
                    >> m_eType
                    >> m_strCompVal
                    >> m_strEvFld;

    // If V2 or beyond, read in the reg ex flags, else default them
    if (c2FmtVersion >= 2)
    {
        strmToReadFrom  >> m_bCompValRegEx
                        >> m_bFldRegEx;
    }
     else
    {
        m_bCompValRegEx = kCIDLib::False;
        m_bFldRegEx = kCIDLib::False;
    }

    //
    //  If less than V3, we adjust a couple of them to enforce some new
    //  consistency rules.
    //
    if (c2FmtVersion < 3)
    {
        switch(m_eType)
        {
            case tCQCKit::ETEvFilters::IsUserActionFor :
            case tCQCKit::ETEvFilters::IsLoadChangeFrom :
            case tCQCKit::ETEvFilters::IsMotionEvFrom :
            case tCQCKit::ETEvFilters::IsZoneAlarmFrom :
                // Move the comp field value to the event field value
                m_strEvFld = m_strCompVal;
                m_strCompVal.Clear();
                break;

            default :
                // Nothing to do
                break;
        };

        //
        //  We have two special cases of user action for and from, which we
        //  we switch around.
        //
        if (m_eType == tCQCKit::ETEvFilters::IsUserActionFor)
            m_eType = tCQCKit::ETEvFilters::IsUserActionFrom;
        else if (m_eType == tCQCKit::ETEvFilters::IsUserActionFrom)
            m_eType = tCQCKit::ETEvFilters::IsUserActionFor;
    }

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Do any common setup
    DoSetup();
}


tCIDLib::TVoid TCQCTEvFilter::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_TrgEvent::c2TEvFFmtVersion
                    << m_bNegate
                    << m_eType
                    << m_strCompVal
                    << m_strEvFld
                    << m_bCompValRegEx
                    << m_bFldRegEx
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCTEvFilter: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Since values can be literal values or regular expressions, this helper
//  let's the filter code above avoid doing a lot of redundant checking.
//
//  If the empty match flag is set, then if our filter value is empty,
//  we will consider it a match no matter what the value.
//
tCIDLib::TBoolean
TCQCTEvFilter::bDoComp( const   tCIDLib::TBoolean   bCompVal
                        , const TString&            strTestVal
                        , const tCIDLib::TBoolean   bEmptyMatch) const
{
    // Do the appropriate type of comparison
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (bCompVal)
    {
        if (bEmptyMatch && m_strCompVal.bIsEmpty())
            return kCIDLib::True;

        if (m_bCompValRegEx)
            bRet = m_pregxComp->bFullyMatches(strTestVal, kCIDLib::True);
        else
            bRet = m_strCompVal.eCompare(strTestVal) == tCIDLib::ESortComps::Equal;
    }
     else
    {
        if (bEmptyMatch && m_strEvFld.bIsEmpty())
            return kCIDLib::True;

        if (m_bFldRegEx)
            bRet = m_pregxFld->bFullyMatches(strTestVal, kCIDLib::True);
        else
            bRet = m_strEvFld.eCompare(strTestVal) == tCIDLib::ESortComps::Equal;
    }
    return bRet;
}


//
//  Handle the special case where they want to check for whether it comes
//  from a specific device moniker, but what we have is a full field
//  name as the actual event source.
//
tCIDLib::TBoolean
TCQCTEvFilter::bDoMonikerComp(  const   tCIDLib::TBoolean   bCompVal
                                , const TString&            strTestVal) const
{
    m_strTmp = strTestVal;
    m_strTmp.bCapAtChar(L'.');

    // Do the appropriate type of comparison
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (bCompVal)
    {
        if (m_bCompValRegEx)
            bRet = m_pregxComp->bFullyMatches(m_strTmp, kCIDLib::True);
        else
            bRet = m_strCompVal.eCompare(m_strTmp) == tCIDLib::ESortComps::Equal;
    }
     else
    {
        if (m_bFldRegEx)
            bRet = m_pregxFld->bFullyMatches(m_strTmp, kCIDLib::True);
        else
            bRet = m_strEvFld.eCompare(m_strTmp) == tCIDLib::ESortComps::Equal;
    }
    return bRet;
}


//
//  Handles any common setup after changes are made to our configuration.
//  Note that, if the value is empty, we will force the reg ex flag off,
//  since empty regular expressions aren't valid and there's no difference
//  anyway, so it's cheaper to do the literal compare.
//
tCIDLib::TVoid TCQCTEvFilter::DoSetup()
{
    // Delete any existing reg ex'es
    delete m_pregxFld;
    m_pregxFld = 0;
    delete m_pregxComp;
    m_pregxComp = 0;

    // If we need to create a new regular expressions, do so
    if (m_bCompValRegEx)
    {
        if (m_strCompVal.bIsEmpty())
            m_bCompValRegEx = kCIDLib::False;
        else
            m_pregxComp = new TRegEx(m_strCompVal);
    }

    if (m_bFldRegEx)
    {
        if (m_strEvFld.bIsEmpty())
            m_bFldRegEx = kCIDLib::False;
        else
            m_pregxFld = new TRegEx(m_strEvFld);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TEventRTVs
//  PREFIX: crtv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TEventRTVs::TEventRTVs(const TCQCUserCtx& cuctxToUse) :

    TCQCCmdRTVSrc(cuctxToUse)
{
}

TEventRTVs::TEventRTVs( const   TCQCUserCtx&        cuctxToUse
                        , const tCQCKit::TCQCEvPtr& cptrData) :

    TCQCCmdRTVSrc(cuctxToUse)
    , m_cptrData(cptrData)
{
}

TEventRTVs::~TEventRTVs()
{
}


// ---------------------------------------------------------------------------
//  TEventRTVs: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEventRTVs::bRTValue( const   TString&    strId
                                        ,       TString&    strToFill) const
{
    // Give our parent a whack at it first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    //
    //  Not one of his, so check for ours. All our start with our event
    //  RTV prefix. If we haven't had an event set, then always report
    //  that we failed.
    //
    const TCQCEvent* pevData = m_cptrData.pobjData();
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (strId.bStartsWith(kCQCKit::strRTVPref_EvFld) && pevData)
    {
        //
        //  Break out the path part and see if it's one of the RTVs we
        //  actually support. If so, then look up the associated event
        //  value.
        //
        if (strId == kCQCKit::strRTVId_TEvClass)
        {
            bRet = pevData->bValueExists(TCQCEvent::strPath_Class, strToFill);
        }
         else if (strId == kCQCKit::strRTVId_TEvSource)
        {
            bRet = pevData->bValueExists(TCQCEvent::strPath_Source, strToFill);
        }
         else if (strId == kCQCKit::strRTVId_TEvNewFldVal)
        {
            bRet = pevData->bValueExists(TCQCEvent::strPath_FldValue, strToFill);
        }
         else if ((strId == kCQCKit::strRTVId_TEvSrcMoniker)
              ||  (strId == kCQCKit::strRTVId_TEvSrcField))
        {
            //
            //  These will only have a value if the event is a type of event
            //  that involves a driver or field source.
            //
            bRet = pevData->bQuerySrc(m_strTmp1);
            if (bRet)
            {
                const tCIDLib::TBoolean bHaveField = m_strTmp1.bSplit
                (
                    m_strTmp2, kCIDLib::chPeriod, kCIDLib::False
                );

                if (strId == kCQCKit::strRTVId_TEvSrcMoniker)
                {
                    // We'll always have at least a moniker at this point
                    strToFill = m_strTmp1;
                }
                 else
                {
                    // If we have a field, give it back, else we failed
                    if (bHaveField)
                    {
                        strToFill = m_strTmp1;
                        strToFill.Append(kCIDLib::chPeriod);
                        strToFill.Append(m_strTmp2);
                    }
                     else
                    {
                        bRet = kCIDLib::False;
                    }
                }
            }
        }
    }
    return bRet;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCTrgEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCTrgEvent: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTrgEvent::TCQCTrgEvent() :

    m_bLoggable(kCIDLib::False)
    , m_bPaused(kCIDLib::False)
    , m_bSerialized(kCIDLib::False)
    , m_colFilters(kCQCKit::c4MaxTEvFilters)
    , m_eContext(tCQCKit::EActCmdCtx::Standard)
    , m_eLogOp(tCQCKit::ETEvFiltLOps::AND)
{
    // We have a common set up method to do the work
    CommInit();
}

TCQCTrgEvent::TCQCTrgEvent(const tCQCKit::EActCmdCtx eContext) :

    m_bLoggable(kCIDLib::False)
    , m_bPaused(kCIDLib::False)
    , m_bSerialized(kCIDLib::False)
    , m_colFilters(kCQCKit::c4MaxTEvFilters)
    , m_eContext(eContext)
    , m_eLogOp(tCQCKit::ETEvFiltLOps::AND)
{
    // We have a common set up method to do the work
    CommInit();
}

TCQCTrgEvent::TCQCTrgEvent( const   TString&            strTitle
                            , const tCQCKit::EActCmdCtx eContext) :

    TCQCStdCmdSrc(strTitle)
    , m_bLoggable(kCIDLib::False)
    , m_bPaused(kCIDLib::False)
    , m_bSerialized(kCIDLib::False)
    , m_colFilters(kCQCKit::c4MaxTEvFilters)
    , m_eContext(eContext)
    , m_eLogOp(tCQCKit::ETEvFiltLOps::AND)
{
    // We have a common set up method to do the work
    CommInit();
}

TCQCTrgEvent::~TCQCTrgEvent()
{
}


// ---------------------------------------------------------------------------
//  TCQCTrgEvent: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCTrgEvent::operator==(const TCQCTrgEvent& csrcSrc) const
{
    if (&csrcSrc != this)
    {
        if (!TParent::operator==(csrcSrc))
            return kCIDLib::False;

        if ((m_bLoggable    != csrcSrc.m_bLoggable)
        ||  (m_bPaused      != csrcSrc.m_bPaused)
        ||  (m_bSerialized  != csrcSrc.m_bSerialized)
        ||  (m_colFilters   != csrcSrc.m_colFilters)
        ||  (m_eContext     != csrcSrc.m_eContext)
        ||  (m_eLogOp       != csrcSrc.m_eLogOp)
        ||  (m_strId        != csrcSrc.m_strId))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCTrgEvent::operator!=(const TCQCTrgEvent& csrcSrc) const
{
    return !operator==(csrcSrc);
}


// ---------------------------------------------------------------------------
//  TCQCTrgEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Evaluate the passed event against all our configured filters
tCIDLib::TBoolean
TCQCTrgEvent::bEvaluate(const   TCQCEvent&              cevToTest
                        , const tCIDLib::TBoolean       bIsNight
                        , const tCIDLib::TEncodedTime   enctNow
                        , const tCIDLib::TCard4         c4Hour
                        , const tCIDLib::TCard4         c4Minute) const
{
    // If paused, just say don't trigger us
    if (m_bPaused)
        return kCIDLib::False;

    // Loop through the triggers and for each one used, evaluate it
    tCIDLib::TCard4   c4TrueCnt = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
    {
        const TCQCTEvFilter& cevfCur = m_colFilters[c4Index];
        if (cevfCur.eType() == tCQCKit::ETEvFilters::Unused)
            continue;

        if (cevfCur.bEvaluate(cevToTest, bIsNight, enctNow, c4Hour, c4Minute))
        {
            switch(m_eLogOp)
            {
                case tCQCKit::ETEvFiltLOps::AND :
                    // No op, we are still good to go
                    break;

                case tCQCKit::ETEvFiltLOps::OR :
                    // We are done, one passed
                    return kCIDLib::True;

                case tCQCKit::ETEvFiltLOps::XOR :
                    // If not the first true, then we failed
                    if (c4TrueCnt)
                        return kCIDLib::False;
                    break;
            }
            c4TrueCnt++;
        }
         else
        {
            switch(m_eLogOp)
            {
                case tCQCKit::ETEvFiltLOps::AND :
                    // We failed since they all have to work
                    return kCIDLib::False;

                case tCQCKit::ETEvFiltLOps::OR :
                case tCQCKit::ETEvFiltLOps::XOR :
                    // A no-op in these cases
                    break;
            }
        }
    }

    //
    //  If we get here and it's an OR, then we failed because we would have
    //  returned true already if any of them passed.
    //
    if (m_eLogOp == tCQCKit::ETEvFiltLOps::OR)
        return kCIDLib::False;

    //
    //  If XOR, the tree count has to equal 1. We would have returned false above if
    //  more than one was true. But we may get here with none true, so check for a
    //  zero true count.
    //
    if (m_eLogOp == tCQCKit::ETEvFiltLOps::XOR)
    {
        if (!c4TrueCnt)
            return kCIDLib::False;
    }

    // If it was AND, then we would have returned false if any failed.
    return kCIDLib::True;
}


// Get or set the loggable status
tCIDLib::TBoolean TCQCTrgEvent::bLoggable() const
{
    return m_bLoggable;
}

tCIDLib::TBoolean TCQCTrgEvent::bLoggable(const tCIDLib::TBoolean bToSet)
{
    m_bLoggable = bToSet;
    return m_bLoggable;
}


// Get or set the paused status
tCIDLib::TBoolean TCQCTrgEvent::bPaused() const
{
    return m_bPaused;
}

tCIDLib::TBoolean TCQCTrgEvent::bPaused(const tCIDLib::TBoolean bToSet)
{
    m_bPaused = bToSet;
    return m_bPaused;
}


// Get or set the serialized status
tCIDLib::TBoolean TCQCTrgEvent::bSerialized() const
{
    return m_bSerialized;
}

tCIDLib::TBoolean TCQCTrgEvent::bSerialized(const tCIDLib::TBoolean bToSet)
{
    m_bSerialized = bToSet;
    return m_bSerialized;
}


// Provide access to our filter list
const TCQCTrgEvent::TFilterList& TCQCTrgEvent::colFilters() const
{
    return m_colFilters;
}

TCQCTrgEvent::TFilterList& TCQCTrgEvent::colFilters()
{
    return m_colFilters;
}


// Get or set the filter logical op we use to combine the filter results
tCQCKit::ETEvFiltLOps TCQCTrgEvent::eFilterLogOp() const
{
    return m_eLogOp;
}

tCQCKit::ETEvFiltLOps
TCQCTrgEvent::eFilterLogOp(const tCQCKit::ETEvFiltLOps eLogOp)
{
    m_eLogOp  = eLogOp;
    return m_eLogOp;
}


//
//  Though we derive from the command source, and the search and replace
//  method is virtual, there's no way for the caller to tell us whether
//  we should search filters and the commands, or vice versa. So we provide
//  this separate non-virtula method for filter search and replace. The
//  client code must call one or the other or both.
//
tCIDLib::TVoid
TCQCTrgEvent::FilterSearchNReplace( const   TString&            strFind
                                    , const TString&            strReplaceWith
                                    , const tCIDLib::TBoolean   bRegEx
                                    , const tCIDLib::TBoolean   bFullMatch
                                    ,       TRegEx&             regxFind)
{
    // Loop through the filters and call their search and replace method
    const tCIDLib::TCard4 c4Count = m_colFilters.tElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colFilters[c4Index].SearchNReplace
        (
            strFind, strReplaceWith, bRegEx, bFullMatch, regxFind
        );
    }
}


// Just needed for the 5.0 upgrade process
const TString& TCQCTrgEvent::strId() const
{
    return m_strId;
}

const TString& TCQCTrgEvent::strId(const TString& strToSet)
{
    m_strId = strToSet;
    return m_strId;
}


// Set the filter at the given index
tCIDLib::TVoid
TCQCTrgEvent::SetAt(const   tCIDLib::TCard4         c4At
                    , const tCIDLib::TBoolean       bNegate
                    , const tCQCKit::ETEvFilters    eType
                    , const TString&                strEvFld
                    , const TString&                strCompVal
                    , const tCIDLib::TBoolean       bCompValRegEx
                    , const tCIDLib::TBoolean       bFldRegEx)
{
    // Just a pass through to set a filter at a given index
    m_colFilters[c4At].Set
    (
        bNegate, eType, strEvFld, strCompVal, bCompValRegEx, bFldRegEx
    );
}


// Set the action context this event will report
tCIDLib::TVoid TCQCTrgEvent::SetContext(const tCQCKit::EActCmdCtx eToSet)
{
    m_eContext = eToSet;

    // Update our event (registered with the base class) with this context info
    SetEventContext(kCQCKit::strEvId_OnTrigger, m_eContext);
}


// ---------------------------------------------------------------------------
//  TCQCTrgEvent: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCTrgEvent::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the version to make sure we understand it
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_TrgEvent::c2TEvFmtVersion))
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

    // Let our parent class stream in
    TParent::StreamFrom(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom  >> m_strId
                    >> m_eLogOp
                    >> m_colFilters;

    // If < V2, then default the paused flag, else read it in
    if (c2FmtVersion < 2)
        m_bPaused = kCIDLib::False;
    else
        strmToReadFrom >> m_bPaused;

    // If < V3, then default the loggable flag, else read it in
    if (c2FmtVersion < 3)
        m_bLoggable = kCIDLib::False;
    else
        strmToReadFrom >> m_bLoggable;

    // If pre-V4, then initialize the old event info to include the context info
    if (c2FmtVersion < 4)
        SetEventContext(kCQCKit::strEvId_OnTrigger, m_eContext);

    // If pre-V5, default the serialized flag, else read it in
    if (c2FmtVersion < 5)
        m_bSerialized = kCIDLib::False;
    else
        strmToReadFrom >> m_bSerialized;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCTrgEvent::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_TrgEvent::c2TEvFmtVersion;

    // Let our parent stream his stuff
    TParent::StreamTo(strmToWriteTo);

    // And stream our stuff
    strmToWriteTo   << m_strId
                    << m_eLogOp
                    << m_colFilters

                    // V2 stuff
                    << m_bPaused

                    // V3 stuff
                    << m_bLoggable

                    // V5 stuff
                    << m_bSerialized

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCTrgEvent: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCTrgEvent::CommInit()
{
    // Add our one supported command event
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrigger)
        , kCQCKit::strEvId_OnTrigger
        , m_eContext
    );

    // Load up the runtime values we support at this level
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midTEvRTV_Class)
        , kCQCKit::strRTVId_TEvClass
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midTEvRTV_NewFldVal)
        , kCQCKit::strRTVId_TEvNewFldVal
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midTEvRTV_Source)
        , kCQCKit::strRTVId_TEvSource
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midTEvRTV_SrcField)
        , kCQCKit::strRTVId_TEvSrcField
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midTEvRTV_SrcMoniker)
        , kCQCKit::strRTVId_TEvSrcMoniker
        , tCQCKit::ECmdPTypes::Text
    );
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCTrgEventEx
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTrgEventEx: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTrgEventEx::TCQCTrgEventEx( const   TCQCTrgEvent&       cevParent
                                , const tCQCKit::TCQCEvPtr& cptrTrigger) :
    TCQCTrgEvent(cevParent)
    , m_cptrTrigger(cptrTrigger)
{
}

TCQCTrgEventEx::TCQCTrgEventEx( const   TString&            strTitle
                                , const tCQCKit::EActCmdCtx eContext) :

    TCQCTrgEvent(strTitle, eContext)
{
}

TCQCTrgEventEx::~TCQCTrgEventEx()
{
}


// ---------------------------------------------------------------------------
//  TCQCTrgEventEx: Public, inherited method
// ---------------------------------------------------------------------------
TCQCCmdRTVSrc* TCQCTrgEventEx::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    return new TEventRTVs(cuctxToUse, m_cptrTrigger);
}

