//
// FILE NAME: CQCKit_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2000
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
//  This is the implementation file for the facility class.
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
RTTIDecls(TFacCQCKit,TFacility)


// -----------------------------------------------------------------------
//   CLASS: TAppItem
//  PREFIX: ai
// -----------------------------------------------------------------------
class TAppItem
{
    public :
        // ---------------------------------------------------------------
        //  Constructors and Destructor
        // ---------------------------------------------------------------
        TAppItem() :

            m_eShow(tCIDLib::EExtProcShows::Normal)
        {
        }

        TAppItem(const  TString&                strApp
                , const TString&                strInitialPath
                , const TString&                strParms
                , const tCIDLib::EExtProcShows  eShow) :

            m_eShow(eShow)
            , m_strApp(strApp)
            , m_strInitialPath(strInitialPath)
        {
            TExternalProcess::c4BreakOutParms(strParms, m_colParms);
        }

        TAppItem(const TAppItem&) = delete;
        TAppItem(TAppItem&&) = delete;

        ~TAppItem()
        {
        }


        // ---------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------
        TAppItem& operator=(const TAppItem&) = delete;
        TAppItem& operator=(TAppItem&&) = delete;


        // ---------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------
        tCIDLib::TBoolean bActivate()
        {
            return TProcess::bActivateProcess(m_extpThis);
        }

        tCIDLib::TBoolean bIsRunning() const
        {
            return m_extpThis.bIsRunning();
        }

        tCIDLib::TBoolean bIsThisApp(const TString& strToCheck) const
        {
            return m_strApp.bCompareI(strToCheck);
        }

        tCIDLib::TBoolean bStart()
        {
            // If running, then do nothing, and say we didn't start it
            if (m_extpThis.bIsRunning())
                return kCIDLib::False;

            m_extpThis.Start
            (
                m_strApp
                , m_strInitialPath
                , m_colParms
                , tCIDLib::EExtProcFlags::Detached
                , m_eShow
            );

            // Return true to indicate that we started it
            return kCIDLib::True;
        }

        tCIDLib::TBoolean bTryAttachTo(const TString& strInfo)
        {
            if (m_extpThis.bTryAttachTo(strInfo))
            {
                m_extpThis.QueryFullPath(m_strApp);
                return kCIDLib::True;
            }
            return kCIDLib::False;
        }

        tCIDLib::EExitCodes
        eWaitForDeath(const tCIDLib::TCard4 c4MilliSecs) const
        {
            return m_extpThis.eWaitForDeath(c4MilliSecs);
        }

        tCIDLib::TVoid FormatAttachData(TString& strToFill)
        {
            m_extpThis.FormatAttachData(strToFill);
        }

        const TString& strApp() const
        {
            return m_strApp;
        }

        tCIDLib::TVoid Update(  const   TString&                strInitialPath
                                , const TString&                strParms
                                , const tCIDLib::EExtProcShows  eShow)
        {
            m_eShow = eShow;
            m_strInitialPath = strInitialPath;
            m_colParms.RemoveAll();
            TExternalProcess::c4BreakOutParms(strParms, m_colParms);
        }


    private :
        // ---------------------------------------------------------------
        //  Private data members
        //
        //  m_colParms
        //      We break out the individual parameters from the provided
        //      string of parameters.
        //
        //  m_eShow
        //      The show flags.
        //
        //  m_extpThis
        //      This is the external process object that we use to keep
        //      track of our process instance.
        //
        //  m_strApp
        //      The application. It should be fully qualified, since there
        //      is no guarantee that the program will be run from any
        //      particular directory.
        //
        //  m_strInitialPath
        //      The initial path to start the app in.
        // ---------------------------------------------------------------
        TVector<TString>        m_colParms;
        tCIDLib::EExtProcShows  m_eShow;
        TExternalProcess        m_extpThis;
        TString                 m_strApp;
        TString                 m_strInitialPath;
};


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_ThisFacility
    {
        // -----------------------------------------------------------------------
        //  The number of old trigger keys we'll keep track of in order to reject dups. And
        //  the number of triggers in our incoming queue before we start dropping them
        //  because they are coming too fast.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4EvInQSize(1024);


        // -----------------------------------------------------------------------
        //  The max number of outgoing triggers we can hold before we have to start dropping
        //  them. If we have more than this that we cannot send, then something is sending
        //  them way, way too fast.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4EvOutQSize(512);


        // -----------------------------------------------------------------------
        //  If client side lat/long info hasn't been set, these default values will
        //  still be there.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TFloat8  f8LocNotSet(-1000.0);


        // -----------------------------------------------------------------------
        //  The list of executed applications that are currently active
        // -----------------------------------------------------------------------
        using TAppList = TRefVector<TAppItem>;
        TAppList    colAppList(tCIDLib::EAdoptOpts::Adopt);
    }
}


// ---------------------------------------------------------------------------
//  Local helper methods
// ---------------------------------------------------------------------------

//
//  Does a lookup in our local list of attached apps to see if the passed
//  path is in tha tlist.
//
static TAppItem* paiFindApp(const TString& strPathToFind)
{
    const tCIDLib::TCard4 c4Count = CQCKit_ThisFacility::colAppList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TAppItem* paiCur = CQCKit_ThisFacility::colAppList[c4Index];
        if (paiCur->bIsThisApp(strPathToFind))
            return paiCur;
    }
    return nullptr;
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCKit
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCKit: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCKit::TFacCQCKit() :

    TFacility
    (
        L"CQCKit"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bDevMode(kCIDLib::False)
    , m_bSecureMasterWS(kCIDLib::False)
    , m_bSysEdit(kCIDLib::False)
    , m_c4EvSeq(1)
    , m_colEvList(CQCKit_ThisFacility::c4EvInQSize, TMD5KeyOps())
    , m_colEvLIFO()
    , m_colEvSQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_eEvProcType(tCQCKit::EEvProcTypes::None)
    , m_f8ClientLat(CQCKit_ThisFacility::f8LocNotSet)
    , m_f8ClientLong(CQCKit_ThisFacility::f8LocNotSet)
    , m_ippnEvents(kCQCKit::ippnEventDefPort)
    , m_ippnMasterWS(0)
    , m_mhashEvId(TUniqueId::mhashMakeId())
    , m_mtxSync()
    , m_strBoolEnd(L"end")
    , m_strBoolEnter(L"enter")
    , m_strBoolExit(L"exit")
    , m_strBoolFalse(L"False")
    , m_strBoolLocked(L"locked")
    , m_strBoolNo(L"No")
    , m_strBoolOff(L"off")
    , m_strBoolOn(L"on")
    , m_strBoolStart(L"start")
    , m_strBoolTrue(L"True")
    , m_strBoolUnlocked(L"unlocked")
    , m_strBoolYes(L"Yes")
    , m_strGenDriverLibName(L"CQCGenDrvS")
    , m_strMacroRootClassScope(L"/User")
    , m_strProtoDriverLibName(L"GenProtoS")
    , m_thrEvReceive
      (
          L"CQCKitEvReceiverThread"
          , TMemberFunc<TFacCQCKit>(this, &TFacCQCKit::eEvReceiveThread)
      )
    , m_thrEvSend
      (
          L"CQCKitEvSenderThread"
          , TMemberFunc<TFacCQCKit>(this, &TFacCQCKit::eEvSendThread)
      )
{
    // Register some stats cache items
    TStatsCache::RegisterItem
    (
        L"/Stats/EvTrig/OutQDropped"
        , tCIDLib::EStatItemTypes::Counter
        , m_sciOutTrigsDropped
    );

    TStatsCache::RegisterItem
    (
        L"/Stats/EvTrig/Received"
        , tCIDLib::EStatItemTypes::Counter
        , m_sciTrigsReceived
    );
}


// ---------------------------------------------------------------------------
//  TFacCQCKit: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Check the passed value to see if it's a valid boolean value. If not, throw
//  an exception.
//
tCIDLib::TBoolean TFacCQCKit::bCheckBoolVal(const TString& strToTest) const
{
    if (strToTest.bCompareI(kCQCKit::pszFld_True) || (strToTest == L"1"))
        return kCIDLib::True;
    else if (strToTest.bCompareI(kCQCKit::pszFld_False) || (strToTest == L"0"))
        return kCIDLib::False;

    // It ain't good, so rethrow
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcParm_NotValidBool
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Format
    );

    // Make the compiler happy
    return kCIDLib::False;
}


//
//  As above but this time not throwing, but return a status. The boolean value
//  converted (if the return is true) is retured in a parm.
//
tCIDLib::TBoolean
TFacCQCKit::bCheckBoolVal(const TString& strToTest, tCIDLib::TBoolean& bRes) const
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    if (strToTest.bCompareI(kCQCKit::pszFld_True) || (strToTest == L"1"))
        bRes = kCIDLib::True;
    else if (strToTest.bCompareI(kCQCKit::pszFld_False) || (strToTest == L"0"))
        bRes = kCIDLib::False;
    else
        bRet = kCIDLib::False;

    return bRet;
}



//
//  This guy checks a requested form of field access with the actual access
//  of the field. These flags are arranged such that the requestd field has
//  two sets of 2 bits, one set indicates if read or write can be allowed and
//  the other if read or write must be allowed. The actual access has two bits
//  that indicate if read or write are allowed.
//
//  So if the 'can' bits of the reqest OR'ed with the actual access doesn't
//  equal the can bits, then some type of access is available that shouldn't
//  be, so that one fails. If the AND of the 'must' bits with the actual
//  access bits isn't equal to the must bits, then some needed access is not
//  available.
//
tCIDLib::TBoolean
TFacCQCKit::bCheckFldAccess(const   tCQCKit::EReqAccess eRequested
                            , const tCQCKit::EFldAccess eFldAccess)
{
    // Get out the encoded can and must bits
    tCIDLib::TCard4 c4CanBits = tCIDLib::TCard4(eRequested) & 0x3;
    tCIDLib::TCard4 c4MustBits = tCIDLib::TCard4(eRequested);
    c4MustBits >>= 2;

    tCIDLib::TCard4 c4Actual = tCIDLib::TCard4(eFldAccess);

    return ((c4CanBits | c4Actual) == c4CanBits)
           && ((c4MustBits & c4Actual) == c4MustBits);
}


//
//  Check to see if a field exists, in a fault tolerant way. So we treat the driver not
//  existing or not being loaded as just a false return. We also treat a badly formatted
//  field name as just a fals return.
//
tCIDLib::TBoolean
TFacCQCKit::bCheckFldExists(const TString& strMoniker, const TString& strField) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv = orbcCQCSrvAdminProxy
        (
            strMoniker, 0, kCIDLib::True
        );

        tCIDLib::TBoolean bDevExists;
        tCIDLib::TBoolean bFldExists;
        bRet = orbcSrv->bCheckFieldExists(strMoniker, bDevExists, strField, bFldExists);
    }

    catch(...)
    {
        // Just fall through with the default false return
    }
    return bRet;
}

tCIDLib::TBoolean TFacCQCKit::bCheckFldExists(const TString& strField) const
{
    TString strMon;
    TString strFld;

    if (!bParseFldName(strField, strMon, strFld))
        return kCIDLib::False;

    // Call the other version
    return bCheckFldExists(strMon, strFld);
}



//
//  There are various places where fields are filtered so as to only
//  present certain types of fields to the user. One of the most common
//  is filtering for numeric fields (card, float, int) that have range
//  limits with a certain spread or less.
//
//  They provide the ranges as Float8 values, though they actual field
//  type can be int, card, or float. They can also tell us that floats
//  are not supported, since some don't deal with that.
//
//  We give back the actual range of the field, if we accept, also as
//  floats since they can support the values of any of the possible
//  return values.
//
//  Note that, if field is of float type, we ignore the minimum range,
//  since floats are infinitely divisible and therefore the minimum
//  range is meaningless. The maximum range is still important since
//  the caller may only allow a certain breadth of values.
//
tCIDLib::TBoolean
TFacCQCKit::bCheckNumRangeFld(  const   TCQCFldDef&         flddCheck
                                , const tCIDLib::TFloat8    f8MinRange
                                , const tCIDLib::TFloat8    f8MaxRange
                                , const tCIDLib::TBoolean   bAllowFloat
                                ,       tCIDLib::TFloat8&   f8RealMin
                                ,       tCIDLib::TFloat8&   f8RealMax)
{
    // Call the other version
    return bCheckNumRangeFld
    (
        flddCheck.strName()
        , flddCheck.eType()
        , flddCheck.strLimits()
        , f8MinRange
        , f8MaxRange
        , bAllowFloat
        , f8RealMin
        , f8RealMax
    );
}

tCIDLib::TBoolean
TFacCQCKit::bCheckNumRangeFld(  const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const TString&            strLimit
                                , const tCIDLib::TFloat8    f8MinRange
                                , const tCIDLib::TFloat8    f8MaxRange
                                , const tCIDLib::TBoolean   bAllowFloat
                                ,       tCIDLib::TFloat8&   f8RealMin
                                ,       tCIDLib::TFloat8&   f8RealMax)
{
    //
    //  Set invalid returns so that, if they use it even though we say
    //  we reject, it'll fail.
    //
    f8RealMin = -1;
    f8RealMax = -1;

    // Check the type. Its got to be a numeric type
    if ((eType != tCQCKit::EFldTypes::Card)
    &&  (eType != tCQCKit::EFldTypes::Float)
    &&  (eType != tCQCKit::EFldTypes::Int))
    {
        return kCIDLib::False;
    }

    // It's numeric, make sure that float is ok, else reject
    if (!bAllowFloat && (eType == tCQCKit::EFldTypes::Float))
        return kCIDLib::False;

    // Get a limits object out for the new field
    TCQCFldLimit* pfldlFld = TCQCFldLimit::pfldlMakeNew(strLimit, strLimit, eType);
    TJanitor<TCQCFldLimit> janLimit(pfldlFld);

    // Get the full range of travel of the range
    if (eType == tCQCKit::EFldTypes::Card)
    {
        TCQCFldCRangeLimit* pfldlCR = static_cast<TCQCFldCRangeLimit*>(pfldlFld);
        if (!pfldlCR)
            return kCIDLib::False;
        const tCIDLib::TCard4 c4RangeTicks = pfldlCR->c4LimitRange();
        if ((tCIDLib::TFloat8(c4RangeTicks) < f8MinRange)
        ||  (tCIDLib::TFloat8(c4RangeTicks) > f8MaxRange))
        {
            return kCIDLib::False;
        }

        f8RealMin = tCIDLib::TFloat8(pfldlCR->c4Min());
        f8RealMax = tCIDLib::TFloat8(pfldlCR->c4Max());
    }
     else if (eType == tCQCKit::EFldTypes::Float)
    {
        TCQCFldFRangeLimit* pfldlFR = static_cast<TCQCFldFRangeLimit*>(pfldlFld);
        if (!pfldlFR)
            return kCIDLib::False;
        const tCIDLib::TFloat8 f8RangeTicks = pfldlFR->f8LimitRange();

        // We don't bother with checking the minimum range here
        if (f8RangeTicks > f8MaxRange)
            return kCIDLib::False;

        f8RealMin = pfldlFR->f8Min();
        f8RealMax = pfldlFR->f8Max();
    }
     else
    {
        TCQCFldIRangeLimit* pfldlIR = static_cast<TCQCFldIRangeLimit*>(pfldlFld);
        if (!pfldlIR)
            return kCIDLib::False;
        const tCIDLib::TInt4 i4RangeTicks = pfldlIR->i4LimitRange();
        if ((tCIDLib::TFloat8(i4RangeTicks) < f8MinRange)
        ||  (tCIDLib::TFloat8(i4RangeTicks) > f8MaxRange))
        {
            return kCIDLib::False;
        }

        f8RealMin = tCIDLib::TFloat8(pfldlIR->i4Min());
        f8RealMax = tCIDLib::TFloat8(pfldlIR->i4Max());
    }
    return kCIDLib::True;
}


//
//  Check the passed value to see if it's a valid yes/no value. If not, throw
//  an exception.
//
tCIDLib::TBoolean TFacCQCKit::bCheckYesNoVal(const TString& strToTest) const
{
    if (strToTest.bCompareI(L"YES"))
        return kCIDLib::True;
    else if (strToTest.bCompareI(L"NO"))
        return kCIDLib::False;

    // It ain't good, so rethrow
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcParm_NotValidYesNo
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Format
        , strToTest
    );

    // Make the compiler happy
    return kCIDLib::False;
}


// Return the dev mode flag
tCIDLib::TBoolean TFacCQCKit::bDevMode() const
{
    return m_bDevMode;
}


//
//  This method does the standard prep work on a configured command's
//  parameter values prior to processing it. We just replace all of the
//  replacement tokens. The return indicates whether any changes were
//  made or not.
//
tCIDLib::TBoolean
TFacCQCKit::bDoCmdPrep(         TCQCCmdCfg&         ccfgPrep
                        , const TCQCCmdRTVSrc&      crtsVals
                        , const TStdVarsTar* const  pctarGVars
                        , const TStdVarsTar* const  pctarLVars
                        ,       TString&            strPVal)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    const tCIDLib::TCard4 c4PCount = ccfgPrep.c4ParmCnt();
    for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PCount; c4PInd++)
    {
        //
        //  Don't expand expression type parameters. These have to be evaluated
        //  by the handler of the command, which needs to know the data types of
        //  the referenced fields and variables.
        //
        if (ccfgPrep.piAt(c4PInd).m_eType == tCQCKit::ECmdPTypes::Expression)
            continue;

        const tCQCKit::ECmdPrepRes eRes = eStdTokenReplace
        (
            ccfgPrep.piAt(c4PInd).m_strValue
            , &crtsVals
            , pctarGVars
            , pctarLVars
            , strPVal
            , tCQCKit::ETokRepFlags::None
        );

        if (eRes == tCQCKit::ECmdPrepRes::Changed)
        {
            ccfgPrep.SetParmAt(c4PInd, strPVal);
            bRes = kCIDLib::True;
        }
         else if (eRes == tCQCKit::ECmdPrepRes::Failed)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcTokR_TokenExpansion
                , ccfgPrep.piAt(c4PInd).m_strValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strPVal
            );
        }
    }
    return bRes;
}


//
//  Check to see if a login is set in the environment and, if found, tries
//  to do it.
//
tCIDLib::TBoolean
TFacCQCKit::bDoEnvLogin( TCQCSecToken&          sectToFill
                        , TCQCUserAccount&      uaccToFill
                        , tCQCKit::ELoginRes&   eRes)
{
    eRes = tCQCKit::ELoginRes::Ok;

    //
    //  Users can set a username/password in the environment, to avoid having
    //  to logon, if they choose not to use security. So check for them and
    //  try a logon if found.
    //
    TString strName;
    TString strPassword;
    if (!TProcEnvironment::bFind(L"CQC_USERNAME", strName)
    ||  !TProcEnvironment::bFind(L"CQC_PASSWORD", strPassword))
    {
        //
        //  Return false, but with result set to OK, meaning no login, but no
        //  failure either, so the caller knows that tere was no env info set.
        //
        return kCIDLib::False;
    }
    return bDoLogin(strName, strPassword, sectToFill, uaccToFill, eRes);
}


//
//  Given a user name and password, tries to do a login. Returns true if it works
//  else false. If false, the eRes value returns why it failed.
//
tCIDLib::TBoolean
TFacCQCKit::bDoLogin(const  TString&            strUserName
                    , const TString&            strPassword
                    ,       TCQCSecToken&       sectToFill
                    ,       TCQCUserAccount&    uaccToFill
                    ,       tCQCKit::ELoginRes& eRes)
{
    eRes = tCQCKit::ELoginRes::Ok;

    try
    {
        // Get a reference to the security server
        tCQCKit::TSecuritySrvProxy orbcSS = orbcSecuritySrvProxy();

        // And ask it to give us a challenge
        TCQCSecChallenge seccLogon;
        if (orbcSS->bLoginReq(strUserName, seccLogon))
        {
            // Lets do a hash of the user's password
            TMD5Hash mhashPW;
            TMessageDigest5 mdigTmp;
            mdigTmp.StartNew();
            mdigTmp.DigestStr(strPassword);
            mdigTmp.Complete(mhashPW);

            // And use that to validate the challenge
            seccLogon.Validate(mhashPW);

            // And send that back to get a security token, assuming its legal
            if (orbcSS->bGetSToken(seccLogon, sectToFill, uaccToFill, eRes))
                return kCIDLib::True;
        }
         else
        {
            // It has to be an unknown name/password
            eRes = tCQCKit::ELoginRes::BadCred;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        // Fall through
        eRes = tCQCKit::ELoginRes::Exception;
    }

    catch(...)
    {
        // Fall through
        eRes = tCQCKit::ELoginRes::Exception;
    }

    if (bLogWarnings())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_EnvLoginFailed
            , tCIDLib::ESeverities::Warn
            , tCIDLib::EErrClasses::Authority
        );
    }
    return kCIDLib::False;
}

//
//  Return whether a given driver moniker has been configured. If so, we give back
//  the host that it is loaded on. So this is going to the installation server and
//  is available whether the CQCServer host that handles this driver is running and
//  available or not.
//
//  Use bDrvIsLoaded below to check for the actual driver being loaded into a CQCServer
//  somewhere.
//
//  If we can't get to the installation server, we throw.
//
tCIDLib::TBoolean
TFacCQCKit::bDrvIsConfigured(const  TString&        strMoniker
                            ,       TString&        strHost
                            , const TCQCSecToken&   sectUser)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // We need an installation server proxy
        tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();

        // Ask for any name match under the driver scope
        bRet = orbcInst->bDrvCfgDriverPresent(strMoniker, strHost, sectUser);

        //
        //  If we got any, we should have only gotten one. If we got more than one
        //  something is wrong.
        //
        if (!bRet)
            strHost.Clear();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


//
//  Just returns true/false to indicate if a driver is loaded into a CQCServer or
//  not. Use bDrvIsConfigured above to see if it is configured to be loaded.
//
//  We have a second one that will get the host on which it is loaded, which is
//  extra overhead if you don't need that info.
//
tCIDLib::TBoolean TFacCQCKit::bDrvIsLoaded(const TString& strMoniker) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Build up the name server binding path for this driver.
        TString strName(TCQCSrvAdminClientProxy::strDrvScope);
        strName = TCQCSrvAdminClientProxy::strDrvScope;
        strName.Append(kCIDLib::pszTreeSepChar);
        strName.Append(strMoniker);

        // See if it is present in the name server
        bRet = facCIDOrbUC().bNSBindingExists(strName);
    }

    catch(...)
    {
        // Just eat it and fall out with false return
    }
    return bRet;
}


tCIDLib::TBoolean
TFacCQCKit::bDrvIsLoaded(const TString& strMoniker, TString& strHost) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // Build up the name server binding path for this driver.
        TString strBinding(TCQCSrvAdminClientProxy::strDrvScope);
        strBinding.Append(kCIDLib::pszTreeSepChar);
        strBinding.Append(strMoniker);


        // Query the name info. For drivers the host name is stored in the description string
        TString strName;
        orbcNS->QueryNameInfo(strBinding, strName, strHost);

        // It is loaded, so return success
        bRet = kCIDLib::True;
    }

    catch(...)
    {
        // Just eat it and fall out with false return
    }
    return bRet;
}


//
//  Does the standard search and replace operation that is supported by
//  the action editor, interface editor, etc... The return indicates
//  whether or not it modified the target string.
//
tCIDLib::TBoolean
TFacCQCKit::bDoSearchNReplace(  const   TString&            strFind
                                , const TString&            strRepStr
                                ,       TString&            strTarget
                                , const tCIDLib::TBoolean   bRegEx
                                , const tCIDLib::TBoolean   bFullMatch
                                ,       TRegEx&             regxFind)
{
    tCIDLib::TBoolean bChanges = kCIDLib::False;
    if (bFullMatch)
    {
        if (bRegEx)
        {
            // It has to match the full regular expression
            bChanges = regxFind.bFullyMatches(strTarget);
            if (bChanges)
                strTarget = strRepStr;
        }
         else
        {
            // It as to be a full match and replacement
            if (strTarget.bCompareI(strFind))
            {
                bChanges = kCIDLib::True;
                strTarget = strRepStr;
            }
        }
    }
     else
    {
        if (bRegEx)
        {
            // The reg ex engine has a method to do this for us
            bChanges = regxFind.bReplaceAll(strTarget, strRepStr);
        }
         else
        {
            //
            //  We can just use the sub-string replacement method in
            //  the string class for this. Tell it to replace all
            //  instances.
            //
            tCIDLib::TCard4 c4At = 0;
            bChanges = strTarget.bReplaceSubStr
            (
                strFind, strRepStr, c4At, kCIDLib::True, kCIDLib::False
            );
        }
    }
    return bChanges;
}



//
//  This is a generic helper that is used anywhere that we support relative paths
//  in our repositories. All such paths will begin with a slash if fully qualified,
//  else a . or .. if relative.
//
//  We get a base path and a new path reference. We return true or false to indicate
//  if it was expanded (i.e. it was a relative reference) or not (it was fully
//  qualified to no need to do anything (or empty.)
//
//  If relative, we remove the last level of the base path and add the relative
//  reference, then normalize.
//
//  See bMakeRelResPath for the opposite operation.
//
tCIDLib::TBoolean
TFacCQCKit::bExpandResPath( const   TString&    strBasePath
                            , const TString&    strNewPath
                            ,       TString&    strExpPath) const
{
    // If the new one is empty, nothing to do
    if (strNewPath.bIsEmpty())
        return kCIDLib::False;

    // If it's fully qualified, take it as is
    if (strNewPath.chFirst() == kCIDLib::chForwardSlash)
    {
        strExpPath = strNewPath;
        return kCIDLib::False;
    }

    //
    //  To make this a lot easier, we convert to regular path format and use our
    //  path string class to do the work, then copy back out and reconvert.
    //
    TPathStr pathTmp(strBasePath);
    pathTmp.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);
    pathTmp.bRemoveLevel();
    pathTmp.AddLevel(strNewPath);
    pathTmp.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);
    pathTmp.Normalize();

    strExpPath = pathTmp;
    strExpPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);
    return kCIDLib::True;
}



//
//  Finds all of the drivers in the network. One just returns monikers. THe
//  other returns extra info that CQCServer stores with the driver bindings
//  when he registers them.
//
tCIDLib::TBoolean
TFacCQCKit::bFindAllDrivers(TCollection<TString>&   colMonikers
                            , TCollection<TString>& colMakes
                            , TCollection<TString>& colModels
                            , tCQCKit::TDevCatList& fcolCats) const
{
    colMonikers.RemoveAll();
    colMakes.RemoveAll();
    colModels.RemoveAll();
    fcolCats.RemoveAll();

    try
    {
        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // Enumerate the CQCServer driver objects into our collection
        TVector<TNameServerInfo> colNSItems;
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            TStringTokenizer stokParse;

            const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TNameServerInfo& nsiCur = colNSItems[c4Index];

                // Store the moniker, make and model
                colMonikers.objAdd(nsiCur.strNodeName());
                colMakes.objAdd(nsiCur.strExtra1());
                colModels.objAdd(nsiCur.strExtra2());

                //
                //  The device category is in the third extra field. We have to
                //  translate it back to the enum.
                //
                fcolCats.c4AddElement(tCQCKit::eXlatEDevCats(nsiCur.strExtra3()));
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TFacCQCKit::bFindAllDrivers(TCollection<TString>& colMonikers) const
{
    colMonikers.RemoveAll();

    try
    {
        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // Enumerate the CQCServer monikers into our collection
        TVector<TNameServerInfo> colNSItems;
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                colMonikers.objAdd(colNSItems[c4Index].strNodeName());
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Finds all drivers registered for a given host
tCIDLib::TBoolean
TFacCQCKit::bFindAllDrivers(const TString& strOnHost, TCollection<TString>& colMonikers) const
{
    colMonikers.RemoveAll();

    try
    {
        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        TVector<TNameServerInfo> colNSItems;
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TNameServerInfo& nsiCur = colNSItems[c4Index];

                // If the description matches the requested host, then take it
                if (nsiCur.strDescription().bCompareI(strOnHost))
                    colMonikers.objAdd(nsiCur.strNodeName());
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Finds all of the drivers in the network that meet the passed filter
//  criteria, adn return a list of driver configuration objects for each
//  of them.
//
tCIDLib::TBoolean
TFacCQCKit::bFindDrivers(const  TString&                        strMonikerFilter
                        , const TString&                        strMakeFilter
                        , const TString&                        strModelFilter
                        , const tCQCKit::TDevCatSet&            btsDevCats
                        ,       TCollection<TCQCDriverObjCfg>&  colToFill) const
{
    colToFill.RemoveAll();

    try
    {
        // Set up the regular expression filters
        const tCIDLib::TBoolean bMonFilter = !strMonikerFilter.bIsEmpty();
        TRegEx  regxMoniker;
        if (bMonFilter)
            regxMoniker.SetExpression(strMonikerFilter);

        const tCIDLib::TBoolean bMakeFilter = !strMakeFilter.bIsEmpty();
        TRegEx  regxMake;
        if (bMakeFilter)
            regxMake.SetExpression(strMakeFilter);

        const tCIDLib::TBoolean bModelFilter = !strModelFilter.bIsEmpty();
        TRegEx  regxModel;
        if (bModelFilter)
            regxModel.SetExpression(strModelFilter);

        //
        //  See if we have any device category bits. If so, then set the flag.
        //  If have less than the full count required to represent all the
        //  flags, then throw.
        //
        const tCIDLib::TBoolean bHaveDevCats = btsDevCats.c4BitCount() != 0;
        #if CID_DEBUG_ON
        if (bHaveDevCats && (btsDevCats.eBitCount() < tCQCKit::EDevCats::Count))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDbg_BadDevBitCount
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        #endif

        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // We are going to enumerate a scope into a vector of name server info objects
        TVector<TNameServerInfo> colNSItems;

        // Enumerate the CQCServer driver objects into our collection
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            TCQCDriverObjCfg        cqcdcCur;
            TOrbObjId               ooidServer;
            TString                 strBinding;

            TVector<TNameServerInfo>::TCursor cursNSItems(&colNSItems);
            cursNSItems.bReset();
            do
            {
                // Get the current name server info
                const TNameServerInfo& nsviCur = cursNSItems.objRCur();

                //
                //  We can do our filtering purely based on the info from
                //  the name server, so that we only have to query driver
                //  config info if it's a keeper.
                //

                //
                //  If we got any device category filter bits, then see if
                //  this guy's bit is on.
                //
                const tCQCKit::EDevCats eDevCat = tCQCKit::eXlatEDevCats
                (
                    nsviCur.strExtra3()
                );

                if (bHaveDevCats
                &&  (eDevCat != tCQCKit::EDevCats::Count)
                &&  !btsDevCats[eDevCat])
                {
                    continue;
                }

                // And run it through all the reg-ex filters we got
                if (bMonFilter && !regxMoniker.bFullyMatches(nsviCur.strNodeName()))
                    continue;

                if (bMakeFilter && !regxMake.bFullyMatches(nsviCur.strExtra1()))
                    continue;

                if (bModelFilter && !regxModel.bFullyMatches(nsviCur.strExtra2()))
                    continue;

                //
                //  Call the helper that gets us a CQCServer client proxy based on host
                //  name, which is stored in the description for these driver oriented
                //  name server netries.
                //
                TCQCSrvAdminClientProxy* porbcAdmin = porbcCQCSrvAdminProxy
                (
                    nsviCur.strDescription(), 0, kCIDLib::True
                );

                // If we couldn't get it, skip this one
                if (!porbcAdmin)
                    continue;

                // Ask it for driver info on this driver. If it fails, skip it
                TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcAdmin);
                if (!porbcAdmin->bQueryDriverInfo(nsviCur.strNodeName(), cqcdcCur))
                    continue;

                // It passed through all the filters, so take it
                colToFill.objAdd(cqcdcCur);

            }   while (cursNSItems.bNext());
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    // Return indicates if we got any or not
    return !colToFill.bIsEmpty();
}


//
//  Finds all of the drivers in the network that are of the passed device
//  category and return their monikers. This one is much more efficient than
//  the one above, since we can operate completely on the info from the
//  name server and don't have to query driver configuration data.
//
tCIDLib::TBoolean
TFacCQCKit::bFindDrivers(const  TString&                strMonikerFilter
                        , const TString&                strMakeFilter
                        , const TString&                strModelFilter
                        , const tCQCKit::TDevCatSet&    btsDevCats
                        ,       tCIDLib::TStrCollect&   colMonikers
                        ,       tCIDLib::TStrCollect&   colMakes
                        ,       tCIDLib::TStrCollect&   colModels) const
{
    colMonikers.RemoveAll();
    colMakes.RemoveAll();
    colModels.RemoveAll();

    try
    {
        // Set up the regular expression filters
        const tCIDLib::TBoolean bMonFilter = !strMonikerFilter.bIsEmpty();
        TRegEx  regxMoniker;
        if (bMonFilter)
            regxMoniker.SetExpression(strMonikerFilter);

        const tCIDLib::TBoolean bMakeFilter = !strMakeFilter.bIsEmpty();
        TRegEx  regxMake;
        if (bMakeFilter)
            regxMake.SetExpression(strMakeFilter);

        const tCIDLib::TBoolean bModelFilter = !strModelFilter.bIsEmpty();
        TRegEx  regxModel;
        if (bModelFilter)
            regxModel.SetExpression(strModelFilter);

        //
        //  See if we have any device category bits. If so, then set the flag.
        //  If have less than the full count required to represent all the
        //  flags, then throw.
        //
        const tCIDLib::TBoolean bHaveDevCats = btsDevCats.c4BitCount() != 0;
        #if CID_DEBUG_ON
        if (bHaveDevCats && (btsDevCats.eBitCount() < tCQCKit::EDevCats::Count))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDbg_BadDevBitCount
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        #endif

        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // We are going to enumerate a scope into a vector of name server info objects
        TVector<TNameServerInfo> colNSItems;

        // Enumerate the CQCServer driver objects into our collection
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            TCQCDriverObjCfg        cqcdcCur;
            TOrbObjId               ooidServer;
            TString                 strBinding;

            TVector<TNameServerInfo>::TCursor cursNSItems(&colNSItems);
            cursNSItems.bReset();
            do
            {
                // Get the current name server info
                const TNameServerInfo& nsviCur = cursNSItems.objRCur();

                //
                //  If we got any device category filter bits, then see if
                //  this guy's bit is on.
                //
                const tCQCKit::EDevCats eDevCat = tCQCKit::eXlatEDevCats
                (
                    nsviCur.strExtra3()
                );

                if (bHaveDevCats
                &&  (eDevCat != tCQCKit::EDevCats::Count)
                &&  !btsDevCats[eDevCat])
                {
                    continue;
                }

                // And run it through all the reg-ex filters we got
                if (bMonFilter && !regxMoniker.bFullyMatches(nsviCur.strNodeName()))
                    continue;

                if (bMakeFilter && !regxMake.bFullyMatches(nsviCur.strExtra1()))
                    continue;

                if (bModelFilter && !regxModel.bFullyMatches(nsviCur.strExtra2()))
                    continue;

                // It passed through all the filters, so take it
                colMonikers.objAdd(nsviCur.strNodeName());
                colMakes.objAdd(nsviCur.strExtra1());
                colModels.objAdd(nsviCur.strExtra2());

            }   while (cursNSItems.bNext());
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    // Return indicates if we got any or not
    return !colMonikers.bIsEmpty();
}


//
//  And an even more stripped down vesrion where the caller really only
//  wants the list of monikers.
//
tCIDLib::TBoolean
TFacCQCKit::bFindDrivers(const  TString&                strMonikerFilter
                        , const TString&                strMakeFilter
                        , const TString&                strModelFilter
                        , const tCQCKit::TDevCatSet&    btsDevCats
                        ,       tCIDLib::TStrCollect&   colMonikers) const
{
    colMonikers.RemoveAll();

    try
    {
        // Set up the regular expression filters
        const tCIDLib::TBoolean bMonFilter = !strMonikerFilter.bIsEmpty();
        TRegEx  regxMoniker;
        if (bMonFilter)
            regxMoniker.SetExpression(strMonikerFilter);

        const tCIDLib::TBoolean bMakeFilter = !strMakeFilter.bIsEmpty();
        TRegEx  regxMake;
        if (bMakeFilter)
            regxMake.SetExpression(strMakeFilter);

        const tCIDLib::TBoolean bModelFilter = !strModelFilter.bIsEmpty();
        TRegEx  regxModel;
        if (bModelFilter)
            regxModel.SetExpression(strModelFilter);

        //
        //  See if we have any device category bits. If so, then set the flag.
        //  If have less than the full count required to represent all the
        //  flags, then throw.
        //
        const tCIDLib::TBoolean bHaveDevCats = btsDevCats.c4BitCount() != 0;
        #if CID_DEBUG_ON
        if (bHaveDevCats && (btsDevCats.eBitCount() < tCQCKit::EDevCats::Count))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDbg_BadDevBitCount
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        #endif

        // Get a name server client proxy so we can get the moniker list
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // We are going to enumerate a scope into a vector of name server info objects
        TVector<TNameServerInfo> colNSItems;

        // Enumerate the CQCServer driver objects into our collection
        if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                    , colNSItems
                                    , kCIDLib::False))
        {
            TCQCDriverObjCfg    cqcdcCur;
            TOrbObjId           ooidServer;
            TString             strBinding;

            TVector<TNameServerInfo>::TCursor cursNSItems(&colNSItems);
            cursNSItems.bReset();
            do
            {
                // Get the current name server info
                const TNameServerInfo& nsviCur = cursNSItems.objRCur();

                //
                //  If we got any device category filter bits, then see if
                //  this guy's bit is on.
                //
                const tCQCKit::EDevCats eDevCat = tCQCKit::eXlatEDevCats
                (
                    nsviCur.strExtra3()
                );

                if (bHaveDevCats
                &&  (eDevCat != tCQCKit::EDevCats::Count)
                &&  !btsDevCats[eDevCat])
                {
                    continue;
                }

                // And run it through all the reg-ex filters we got
                if (bMonFilter && !regxMoniker.bFullyMatches(nsviCur.strNodeName()))
                    continue;

                if (bMakeFilter && !regxMake.bFullyMatches(nsviCur.strExtra1()))
                    continue;

                if (bModelFilter && !regxModel.bFullyMatches(nsviCur.strExtra2()))
                    continue;

                // It passed through all the filters, so take it
                colMonikers.objAdd(nsviCur.strNodeName());

            }   while (cursNSItems.bNext());
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    // Return indicates if we got any or not
    return !colMonikers.bIsEmpty();
}


//
//  A helper based on the bFindDrivers method above, which returns a list of
//  drivers that are media repository type drivers. They can provide a model
//  filter to look for instances of a particular type of repo driver. Pass an
//  empty string for no model filter.
//
tCIDLib::TBoolean
TFacCQCKit::bFindRepoDrivers(       TCollection<TString>&   colToFill
                            , const TString&                strModelFilter)
{
    tCQCKit::TDevCatSet btsCats;
    btsCats.bSetBitState(tCQCKit::EDevCats::MediaRepository, kCIDLib::True);
    TVector<TCQCDriverObjCfg> colInfo;
    const tCIDLib::TBoolean bGotDrvs = bFindDrivers
    (
        TString::strEmpty(), TString::strEmpty(), strModelFilter, btsCats, colInfo
    );

    colToFill.RemoveAll();
    if (bGotDrvs)
    {
        const tCIDLib::TCard4 c4Count = colInfo.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colToFill.objAdd(colInfo[c4Index].strMoniker());
    }
    return bGotDrvs;
}


//
//  This will find the web server on the master server, and return the information
//  needed to create a URL to invoke things against that server, mostly help which is
//  served up by the MS' web server.
//
tCIDLib::TBoolean
TFacCQCKit::bFindHelpWS(TString& strHost, tCIDLib::TIPPortNum& ippnWS, tCIDLib::TBoolean& bSecure)
{
    // If we already have this info, then return it
    {
        TLocker lockSync(&m_mtxSync);
        if (m_ippnMasterWS)
        {
            bSecure = m_bSecureMasterWS;
            ippnWS = m_ippnMasterWS;
            strHost = m_strMasterWSHost;
            return kCIDLib::True;
        }
    }

    // Else we need to try to look it up
    try
    {
        //
        //  What we do is ask the name server for any registered web servers. The
        //  master server one will have Master in the Extra1 value, it's port in
        //  Extra3, it's host name in Extra2, and either Secure or Insecure in Extra4.
        //
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        TVector<TNameServerInfo> colList;

        const tCIDLib::TCard4 c4Count = orbcNS->c4EnumObjects
        (
            kCQCKit::pszCQCWebSrvCoreAdmins, colList, kCIDLib::True
        );

        // Go through them and find the marked one
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TNameServerInfo& nsviCur = colList[c4Index];

            if (nsviCur.strExtra1() == kCQCKit::strMSWebSrv)
            {
                // Lock and store the cached info as well
                TLocker lockSync(&m_mtxSync);
                m_ippnMasterWS = nsviCur.strExtra3().c4Val();
                m_strMasterWSHost = nsviCur.strExtra2();
                m_bSecureMasterWS = nsviCur.strExtra4().bCompareI(L"Secure");

                bSecure = m_bSecureMasterWS;
                ippnWS = m_ippnMasterWS;
                strHost = m_strMasterWSHost;
                return kCIDLib::True;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::False;
}



//
//  A helper to get the lat/long info stored in the master server configuration
//  server. We return true if we get the data and can set it on a time object
//  successfully (which means it's reasonable for our local time zone.)
//
tCIDLib::TBoolean
TFacCQCKit::bGetLocationInfo(const  tCIDLib::ECSSides       eSide
                            ,       tCIDLib::TFloat8&       f8Lat
                            ,       tCIDLib::TFloat8&       f8Long
                            ,       const tCIDLib::TBoolean bThrowIfNot
                            , const TCQCSecToken&           sectToUse)
{
    // We need to get an installation server proxy
    tCIDLib::TBoolean bGotIt = kCIDLib::False;
    try
    {
        if (eSide == tCIDLib::ECSSides::Server)
        {
            tCQCKit::TInstSrvProxy orbcInst = orbcInstSrvProxy();
            bGotIt = orbcInst->bQueryLocationInfo(f8Lat, f8Long, sectToUse);
        }
        else
        {
            if ((m_f8ClientLat == CQCKit_ThisFacility::f8LocNotSet)
            ||  (m_f8ClientLong == CQCKit_ThisFacility::f8LocNotSet))
            {
                // They've not been set
                if (bThrowIfNot)
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcCfg_NoLocInfo
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotReady
                        , TString(L"Client")
                    );
                }
            }
            else
            {
                // We have it, so return it
                f8Lat = m_f8ClientLat;
                f8Long = m_f8ClientLong;
                bGotIt = kCIDLib::True;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (bThrowIfNot)
            throw errToCatch;
        LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    if (!bGotIt && bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_NoLocInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , TString((eSide == tCIDLib::ECSSides::Server) ? L"Server" : L"Client")
        );
    }
    return bGotIt;
}


//
//  Checks that the passed text is valid as the base part of a field name, i.e.
//  the part after any V2 prefix and before any suffix. So it has to start with a
//  character or number, and after that have only numbers, characters, underscopes
//  and hyphens.
//
tCIDLib::TBoolean TFacCQCKit::bIsValidBaseFldName(const TString& strToCheck)
{
    const tCIDLib::TCard4 c4Count = strToCheck.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!bIsValidFldNameChar(strToCheck[c4Index], c4Index))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Makes sure that a field name is at least basically syntactically correct.
//  If it contains a # mark, we assume it must be a V2 compatible field and
//  will insure that the part before the # matches a defined device class.
//
//  Note that this guy only deals with the field name part, not the whole
//  moniker.field format.
//
tCIDLib::TBoolean
TFacCQCKit::bIsValidFldName(const   TString&                strToCheck
                            , const tCIDLib::TBoolean       bInternal) const
{
    // Call the other version with a dummy device class and toss it
    tCQCKit::EDevClasses eDevClass;
    return bIsValidFldName(strToCheck, bInternal, eDevClass);
}

tCIDLib::TBoolean
TFacCQCKit::bIsValidFldName(const   TString&                strToCheck
                            , const tCIDLib::TBoolean       bInternal
                            ,       tCQCKit::EDevClasses&   eDevClass) const
{
    // States for our state machine
    enum EStates
    {
        EState_DevClass
        , EState_Sub
        , EState_Suffix
    };

    // Clear outputs before starting, which simplifies below
    eDevClass = tCQCKit::EDevClasses::None;

    // To make things more efficient, do a quick scan for two special cases
    const tCIDLib::TBoolean bDevClass = strToCheck.bContainsChar(kCIDLib::chPoundSign);
    const tCIDLib::TBoolean bSubUnit = strToCheck.bContainsChar(kCIDLib::chTilde);

    //
    //  There cannot be a sub-unit without a device class. Only V2 style fields
    //  can have sub-units.
    //
    if (bSubUnit && !bDevClass)
        return kCIDLib::False;

    const tCIDLib::TCard4 c4Count = strToCheck.c4Length();
    tCIDLib::TCard4 c4Index = 0;
    TString strTmp;

    EStates eState;
    if (bDevClass)
        eState = EState_DevClass;
    else if (bSubUnit)
        eState = EState_Sub;
    else
        eState = EState_Suffix;

    while (c4Index < c4Count)
    {
        const tCIDLib::TCh chCur = strToCheck[c4Index++];

        switch(eState)
        {
            case EState_DevClass :
                //
                // If we hit a #, then we have the device class, else accumulate
                //  as device class name chars.
                //
                if (chCur == kCIDLib::chPoundSign)
                {
                    // Make sure the device class is valid
                    eDevClass = tCQCKit::eAltXlatEDevClasses(strTmp);
                    if (eDevClass == tCQCKit::EDevClasses::Count)
                        return kCIDLib::False;

                    // Next has to be either sub-unit or suffix
                    if (bSubUnit)
                        eState = EState_Sub;
                    else
                        eState = EState_Suffix;

                    strTmp.Clear();
                }
                 else if (strTmp.bIsEmpty())
                {
                    // First char so has to be alpha
                    if (!TRawStr::bIsAlpha(chCur))
                        return kCIDLib::False;

                    strTmp.Append(chCur);
                }
                 else
                {
                    // Has to be alpha
                    if (!TRawStr::bIsAlpha(chCur))
                        return kCIDLib::False;

                    strTmp.Append(chCur);
                }
                break;

            case EState_Sub :
                if (chCur == kCIDLib::chTilde)
                {
                    // Sub unit cannot be empty
                    if (strTmp.bIsEmpty())
                        return kCIDLib::False;

                    // We have to move to the suffix now
                    eState = EState_Suffix;
                    strTmp.Clear();
                }
                 else if (strTmp.bIsEmpty())
                {
                    // First char so has to be alphanum
                    if (!TRawStr::bIsAlphaNum(chCur))
                        return kCIDLib::False;

                    strTmp.Append(chCur);
                }
                 else
                {
                    // Has to be alphanum, hyphen or underscore
                    if (TRawStr::bIsAlphaNum(chCur)
                    ||  (chCur == kCIDLib::chHyphenMinus)
                    ||  (chCur == kCIDLib::chUnderscore))
                    {
                        strTmp.Append(chCur);
                    }
                     else
                    {
                        return kCIDLib::False;
                    }
                }
                break;

            case EState_Suffix :
                if (strTmp.bIsEmpty())
                {
                    //
                    //  First char has to be either alpha num, or dollar for the
                    //  special driver fields.
                    //
                    if (!TRawStr::bIsAlphaNum(chCur)
                    &&  (chCur != kCIDLib::chDollarSign))
                    {
                        return kCIDLib::False;
                    }
                    strTmp.Append(chCur);
                }
                 else if (TRawStr::bIsAlphaNum(chCur)
                      ||  (chCur == kCIDLib::chHyphenMinus)
                      ||  (chCur == kCIDLib::chUnderscore))
                {
                    // Else can be a general field character
                    strTmp.Append(chCur);
                }
                 else
                {
                    return kCIDLib::False;
                }
                break;

            default :
                CIDAssert2(L"Unknown fld name state")
                break;
        };
    }

    //
    //  We have to have made it to the suffix state, and the suffix cannot be
    //  empty.
    //
    return ((eState == EState_Suffix) && !strTmp.bIsEmpty());
}



//
//  Checks if a character would be a legal field name char at the passed
//  index.
//
tCIDLib::TBoolean
TFacCQCKit::bIsValidFldNameChar(const   tCIDLib::TCh    chNew
                                ,       tCIDLib::TCard4 c4Index) const
{
    // The first char has to be alphanum
    if (!c4Index)
        return TRawStr::bIsAlphaNum(chNew);

    // Otherwise it can be alphanum or hyphen or underscore
    return TRawStr::bIsAlphaNum(chNew)
           || (chNew == kCIDLib::chHyphenMinus)
           || (chNew == kCIDLib::chUnderscore);
}


// Checks the passed string to see if it's a valid moniker
tCIDLib::TBoolean TFacCQCKit::bIsValidMoniker(const TString& strToCheck)
{
    const tCIDLib::TCard4 c4Count = strToCheck.c4Length();
    if (!c4Count)
        return kCIDLib::False;

    // The first char must be an alpha
    if (!TRawStr::bIsAlphaNum(strToCheck[0]))
        return kCIDLib::False;

    // now check the rest of them
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCh chCur = strToCheck[c4Index];

        // If not either alphanum or hyphen or underscore, then it's bad
        if (!TRawStr::bIsAlphaNum(chCur)
        &&  (chCur != kCIDLib::chHyphenMinus)
        &&  (chCur != kCIDLib::chUnderscore))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Does a simple syntactical level validation of a type relative path. It may be non-fully
//  qualified, i.e. ./whatever. Or it could be /User/whatever or /System/whatever.
//  We make sure there are no double (or worse) separators, that there's only leading
//  ./ or ../ entries, none in the middle of paths,
//
//  Sometimes only fully qualified paths are acceptable, so they can tell us if that's
//  the case. Then it has to start with /User or /System.
//
//  This is really related to the remote browsing stuff, but TFacCQCRemBrws is somewhat
//  high level and we need this validation lower down in some cases, such as in the
//  core action command targets, some of which operate on remote files.
//
tCIDLib::TBoolean
TFacCQCKit::bIsValidRemTypeRelPath( const   TString&            strToCheck
                                    ,       TString&            strErrMsg
                                    , const tCIDLib::TBoolean   bRelativeOK) const
{
    // It obviously cannot be empty
    if (strToCheck.bIsEmpty())
    {
        strErrMsg = L"The path cannot be empty";
        return kCIDLib::False;
    }

    // If relative paths aren't valid, then reject if first char is not a forward dlash
    if (!bRelativeOK && (strToCheck.chFirst() != kCIDLib::chForwardSlash))
    {
        if (!bRelativeOK)
        {
            strErrMsg = L"Relative paths are not accepted here";
            return kCIDLib::False;
        }
    }

    // Otehrwise it has to start with either /User/, /System, ., or ..
    if (!strToCheck.bStartsWithI(L"/User")
    &&  !strToCheck.bStartsWithI(L"/System")
    &&  !strToCheck.bStartsWithI(L"../")
    &&  !strToCheck.bStartsWithI(L"./"))
    {
        strErrMsg = L"The path must start with /User, /System, or a relative path (. or ..)";
        return kCIDLib::False;
    }

    //
    //  OK, that's the obvious stuff. So let's do a smalls state machine where we run
    //  through all the characters and check them.
    //
    enum EStates
    {
        EState_FirstChar
        , EState_SawPeriod
        , EState_SawPeriod2
        , EState_SawSlash
        , EState_InPathComp
    };

    tCIDLib::TBoolean bWasFQ = kCIDLib::False;
    tCIDLib::TBoolean bHaveRealComps = kCIDLib::False;
    EStates eState = EState_FirstChar;
    tCIDLib::TCard4 c4CompCnt = 0;
    TString strCurComp;
    const tCIDLib::TCard4 c4Count = strToCheck.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCh chCur = strToCheck[c4Index];

        switch(eState)
        {
            case EState_FirstChar :
                // Has to be a relative or fully qualified character
                if (chCur == kCIDLib::chPeriod)
                {
                    eState = EState_SawPeriod;
                }
                 else if (chCur == kCIDLib::chForwardSlash)
                {
                    eState = EState_InPathComp;
                    strCurComp.Clear();
                    c4CompCnt++;

                    // Remember we are fully qualified
                    bWasFQ = kCIDLib::True;
                }
                 else
                {
                    strErrMsg = L"The path must start wither . or /";
                    return kCIDLib::False;
                }
                break;

            case EState_SawPeriod :

                // If the path was fully qualified, then periods are never valid
                if (bWasFQ)
                {
                    strErrMsg = L"Periods are only valid in relative paths";
                    return kCIDLib::False;
                }

                //
                //  If we have any path components so far (not . or .. ones but real ones)
                //  then periods are never allowed. They are only allowed as leading
                //  content.
                //
                if (bHaveRealComps)
                {
                    strErrMsg = L".. is not allowed within a path, only as leading values";
                    return kCIDLib::False;
                }

                //
                //  Here we can only see either a second period, or a slash. If
                //  we are past offset 1, then it has to be period since we only allow
                //  a first single period.
                //
                if (chCur == kCIDLib::chPeriod)
                {
                    eState = EState_SawPeriod2;
                }
                 else if (chCur == kCIDLib::chForwardSlash)
                {
                    if (c4Index > 1)
                    {
                        strErrMsg = L"A single period can only be the first character";
                        return kCIDLib::False;
                    }
                     else
                    {
                        eState = EState_InPathComp;
                        strCurComp.Clear();
                        c4CompCnt++;
                    }
                }
                 else
                {
                    strErrMsg = L"Expected either . or / at offset ";
                    strErrMsg.AppendFormatted(c4Index - 1);
                    return kCIDLib::False;
                }
                break;

            case EState_SawPeriod2 :
                // We have to see a forward slash here
                if (chCur == kCIDLib::chForwardSlash)
                {
                    eState = EState_InPathComp;
                    strCurComp.Clear();
                    c4CompCnt++;
                }
                 else
                {
                    strErrMsg = L"Only a forward slash is valid following a ..";
                    return kCIDLib::False;
                }
                break;

            case EState_InPathComp :
                //
                //  If we see a forward slash then we have to have at least one char
                //  in strCurComp. Else its a double slash. Else it has to be a valid
                //  character, so alpha num, underscore, hyphen, or space.
                //
                //  As long as there have been no components seen yet, we can see
                //  a .. but not a . by itself.
                //
                if (chCur == kCIDLib::chForwardSlash)
                {
                    if (strCurComp.bIsEmpty())
                    {
                        strErrMsg = L"Multiple adjacent slashes are not allowed";
                        return kCIDLib::False;
                    }

                    // Else we clear the component text and stay in this state
                    strCurComp.Clear();
                    c4CompCnt++;
                }
                 else if (chCur == kCIDLib::chPeriod)
                {
                    //
                    //  Has to be the first character in the component, since it can
                    //  only be used as a . or .. separate path component.
                    //
                    if (strCurComp.bIsEmpty())
                    {
                        eState = EState_SawPeriod;
                    }
                     else
                    {
                        strErrMsg = L"Period is only valid as a separate path component";
                        return kCIDLib::False;
                    }
                }
                 else
                {
                    if (!TRawStr::bIsAlphaNum(chCur)
                    &&  (chCur != kCIDLib::chHyphenMinus)
                    &&  (chCur != kCIDLib::chUnderscore)
                    &&  (chCur != kCIDLib::chSpace))
                    {
                        strErrMsg = L"Character at offset ";
                        strErrMsg.AppendFormatted(c4Index - 1);
                        strErrMsg.Append(L" is not alpha-numeric, hyphen, or underscore");
                        return kCIDLib::False;
                    }

                    // Add this to our current component
                    strCurComp.Append(chCur);

                    //
                    //  At this point we have a real path component, so no more
                    //  relative components allowed.
                    //
                    bHaveRealComps = kCIDLib::True;
                }
                break;

            default :
                CIDAssert2(L"Unknown rembrws path validation state")
                break;
        };
    }

    //
    //  If we get here in anything other than InPathComp mode, or if we are but
    //  the compnent text is empty, that's not good. The latter means that it ended
    //  with a slash.
    //
    if ((eState != EState_InPathComp) || !strCurComp.c4Length())
    {
        strErrMsg = L"The path cannot end with any special character";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



//
//  Locals up standard command line and environmental information that
//  all CQC apps do or might use. This is fundamental, so we catch any
//  exceptions and just return a status result and, if failed, a fail
//  reason. The logging system won't even be set up by this point.
//
tCIDLib::TBoolean
TFacCQCKit::bLoadEnvInfo(       TString&            strFailReason
                        , const tCIDLib::TBoolean   bIsClientApp)
{
    // Clear any existing stuff
    m_strDataDir.Clear();

    try
    {
        //
        //  Figure out if we are running in a development scenario. We have to do
        //  some paths differently if so. We want to keep data with the code it's
        //  being development with, so we put stuff under CQCData instead of the
        //  shared ProgramData directory (even if a client app.)
        //
        //  Other servers and clients may do similarly, so we store the is client
        //  app flag and provide a method to get it.
        //
        TPathStr pathTmp;
        m_bDevMode = TProcEnvironment::bFind(L"CQC_INDEVENV", pathTmp);
        m_bSysEdit = TProcEnvironment::bFind(L"CQC_SYSEDIT", pathTmp);

        //
        //  First look in the environment. We'll let any command line stuff
        //  override anything we find here.
        //
        TProcEnvironment::bFind(kCQCKit::pszDataDirVarName, m_strDataDir);

        // We are possibly removing values, so just loop while the cursor is valid
        TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
        TString strCurParm;
        while (cursParms)
        {
            strCurParm = *cursParms;

            tCIDLib::TBoolean bMagic = kCIDLib::True;
            if (strCurParm.bStartsWithI(L"/DataDir="))
            {
                strCurParm.Cut(0, 9);
                m_strDataDir = strCurParm;
            }
             else if (strCurParm.bStartsWithI(L"/EventPort="))
            {
                strCurParm.Cut(0, 11);
                m_ippnEvents = strCurParm.c4Val(tCIDLib::ERadices::Dec);
            }
             else
            {
                // Not a magic parameter
                bMagic = kCIDLib::False;
            }


            //
            //  Remove the 'magic' parameters now, so that apps don't have
            //  to explicitly skip over them in order to correctly check for
            //  unknown parameters. If not a magic one move the index forward.
            //
            if (bMagic)
                TSysInfo::RemoveCmdLineParm(cursParms);
            else
                ++cursParms;
        }

        //
        //  If no data dir and this is not a client app or we are in
        //  development mode, that's bad
        //
        if ((m_bDevMode || !bIsClientApp) && m_strDataDir.bIsEmpty())
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCfg_NoDataDir
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }


        //
        //  This is a read only directory for data we want to make available locally
        //  on all machines, instead of downloading. Since it's read only we always
        //  use the server side data dir, so do it now before we might change the
        //  data directory below.
        //
        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Local");
        m_strLocalDataDir = pathTmp;

        //
        //  So if a client app and not in dev mode, move the data dir, setting the
        //  appropriate access rights if we need to create the directories.
        //
        m_strOrgDataDir = m_strDataDir;
        if (bIsClientApp && !m_bDevMode)
        {
            pathTmp = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonAppData);
            pathTmp.AddLevel(L"CQCData");

            if (!TFileSys::bIsDirectory(pathTmp))
                TFileSys::MakeDirectory(pathTmp, kCIDLib::False, kCIDLib::True);

            m_strDataDir = pathTmp;
        }

        // Now set up stuff that is relative to the adjusted data dir

        //
        //  The client side data file is done in the adjusted data dir, so that the
        //  client side driver can write to it if needed. Otherwise all of the driver
        //  related directories are done above on the normal CQCData dir.
        //
        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Server");
        m_strServerDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevels(L"Server", L"Data");
        m_strServerDataDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Client");
        m_strClientDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevels(L"Client", L"Data");
        m_strClientDataDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"MacroFileRoot");
        m_strMacroRootPath = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Logs");
        m_strLocalLogDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Repository");
        m_strRepositoryDir = pathTmp;

        pathTmp = m_strDataDir;
        pathTmp.AddLevel(L"Users");
        m_strUsersDir = pathTmp;

        // The binary directory is the same as our facility path
        m_strBinDir = strPath();

        //
        //  If in system edit mode, then set the top level macro class scope to the root.
        //  Otherwise it will remain at the default /User scope.
        //
        if (m_bSysEdit)
            m_strMacroRootClassScope = L"/";

        //
        //  Create any of the dirs that don't exist. The main data dir must
        //  already exist, but we can create any underneath it that haven't
        //  been created yet. Tell it its not an error if it already exists.
        //
        //  If we have redirected to the common app path, then we created it
        //  with the appropriate rights and any paths we create under it
        //  should inherit those rights.
        //
        TFileSys::MakeDirectory(m_strDataDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strClientDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strClientDataDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strLocalDataDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strLocalLogDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strMacroRootPath, kCIDLib::False);
        TFileSys::MakeDirectory(m_strRepositoryDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strServerDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strServerDataDir, kCIDLib::False);
        TFileSys::MakeDirectory(m_strUsersDir, kCIDLib::False);
    }

    catch(const TError& errToCatch)
    {
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is a generic helper that is used anywhere that we support relative paths
//  in our repositories. All such paths will begin with a slash if fully qualified,
//  else a . or .. if relative.
//
//  We return true if we are able to return a relative path, else false to indicate
//  there was no common point and we had to return a full path.
//
//  See bExpandResPath for the opposite operation to expand one back out.
//
tCIDLib::TBoolean
TFacCQCKit::bMakeRelResPath(const   TString&    strBasePath
                            , const TString&    strTarPath
                            ,       TString&    strNewPath) const
{
    //
    //  If the target path is itself relative, then they entered a relative path, so just
    //  take it as is.
    //
    if (strTarPath.bStartsWith(L"."))
    {
        strNewPath = strTarPath;
        return kCIDLib::True;
    }

    //
    //  To make things easier we use the path string class which has functionality for this.
    //  We just have to flip the separators.
    //
    TPathStr pathBase(strBasePath);
    TPathStr pathTar(strTarPath);
    pathBase.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);
    pathTar.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);

    //
    //  Normalize them both. This will get rid of any .. type entries that would
    //  complicate things.
    //
    pathBase.Normalize();
    pathTar.Normalize();

    // Remove the name.ext bit of the base
    pathBase.bRemoveNameExt();

    // And save away the name/ext of the target
    TString strNameExt;
    pathTar.bExtractNameExt(strNameExt);

    // Make sure the base has a trailing separator
    pathBase.AddTrailingSeparator();

    //
    //  Check the easy one first. If the target starts with the base, then it's a
    //  simple relationship. We just cut the base part off of the target and prepend
    //  a ./. Adding the trailing separator above means we can't get a partial match
    //  that's not on a component boundary.
    //
    //  Else it's got to be an up and back down scenario, so we have to do more work.
    //
    if (pathTar.bStartsWithI(pathBase))
    {
        pathTar.Cut(0, pathBase.c4Length());
        pathTar.Prepend(L".\\");

        // Put the name/ext back on it
        if(!strNameExt.bIsEmpty())
            pathTar.AddLevel(strNameExt);

        strNewPath = pathTar;
    }
     else
    {
        //
        //  Break them out into separate lists of components that we can compare
        //
        tCIDLib::TStrList colBase;
        tCIDLib::TStrList colTar;

        TStringTokenizer stokPath(&pathBase, L"\\");
        stokPath.c4BreakApart(colBase);
        stokPath.Reset(&pathTar, L"\\");
        stokPath.c4BreakApart(colTar);


        // Throw away components from both as long as they are matching
        tCIDLib::TCard4 c4Count(tCIDLib::MinVal(colBase.c4ElemCount(),colTar.c4ElemCount()));
        const tCIDLib::TCard4 c4OrgCount = c4Count;
        do
        {
            if (colBase[0].bCompareI(colTar[0]))
            {
                colBase.RemoveAt(0);
                colTar.RemoveAt(0);
                c4Count--;
            }
             else
            {
                break;
            }
        }   while (c4Count);

        //
        //  If none, then just take the full path, which probably means it's bad but not our
        //  problem. Indicate we couldn't relativize it.
        //
        if (c4Count == c4OrgCount)
        {
            strNewPath = strTarPath;
            return kCIDLib::False;
        }

        //
        //  Now, for any that are left in the base path, we have to start off with
        //  ..\ entries in the target.
        //
        strNewPath.Clear();
        c4Count = colBase.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            strNewPath.Append(L"..\\");

        // And now add any remaining values in the target
        c4Count = colTar.c4ElemCount();
        for(tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (c4Index)
                strNewPath.Append(kCIDLib::chBackSlash);
            strNewPath.Append(colTar[c4Index]);
        }

        // And finally add the name/ext back
        if (!strNameExt.bIsEmpty())
        {
            // Add a trailing slash if needed
            if (strNewPath.chLast() != kCIDLib::chBackSlash)
                strNewPath.Append(kCIDLib::chBackSlash);
            strNewPath.Append(strNameExt);
        }
    }

    // Now flip the seprators back
    strNewPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);

    return kCIDLib::True;
}


//
//  Breaks out the monikier/field name parts of a fully qualified field
//  name, and verifies that they are correct in the process.
//
tCIDLib::TBoolean
TFacCQCKit::bParseFldName(  const   TString& strToParse
                            ,       TString& strMoniker
                            ,       TString& strFldName) const
{
    // Call the other version with a dummy device class
    tCQCKit::EDevClasses eDevClass;
    return bParseFldName(strToParse, strMoniker, eDevClass, strFldName);
}

tCIDLib::TBoolean
TFacCQCKit::bParseFldName(  const   TString&                strToParse
                            ,       TString&                strMoniker
                            ,       tCQCKit::EDevClasses&   eDevClass
                            ,       TString&                strFldName
                            , const tCIDLib::TBoolean       bNoMon) const
{
    //
    //  Call the ther version and just combine the parts to return the whole field
    //  name bit.
    //
    TString strSuffix;
    TString strSub;
    if (!bParseFldName(strToParse, strMoniker, eDevClass, strSub, strSuffix, bNoMon))
        return kCIDLib::False;

    strFldName.Clear();
    if (eDevClass != tCQCKit::EDevClasses::None)
    {
        // This also insures taht the device class is the right case
        strFldName.Append(tCQCKit::strAltXlatEDevClasses(eDevClass));
        strFldName.Append(kCIDLib::chPoundSign);
    }

    if (!strSub.bIsEmpty())
    {
        strFldName.Append(strSub);
        strFldName.Append(kCIDLib::chTilde);
    }

    // And finally the trailing field part
    strFldName.Append(strSuffix);

    return kCIDLib::True;
}

tCIDLib::TBoolean
TFacCQCKit::bParseFldName(  const   TString&                strToParse
                            ,       tCQCKit::EDevClasses&   eDevClass
                            ,       TString&                strSubUnit
                            ,       TString&                strSuffix) const
{
    //
    //  Call the other version with a dummy moniker output and tell it to skip the
    //  moniker.
    //
    TString strMon;
    return bParseFldName
    (
        strToParse, strMon, eDevClass, strSubUnit, strSuffix, kCIDLib::True
    );
}

tCIDLib::TBoolean
TFacCQCKit::bParseFldName(  const   TString&                strToParse
                            ,       TString&                strMoniker
                            ,       tCQCKit::EDevClasses&   eDevClass
                            ,       TString&                strSubUnit
                            ,       TString&                strSuffix
                            , const tCIDLib::TBoolean       bNoMon) const
{
    // States for our state machine
    enum EStates
    {
        EState_Driver
        , EState_DevClass
        , EState_Sub
        , EState_Suffix
    };

    // Clear outputs before starting, which simplifies below
    strMoniker.Clear();
    strSuffix.Clear();
    strSubUnit.Clear();
    eDevClass = tCQCKit::EDevClasses::None;

    // To make things more efficient, do a quick scan for two special cases
    const tCIDLib::TBoolean bDevClass = strToParse.bContainsChar(kCIDLib::chPoundSign);
    const tCIDLib::TBoolean bSubUnit = strToParse.bContainsChar(kCIDLib::chTilde);

    //
    //  There cannot be a sub-unit without a device class. Only V2 style fields
    //  can have sub-units.
    //
    if (bSubUnit && !bDevClass)
        return kCIDLib::False;

    const tCIDLib::TCard4 c4Count = strToParse.c4Length();
    tCIDLib::TCard4 c4Index = 0;

    // Set the starting state appropriately
    EStates eState = EState_Driver;
    if (bNoMon)
    {
        // If no device class, then it can only be a suffix
        if (bDevClass)
            eState = EState_DevClass;
        else
            eState = EState_Suffix;
    }
     else
    {
        eState = EState_Driver;
    }

    while (c4Index < c4Count)
    {
        const tCIDLib::TCh chCur = strToParse[c4Index++];

        switch(eState)
        {
            case EState_Driver :
                //
                //  If we hit the period, we move forward. Else it has
                //  to be alphanum.
                //
                if (chCur == kCIDLib::chPeriod)
                {
                    // The moniker cannot be empty
                    if (strMoniker.bIsEmpty())
                        return kCIDLib::False;

                    // We should either have a device class or suffix
                    if (bDevClass)
                        eState = EState_DevClass;
                    else
                        eState = EState_Suffix;
                }
                 else if (strMoniker.bIsEmpty())
                {
                    // First char so has to be alpha
                    if (!TRawStr::bIsAlpha(chCur))
                        return kCIDLib::False;

                    strMoniker.Append(chCur);
                }
                 else
                {
                    // It has to be alpha num, hyphen or underscore
                    if (TRawStr::bIsAlphaNum(chCur)
                    ||  (chCur == kCIDLib::chHyphenMinus)
                    ||  (chCur == kCIDLib::chUnderscore))
                    {
                        strMoniker.Append(chCur);
                    }
                     else
                    {
                        return kCIDLib::False;
                    }
                }
                break;

            case EState_DevClass :
                //
                //  WE USE the suffix output parm for this, to avoid creating
                //  another string.
                //
                // If we hit a #, then we have the device class, else accumulate
                //  as device class name chars.
                //
                if (chCur == kCIDLib::chPoundSign)
                {
                    // Make sure the device class is valid
                    eDevClass = tCQCKit::eAltXlatEDevClasses(strSuffix);
                    if (eDevClass == tCQCKit::EDevClasses::Count)
                        return kCIDLib::False;

                    // Next has to be either sub-unit or suffix
                    if (bSubUnit)
                        eState = EState_Sub;
                    else
                        eState = EState_Suffix;

                    // Be sure to clear this now
                    strSuffix.Clear();
                }
                 else if (strSuffix.bIsEmpty())
                {
                    // First char so has to be alpha
                    if (!TRawStr::bIsAlpha(chCur))
                        return kCIDLib::False;

                    strSuffix.Append(chCur);
                }
                 else
                {
                    // Has to be alpha
                    if (!TRawStr::bIsAlpha(chCur))
                        return kCIDLib::False;

                    strSuffix.Append(chCur);
                }
                break;

            case EState_Sub :
                if (chCur == kCIDLib::chTilde)
                {
                    // Sub unit cannot be empty
                    if (strSubUnit.bIsEmpty())
                        return kCIDLib::False;

                    // We have to move to the suffix now
                    eState = EState_Suffix;
                }
                 else if (strSubUnit.bIsEmpty())
                {
                    // First char so has to be alphanum
                    if (!TRawStr::bIsAlphaNum(chCur))
                        return kCIDLib::False;

                    strSubUnit.Append(chCur);
                }
                 else
                {
                    // Has to be alphanum, hyphen or underscore
                    if (TRawStr::bIsAlphaNum(chCur)
                    ||  (chCur == kCIDLib::chHyphenMinus)
                    ||  (chCur == kCIDLib::chUnderscore))
                    {
                        strSubUnit.Append(chCur);
                    }
                     else
                    {
                        return kCIDLib::False;
                    }
                }
                break;

            case EState_Suffix :
                if (strSuffix.bIsEmpty())
                {
                    //
                    //  First char has to be either alpha num, or dollar for the
                    //  special driver fields.
                    //
                    if (!TRawStr::bIsAlphaNum(chCur)
                    &&  (chCur != kCIDLib::chDollarSign))
                    {
                        return kCIDLib::False;
                    }
                    strSuffix.Append(chCur);
                }
                 else if (TRawStr::bIsAlphaNum(chCur)
                      ||  (chCur == kCIDLib::chHyphenMinus)
                      ||  (chCur == kCIDLib::chUnderscore))
                {
                    strSuffix.Append(chCur);
                }
                 else
                {
                    return kCIDLib::False;
                }
                break;

            default :
                CIDAssert2(L"Unknown fld name state")
                break;
        };
    }

    // We have to have made it to the suffix state
    return ((eState == EState_Suffix) && !strSuffix.bIsEmpty());
}


//
//  A quicky method for getting the field definition of a field, if it's not already
//  available from some lighter source. We go out to the driver to get it.
//
tCIDLib::TBoolean
TFacCQCKit::bQueryFldDef(const  TString&            strMoniker
                        , const TString&            strField
                        ,       TCQCFldDef&         flddToFill
                        , const tCIDLib::TBoolean   bQuickTest) noexcept
{
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv = orbcCQCSrvAdminProxy(strMoniker, 0, bQuickTest);
        orbcSrv->QueryFieldDef(strMoniker, strField, flddToFill);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  A fairly general purpose bulk token replacer. It allows the caller to
//  pass in list of key/value pairs and it will replace any instances of the
//  keys found in the text with their values.
//
tCIDLib::TBoolean
TFacCQCKit::bReplaceTokens( const   tCIDLib::TKVPCollect&   colList
                            ,       TString&                strToUpdate)
{
    // If the replacement list is empty, just return now
    if (colList.bIsEmpty())
        return kCIDLib::True;

    // Get a cursor over the list
    TColCursor<TKeyValuePair>* pcursToks = colList.pcursNew();
    TJanitor<TColCursor<TKeyValuePair> > janCursor(pcursToks);

    // Keep looping as long as we find replacement tokens to replace
    tCIDLib::TBoolean bGotOne;
    tCIDLib::TBoolean bAnyBad = kCIDLib::False;
    do
    {
        pcursToks->bReset();
        bGotOne = kCIDLib::False;
        do
        {
            // Get the current token, which is the name of the variable
            const TString& strToken = pcursToks->objRCur().strKey();

            tCIDLib::TCard4 c4Pos;
            const tCIDLib::TBoolean bRes = strToUpdate.bFirstOccurrence
            (
                strToken, c4Pos, kCIDLib::False, kCIDLib::False
            );

            if (bRes)
            {
                // Remember we saw one this time through
                bGotOne = kCIDLib::True;

                // Cut out the original text and insert the value
                const TString& strValue = pcursToks->objRCur().strValue();
                strToUpdate.Cut(c4Pos, strToken.c4Length());
                strToUpdate.Insert(strValue, c4Pos);

                //
                //  If the value was empty, then consider it bad, since this
                //  means we replaced it with nothing.
                //
                if (strValue.bIsEmpty())
                    bAnyBad = kCIDLib::True;
            }
        }   while (pcursToks->bNext());
    }   while (bGotOne);
    return !bAnyBad;
}


// Return the system edit flag
tCIDLib::TBoolean TFacCQCKit::bSysEdit() const
{
    return m_bSysEdit;
}


// Creates a standard 3 part compacted CID/CQC version in a TCard8 value
tCIDLib::TCard8
TFacCQCKit::c8MakeVersion(  const   tCIDLib::TCard4 c4Major
                            , const tCIDLib::TCard4 c4Minor
                            , const tCIDLib::TCard4 c4Revision)
{
    tCIDLib::TCard8 c8Ret;
    c8Ret = c4Major;
    c8Ret <<= 16;
    c8Ret |= c4Minor;
    c8Ret <<= 32;
    c8Ret |= c4Revision;
    return c8Ret;
}



//
//  There are some places where we take a string that has replacement tokens
//  in it representing fields, in the form:
//
//      $(moniker.field)
//      %(rtv)
//
//  The former refers to a field value, which we replace them with the actual
//  fieldvalue read from the device driver. The later refers to a runtime value
//  which we will look up in the passed (all optional) runtime value objects or
//  local/global variables objects or the event object.
//
//  The runtime values have two parts, like %(Pref:Key), where the prefix
//  indicates it's a runtime value of a particular type, and the key part
//  indicates what specific value of that type the token represents.
//
//  The tokens can have one or two option parameters, as in:
//
//      $(moniker.field, "pattern", "separator")
//
//  where pattern is a string in which the actual value is embedded. It will
//  have either a %1 or %* value in it, which will be replaced with the actual
//  value. So a token might be: $(moniker.field, "/Value=%1"). %1 means replace
//  it with the formatted value and %* means replace it with unformatted
//  value (i.e. the separator stuff will be ignored if it's a multi-valued
//  replacement value.)
//
//  Another special case pattern is "^1", which indicates that the value
//  should not be futher expanded. It should be taken as is. This can be
//  used to get things like file paths into the output without having to
//  escape every directory separator.
//
//  The separator only comes into play if field/rtv is a multi-valued thing.
//  This would mean a string list field, or a source value that has multiple,
//  new-line separated lines. Each available part of the value will be put
//  into the pattern, then the pattern appended to the output. The separator
//  is inserted before each value but the first. The default for the pattern
//  is just "%1", i.e. just the value, and the default for the separator is
//  just a space.
//
//  If anything fails, it just puts an error message into the output
//  string and returns false.
//
//  They can ask us to just test only, in which case we won't try to do
//  any actual field value lookup, we'll just do a syntax scan. And they
//  ask us not to do escapement, just replace tokens found.
//
//  Escaping is supported via the \ character. The following charaters
//  are escaped:
//
//  1. Outside of tokens: " \ $ %
//  2. Inside quoted parts of tokens: " \
//
//  All others are ignored. Note that the % is not escaped in the quoted
//  parts, so you cannot escape away a %1 or %* token sequence in the pattern
//  part.
//
//  The reason the second two parameters are quoted is that leading/trailing
//  whitespace can be significant in those, whereas it is not in the first
//  parameter.
//
tCQCKit::ECmdPrepRes
TFacCQCKit::eStdTokenReplace(const  TString&                strPattern
                            , const TCQCCmdRTVSrc* const    pmcrtvToUse
                            , const TStdVarsTar* const      pctarGVars
                            , const TStdVarsTar* const      pctarLVars
                            ,       TString&                strResult
                            , const tCQCKit::ETokRepFlags   eFlags)
{
    // Call the recursive helper with the passed stuff and a zero start depth
    return eStdTokenRepHelper
    (
        strPattern
        , pmcrtvToUse
        , pctarGVars
        , pctarLVars
        , strResult
        , eFlags
        , 0
    );
}


//
//  Exectute an external application. We provide replacement parameters
//  for common values, and we add it to the managed list of apps so that
//  the caller can control the app after it starts.
//
tCIDLib::TVoid
TFacCQCKit::ExecApp(const   TString&                strApp
                    , const TString&                strInitPath
                    , const TString&                strParams
                    , const tCIDLib::EExtProcShows  eShow
                    , const TString&                strUserName)
{
    // Update all the strings by replacing any replacement tokens
    TString strActPath(strApp);
    if (!facCQCKit().bReplacePathTokens(strActPath, strUserName))
        return;

    TString strActInit(strInitPath);
    if (!facCQCKit().bReplacePathTokens(strActInit, strUserName))
        return;

    TString strActParams(strParams);
    if (!facCQCKit().bReplacePathTokens(strActParams, strUserName))
        return;

    //
    //  Ok, we can go for it now. First look to see if this app is already
    //  running. If so, then just bring it forward. Else, we have to run
    //  it.
    //
    //  If it's a new one, we have to create a new app item to add. If it
    //  is an existing one, update the init path and params in case they've
    //  changed.
    //
    tCIDLib::TBoolean bNew;
    TAppItem* paiRun = paiFindApp(strActPath);
    if (paiRun)
    {
        bNew = kCIDLib::False;
        paiRun->Update(strActInit, strActParams, eShow);
    }
     else
    {
        bNew = kCIDLib::True;
        paiRun = new TAppItem(strActPath, strInitPath, strActParams, eShow);
    }

    try
    {
        //
        //  If not running, then try to start it. If it is running, then
        //  try to bring it forward if the show option indicates it would
        //  have come forward when started.
        //
        if (!paiRun->bIsRunning())
        {
            if (paiRun->bStart())
            {
                // It ran, so add it to the list if new
                if (bNew)
                    CQCKit_ThisFacility::colAppList.Add(paiRun);
            }
             else
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcExec_CantActivateApp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }
         else
        {
            if (eShow == tCIDLib::EExtProcShows::Normal)
            {
                if (!paiRun->bActivate())
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcExec_CantActivateApp
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        // If it's a new one, then delete it
        if (bNew)
            delete paiRun;

        throw;
    }
}



// Takes a standard 3 part (compacted in a TCard8) CQC version and formats it
tCIDLib::TVoid
TFacCQCKit::FormatVersionStr(TString& strToFill, const tCIDLib::TCard8 c8Ver)
{
    tCIDLib::TCard4 c4Maj, c4Min, c4Revision;
    c4Maj = tCIDLib::TCard4(c8Ver >> 48);
    c4Min = tCIDLib::TCard4((c8Ver >> 32) & 0xFFFF);
    c4Revision = tCIDLib::TCard4(c8Ver & 0xFFFFFFFF);
    strToFill.SetFormatted(c4Maj);
    strToFill.Append(kCIDLib::chPeriod);
    strToFill.AppendFormatted(c4Min);
    strToFill.Append(kCIDLib::chPeriod);
    strToFill.AppendFormatted(c4Revision);
}



// Frees any external applications that we've started and have been tracking
tCIDLib::TVoid TFacCQCKit::FreeApps()
{
    CQCKit_ThisFacility::colAppList.RemoveAll();
}


// Set or get the port that the event system uses
tCIDLib::TIPPortNum TFacCQCKit::ippnEvents() const
{
    return m_ippnEvents;
}

tCIDLib::TIPPortNum TFacCQCKit::ippnEvents(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnEvents = ippnToSet;
    return m_ippnEvents;
}


//
//  For 5.0 we finally got rid of the awkward backslash path format for server based
//  resources (an issue due to the backslash being the escapement character). So we
//  will be converting paths all over the place as we we read them in. This keeps that
//  in one place
//
tCIDLib::TVoid TFacCQCKit::Make50Path(TString& strToConvert)
{
    // First do double slashes
    tCIDLib::TCard4 c4At = 0;
    strToConvert.bReplaceSubStr(L"\\\\", L"/", c4At, kCIDLib::True);

    // Then single
    strToConvert.bReplaceChar(L'\\', L'/');
}


//
//  This is called to get the next available unique event id, to be put into outgoing
//  events. To keep overhead down, we calculate a new hash every 64K times, and then
//  we just bump the last two bytes to create 64K new varations of that.
//
//  No sync required here, this is only called by the send thread.
//
tCIDLib::TVoid TFacCQCKit::GetNextEventId(TMD5Hash& mhashToFill)
{
    // Update the sequence number
    tCIDLib::TCard2 c2Seq;
    m_c4EvSeq++;
    if (m_c4EvSeq >= kCIDLib::c2MaxCard)
    {
        // Make a new hash
        m_mhashEvId = TUniqueId::mhashMakeId();
        m_c4EvSeq = 0;
    }
    c2Seq = tCIDLib::TCard2(m_c4EvSeq);

    // Update it to the next sequence id and store on the event
    m_mhashEvId.PutAt(14, tCIDLib::TCard1(c2Seq & 0xFF));
    m_mhashEvId.PutAt(15, tCIDLib::TCard1(c2Seq >> 8));
    mhashToFill = m_mhashEvId;
}


//
//  Get a proxy for the installation server interface
//
tCQCKit::TInstSrvProxy
TFacCQCKit::orbcInstSrvProxy(const  tCIDLib::TCard4     c4WaitUpTo
                            , const tCIDLib::TBoolean   bQuickTest) const
{
    TCQCInstClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCInstClientProxy>
    (
        TCQCInstClientProxy::strBinding, c4WaitUpTo, bQuickTest
    );

    if (!porbcProxy)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_InstServerNotAvail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return tCQCKit::TInstSrvProxy(porbcProxy);
}


//
//  Gets a CQCServer admin proxy client for the CQCServer that is hosting the
//  indicated driver. So it does the name server lookup to find the server, and then
//  gets an admin proxy.
//
//  Note that it's returning a reference counted admin proxy wrapper here not the
//  raw proxy object pointer. This lets us use by-value semantics on these.
//
tCQCKit::TCQCSrvProxy
TFacCQCKit::orbcCQCSrvAdminProxy(const  TString&            strDriverMoniker
                                , const tCIDLib::TCard4     c4WaitFor
                                , const tCIDLib::TBoolean   bQuickTest) const
{
    // If they pass zero time, choose a default
    const tCIDLib::TCard4 c4WaitMS = c4WaitFor ? c4WaitFor : 5000;

    // We need a name server proxy for this
    tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(c4WaitMS);

    //
    //  Build up the name server binding path for this driver. It contains an 'alias'
    //  OOID for the CQCServer that hosts it.
    //
    TString strName;
    TString strDescr;

    TString strPath(TCQCSrvAdminClientProxy::strDrvScope);
    strPath.Append(kCIDLib::pszTreeSepChar);
    strPath.Append(strDriverMoniker);

    //
    //  The ORB utilities client has a helper we use to get the OOID for this binding
    //  path. This goes through a cache it provides.
    //
    TOrbObjId ooidDrv;
    if (!facCIDOrbUC().bGetNSObject(orbcNS, strPath, ooidDrv, c4WaitMS, bQuickTest))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_DrvNotFound
            , strPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strDriverMoniker
        );
    }

    // Gen up the client proxy based on the object id we got, and return in a cptr
    TCQCSrvAdminClientProxy* porbcRet = new TCQCSrvAdminClientProxy(ooidDrv, strPath);
    return tCQCKit::TCQCSrvProxy(porbcRet);
}


//
//  Get a proxy for the security server interface
//
tCQCKit::TSecuritySrvProxy
TFacCQCKit::orbcSecuritySrvProxy(const  tCIDLib::TCard4     c4WaitUpTo
                                , const tCIDLib::TBoolean   bQuickTest) const
{
    TCQCSecureClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSecureClientProxy>
    (
        TCQCSecureClientProxy::strBinding, c4WaitUpTo, bQuickTest
    );

    if (!porbcProxy)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_SecServerNotAvail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return tCQCKit::TSecuritySrvProxy(porbcProxy);
}


//
//  Given the indicated driver and field id and field type, creates a
//  field value object for it. It's not fully set up, but this saves a
//  lot of grunt work that would otherwise be done in a number of places.
//
TCQCFldValue*
TFacCQCKit::pfvMakeNewFor(  const   tCIDLib::TCard4     c4DriverId
                            , const tCIDLib::TCard4     c4FldId
                            , const tCQCKit::EFldTypes  eType)
{
    TCQCFldValue* pfvNew = nullptr;
    switch(eType)
    {
        case tCQCKit::EFldTypes::Boolean :
            pfvNew = new TCQCBoolFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::Card :
            pfvNew = new TCQCCardFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::Float :
            pfvNew = new TCQCFloatFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::Int :
            pfvNew = new TCQCIntFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::String :
            pfvNew = new TCQCStringFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::StringList :
            pfvNew = new TCQCStrListFldValue(c4DriverId, c4FldId);
            break;

        case tCQCKit::EFldTypes::Time :
            pfvNew = new TCQCTimeFldValue(c4DriverId, c4FldId);
            break;

        default :
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(eType))
            );
            break;
    };
    return pfvNew;
}


//
//  Return a CQC Server Admin client proxy for a CQC Server running on the
//  indicated host. This is used by other versions above, which return the same
//  wrapped in the standard counted pointer.
//
//  If you just have a driver moniker use the version that takes that, since there
//  are 'alias' name server entries for each driver that points to the admin intf
//  for the hosting CQCServer.
//
TCQCSrvAdminClientProxy*
TFacCQCKit::porbcCQCSrvAdminProxy(  const   TString&            strHostName
                                    , const tCIDLib::TCard4     c4WaitUpTo
                                    , const tCIDLib::TBoolean   bQuickTest) const
{
    // Build up the name server binding for our target interface/host
    TString strPath = TCQCSrvAdminClientProxy::strAdminScope;
    strPath.Append(kCIDLib::pszTreeSepChar);
    strPath.Append(strHostName);

    TCQCSrvAdminClientProxy* porbcProxy
    (
        facCIDOrbUC().porbcMakeClient<TCQCSrvAdminClientProxy>
        (
            strPath, c4WaitUpTo, bQuickTest
        )
    );

    if (!porbcProxy)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_CQCSrvNotFound
            , strPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strHostName
        );
    }

    // If we get here, we should have the object
    CIDAssert(porbcProxy != nullptr, L"Did not allocate CQCServer Admin proxy");
    return porbcProxy;
}



//
//  These methods will get an image ready to be either uploaded to the master server
//  or added to a package. It will flatten the image out to the provided image stream,
//  then it will create at thumbnail from that and flatten it out to the provided
//  thumb image. It will extract some info from the image and give it back, which will
//  be useful to the caller for upload and such.
//
//  These will typically then be placed into a TChunkedFile object for upload to the
//  MS or storage in a package.
//
//  WE have one that takes a source file and creates the image for the caller, and
//  then calls the other version which just takes an already existing PNG image object.
//
tCIDLib::TVoid
TFacCQCKit::PackageImg( const   TString&                strSrcFile
                        , const TPoint&                 pntTransAt
                        , const tCIDLib::TBoolean       bTrans
                        ,       TBinOutStream&          strmImg
                        ,       TBinOutStream&          strmThumb
                        ,       tCIDImage::EPixFmts&    eFmt
                        ,       tCIDImage::EBitDepths&  eDepth
                        ,       TSize&                  szImg
                        ,       TSize&                  szThumb)
{
    //
    //  Load the file, converting to PNG if needed. Tell it to throw if
    //  it fails.
    //
    TPNGImage imgPNG;
    facCIDImgFact().bLoadToPNG(strSrcFile, imgPNG, kCIDLib::True);

    // Ask the image for the color at the indicated point
    const tCIDLib::TCard4 c4Val = imgPNG.c4ClrAt(pntTransAt);

    // And set that as the transparency color if asked to
    imgPNG.bTransClr(bTrans);
    if (bTrans)
        imgPNG.c4TransClr(c4Val);

    //
    //  And now we can call the common method to package the image. He'll
    //  give us back the flattened image and thumb and their sizes, which
    //  we pass back in turn.
    //
    PackageImg(imgPNG, strmImg, strmThumb, szThumb);

    // Return some image info now that we know it packed ok
    const TPixelArray& pixaImg = imgPNG.pixaBits();
    szImg = pixaImg.szImage();
    eDepth = pixaImg.eBitDepth();
    eFmt = pixaImg.ePixFmt();
}

tCIDLib::TVoid
TFacCQCKit::PackageImg( const   TPNGImage&      imgToPack
                        ,       TBinOutStream&  strmImg
                        ,       TBinOutStream&  strmThumb
                        ,       TSize&          szThumb)
{
    // Get some info out of the image for our local use
    const TPixelArray& pixaImg = imgToPack.pixaBits();
    const TSize& szImg = pixaImg.szImage();

    //
    //  Create the thumbnail version of the image. If it's smaller than the
    //  thumbnail size, then we just use the image directly. Else, we
    //  do a scaling operation to create a new version of the image that
    //  will fit the thumb size.
    //
    //  If it'll fit, then take it as is, else size it so that the either
    //  the axis that won't fit or the larger axis if neither do, and
    //  the other is scaled to maintain the aspect ratio.
    //
    szThumb = kCQCKit::szThumbnail;

    if ((szImg.c4Width() <= kCQCKit::szThumbnail.c4Width())
    &&  (szImg.c4Height() <= kCQCKit::szThumbnail.c4Height()))
    {
        szThumb = szImg;
    }
     else
    {
        // Calculate the AR of the source and target
        const tCIDLib::TFloat8 f8OrgAR
        (
            tCIDLib::TFloat8(szImg.c4Width())
            / tCIDLib::TFloat8(szImg.c4Height())
        );

        const tCIDLib::TFloat8 f8TarAR
        (
            tCIDLib::TFloat8(szThumb.c4Width())
            / tCIDLib::TFloat8(szThumb.c4Height())
        );

        //
        //  If the target AR is wider, then let the height fit and
        //  scale the width to keep the original AR. Otherwise, do the
        //  opposite.
        //
        if (f8TarAR > f8OrgAR)
            szThumb.c4Width(tCIDLib::TCard4(szThumb.c4Height() * f8OrgAR));
        else
            szThumb.c4Height(tCIDLib::TCard4(szThumb.c4Width() / f8OrgAR));
    }

    //
    //  Stream out the original image now, since we may want to mangle it
    //  a bit below while creating the thumb.
    //
    strmImg.Reset();
    strmImg << imgToPack << kCIDLib::FlushIt;

    //
    //  If the image was smaller than the thumb, then we just take the image
    //  as is for the thumb data. Otherwise, we have to scale it down. So
    //  create a new image object with the same attributes except for the
    //  size.
    //
    strmThumb.Reset();
    if (szImg == szThumb)
    {
        strmThumb << imgToPack << kCIDLib::FlushIt;
    }
     else
    {
        //
        //  Note that this ctor doesn't copy the pixel data, it just creates
        //  a new image object with the same data format, but with the new
        //  size. We can then scale from the original image into it.
        //
        TPNGImage imgThumb(imgToPack, szThumb);
        imgThumb.MakeScaledFrom(imgToPack, 2);
        strmThumb << imgThumb << kCIDLib::FlushIt;

        // Get the new size back out
        szThumb = imgThumb.szImage();
    }
}



//
//  Breaks out the monikier/field name parts of a fully qualified field
//  name.
//
tCIDLib::TVoid
TFacCQCKit::ParseFldName(const  TString& strToParse
                        ,       TString& strMoniker
                        ,       TString& strFldName) const
{
    // Call the other version, and if it returns false, then throw
    if (!bParseFldName(strToParse, strMoniker, strFldName))
    {
        // Something wasn't right, so throw
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_BadFldNameFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strToParse
        );
    }
}


//
//  We have an issue where remote browser paths cannot have consequetive slashes in
//  them or bad problems occur. The OS level file I/O will work since the OS ignores them
//  but we then end up with the same file in multiple data server cache slots. This is
//  called to create a version that doesn't have any such, and will check for some other
//  things as well potentially.
//
//  We have one that goes from one string to another and another that does it in place.
//
tCIDLib::TVoid TFacCQCKit::PrepRemBrwsPath(const TString& strSrc, TString& strPrepped)
{
    tCIDLib::TBoolean bSawSlash = kCIDLib::False;

    strPrepped.Clear();
    const tCIDLib::TCard4 c4SrcCnt = strSrc.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCnt; c4Index++)
    {
        tCIDLib::TCh chCur = strSrc[c4Index];

        // Flip any backslashes to forward
        if (chCur == kCIDLib::chBackSlash)
            chCur = kCIDLib::chForwardSlash;

        if (chCur == kCIDLib::chForwardSlash)
        {
            // If we didn't just see a slash, take it and saw we saw one
            if (!bSawSlash)
            {
                strPrepped.Append(chCur);
                bSawSlash = kCIDLib::True;
            }
        }
         else
        {
            // Store and go out of slash mode
            strPrepped.Append(chCur);
            bSawSlash = kCIDLib::False;
        }
    }
}

tCIDLib::TVoid TFacCQCKit::PrepRemBrwsPath(TString& strToPrep)
{
    static const TString strDbl(L"//");
    static const TString strSingle(L"/");

    //
    //  Just do a loop until we get no more replacements. To be absolutely safe, use a
    //  counter. It's unlikely to ever run more than once, but if we got a double or
    //  quadruple slash, we need to handle that.
    //
    tCIDLib::TCard4 c4Count = 0;
    tCIDLib::TCard4 c4At = 0;
    while (strToPrep.bReplaceSubStr(strDbl, strSingle, c4At, kCIDLib::True))
    {
        c4At = 0;
        c4Count++;
        if (c4Count > 256)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcRem_CanPrepPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
    }
}


//
//  We'll format out and return info on all of the applications that have
//  been started via our ExecApp method, and which therefore we are tracking.
//  This is used by some apps to store that info while they are down and
//  then reattach when they come back up.
//
tCIDLib::TVoid TFacCQCKit::QueryAppAttachInfo(TString& strToFill)
{
    strToFill.Clear();
    try
    {
        const tCIDLib::TCard4 c4Count = CQCKit_ThisFacility::colAppList.c4ElemCount();
        if (c4Count)
        {
            // We have some, so set a stream on the caller's string
            TTextStringOutStream strmOut(&strToFill);

            //
            //  Run through our list of controlled apps and, for any that
            //  are still running, format out their attach info. We collect
            //  them into a newline separated buffer.
            //
            TString strCur;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TAppItem* paiCur = CQCKit_ThisFacility::colAppList[c4Index];

                paiCur->FormatAttachData(strCur);
                if (!strCur.bIsEmpty())
                    strmOut << strCur << kCIDLib::NewLn;
            }
            strmOut.Flush();
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  Queues up a standard event trigger. It is assumed that the caller has provided
//  reasonable values for the type of event indicated.
//
tCIDLib::TVoid
TFacCQCKit::QueueStdEventTrig(  const   tCQCKit::EStdDrvEvs eEvent
                                , const TString&            strMoniker
                                , const TString&            strField
                                , const TString&            strVal1
                                , const TString&            strVal2
                                , const TString&            strVal3
                                , const TString&            strVal4)
{
    // If event sending isn't enabled, then just ignore it
    if (!tCIDLib::bAllBitsOn(m_eEvProcType, tCQCKit::EEvProcTypes::Send))
        return;

    // If the queue is full, then log an error and return
    if (m_colEvSQ.bIsFull(CQCKit_ThisFacility::c4EvOutQSize))
    {
        // Update our dropped outout triggers stat
        TStatsCache::c8IncCounter(m_sciOutTrigsDropped);

        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvTrg_SendQFull
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::OutResource
            );
        }
        return;
    }

    //
    //  Create a new object and fill it in
    //
    TCQCEvent* pcevNew = new TCQCEvent;
    TJanitor<TCQCEvent> janEvent(pcevNew);

    // Ask it to build a standard event
    pcevNew->BuildStdDrvEvent
    (
        eEvent, strMoniker, strField, strVal1, strVal2, strVal3, strVal4
    );

    //
    //  Now queue it up on the send queue. The send queue collection is thread safe,
    //  so we don't have to sync in order to add to it. The sending thread will set a
    //  unique id on it on the way out.
    //
    m_colEvSQ.Add(janEvent.pobjOrphan());
}


//
//  Given a driver moniker, it gets the admin proxy for the CQCServer that is hosting
//  that driver (using a helper provided above) and then queries the driver object
//  configuration for that driver.
//
//  So this is the LIVE driver configuration, only available if the driver is actually
//  running. To get the actual configuration, use either bQueryDrvHostCfg to get all
//  of the drivers configured for a specific server, or bQueryDrvCfg to get the config
//  for a single driver. Those go to the master config server to get the actual
//  configured data.
//
tCIDLib::TVoid
TFacCQCKit::QueryLiveDrvCfg(const   TString&            strDriverMoniker
                            ,       TCQCDriverObjCfg&   cqcdcToFill) const
{
    // Call the method above to get us an admin proxy
    tCQCKit::TCQCSrvProxy orbcTmp(orbcCQCSrvAdminProxy(strDriverMoniker));

    //
    //  Get the driver config data for this moniker. If we can't get it,
    //  then we give up.
    //
    if (!orbcTmp->bQueryDriverInfo(strDriverMoniker, cqcdcToFill))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_MonikerNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strDriverMoniker
        );
    }
}



// Porvide access to the bin directory
const TString& TFacCQCKit::strBinDir() const
{
    return m_strBinDir;
}


//
//  Some common boolean string translations. Mostly these are for use by
//  code that deals with event triggers, some of which use these values.
//
const TString& TFacCQCKit::strBoolOffOn(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolOn : m_strBoolOff;
}

const TString& TFacCQCKit::strBoolEnterExit(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolEnter : m_strBoolExit;
}

const TString&
TFacCQCKit::strBoolLockedUnlocked(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolLocked : m_strBoolUnlocked;
}

const TString& TFacCQCKit::strBoolStartEnd(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolStart : m_strBoolEnd;
}

const TString& TFacCQCKit::strBoolVal(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolTrue : m_strBoolFalse;
}

const TString& TFacCQCKit::strBoolYesNo(const tCIDLib::TBoolean bVal) const
{
    return bVal ? m_strBoolYes : m_strBoolNo;
}



//
//  Returns the 'public' name for a device class. This is neither the
//  text format of the enumeration value, or the loadable text versio of
//  the enum value. It's the text format of the enum, without the EDevCl_
//  prefix part. So the standard enum translation stuff won't work in this
//  case.
//
//  We fault the list upon first access.
//
const TString& TFacCQCKit::strDevClassName(const tCQCKit::EDevClasses eClass) const
{
    static TAtomicFlag  atomLoaded;
    static TString      astrValues[tCIDLib::c4EnumOrd(tCQCKit::EDevClasses::Count)];

    if (!atomLoaded)
    {
        TBaseLock lockInit;
        if (!atomLoaded)
        {
            tCQCKit::EDevClasses eClass = tCQCKit::EDevClasses::Min;
            while (eClass < tCQCKit::EDevClasses::Max)
            {
                astrValues[tCIDLib::c4EnumOrd(eClass)] = tCQCKit::strXlatEDevClasses(eClass);
                astrValues[tCIDLib::c4EnumOrd(eClass)].Cut(0, 10);
                eClass++;
            }
            atomLoaded.Set();
        }
    }

    if (eClass >= tCQCKit::EDevClasses::Count)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_BadEnumValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TInteger(tCIDLib::i4EnumOrd(eClass))
            , TString(L"tCQCKit::EDevClasses")
        );
    }
    return astrValues[tCIDLib::c4EnumOrd(eClass)];
}


//
//  A convenience method to help us during the transition from V1 to V2
//  device drivers. It will return either the base name or the V2 adjusted
//  name, so it can be used inline to generate field names.
//
TString
TFacCQCKit::strMakeFldName( const   TString&                strBase
                            , const tCQCKit::EDevClasses    eClass
                            , const tCIDLib::TBoolean       bV2Type)
{
    if (bV2Type)
    {
        const TString& strClass = tCQCKit::strAltXlatEDevClasses(eClass);
        TString strRet(strClass.c4Length() + strBase.c4Length() + 1);

        strRet = strClass;
        strRet.Append(kCIDLib::chPoundSign);
        strRet.Append(strBase);
        return strRet;
    }
    return strBase;
}


//
//  This is a helper method to send an e-mail to one of the defined CQC
//  E-mail accounts. We got to the master config server and get the info
//  for the account, then send off the e-mail.
//
tCIDLib::TVoid
TFacCQCKit::SendEMail(  const   TString&            strAccount
                        , const TString&            strAddress
                        , const TString&            strSubject
                        , const TString&            strMessage
                        , const tCIDLib::TBoolean   bIsHTML
                        , const TCQCSecToken&       sectUser)
{
    //
    //  Let's try to get the account info. We need a master server config
    //  server proxy, and the config server path to the account info.
    //
    TCQCEMailAccount emacctTar;
    {
        // We need an installation server proxy
        tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
        orbcInst->bQueryEmailAccount(strAccount, emacctTar, kCIDLib::True, sectUser);
    }

    // Decide if we are to do a TLS type connnection
    const tCIDLib::TBoolean bSecure(emacctTar.eType() == tCQCKit::EEMailTypes::SMTPS);

    // Set up the connection info based on account stuff
    TSMTPClient smtpTar;
    smtpTar.SetConnInfo
    (
        TString::strEmpty()
        , emacctTar.strServer()
        , emacctTar.ippnMail()
        , bSecure
        , emacctTar.strUserName()
        , emacctTar.strPassword()
    );

    // If HTML, then set the content and encoding for that
    if (bIsHTML)
        smtpTar.SetContentType(L"text/html; encoding='ISO-8859-1';", L"ISO-8859-1");

    //
    //  If the mail type is SMTP or secure SMTP, then set the authentication type
    //  to SMTP.
    //
    if ((emacctTar.eType() == tCQCKit::EEMailTypes::SMTP)
    ||  (emacctTar.eType() == tCQCKit::EEMailTypes::SMTPS))
    {
        smtpTar.eAuthType(tCIDNet::EMailAuthTypes::SMTP);
    }

    smtpTar.AddMsgToQueue(emacctTar.strFromAddr(), strAddress, strSubject, strMessage);
    smtpTar.SendMsgs(8000);
}



//
//  Same as above but the content is an attachment, which gets moved into the
//  target, since seldom fo the builders of e-mails need to keep attachements
//  around.
//
tCIDLib::TVoid
TFacCQCKit::SendEMail(  const   TString&            strAccount
                        , const TString&            strAddress
                        , const TString&            strSubject
                        ,       THeapBuf&&          mbufAttach
                        , const tCIDLib::TCard4     c4Bytes
                        , const TString&            strContType
                        , const TString&            strFileName
                        , const TCQCSecToken&       sectUser)
{
    //
    //  Let's try to get the account info. We need a master server config
    //  server proxy, and the config server path to the account info.
    //
    TCQCEMailAccount emacctTar;
    {
        // We need an installation server proxy
        tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
        orbcInst->bQueryEmailAccount(strAccount, emacctTar, kCIDLib::True, sectUser);
    }

    // OK, we got it, so let's set up an email message object
    TEmailMsg emsgToSend;
    emsgToSend.AddToAddr(strAddress);
    emsgToSend.strTopic(strSubject);
    emsgToSend.strMsg(L"Image file attached");
    emsgToSend.strFrom(emacctTar.strFromAddr());

    // Decide if we are to do a TLS type connnection
    const tCIDLib::TBoolean bSecure
    (
        emacctTar.eType() == tCQCKit::EEMailTypes::SMTPS
    );

    // Set up the connection info based on account stuff
    TSMTPClient smtpTar;
    smtpTar.SetConnInfo
    (
        TString::strEmpty()
        , emacctTar.strServer()
        , emacctTar.ippnMail()
        , bSecure
        , emacctTar.strUserName()
        , emacctTar.strPassword()
    );

    //
    //  If the mail type is SMTP or secure SMTP, then set the authentication type
    //  to SMTP.
    //
    if ((emacctTar.eType() == tCQCKit::EEMailTypes::SMTP)
    ||  (emacctTar.eType() == tCQCKit::EEMailTypes::SMTPS))
    {
        smtpTar.eAuthType(tCIDNet::EMailAuthTypes::SMTP);
    }

    // Add the attachement to the msg
    emsgToSend.AddAttachment
    (
        c4Bytes, tCIDLib::ForceMove(mbufAttach), strContType, strFileName
    );

    smtpTar.AddMsgToQueue(emsgToSend);
    smtpTar.SendMsgs(8000);
}


//
//  Sets the app attach info for the containing app, which is stored info
//  about any external apps that were previously running, so that we can
//  reattach to them if they are still there.
//
tCIDLib::TVoid TFacCQCKit::SetAppAttachInfo(const TString& strToSet)
{
    //
    //  Clear the list, since it is being reset. Normally there should be
    //  nothing in it, since it is only called on program startup to re-
    //  attach to apps stored when last shut down. But better to be safe
    //  than sorry.
    //
    CQCKit_ThisFacility::colAppList.RemoveAll();

    // Put a stream on the caller's string
    TTextStringInStream strmIn(&strToSet);
    TAppItem* paiNew;

    //
    //  We create a new item and keep trying new attach
    //  operations on it till one works. Then we put it
    //  in the list and create another.
    //
    paiNew = new TAppItem;
    TString strCur;
    while (!strmIn.bEndOfStream())
    {
        strmIn >> strCur;
        if (paiNew->bTryAttachTo(strCur))
        {
            CQCKit_ThisFacility::colAppList.Add(paiNew);
            paiNew = new TAppItem;
        }
    }

    // If the last one didn't get used, clean it up
    delete paiNew;
}


// Set the local (client) location info
tCIDLib::TVoid
TFacCQCKit::SetClientLocInfo(const  tCIDLib::TFloat8&   f8Lat
                            , const tCIDLib::TFloat8&   f8Long)
{
    m_f8ClientLat = f8Lat;
    m_f8ClientLong = f8Long;
}


//
//  Enables event receiving and/or sending functions. So it's used used to start up
//  processing of events. The caller must provided us a valid security token, which our
//  send receive threads need in order to talk to the event server and get the encryption
//  key required.
//
tCIDLib::TVoid
TFacCQCKit::StartEventProcessing(const  tCQCKit::EEvProcTypes   eType
                                , const TCQCSecToken&           sectToUse)
{
    TLocker lockSync(&m_mtxSync);

    // First stop processing
    StopEventProcessing();

    // And start back up whichever are enabled
    if (tCIDLib::bAllBitsOn(eType, tCQCKit::EEvProcTypes::Receive))
    {
        //
        //  If we have not published our topic yet, do that. Though not normally an
        //  issue, we need to protect this faulting in.
        //
        {
            if (!m_pstopEvTrigs.bIsPublished())
            {
                m_pstopEvTrigs = TPubSubTopic::pstopCreateTopic
                (
                    kCQCKit::strPubTopic_EvTriggers, TCQCEvent::clsThis()
                );
            }
        }

        // If the receiver thread isn't running, then start it
        if (!m_thrEvReceive.bIsRunning())
        {
            // Make a non-const copy of the security token we can pass into the thread
            TCQCSecToken sectCopy(sectToUse);
            m_thrEvReceive.Start(&sectCopy);
        }
    }

    // Start or stop sending
    if (tCIDLib::bAllBitsOn(eType, tCQCKit::EEvProcTypes::Send))
    {
        if (!m_thrEvSend.bIsRunning())
        {
            // Make a non-const copy of the security token we can pass into the thread
            TCQCSecToken sectCopy(sectToUse);
            m_thrEvSend.Start(&sectCopy);
        }
    }

    // Store the new processing type
    m_eEvProcType = eType;
}



tCIDLib::TVoid TFacCQCKit::StopEventProcessing()
{
    // We need to lock during this, though it may take a little while
    TLocker lockSync(&m_mtxSync);

    if (m_thrEvReceive.bIsRunning())
    {
        try
        {
            m_thrEvReceive.ReqShutdownSync(5000);
            m_thrEvReceive.eWaitForDeath(10000);
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }
        }
    }

    if (m_thrEvSend.bIsRunning())
    {
        try
        {
            m_thrEvSend.ReqShutdownSync(5000);
            m_thrEvSend.eWaitForDeath(10000);
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }
        }
    }
    m_colEvSQ.RemoveAll();

    // Store the new processing type
    m_eEvProcType = tCQCKit::EEvProcTypes::None;
}


//
//  Tests a received security token to see if it is at least of the indicated
//  minimum role. If not, it throws an error. We have one that will create a
//  proxy for the caller and another that accepts one that already exists.
//
//  And we have simpler ones that just let the client do a local test as well,
//  to avoid making a call to the server. The token will stil have to be passed
//  in so there's no security problem with that.
//
tCIDLib::TVoid
TFacCQCKit::TestPrivileges( const   TCQCSecToken&           sectUser
                            , const tCQCKit::EUserRoles     eMinRole)
{
    // Create a proxy and call the other version
    tCQCKit::TSecuritySrvProxy orbcSS = orbcSecuritySrvProxy();
    TestPrivileges(sectUser, eMinRole, orbcSS);
}

tCIDLib::TVoid
TFacCQCKit::TestPrivileges( const   TCQCSecToken&               sectUser
                            , const tCQCKit::EUserRoles         eMinRole
                            ,       tCQCKit::TSecuritySrvProxy& orbcSS)
{
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmp;
    orbcSS->DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmp);
    if (eRole < eMinRole)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );
    }
}

tCIDLib::TVoid
TFacCQCKit::TestPrivileges( const   tCQCKit::EUserRoles eTestRole
                            , const tCQCKit::EUserRoles eMinRole)
{
    if (eTestRole < eMinRole)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }
}

tCIDLib::TVoid
TFacCQCKit::TestPrivileges( const   tCQCKit::EUserRoles eTestRole
                            , const tCQCKit::EUserRoles eMinRole
                            , const TString&            strUserName)
{
    if (eTestRole < eMinRole)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strUserName
        );
    }
}


//
//  Checks that a driver config is of the expected side (client or server)
//  and for the expected C++ facility (library) name.
//
tCIDLib::TVoid
TFacCQCKit::ValidateDrvCfg( const   tCIDLib::ECSSides   eSide
                            , const TCQCDriverCfg&      cqcdcToCheck
                            , const TString&            strLibName)
{
    #if CID_DEBUG_ON
    if ((eSide != tCIDLib::ECSSides::Client) && (eSide != tCIDLib::ECSSides::Server))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConfigSide
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    #endif

    const TString& strWhichLib = (eSide == tCIDLib::ECSSides::Client)
                                 ? cqcdcToCheck.strClientLibName()
                                 : cqcdcToCheck.strServerLibName();

    // Check that the library names match
    if (strWhichLib != strLibName)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConfigLib
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , strWhichLib
            , strLibName
        );
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCKit: Private, static data members
// ---------------------------------------------------------------------------
TStatsCacheItem  TFacCQCKit::m_sciOutTrigsDropped;
TStatsCacheItem  TFacCQCKit::m_sciTrigsReceived;



// ---------------------------------------------------------------------------
//  TFacCQCKit: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is used with the application execution scheme, to allow the user
//  to make the paths to programs and files configured into exec buttons
//  flexible. We'll search the passed string for the supported tokens and
//  replace them with the referenced values.
//
tCIDLib::TBoolean
TFacCQCKit::bReplacePathTokens(TString& strToUpdate, const TString& strUserName)
{
    static tCIDLib::TKVHashSet* pcolList = nullptr;

    // If we haven't created our token list yet, then fault that in
    if (!TAtomic::pFencedGet(&pcolList))
    {
        TBaseLock lockInit;
        if (!TAtomic::pFencedGet(&pcolList))
        {
            tCIDLib::TKVHashSet* pcolNew = new tCIDLib::TKVHashSet
            (
                29, TStringKeyOps(), &TKeyValuePair::strExtractKey
            );
            TJanitor<tCIDLib::TKVHashSet> janNew(pcolNew);

            // Put in the values of the 9 special CQCAppX tokens
            TString strName;
            TString strVal;
            for (tCIDLib::TCard4 c4Index = 1; c4Index <= 9; c4Index++)
            {
                strName = L"%CQCApp";
                strName.AppendFormatted(c4Index);
                strName.Append(L"%");

                // If not there, put in an empty value
                if (!TProcEnvironment::bFind(strName, strVal))
                    strVal.Clear();
                pcolList->objPlace(strName, strVal);
            }

            // And now put in the others
            pcolList->objPlace
            (
                L"%System%"
                , TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::SystemPath)
            );

            pcolList->objPlace
            (
                L"%SystemApps%"
                , TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::ProgramFiles)
            );

            pcolList->objPlace(L"%CurOSUser%", TSysInfo::strUserName());
            pcolList->objPlace(L"%CurCQCUser%", strUserName);
            pcolList->objPlace(L"%HostName%", TSysInfo::strIPHostName());
            pcolList->objPlace
            (
                L"%UserData%"
                , TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData)
            );
            pcolList->objPlace
            (
                L"%MyPictures%"
                , TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::MyPictures)
            );

            pcolList->objPlace
            (
                L"%MyMusic%", TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::MyMusic)
            );

            TAtomic::FencedSet(&pcolList, janNew.pobjOrphan());
        }
    }

    if (!bReplaceTokens(*pcolList, strToUpdate))
    {
        // One or more of them had no values, tell them
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcExec_AppVarNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return kCIDLib::True;
}


//
//  This thread is only started up if the containing application calls the method
//  that enables event processing and asks for event receiving.
//
//  If started, we listen for broadcast events on the appointed event port and
//  publish them via our event trigger pub/sub topic.
//
tCIDLib::EExitCodes TFacCQCKit::eEvReceiveThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    //
    //  The data points to a security token we need to get the event key, so get a
    //  copy before we let the caller call.
    //
    const TCQCSecToken sectToUse = *static_cast<TCQCSecToken*>(pData);

    // Now let our caller go
    thrThis.Sync();

    // Do a sanity check. The topic should be published if we are running
    if (!m_pstopEvTrigs.bIsPublished())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvTrg_NoPubTopic
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
        return tCIDLib::EExitCodes::Normal;
    }

    //
    //  Create a datagram socket for us to listen on. Since it's going
    //  bind the broadcast address, we have to default ctor first, then
    //  open it, enable broadcasts, and then bind.
    //
    //  Note that we also have to enable 'reuse addr' since there will be
    //  multiple apps all listening on this port.
    //
    TServerDatagramSocket sockEvent;
    try
    {
        //
        //  Set up to listen on the port we want, on the broadcast address. For
        //  now we ony support these on V4 clients.
        //
        //  <IPV6> Remind us to come back here...
        //
        const TIPEndPoint ipepEvents
        (
            tCIDSock::ESpecAddrs::Any, tCIDSock::EAddrTypes::IPV4, m_ippnEvents
        );

        sockEvent.Open(tCIDSock::ESockProtos::UDP, ipepEvents.eAddrType());
        sockEvent.bAllowBroadcast(kCIDLib::True);
        sockEvent.bReuseAddr(kCIDLib::True);
        sockEvent.BindListen(ipepEvents);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        //
        //  We can't do it because we can't open the socket, so log the
        //  error, turn off receiving in the event processing type flag
        //  and return.
        //
        if (facCQCKit().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The event listener socket could not be created. "
                  L"Incoming events will not be processed"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        m_eEvProcType = tCIDLib::eClearEnumBits(m_eEvProcType, tCQCKit::EEvProcTypes::Receive);
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  We need one buffer to read the encrypted msgs into. Then we decrypt
    //  from that into another buffer which has an input stream over it that
    //  we use to stream in the event object.
    //
    THeapBuf mbufCypher(4096);
    THeapBuf mbufPlain(4096);
    TBinMBufInStream strmSrc(&mbufPlain);

    //
    //  This could cause a lot of logged msgs if something goes wrong. So we use a
    //  'log limiter' object to avoid that. Set a 60 second time threshold.
    //
    TLogLimiter loglimErrs(60);

    // And loop until asked to stop
    tCIDLib::TBoolean bHaveKey = kCIDLib::False;
    TIPEndPoint ipepFrom;
    TBlowfishEncrypter crypEvents;
    TMD5Hash mhashDupId;
    TMD5Hash mhashEventSysId;
    TMD5Hash mhashSysId;
    while(kCIDLib::True)
    {
        try
        {
            // Watch for shutdown requests
            if (thrThis.bCheckShutdownRequest())
                break;

            //
            //  Wait a while for data to arrive on the socket. If none,
            //  go back to the top and try again. This guy will watch for
            //  shutdown requests so we can do a reasonably long wait.
            //
            if (!sockEvent.bWaitForDataReadyMS(2500))
                continue;

            // Read in the message into our local buffer
            const tCIDLib::TCard4 c4CypherLen = sockEvent.c4ReceiveMBufFrom
            (
                ipepFrom, mbufCypher, 2 * kCIDLib::enctOneSecond
            );

            //
            //  If we haven't gotten the event key yet, then we need to do that. If
            //  we can't, then we can't send the event.
            //
            if (!bHaveKey)
            {
                try
                {
                    // Get a security server proxy and ask it for the key has
                    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

                    // If we get it, set it as the new key
                    TMD5Hash mhashKey;
                    if (orbcSS->bQueryEventKey(mhashKey, mhashSysId, sectToUse))
                    {
                        crypEvents.SetNewKey(TCryptoKey(mhashKey));
                        bHaveKey = kCIDLib::True;
                    }
                }

                catch(TError& errToCatch)
                {
                    if (bLogStatus())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        LogEventObj(errToCatch);
                    }
                }
            }

            if (bHaveKey)
            {
                // Decrypt the data to the plain text buffer
                const tCIDLib::TCard4 c4PlainLen = crypEvents.c4Decrypt(mbufCypher, mbufPlain, c4CypherLen);

                // Set up the input stream to use the plain length as its end of stream
                strmSrc.Reset();
                strmSrc.SetEndIndex(c4PlainLen);

                // It should start wtih a start object marker
                strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

                //
                //  Stream in the dup and system hashes and check them. If we don't already have
                //  this one and it's for our system id, let's take it.
                //
                strmSrc >> mhashDupId >> mhashEventSysId;
                if (!m_colEvList.bHasElement(mhashDupId) && (mhashSysId == mhashEventSysId))
                {
                    // And there should be a frame marker before the rest
                    strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);

                    // That worked so create a new event object and read it in
                    TCQCEvent* pcevNew = new TCQCEvent();
                    TJanitor<TCQCEvent> janEvent(pcevNew);
                    strmSrc >> *pcevNew;

                    //
                    //  Add it to the dup list. If the list is full, then we need to remove
                    //  the oldest one first.
                    //
                    if (m_colEvLIFO.bIsFull(CQCKit_ThisFacility::c4EvInQSize))
                    {
                        //
                        //  Get a ref to the bottom-most one in the LIFO collection.
                        //  Use that to remove it from the regular hash list, then
                        //  discard it from the LIFO.
                        //
                        const TMD5Hash& mhashOldest = m_colEvLIFO.objPeekAtBottom();
                        m_colEvList.bRemove(mhashOldest);
                        m_colEvLIFO.DiscardBottom();
                    }

                    // Now add the new dup hash to the two dup rejection lists
                    m_colEvLIFO.objAddAtTop(mhashDupId);
                    m_colEvList.objAdd(mhashDupId);

                    // Bump the triggers received stat
                    TStatsCache::c8IncCounter(m_sciTrigsReceived);

                    // And finally we can orphan the trigger out and publish it
                    m_pstopEvTrigs.Publish(janEvent.pobjOrphan());
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Exception in the event receiver thread"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unknown exception in the event receiver thread"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This thread is only started up if the containing application calls the
//  method that starts up event processing and asks for event sending.
//  If started, we listen for events to show up in our send queue, which
//  we grab off and send.
//
//  Each event is given a unique stamp and is sent three times, with a
//  small pause in between, so as to ensure that everyone interested sees
//  them. Since they are uniquely stamped, the receivers can reject the
//  duplicates if it gets more than one. Because they are just broadcast,
//  this helps insure that they are seen.
//
tCIDLib::EExitCodes
TFacCQCKit::eEvSendThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    //
    //  The data points to a security token we need to get the event key, so get a
    //  copy before we let the caller call.
    //
    const TCQCSecToken sectToUse = *static_cast<TCQCSecToken*>(pData);

    // Now let our caller go
    thrThis.Sync();

    //
    //  Set up a local network broadcast address for the port we got. For
    //  now we only support this for IPV4 clients.
    //
    //  <IPV6> Remind us to come back to this...
    //
    const TIPEndPoint ipepEvents
    (
        tCIDSock::ESpecAddrs::Broadcast, tCIDSock::EAddrTypes::IPV4, m_ippnEvents
    );

    //
    //  Create a datagram socket for us to send on. We need to enable
    //  broadcasts on this guy.
    //
    TClientDatagramSocket sockEvent;
    try
    {
        sockEvent.Open(tCIDSock::ESockProtos::UDP, ipepEvents.eAddrType());
        sockEvent.bAllowBroadcast(kCIDLib::True);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        //
        //  We can't do it because we can't open the socket, so log the error,
        //  turn off sending in the event processing type flag and return.
        //
        if (facCQCKit().bLogFailures())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The event sender socket could not be created. Events will not be sent"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        m_eEvProcType = tCIDLib::eClearEnumBits
        (
            m_eEvProcType, tCQCKit::EEvProcTypes::Send
        );
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  We need a binary buffer stream to flatten objects for sending, and another
    //  that we encrypt that data into
    //
    THeapBuf mbufCypher(4096);
    THeapBuf mbufPlain(4096);
    TBinMBufOutStream strmOut(&mbufPlain);

    // And loop until asked to stop
    TLogLimiter loglimErr(60);
    TBlowfishEncrypter crypEvents;
    tCIDLib::TBoolean bHaveKey = kCIDLib::False;
    TMD5Hash mhashSysId;
    TMD5Hash mhashDupId;
    while (!thrThis.bCheckShutdownRequest())
    {
        try
        {
            // Block for a bit waiting for a new event to become available
            TCQCEvent* pevCur = m_colEvSQ.pobjGetNext(1000, kCIDLib::False);
            TJanitor<TCQCEvent> janEvent(pevCur);

            //
            //  If we haven't gotten the event key yet and need to send, then we
            //  need to do that. If we can't, then we can't send the event.
            //
            if (pevCur && !bHaveKey)
            {
                try
                {
                    // Ask the security server for the event key
                    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

                    // Ask it for the key, create a hash from it, and
                    TMD5Hash mhashKey;
                    if (orbcSS->bQueryEventKey(mhashKey, mhashSysId, sectToUse))
                    {
                        crypEvents.SetNewKey(TCryptoKey(mhashKey));
                        bHaveKey = kCIDLib::True;
                    }
                }

                catch(TError& errToCatch)
                {
                    if (bLogStatus())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        LogEventObj(errToCatch);
                    }
                }
            }

            // If we got an event and we have our key, we can send the event
            if (bHaveKey && pevCur)
            {
                try
                {
                    // Get the next unique id on it
                    GetNextEventId(mhashDupId);

                    //
                    //  Flatten out the dup id, the system id, and then the event content,
                    //  then we encrypt it. We use some stream markers for extra safety.
                    //
                    strmOut.Reset();
                    strmOut << tCIDLib::EStreamMarkers::StartObject
                            << mhashDupId << mhashSysId
                            << tCIDLib::EStreamMarkers::Frame << *pevCur
                            << kCIDLib::FlushIt;

                    const tCIDLib::TCard4 c4CypherLen = crypEvents.c4Encrypt
                    (
                        mbufPlain, mbufCypher, strmOut.c4CurPos()
                    );

                    // And broadcast the encrypted buffer
                    sockEvent.c4SendTo(ipepEvents, mbufCypher, c4CypherLen);

                    //
                    //  Do a short pause, and send it again. This helps insure
                    //  that they get seen, since any given packet might not
                    //  make it. The receiving side handles the rejection of
                    //  duplicates so this isn't a problem.
                    //
                    TThread::Sleep(5);
                    sockEvent.c4SendTo(ipepEvents, mbufCypher, c4CypherLen);
                }

                catch(TError& errToCatch)
                {
                    loglimErr.bLogErr(errToCatch, CID_FILE, CID_LINE);
                }

                catch(...)
                {
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (loglimErr.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Exception in the event out spooler thread"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unknown exception in the event out spooler thread"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This is called by the public eStdTokenReplace() method. This operation
//  needs to be recursive, so that guy just calls us with an initial depth
//  of zero. We recursively expand the pattern. See the public method for
//  comments on the token scheme.
//
//  Note that our recursion doesn't handle nested tokens, i.e. something like
//  $(%(LVar:DevName), %(LVar:FldName)). We are recursing to handle a token
//  that expands to a reference to a token and so forth.
//
//  To avoid infinite recursion (variable 1 resolves to a reference to
//  variable 2, and variable 2 resolves to a reference to variable 1), we
//  will error out if we end up more than 8 recursive calls in depth.
//
tCQCKit::ECmdPrepRes
TFacCQCKit::eStdTokenRepHelper( const   TString&                strPattern
                                , const TCQCCmdRTVSrc* const    pmcrtvToUse
                                , const TStdVarsTar* const      pctarGVars
                                , const TStdVarsTar* const      pctarLVars
                                ,       TString&                strResult
                                , const tCQCKit::ETokRepFlags   eFlags
                                , const tCIDLib::TCard4         c4Depth)
{
    enum EStates
    {
        EState_WaitToken
        , EState_WaitOpen
        , EState_WaitName
        , EState_WaitTPatStart
        , EState_WaitTPat
        , EState_WaitTPatEnd
        , EState_WaitSepStart
        , EState_WaitSep
        , EState_WaitSepEnd
        , EState_WaitClose
        , EState_GotToken
    };

    // Assume unchanged unless proven otherwise
    tCQCKit::ECmdPrepRes eRes = tCQCKit::ECmdPrepRes::Unchanged;

    // Clear the output so we can start appending
    strResult.Clear();

    // Get some short cuts for the flags
    const tCIDLib::TBoolean bTestOnly(tCIDLib::bAllBitsOn(eFlags, tCQCKit::ETokRepFlags::TestOnly));
    const tCIDLib::TBoolean bDoEscapes(!tCIDLib::bAllBitsOn(eFlags, tCQCKit::ETokRepFlags::NoEscape));
    const tCIDLib::TBoolean bGVarsOnly(tCIDLib::bAllBitsOn(eFlags, tCQCKit::ETokRepFlags::GVarsOnly));

    //
    //  Since most will not have any tokens, we can optimize by doing
    //  a tight initial loop where we just copy characters until we see
    //  a dollar/percent or escape char. If we don't, then we just return
    //  immediately with unchanged. Then we won't have to construct all
    //  the temp strings, or copy any chars.
    //
    tCIDLib::TCard4     c4Index = 0;
    tCIDLib::TCard4 c4PatLen = strPattern.c4Length();
    while (c4Index < c4PatLen)
    {
        //
        //  Don't increment till the end, so that, if we break out, we
        //  will pick up on the character that broke us out.
        //
        const tCIDLib::TCh chCur = strPattern[c4Index];
        if ((chCur == L'%') || (chCur == L'$'))
            break;

        // Only do this one if we are doing escapes
        if (bDoEscapes && (chCur == L'\\'))
            break;

        c4Index++;
    }

    // If we got to the end, then just return unchanged
    if (c4Index == c4PatLen)
        return eRes;

    //
    //  Copy over the chars we got done in the loop above, and then we can
    //  fall through and do the hard work.
    //
    if (c4Index)
        strResult.CopyInSubStr(strPattern, 0, c4Index);

    //
    //  Now we do a little state machine to parse the input. We basically
    //  just copy over characters from the source to the target until we
    //  hit a token, then we get the value of that token and append it to
    //  the target.
    //
    tCIDLib::TBoolean   bFldToken = kCIDLib::False;
    EStates             eCurState = EState_WaitToken;
    TString             strFld;
    TString             strMon;
    TString             strName;
    TString             strRepVal;
    TString             strSep;
    TString             strTPat;
    TString             strVal;
    while (c4Index < c4PatLen)
    {
        // Get the current character out of the pattern
        const tCIDLib::TCh chCur = strPattern[c4Index++];

        //
        //  If we get a backslash, and it's not the last character in the
        //  input pattern, then it could be an escape.
        //
        tCIDLib::TBoolean bEscaped = kCIDLib::False;
        if ((chCur == L'\\') && (c4Index < c4PatLen))
        {
            if (eCurState == EState_WaitOpen)
            {
                //
                //  We saw a percent or dollar, but now we've seen a slash. So
                //  we want to pay back that character, since it can't be a
                //  token. We put us back to wait token, and then fall through
                //  for regular processing.
                //
                if (bFldToken)
                    strResult.Append(kCIDLib::chDollarSign);
                else
                    strResult.Append(kCIDLib::chPercentSign);
                eCurState = EState_WaitToken;
            }

            //
            //  Get the next char. According to where we are in the
            //  pattern, different things are escaped.
            //
            const tCIDLib::TCh chNext = strPattern[c4Index];
            if (eCurState == EState_WaitTPat)
            {
                if ((chNext == L'"') || (chNext == L'\\'))
                {
                    strTPat.Append(chNext);
                    c4Index++;
                    bEscaped = kCIDLib::True;
                }
            }
             else if (eCurState == EState_WaitSep)
            {
                if ((chNext == L'"') || (chNext == L'\\'))
                {
                    strTPat.Append(chNext);
                    c4Index++;
                    bEscaped = kCIDLib::True;
                }
            }
             else if (eCurState == EState_WaitToken)
            {
                //
                //  Outside of any token, we allow escaping of percent
                //  and dollar signs, as well as quotes and escape
                //  chars.
                //
                if ((chNext == L'%') || (chNext == L'$')
                ||  (chNext == L'"') || (chNext == L'\\'))
                {
                    strResult.Append(chNext);
                    c4Index++;
                    bEscaped = kCIDLib::True;
                }
            }

            //
            //  If we escaped, then just go back and start at the top. Else,
            //  fall through with the char we have. And set the return value
            //  to changed, since we have changed it.
            //
            if (bEscaped)
            {
                eRes = tCQCKit::ECmdPrepRes::Changed;
                continue;
            }
        }

        //
        //  Process the character based on our state machine state. We don't
        //  have to deal with escapes here since they are all handled above.
        //
        switch(eCurState)
        {
            case EState_WaitToken :
            {
                if (chCur == L'$')
                {
                    eCurState = EState_WaitOpen;
                    bFldToken = kCIDLib::True;
                    eRes = tCQCKit::ECmdPrepRes::Changed;
                }
                 else if (chCur == L'%')
                {
                    eCurState = EState_WaitOpen;
                    bFldToken = kCIDLib::False;
                    eRes = tCQCKit::ECmdPrepRes::Changed;
                }
                 else
                {
                    // Nothing special, so just append to result
                    strResult.Append(chCur);
                }
                break;
            }

            case EState_WaitOpen :
            {
                if (chCur == kCIDLib::chOpenParen)
                {
                    // Ok, let's start waiting for the name of a new token
                    strName.Clear();
                    strTPat.Clear();
                    strSep.Clear();
                    eCurState = EState_WaitName;
                }
                 else
                {
                    //
                    //  It was just a raw $ or %, so 'pay back' the owed
                    //  original character and then append the current char,
                    //  then go back to base state.
                    //
                    if (bFldToken)
                        strResult.Append(kCIDLib::chDollarSign);
                    else
                        strResult.Append(kCIDLib::chPercentSign);
                    strResult.Append(chCur);
                    eCurState = EState_WaitToken;
                }
                break;
            }

            case EState_WaitName :
            {
                //
                //  Store name chars till we hit the close paren, or a comma
                //  before the token pattern.
                //
                if (chCur == kCIDLib::chCloseParen)
                    eCurState = EState_GotToken;
                else if (chCur == kCIDLib::chComma)
                    eCurState = EState_WaitTPatStart;
                else
                    strName.Append(chCur);
                break;
            }

            case EState_WaitTPatStart :
            {
                //
                //  Eat whitespace till we hit the start quote. If we hit a
                //  close paren, then we have a token.
                //
                if (chCur == kCIDLib::chQuotation)
                    eCurState = EState_WaitTPat;
                else if (chCur == kCIDLib::chCloseParen)
                    eCurState = EState_GotToken;
                else if (!TRawStr::bIsSpace(chCur))
                {
                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_ExpectWS2
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }
                break;
            }

            case EState_WaitTPat :
            {
                //
                //  Store token pattern chars till we hit the closing
                //  quote.
                //
                if (chCur == kCIDLib::chQuotation)
                    eCurState = EState_WaitTPatEnd;
                else
                    strTPat.Append(chCur);
                break;
            }

            case EState_WaitTPatEnd :
            {
                //
                //  We can see whitespace or close paren. If a comma, move
                //  to the separator.
                //
                if (chCur == kCIDLib::chComma)
                    eCurState = EState_WaitSepStart;
                else if (chCur == kCIDLib::chCloseParen)
                    eCurState = EState_GotToken;
                else if (!TRawStr::bIsSpace(chCur))
                {
                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_ExpectWS2
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }
                break;
            }

            case EState_WaitSepStart :
            {
                //
                //  Eat whitespace till we hit the start quote. If we hit a
                //  close paren, then we have a token.
                //
                if (chCur == kCIDLib::chQuotation)
                    eCurState = EState_WaitSep;
                else if (chCur == kCIDLib::chCloseParen)
                    eCurState = EState_GotToken;
                else if (!TRawStr::bIsSpace(chCur))
                {
                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_ExpectWS2
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }
                break;
            }

            case EState_WaitSep :
            {
                //
                //  Store separator chars until we hit the close quote. When
                //  we hit the close quote, we are now looking for the close
                //  paren.
                //
                if (chCur == kCIDLib::chQuotation)
                    eCurState = EState_WaitClose;
                else
                    strSep.Append(chCur);
                break;
            }

            case EState_WaitClose :
            {
                if (chCur == kCIDLib::chCloseParen)
                    eCurState = EState_GotToken;
                else if (!TRawStr::bIsSpace(chCur))
                {
                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_ExpectWS1
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }
                break;
            }
        };

        // If no token yet, then go back to the top and keep going.
        if (eCurState != EState_GotToken)
            continue;

        // Go back to initial state now now matter what
        eCurState = EState_WaitToken;

        //
        //  If we get here, then we are going to either change the value
        //  or get an error.
        //
        eRes = tCQCKit::ECmdPrepRes::Changed;

        //
        //  Strip whitspace from the name part, since it is not a quoted value
        //  and therefore could have had leading/trailing whitespace.
        //
        strName.StripWhitespace();

        if (bFldToken)
        {
            // If only allowing global vars, then we failed
            if (bGVarsOnly)
            {
                strResult.LoadFromMsg(kKitErrs::errcTokR_OnlyGVars, facCQCKit());
                return tCQCKit::ECmdPrepRes::Failed;
            }

            // Break the remaining token into moniker/field names
            if (!bParseFldName(strName, strMon, strFld))
            {
                strResult.LoadFromMsg
                (
                    kKitErrs::errcTokR_BadField
                    , facCQCKit()
                    , TCardinal(c4Index)
                );
                return tCQCKit::ECmdPrepRes::Failed;
            }

            if (!bTestOnly)
            {
                // Not just testing so try to get the value of the field
                try
                {
                    // Get a proxy for the CQCServer that owns this moniker
                    tCQCKit::TCQCSrvProxy  orbcSrv
                    (
                        orbcCQCSrvAdminProxy(strMon, 2500, kCIDLib::True)
                    );

                    // Try to get the value for this field
                    tCIDLib::TCard4 c4SerNum = 0;
                    tCQCKit::EFldTypes eType;
                    if (!orbcSrv->bReadFieldByName(c4SerNum, strMon, strFld, strRepVal, eType))
                    {
                        strResult.LoadFromMsg
                        (
                            kKitErrs::errcTokR_GetFldVal
                            , facCQCKit()
                            , strMon
                            , strFld
                        );
                        return tCQCKit::ECmdPrepRes::Failed;
                    }
                }

                catch(const TError& errToCatch)
                {
                    if (bShouldLog(errToCatch))
                        LogEventObj(errToCatch);

                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_Except
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }

                catch(...)
                {
                    strResult.LoadFromMsg
                    (
                        kKitErrs::errcTokR_Except
                        , facCQCKit()
                        , TCardinal(c4Index)
                    );
                    return tCQCKit::ECmdPrepRes::Failed;
                }
            }
             else
            {
                // Make the replacement value the token itself
                strRepVal = L"$(";
                strRepVal.Append(strName);
                strRepVal.Append(L")");
            }
        }
         else
        {
            //
            //  See if we have this token. It's got to be a variable, or
            //  or some runtime value.
            //
            if (strName.bStartsWith(kCQCKit::strActVarPref_GVar))
            {
                if (!bTestOnly)
                {
                    if (!pctarGVars || !pctarGVars->bVarValue(strName, strRepVal))
                    {
                        strResult.LoadFromMsg
                        (
                            kKitErrs::errcTokR_VarNotFound, facCQCKit(), strName
                        );
                        return tCQCKit::ECmdPrepRes::Failed;
                    }
                }
            }
             else if (strName.bStartsWith(kCQCKit::strActVarPref_LVar))
            {
                // If only allowing global vars, then we failed
                if (bGVarsOnly)
                {
                    strResult.LoadFromMsg(kKitErrs::errcTokR_OnlyGVars, facCQCKit());
                    return tCQCKit::ECmdPrepRes::Failed;
                }

                if (!bTestOnly)
                {
                    if (!pctarLVars || !pctarLVars->bVarValue(strName, strRepVal))
                    {
                        strResult.LoadFromMsg
                        (
                            kKitErrs::errcTokR_VarNotFound, facCQCKit(), strName
                        );
                        return tCQCKit::ECmdPrepRes::Failed;
                    }
                }
            }
             else
            {
                // If only allowing global vars, then we failed
                if (bGVarsOnly)
                {
                    strResult.LoadFromMsg(kKitErrs::errcTokR_OnlyGVars, facCQCKit());
                    return tCQCKit::ECmdPrepRes::Failed;
                }

                if (!bTestOnly)
                {
                    if (!pmcrtvToUse || !pmcrtvToUse->bRTValue(strName, strRepVal))
                    {
                        strResult.LoadFromMsg
                        (
                            kKitErrs::errcTokR_RTVNotFound
                            , facCQCKit()
                            , strName
                        );
                        return tCQCKit::ECmdPrepRes::Failed;
                    }
                }
            }
        }

        //
        //  If just testing, we jump back to the top again, since we don't
        //  have any replacement value and don't want to try to expand
        //  anything.
        //
        if (bTestOnly)
            continue;

        //
        //  Ok, if we got here, then we have a replacement value in strRepVal,
        //  and we have the token pattern and separator info. So we need to
        //  process the value and append the results onto the result string.
        //
        //  If we didn't get a token pattern or separator, then default them.
        //  If the pattern isn't empty, it must have one and only one '%1'
        //  replacement token in it, or it must be the '%*' pattern which
        //  indicates a literal taking of the replacement value without
        //  processing. Or ^1, which is the same, with no recursion on the
        //  replacement value.
        //
        //  We'll use strName as a temp here to expand the token out into.
        //
        tCIDLib::TBoolean bNoRecurse = kCIDLib::False;
        tCIDLib::TCard4 c4TPatOfs = 0;
        if (strTPat.bIsEmpty())
            strTPat = L"%1";
        if (strSep.bIsEmpty())
            strSep = L" ";

        strName.Clear();
        if (strTPat.bFirstOccurrence(L"%1", c4TPatOfs))
        {
            FormatTokenVals(strName, strRepVal, strTPat, c4TPatOfs, strSep);
        }
         else if (strTPat.bFirstOccurrence(L"%*", c4TPatOfs))
        {
            // Cut out the pattern and put the replacement value in literally
            strName = strTPat;
            strName.Cut(c4TPatOfs, 2);
            strName.Insert(strRepVal, c4TPatOfs);
        }
         else if (strTPat.bFirstOccurrence(L"^1", c4TPatOfs))
        {
            strName = strTPat;
            strName.Cut(c4TPatOfs, 2);
            strName.Insert(strRepVal, c4TPatOfs);
            bNoRecurse = kCIDLib::True;
        }
         else
        {
            strResult.LoadFromMsg(kKitErrs::errcTokR_BadTPattern, facCQCKit());
            return tCQCKit::ECmdPrepRes::Failed;
        }

        //
        //  Now, we have to deal with possible token recursion. So let's
        //  call ourself on on the expanded token, and let it fill in the
        //  rep val string with any next level expansion.
        //
        //  If we've already hit a depth of 8, then assume we are in a
        //  recursive expansion and fail it.
        //
        //  If the replacement token was ^1, then we don't recurse and just
        //  take the value as is.
        //
        if (c4Depth >= 8)
        {
            strResult.LoadFromMsg(kKitErrs::errcTokR_RecursiveExp, facCQCKit());
            return tCQCKit::ECmdPrepRes::Failed;
        }

        //
        //  If recursion wasn't suppressed on this one, then recurse. If
        //  it was, then we jsut fall through with an unchanged status.
        //
        tCQCKit::ECmdPrepRes eNestedRes = tCQCKit::ECmdPrepRes::Unchanged;
        if (!bNoRecurse)
        {
            eNestedRes = eStdTokenRepHelper
            (
                strName
                , pmcrtvToUse
                , pctarGVars
                , pctarLVars
                , strRepVal
                , eFlags
                , c4Depth + 1
            );
        }

        //
        //  If it was expanded further, then append the rep val to our result,
        //  else the name string with the original content. If it failed, then
        //  we failed.
        //
        if (eNestedRes == tCQCKit::ECmdPrepRes::Failed)
            return tCQCKit::ECmdPrepRes::Failed;

        if (eNestedRes == tCQCKit::ECmdPrepRes::Changed)
            strResult.Append(strRepVal);
        else
            strResult.Append(strName);
    }

    //
    //  If we aren't on WaitToken state, then there was an unterminated
    //  token. But, if we are in WaitOpen state, it was just a trailing $ or
    //  % char, so just put append the owed character.
    //
    if (eCurState == EState_WaitOpen)
    {
        if (bFldToken)
            strResult.Append(kCIDLib::chDollarSign);
        else
            strResult.Append(kCIDLib::chPercentSign);
    }
     else if (eCurState != EState_WaitToken)
    {
        strResult.LoadFromMsg(kKitErrs::errcTokR_Unterminated, facCQCKit());
        return tCQCKit::ECmdPrepRes::Failed;
    }
    return eRes;
}


//
//  A helper used by eStdTokenHelper, so a helper's helper I guess. It
//  handles token expansion based on a token expansion pattern and separator
//  character. So it handles single or multi-line expansion values.
//
tCIDLib::TVoid
TFacCQCKit::FormatTokenVals(        TString&        strOut
                            , const TString&        strRepVal
                            , const TString&        strTPat
                            , const tCIDLib::TCard4 c4TPatOfs
                            , const TString&        strSep)
{
    //
    //  Put an input stream over the rep value, and we'll just read lines
    //  till we hit the end of stream. For each line, we'll process it based
    //  on the passed pattern and separator.
    //
    tCIDLib::TCard4 c4Line = 0;
    TTextStringInStream strmSrc(&strRepVal);
    TString strLine;
    TString strFmt;
    while (!strmSrc.bEndOfStream())
    {
        //
        //  If not the first line, add a separator before the next. Note this
        //  means we don't put on a trailing one! It is just a separator, not
        //  a terminator.
        //
        if (c4Line)
            strOut.Append(strSep);
        strmSrc >> strLine;

        //
        //  Put the pattern in a temp, cut the replacement token out, and
        //  insert the current line into that slot, then append the result
        //  onto the output string.
        //
        strFmt = strTPat;
        strFmt.Cut(c4TPatOfs, 2);
        strFmt.Insert(strLine, c4TPatOfs);
        strOut.Append(strFmt);

        c4Line++;
    }
}


//
//  A small helper to save away downloaded driver files, that we pull down from the
//  installation server, or possibly other places eventually.
//
tCIDLib::TVoid TFacCQCKit::SaveFile(const   TString&        strFileName
                                    , const TMemBuf&        mbufData
                                    , const tCIDLib::TCard4 c4Size)
{
    //
    //  If the file already exists, then reject this action. In theory, this
    //  should never happen since we already checked above whether we have
    //  this driver loaded. But hey, this is software.
    //
    if (TFileSys::bExists(strFileName, tCIDLib::EDirSearchFlags::AllFiles))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcInst_FileExists
            , strFileName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    // Seems ok, so lets do it
    TBinaryFile flOut(strFileName);
    flOut.Open
    (
        tCIDLib::EAccessModes::Write
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::AllOwnerAccess
        , tCIDLib::EFileFlags::SequentialScan
    );

    if (flOut.c4WriteBuffer(mbufData, c4Size) != c4Size)
    {
        // Try to clean up what we did write
        flOut.Close();
        TFileSys::DeleteFile(strFileName);

        // And throw back an error
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcInst_FileWrite
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strFileName
        );
    }
}
