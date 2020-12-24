//
// FILE NAME: CQCAct_StdCmdTargets.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2005
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
//  This file implements some of the standard command targets that are used almost
//  universally wherever commands are supported. The System target is separate because
//  it's quite large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAct_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TStdFieldCmdTar,TObject)
RTTIDecls(TStdMacroCmdTar,TObject)
RTTIDecls(TTrigEvCmdTarget,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TStdFieldCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TStdFieldCmdTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TStdFieldCmdTar::TStdFieldCmdTar() :

    MCQCCmdTarIntf
    (
        kCQCKit::strCTarId_Fields
        , facCQCAct().strMsg(kActMsgs::midCmdTar_Devices)
        , kCQCKit::strCTarId_Fields
        , L"/Reference/Actions/Devices"
    )
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);
}

TStdFieldCmdTar::~TStdFieldCmdTar()
{
}


// ---------------------------------------------------------------------------
//  TStdFieldCmdTar: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking
tCIDLib::TBoolean
TStdFieldCmdTar::bValidateParm( const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_GetSemFields) && (c4Index == 2))
    {
        // It should be two variable names, space separated
        TString strVar1 = strValue;
        TString strVar2;
        if (!strVar1.bSplit(strVar2, kCIDLib::chSpace))
        {
            strErrText = facCQCAct().strMsg(kActErrs::errcCmd_BadFldVars, TString(L"GetSemFields"));
            return kCIDLib::False;
        }

        if (!TStdVarsTar::bValidVarName(strVar1, TString::strEmpty(), strErrText)
        ||  !TStdVarsTar::bValidVarName(strVar2, TString::strEmpty(), strErrText))
        {
            return kCIDLib::False;
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_TimedFldChange) && (c4Index == 3))
    {
        // We have a helper to do this
        tCIDLib::TCard4 c4Secs;
        if (!bParseTimedFldChInt(strValue, strErrText, c4Secs))
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TStdFieldCmdTar::CmdTarCleanup()
{
    // A no-op for us
}


tCIDLib::TVoid TStdFieldCmdTar::CmdTarInitialize(const TCQCActEngine&)
{
    // A no-op for us
}


// We override this to process command requests
tCQCKit::ECmdRes
TStdFieldCmdTar::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
                        , const tCIDLib::TCard4
                        , const TString&
                        , const TString&
                        ,       TStdVarsTar&            ctarGlobalVars
                        ,       tCIDLib::TBoolean&      bResult
                        ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CancelTimedFldChange)
    {
        const TString& strTarFld = ccfgToDo.piAt(0).m_strValue;
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strTarFld, strMoniker, strField);
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
        orbcAdmin->CancelTimedWrite(strMoniker, strField, acteTar.sectUser());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ClearDrvStats)
    {
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
        orbcAdmin->ClearDrvStats(strMoniker);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_DriverReady)
    {
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;
        try
        {
            // Do this with a really quick timeout. We only care about what's there now
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker, 50));
            tCIDLib::TCard4         c4ThreadId;
            tCQCKit::EDrvStates     eState;
            tCQCKit::EVerboseLvls   eVerbose;
            orbcAdmin->QueryDriverState(strMoniker, eState, eVerbose, c4ThreadId);
            bResult = (eState == tCQCKit::EDrvStates::Connected);
        }

        catch(TError& errToCatch)
        {
            if (facCQCAct().bLogWarnings() && !errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // If not accessible, assume no
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FieldRead)
    {
        const TString& strFld = ccfgToDo.piAt(0).m_strValue;
        facCQCKit().ParseFldName(strFld, m_strTmp1, m_strTmp2);

        try
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(m_strTmp1));
            tCIDLib::TCard4 c4SerNum = 0;
            tCQCKit::EFldTypes eType;
            orbcAdmin->bReadFieldByName(c4SerNum, m_strTmp1, m_strTmp2, m_strTmp3, eType);
            bResult = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            //
            //  If we were told to fail on error, then throw. Else just return
            //  false.
            //
            if (facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue))
                throw;
            bResult = kCIDLib::False;
            return tCQCKit::ECmdRes::Ok;
        }

        // Find the target variable and fill it in
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(m_strTmp3) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FieldRead2)
    {
        const TString& strDrv = ccfgToDo.piAt(0).m_strValue;
        const TString& strFld = ccfgToDo.piAt(1).m_strValue;

        try
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strDrv));
            tCIDLib::TCard4 c4SerNum = 0;
            tCQCKit::EFldTypes eType;
            orbcAdmin->bReadFieldByName(c4SerNum, strDrv, strFld, m_strTmp1, eType);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            //
            //  If we were told to fail on error, then throw. Else just return
            //  false.
            //
            if (facCQCKit().bCheckBoolVal(ccfgToDo.piAt(3).m_strValue))
                throw;
            bResult = kCIDLib::False;
            return tCQCKit::ECmdRes::Ok;
        }

        // Find the target variable and fill it in
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());

        //
        //  Doesn't matter if we set it for the non-conditional scenario,
        //  so just set it either way.
        //
        bResult = kCIDLib::True;
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FieldWrite)
    {
        const TString& strFld = ccfgToDo.piAt(0).m_strValue;
        const TString& strVal = ccfgToDo.piAt(1).m_strValue;

        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strFld, strMoniker, strField);

        try
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
            orbcAdmin->WriteFieldByName
            (
                strMoniker
                , strField
                , strVal
                , acteTar.sectUser()
                , tCQCKit::EDrvCmdWaits::DontCare
            );
            bResult = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCAct().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_FldWrite
                , strVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strFld
            );

            //
            //  If we were told to fail on error, then throw. Else just return
            //  false.
            //
            if (facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue))
                throw;
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FieldWrite2)
    {
        const TString& strDrv = ccfgToDo.piAt(0).m_strValue;
        const TString& strFld = ccfgToDo.piAt(1).m_strValue;
        const TString& strVal = ccfgToDo.piAt(2).m_strValue;

        try
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strDrv));
            orbcAdmin->WriteFieldByName
            (
                strDrv, strFld, strVal, acteTar.sectUser(), tCQCKit::EDrvCmdWaits::DontCare
            );
            bResult = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            TString strMonFld(strDrv);
            strMonFld.Append(kCIDLib::chPeriod);
            strMonFld.Append(strFld);
            facCQCAct().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_FldWrite
                , strVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , strMonFld
            );

            //
            //  If we were told to fail on error, then throw. Else just return
            //  false.
            //
            if (facCQCKit().bCheckBoolVal(ccfgToDo.piAt(3).m_strValue))
                throw;
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FindDrivers)
    {
        //
        //  Set up an appropriate bit set with the desired type set. If we
        //  didn't get a value, leave the set empty.
        //
        tCQCKit::TDevCatSet btsCats;
        if (!ccfgToDo.piAt(2).m_strValue.bIsEmpty())
        {
            const tCQCKit::EDevCats eDevCat = tCQCKit::eXlatEDevCats
            (
                ccfgToDo.piAt(2).m_strValue
            );
            if (eDevCat == tCQCKit::EDevCats::Count)
                ThrowBadParmVal(ccfgToDo, 0);
            btsCats.bSetBitState(eDevCat, kCIDLib::True);
        }

        // And do the query, passing in the filter values
        tCIDLib::TStrList colMakes;
        tCIDLib::TStrList colModels;
        tCIDLib::TStrList colMonikers;
        TString strVal;
        const tCIDLib::TBoolean bRes = facCQCKit().bFindDrivers
        (
            TString::strEmpty()
            , ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
            , btsCats
            , colMonikers
            , colMakes
            , colModels
        );

        // If we got any, set up the list
        if (bRes)
        {
            const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                TStringTokenizer::BuildQuotedCommaList(colMonikers[c4Index], strVal);
        }

        // And set up the caller's variable
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        if (varTar.bSetValue(strVal) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FindFields)
    {
        //
        //  This command will get all of the fields of a given driver that have
        //  particular access and whose names match a provided regular expression.
        //
        const TString& strMon = ccfgToDo.piAt(0).m_strValue;
        const TString& strRegEx = ccfgToDo.piAt(1).m_strValue;

        // Translate the requested access enum
        tCQCKit::EReqAccess eAccess = tCQCKit::eXlatEReqAccess(ccfgToDo.piAt(3).m_strValue);

        // Call a helper to do this
        TString strFldNames, strNames;
        bResult = bFindFields(strMon, strFldNames, strNames, strRegEx, eAccess);

        if (bResult)
        {
            // Parse out the two variable names
            TString strFldsVar = ccfgToDo.piAt(2).m_strValue;
            TString strNamesVar;
            if (!strFldsVar.bSplit(strNamesVar, kCIDLib::chSpace))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_BadFldVars
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TString(L"FindFields")
                );
            }

            // Set up the return variables
            TCQCActVar& varFlds = TStdVarsTar::varFind
            (
                strFldsVar
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            TCQCActVar& varNames = TStdVarsTar::varFind
            (
                strNamesVar
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            if (varFlds.bSetValue(strFldNames) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varFlds.strName(), varFlds.strValue());

            if (varNames.bSetValue(strNames) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varNames.strName(), varNames.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetSemFields)
    {
        //
        //  This command will get all of the fields of a given driver that have a
        //  particular semantic type. It will return two quoted comma lists. One
        //  will have the field names (in moniker.field format) and the other will
        //  have the basic part of the field names. For V1 drivers, there might not
        //  be much distinction other than that 2nd list doesn't have the 'moniker.'
        //  part. For V2 drivers we have to remove the V2 stuff.
        //
        //  These two lists are particularly useful for the DynOverLoad command of
        //  overlays in the IV system, but might have other uses.
        //
        const TString& strMon = ccfgToDo.piAt(0).m_strValue;

        // Make sure the semantic type is correct
        const tCQCKit::EFldSTypes eSemType = tCQCKit::eAltXlatEFldSTypes(ccfgToDo.piAt(1).m_strValue);
        if (eSemType == tCQCKit::EFldSTypes::Count)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownSemType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , ccfgToDo.piAt(1).m_strValue
            );
        }

        // Translate the requested access enum
        tCQCKit::EReqAccess eAccess = tCQCKit::eXlatEReqAccess(ccfgToDo.piAt(3).m_strValue);

        // Call a helper to do this
        TString strFldNames, strNames;
        bResult = bQuerySemFields(strMon, eSemType, strFldNames, strNames, eAccess);

        if (bResult)
        {
            // Parse out the two variable names
            TString strFldsVar = ccfgToDo.piAt(2).m_strValue;
            TString strNamesVar;
            if (!strFldsVar.bSplit(strNamesVar, kCIDLib::chSpace))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_BadFldVars
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TString(L"GetSemFields")
                );
            }

            // Set up the return variables
            TCQCActVar& varFlds = TStdVarsTar::varFind
            (
                strFldsVar
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            TCQCActVar& varNames = TStdVarsTar::varFind
            (
                strNamesVar
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            if (varFlds.bSetValue(strFldNames) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varFlds.strName(), varFlds.strValue());

            if (varNames.bSetValue(strNames) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varNames.strName(), varNames.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetNumericRange)
    {
        // Find the server hosting the indicated field
        const TString& strTarFld = ccfgToDo.piAt(0).m_strValue;
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strTarFld, strMoniker, strField);

        // And ask for the field definition info for this field
        TCQCFldDef flddTmp;
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
            orbcAdmin->QueryFieldDef(strMoniker, strField, flddTmp);
        }
        flddTmp.QueryNumericRange(m_strTmp1, m_strTmp2);

        // And store the values away
        TCQCActVar& varMinVal = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varMinVal.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMinVal.strName(), varMinVal.strValue());

        TCQCActVar& varMaxVal = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varMaxVal.bSetValue(m_strTmp2) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMaxVal.strName(), varMaxVal.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_HasQueuedFldChange)
    {
        const TString& strTarFld = ccfgToDo.piAt(0).m_strValue;
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strTarFld, strMoniker, strField);
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
        tCIDLib::TCard8 c8EndTime;
        bResult = orbcAdmin->bHasQueuedTimedWrite(strMoniker, strField, c8EndTime);
        if (bResult)
        {
            // Find the target variable and fill it in
            TCQCActVar& varTar = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(1).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varTar.bSetValue(c8EndTime) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_QueryDrvBoolVal)
    {
        // We get a moniker and a query id, and get back a boolean value
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;

        // Get the proxy for the driver
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));

        // And just make the call and put the return into the command result
        bResult = orbcAdmin->bQueryVal(strMoniker, ccfgToDo.piAt(1).m_strValue);
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_QueryDrvCardVal)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_QueryDrvIntVal))
    {
        // We get a moniker, a value id to query, and a variable to put the result into
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;

        // Get the proxy for the driver
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));

        // And just make the call and put the return into the command result
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_QueryDrvCardVal)
        {
            const tCIDLib::TCard4 c4Res = orbcAdmin->c4QueryVal
            (
                strMoniker, ccfgToDo.piAt(1).m_strValue
            );
            m_strTmp1.SetFormatted(c4Res);
        }
         else
        {
            const tCIDLib::TCard4 i4Res = orbcAdmin->i4QueryVal
            (
                strMoniker, ccfgToDo.piAt(1).m_strValue
            );
            m_strTmp1.SetFormatted(i4Res);
        }

        // Set up the variable with the result
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_QueryDrvText)
    {
        //
        //  We get a moniker, a query id, and a data name, which are
        //  used to make the call to the driver, and a variable to fill
        //  with the resulting output.
        //
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;

        // Get the proxy for the driver
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));

        // And do the query into a temp string
        bResult = orbcAdmin->bQueryTextVal
        (
            strMoniker
            , ccfgToDo.piAt(1).m_strValue
            , ccfgToDo.piAt(2).m_strValue
            , m_strTmp1
        );

        // Set up the variable with the result
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SendDrvCmd)
    {
        // We get a moniker, a command id , and the value to send
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;

        // Get the proxy for the driver
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
        const tCIDLib::TCard4 c4Res = orbcAdmin->c4SendCmd
        (
            strMoniker
            , ccfgToDo.piAt(1).m_strValue
            , ccfgToDo.piAt(2).m_strValue
            , acteTar.sectUser()
        );

        // If the return value is non-zero, set the result to true
        bResult = (c4Res != 0);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_TimedFldChange)
    {
        const TString& strTarFld = ccfgToDo.piAt(0).m_strValue;
        const TString& strNewVal = ccfgToDo.piAt(1).m_strValue;
        const TString& strFinalVal = ccfgToDo.piAt(2).m_strValue;

        // Figure out the delay in seconds
        tCIDLib::TCard4 c4DelaySecs;
        TString strErr;
        if (!bParseTimedFldChInt(ccfgToDo.piAt(3).m_strValue, strErr, c4DelaySecs))
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcFld_BadTimedFldWrtInt
                , strErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , ccfgToDo.piAt(3).m_strValue
            );
        }

        // Make sure the field name is good
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strTarFld, strMoniker, strField);

        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
        orbcAdmin->DoTimedWrite
        (
            strMoniker
            , strField
            , strNewVal
            , strFinalVal
            , c4DelaySecs
            , acteTar.sectUser()
            , tCQCKit::EDrvCmdWaits::DontCare
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WaitDriverReady)
    {
        const TString& strMoniker = ccfgToDo.piAt(0).m_strValue;
        const TString& strMaxWait = ccfgToDo.piAt(1).m_strValue;
        const tCIDLib::TBoolean bThrowIfNot
        (
            facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue)
        );

        // Make sure the max wait is a valid number
        tCIDLib::TBoolean bOk;
        const tCIDLib::TCard4 c4Millis = TRawStr::c4AsBinary(strMaxWait.pszBuffer(), bOk);
        if (!bOk)
            ThrowBadParmVal(ccfgToDo, 1);

        // We need to watch for shutdown requests, so get our thread object
        TThread* pthrMe = TThread::pthrCaller();

        // And calculate the end time
        const tCIDLib::TEncodedTime enctEnd
        (
            TTime::enctNow() + (c4Millis * kCIDLib::enctOneMilliSec)
        );

        // Get an admin proxy for the CQCServer that hosts the target device
        tCQCKit::TCQCSrvProxy orbcAdmin;
        while (kCIDLib::True)
        {
            try
            {
                orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
                break;
            }

            catch(...)
            {
            }

            // If we've run out of time, then give up
            if (TTime::enctNow() >= enctEnd)
            {
                if (bThrowIfNot)
                {
                    facCQCAct().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kActErrs::errcDrv_NeverWasReady
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Timeout
                        , strMoniker
                    );
                }
                bResult = kCIDLib::False;
                return tCQCKit::ECmdRes::Ok;
            }

            // Not yet, so sleep a bit. If we get a shutdown request, break out
            if (!pthrMe->bSleep(500))
                return tCQCKit::ECmdRes::Stop;
        }

        // Until we run out of time, keep waiting
        while (kCIDLib::True)
        {
            // Get the driver status
            tCIDLib::TCard4         c4ThreadId;
            tCQCKit::EDrvStates     eState;
            tCQCKit::EVerboseLvls   eVerbose;
            try
            {
                orbcAdmin->QueryDriverState(strMoniker, eState, eVerbose, c4ThreadId);

                // If it's online, then we are done
                if (eState == tCQCKit::EDrvStates::Connected)
                    break;
            }

            catch(...)
            {
                // Just eat it and keep waiting
            }

            // If we've run out of time, then give up
            if (TTime::enctNow() >= enctEnd)
            {
                if (bThrowIfNot)
                {
                    facCQCAct().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kActErrs::errcDrv_NeverWasReady
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Timeout
                        , strMoniker
                    );
                }
                bResult = kCIDLib::False;
                return tCQCKit::ECmdRes::Ok;
            }

            // Not yet, so sleep a bit. If we get a shutdown request, break out
            if (!pthrMe->bSleep(500))
                return tCQCKit::ECmdRes::Stop;
        }

        // Must have worked
        bResult = kCIDLib::True;
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_SafeWaitFieldVal)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WaitFieldVal))
    {
        //
        //  These commands both do the same thing. One just returns a true/false
        //  status (the Safe one) while the other throws if the field never
        //  gets to the desired value.
        //
        const TString& strFld = ccfgToDo.piAt(0).m_strValue;
        const TString& strVal = ccfgToDo.piAt(1).m_strValue;
        const TString& strRel = ccfgToDo.piAt(2).m_strValue;
        const TString& strMaxWait = ccfgToDo.piAt(3).m_strValue;

        // Make sure the max wait is a valid number
        tCIDLib::TBoolean bOk;
        const tCIDLib::TCard4 c4Millis= TRawStr::c4AsBinary(strMaxWait.pszBuffer(), bOk);
        if (!bOk)
            ThrowBadParmVal(ccfgToDo, 3);

        // We need to watch for shutdown requests, so get our thread object
        TThread* pthrMe = TThread::pthrCaller();

        // And calculate the end time
        const tCIDLib::TEncodedTime enctEnd(TTime::enctNowPlusMSs(c4Millis));

        // Convert the relationship value to an expression statement type
        tCQCKit::EStatements eStmt = tCQCKit::EStatements::Count;
        if (strRel == L"LessThan")
            eStmt = tCQCKit::EStatements::IsLsThan;
        else if (strRel == L"GreaterThan")
            eStmt = tCQCKit::EStatements::IsGThan;
        else if (strRel == L"Equal")
            eStmt = tCQCKit::EStatements::IsEqual;
        else if (strRel == L"NotEqual")
            eStmt = tCQCKit::EStatements::NotEqual;
        else
            ThrowBadParmVal(ccfgToDo, 2);

        // Break out the mon/fld into separate names
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strFld, strMoniker, strField);

        // Create an expression object to use for the comparison
        TCQCExpression exprTmp
        (
            TString::strEmpty()
            , tCQCKit::EExprTypes::Statement
            , eStmt
            , strVal
            , kCIDLib::False
        );

        // The states we use in a little state machine below
        enum EStates
        {
            EState_GetProxy
            , EState_GetFldDef
            , EState_WaitVal
            , EState_Done
        };

        // And now loop until we get a good value or we time out
        tCIDLib::TCard4         c4SerialNum;
        EStates                 eState = EState_GetProxy;
        tCQCKit::TCQCSrvProxy   orbcAdmin;
        TCQCFldDef              flddSrc;
        TCQCStringFldValue      fvCurVal;
        TString                 strTmp;
        while (eState < EState_Done)
        {
            try
            {
                //
                //  We will try to go straight to the target state and read
                //  the field value, so as long as the first states pass, we
                //  just fall through. If they don't pass, they will throw so
                //  we still effectively do a break out of the switch statement
                //  in those cases.
                //
                //  This makes it much faster and more efficient.
                //
                switch(eState)
                {
                    case EState_GetProxy :
                        orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
                        eState = EState_GetFldDef;
                        // Let it fall through since we worked ok

                    case EState_GetFldDef :
                        orbcAdmin->QueryFieldDef(strMoniker, strField, flddSrc);
                        eState = EState_WaitVal;
                        // Let it fall through since we worked ok

                    case EState_WaitVal :
                        c4SerialNum = 0;
                        tCQCKit::EFldTypes eType;
                        if (orbcAdmin->bReadFieldByName(c4SerialNum
                                                        , strMoniker
                                                        , strField
                                                        , strTmp
                                                        , eType))
                        {
                            //
                            //  If it evaluates true, then we are done. We can
                            //  allow for global variable references here.
                            //
                            fvCurVal.bSetValue(strTmp);
                            if (exprTmp.bEvaluate(fvCurVal, &ctarGlobalVars))
                                eState = EState_Done;
                        }
                        break;
                };
            }

            catch(...)
            {
                // Eat it and keep waiting
            }

            // If we are done, then break out now
            if (eState == EState_Done)
                break;

            if (TTime::enctNow() >= enctEnd)
            {
                // We've run out of time, so give up
                if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WaitFieldVal)
                {
                    // We are in the throwing version of this command
                    if (eState < EState_WaitVal)
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcDrv_NeverWasReady
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Timeout
                            , strMoniker
                        );
                    }
                     else
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcFld_NeverReachedVal
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Timeout
                            , strFld
                            , strVal
                        );
                    }
                }
                 else
                {
                    //
                    //  We are in the non-throwing version, so just break out.
                    //  If the state is less than done, we'll return failure.
                    //
                    break;
                }
            }
             else
            {
                //
                //  Still some time, so sleep a bit. If we get a shutdown
                //  request, break out with a stop value to cause the action
                //  to stop processing and exit.
                //
                if (!pthrMe->bSleep(500))
                    return tCQCKit::ECmdRes::Stop;
            }
        }

        //
        //  Set the results based on the state machine state. Doesn't really
        //  matter if we are in the non-conditional version of it. It won't
        //  get used in that case.
        //
        bResult = (eState == EState_Done);
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }
    return tCQCKit::ECmdRes::Ok;
}


//
//  Note that we don't return any outgoing type commands if we are in the IV
//  Event context.
//
tCIDLib::TVoid
TStdFieldCmdTar::QueryCmds(         TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    //
    //  Cancel any pending timed field change for a field.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdCanTimedCh
        (
            kCQCKit::strCmdId_CancelTimedFldChange
            , facCQCAct().strMsg(kActMsgs::midCmd_CancelTimedFldChange)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
        );
        cmdCanTimedCh.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::ReadOrWrite);
        colToFill.objAdd(cmdCanTimedCh);
    }

    //
    //  Clear the stats fields of a driver
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdClearDrvStats
        (
            kCQCKit::strCmdId_ClearDrvStats
            , facCQCAct().strMsg(kActMsgs::midCmd_ClearDrvStats)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
        );
        colToFill.objAdd(cmdClearDrvStats);
    }

    //
    //  Do a field read. It has two parameters. The device.field to
    //  read from, and the variable store the value in.
    //
    {
        TCQCCmd cmdFldRead
        (
            kCQCKit::strCmdId_FieldRead
            , facCQCAct().strMsg(kActMsgs::midCmd_FieldRead)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_FailOnError)
        );
        cmdFldRead.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
        cmdFldRead.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdFldRead);
    }

    // Same as above, but separate driver/field values
    {
        TCQCCmd cmdFldRead2
        (
            kCQCKit::strCmdId_FieldRead2
            , facCQCAct().strMsg(kActMsgs::midCmd_FieldRead2)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_FailOnError)
        );
//        cmdFldRead.cmdpAt(1).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
        cmdFldRead2.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdFldRead2);
    }

    // A conditional expression to see if a device driver is ready
    {
        TCQCCmd cmdDriverReady
        (
            kCQCKit::strCmdId_DriverReady
            , facCQCAct().strMsg(kActMsgs::midCmd_DriverReady)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
        );
        cmdDriverReady.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdDriverReady);
    }

    //
    //  Do the field write command. It has two parameters. The device.field to
    //  write to, and the value to write to it. The value is optional.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdFldWrt
        (
            kCQCKit::strCmdId_FieldWrite
            , facCQCAct().strMsg(kActMsgs::midCmd_FieldWrite)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_FailOnError)
        );
        cmdFldWrt.eType(tCQCKit::ECmdTypes::Both);
        cmdFldWrt.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::CReadMWrite);
        cmdFldWrt.SetParmReqAt(1, kCIDLib::False);
        colToFill.objAdd(cmdFldWrt);
    }

    // Same as above, but separate driver/field values
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdFldWrt2
        (
            kCQCKit::strCmdId_FieldWrite2
            , facCQCAct().strMsg(kActMsgs::midCmd_FieldWrite2)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_FailOnError)
        );
        cmdFldWrt2.eType(tCQCKit::ECmdTypes::Both);
        cmdFldWrt2.SetParmReqAt(2, kCIDLib::False);
        colToFill.objAdd(cmdFldWrt2);
    }

    //
    //  Get a quoted comma list of all loaded drivers that are of a particular
    //  device category.
    //
    {
        TCQCCmd cmdFindDrivers
        (
            kCQCKit::strCmdId_FindDrivers
            , facCQCAct().strMsg(kActMsgs::midCmd_FindDrivers)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Make)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Model)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Category)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
        );

        // Each of the filter criteria can be empty
        cmdFindDrivers.SetParmReqAt(0, kCIDLib::False);
        cmdFindDrivers.SetParmReqAt(1, kCIDLib::False);
        cmdFindDrivers.SetParmReqAt(2, kCIDLib::False);
        colToFill.objAdd(cmdFindDrivers);
    }


    //
    //  Get a quoted comma list of all fields of the indicated driver that
    //  have the indicated access (at least) and whose names match the passed
    //  regular expression.
    //
    {
        TCQCCmd cmdFindFields
        (
            kCQCKit::strCmdId_FindFields
            , facCQCAct().strMsg(kActMsgs::midCmd_FindFields)
            , 4
        );

        cmdFindFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Driver), tCQCKit::ECmdPTypes::Driver
            )
        );

        // A regular expression for the field names
        cmdFindFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern), tCQCKit::ECmdPTypes::RegXText
            )
        );

        // Two variable names, space separated, one for fields one for names
        cmdFindFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVars), tCQCKit::ECmdPTypes::Text
            )
        );

        // And enum of available requested access types
        TString strAccLims;
        tCQCKit::FormatEReqAccess(strAccLims, TString::strEmpty(), kCIDLib::chComma);
        cmdFindFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_RequiredAccess), strAccLims
            )
        );

        cmdFindFields.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdFindFields);
    }

    // Get the numeric range of a numeric typed field
    {
        TCQCCmd cmdGetNumR
        (
            kCQCKit::strCmdId_GetNumericRange
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetNumericRange)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MinVal)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MaxVal)
        );
        cmdGetNumR.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::CReadMWrite);
        colToFill.objAdd(cmdGetNumR);
    }

    // Get semantic fieldname/name lists
    {
        TCQCCmd cmdGetSemFields
        (
            kCQCKit::strCmdId_GetSemFields
            , facCQCAct().strMsg(kActMsgs::midCmd_GetSemFields)
            , 4
        );

        cmdGetSemFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Driver), tCQCKit::ECmdPTypes::Driver
            )
        );

        cmdGetSemFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_SemanticType)
                , L"AnalogIO, BoolSwitch, CurTemp, DigitalIO, Dimmer, LightSwitch, Mute, "
                  L"Power, SecZoneStat, Volume, VolumeAdj"
            )
        );

        // Two variable names, space separated, one for fields one for names
        cmdGetSemFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVars), tCQCKit::ECmdPTypes::Text
            )
        );

        // And enum of available requested access types
        TString strAccLims;
        tCQCKit::FormatEReqAccess(strAccLims, TString::strEmpty(), kCIDLib::chComma);
        cmdGetSemFields.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_RequiredAccess), strAccLims
            )
        );

        cmdGetSemFields.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdGetSemFields);
    }

    // See if a field has a pending timed field change
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdHasTimedCh
        (
            kCQCKit::strCmdId_HasQueuedFldChange
            , facCQCAct().strMsg(kActMsgs::midCmd_HasQueuedFldChange)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Time)
        );
        cmdHasTimedCh.eType(tCQCKit::ECmdTypes::Conditional);
        cmdHasTimedCh.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::ReadOrWrite);
        colToFill.objAdd(cmdHasTimedCh);
    }

    // Query a boolean result from a driver
    {
        TCQCCmd cmdQueryDrvBoolVal
        (
            kCQCKit::strCmdId_QueryDrvBoolVal
            , facCQCAct().strMsg(kActMsgs::midCmd_QueryDrvBoolVal)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
        );
        cmdQueryDrvBoolVal.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdQueryDrvBoolVal);
    }

    // Query a card result from a driver
    {
        TCQCCmd cmdQueryDrvCardVal
        (
            kCQCKit::strCmdId_QueryDrvCardVal
            , facCQCAct().strMsg(kActMsgs::midCmd_QueryDrvCardVal)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        );
        colToFill.objAdd(cmdQueryDrvCardVal);
    }

    // Query an int result from a driver
    {
        TCQCCmd cmdQueryDrvIntVal
        (
            kCQCKit::strCmdId_QueryDrvIntVal
            , facCQCAct().strMsg(kActMsgs::midCmd_QueryDrvIntVal)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        );
        colToFill.objAdd(cmdQueryDrvIntVal);
    }

    // Query text from a driver
    {
        TCQCCmd cmdQueryDrvText
        (
            kCQCKit::strCmdId_QueryDrvText
            , facCQCAct().strMsg(kActMsgs::midCmd_QueryDrvText)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        );
        cmdQueryDrvText.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdQueryDrvText);
    }

    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdSafeWaitFldVal
        (
            kCQCKit::strCmdId_SafeWaitFieldVal
            , facCQCAct().strMsg(kActMsgs::midCmd_SafeWaitFieldVal)
            , 4
        );

        TCQCCmdParm& cmdpFld = cmdSafeWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
                , tCQCKit::ECmdPTypes::Field
            )
        );
        cmdpFld.eFldAccess(tCQCKit::EReqAccess::MReadCWrite);

        // The value can be empty so indicate it's not required
        cmdSafeWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::False
            )
        );

        cmdSafeWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Relationship)
                , TString(L"LessThan, GreaterThan, Equal, NotEqual")
            )
        );

        cmdSafeWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );

        cmdSafeWaitFldVal.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdSafeWaitFldVal);
    }

    // Send text to a driver
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdSendDrvCmd
        (
            kCQCKit::strCmdId_SendDrvCmd
            , facCQCAct().strMsg(kActMsgs::midCmd_SendDrvCmd)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
        );
        cmdSendDrvCmd.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdSendDrvCmd);
    }

    //
    //  Do a timed field change
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdTimedCh
        (
            kCQCKit::strCmdId_TimedFldChange
            , facCQCAct().strMsg(kActMsgs::midCmd_TimedFldChange)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Delay)
        );
        cmdTimedCh.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::CReadMWrite);
        colToFill.objAdd(cmdTimedCh);
    }

    // Wait's for a driver to come online
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdWaitDrv
        (
            kCQCKit::strCmdId_WaitDriverReady
            , facCQCAct().strMsg(kActMsgs::midCmd_WaitDriverReady)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_FailOnError)

        );
        cmdWaitDrv.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdWaitDrv);
    }

    //
    //  Wait's for a field's value to equal or not equal a given value.
    //  We have one version that is conditional and one that is not.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdWaitFldVal
        (
            kCQCKit::strCmdId_WaitFieldVal
            , facCQCAct().strMsg(kActMsgs::midCmd_WaitFieldVal)
            , 4
        );

        TCQCCmdParm& cmdpFld = cmdWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
                , tCQCKit::ECmdPTypes::Field
            )
        );
        cmdpFld.eFldAccess(tCQCKit::EReqAccess::MReadCWrite);

        // The value can be empty so indicate it's not required
        cmdWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::False
            )
        );

        cmdWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Relationship)
                , TString(L"LessThan, GreaterThan, Equal, NotEqual")
            )
        );

        cmdWaitFldVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );
        colToFill.objAdd(cmdWaitFldVal);
    }
}


// Provide default parameters for some of our commands
tCIDLib::TVoid TStdFieldCmdTar::SetDefParms(TCQCCmdCfg& ccfgUpdate) const
{
    // Default any parameters that we can reasonably default
    if ((ccfgUpdate.strCmdId() == kCQCKit::strCmdId_FieldRead)
    ||  (ccfgUpdate.strCmdId() == kCQCKit::strCmdId_FieldWrite))
    {
        ccfgUpdate.piAt(2).m_strValue = kCQCKit::pszFld_True;
    }
     else if ((ccfgUpdate.strCmdId() == kCQCKit::strCmdId_FieldRead2)
          ||  (ccfgUpdate.strCmdId() == kCQCKit::strCmdId_FieldWrite2))
    {
        ccfgUpdate.piAt(3).m_strValue = kCQCKit::pszFld_True;
    }
     else if (ccfgUpdate.strCmdId() == kCQCKit::strCmdId_TimedFldChange)
    {
        ccfgUpdate.piAt(3).m_strValue = L"5";
    }
     else if (ccfgUpdate.strCmdId() == kCQCKit::strCmdId_WaitDriverReady)
    {
        ccfgUpdate.piAt(1).m_strValue = L"5000";
    }
     else if (ccfgUpdate.strCmdId() == kCQCKit::strCmdId_WaitFieldVal)
    {
        ccfgUpdate.piAt(3).m_strValue = L"5000";
    }
}


// ---------------------------------------------------------------------------
//  TStdFieldCmdTar: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TStdFieldCmdTar::bFindFields(   const   TString&                strMon
                                ,       TString&                strFldNames
                                ,       TString&                strNames
                                , const TString&                strRegEx
                                , const tCQCKit::EReqAccess     eReqAccess)
{
    strFldNames.Clear();
    strNames.Clear();

    try
    {
        // Get a client proxy for the CQC server that has this driver
        tCQCKit::TCQCSrvProxy orbcCQC = facCQCKit().orbcCQCSrvAdminProxy(strMon);

        TVector<TCQCFldDef> colFlds;
        const tCIDLib::TCard4 c4Count = orbcCQC->c4QueryFieldNamesRX
        (
            strMon, colFlds, strRegEx, eReqAccess
        );

        // If none, just return with cleared lists
        if (!c4Count)
            return kCIDLib::False;

        // Let's process what we got
        TString strName;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCFldDef& flddCur = colFlds[c4Index];

            strName = strMon;
            strName.Append(kCIDLib::chPeriod);
            strName.Append(flddCur.strName());
            TStringTokenizer::BuildQuotedCommaList(strName, strFldNames);

            //
            //  Get the basic name part. If it has a # in it, we have to get rid of
            //  that V2 prefix bit.
            //
            strName = flddCur.strName();

            tCIDLib::TCard4 c4At;
            if (strName.bFirstOccurrence(kCIDLib::chPoundSign, c4At))
            {
                strName.Cut(0, c4At + 1);

                //
                //  It was V2, so check for the semantic types that have a further
                //  prefix.
                //
                if ((flddCur.eSemType() == tCQCKit::EFldSTypes::AnalogIO)
                ||  (flddCur.eSemType() == tCQCKit::EFldSTypes::DigitalIO))
                {
                    if (strName.bStartsWith(L"In_"))
                        strName.Cut(0, 3);
                    else
                        strName.Cut(0, 4);
                }
                 else if (flddCur.eSemType() == tCQCKit::EFldSTypes::Dimmer)
                {
                    if (strName.bStartsWith(L"Dim_"))
                        strName.Cut(0, 4);
                }
                 else if (flddCur.eSemType() == tCQCKit::EFldSTypes::LightSwitch)
                {
                    if (strName.bStartsWith(L"Sw_"))
                        strName.Cut(0, 3);
                }

                // Replace any ~ and underscore characters with a space
                strName.bReplaceChar(kCIDLib::chTilde, kCIDLib::chSpace);
                strName.bReplaceChar(kCIDLib::chUnderscore, kCIDLib::chSpace);
            }

            TStringTokenizer::BuildQuotedCommaList(strName, strNames);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TStdFieldCmdTar::bQuerySemFields(const  TString&            strMon
                                , const tCQCKit::EFldSTypes eSemType
                                ,       TString&            strFldNames
                                ,       TString&            strNames
                                , const tCQCKit::EReqAccess eReqAccess)
{
    strFldNames.Clear();
    strNames.Clear();

    try
    {
        // Get a client proxy for the CQC server that has this driver
        tCQCKit::TCQCSrvProxy orbcCQC = facCQCKit().orbcCQCSrvAdminProxy(strMon);

        // And ask it for any fields that have the indicated semantic type
        TFundVector<tCQCKit::EFldSTypes> fcolSTypes;
        fcolSTypes.c4AddElement(eSemType);
        TVector<TCQCDrvFldDef> colFlds;
        const tCIDLib::TCard4 c4Count = orbcCQC->c4QuerySemFields
        (
            fcolSTypes, eReqAccess, colFlds, strMon
        );

        // If none, just return with cleared lists
        if (!c4Count)
            return kCIDLib::False;

        // Let's process what we got
        TString strName;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strName = strMon;
            strName.Append(kCIDLib::chPeriod);
            strName.Append(colFlds[c4Index].strName());
            TStringTokenizer::BuildQuotedCommaList(strName, strFldNames);

            //
            //  Get the basic name part. If it has a # in it, we have to get rid of
            //  that V2 prefix bit.
            //
            strName = colFlds[c4Index].strName();

            tCIDLib::TCard4 c4At;
            if (strName.bFirstOccurrence(kCIDLib::chPoundSign, c4At))
            {
                strName.Cut(0, c4At + 1);

                //
                //  It was V2, so check for the semantic types that have a further
                //  prefix.
                //
                if ((eSemType == tCQCKit::EFldSTypes::AnalogIO)
                ||  (eSemType == tCQCKit::EFldSTypes::DigitalIO))
                {
                    if (strName.bStartsWith(L"In_"))
                        strName.Cut(0, 3);
                    else
                        strName.Cut(0, 4);
                }
                 else if (eSemType == tCQCKit::EFldSTypes::Dimmer)
                {
                    if (strName.bStartsWith(L"Dim_"))
                        strName.Cut(0, 4);
                }
                 else if (eSemType == tCQCKit::EFldSTypes::LightSwitch)
                {
                    if (strName.bStartsWith(L"Sw_"))
                        strName.Cut(0, 3);
                }

                // Replace any ~ and underscore characters with a space
                strName.bReplaceChar(kCIDLib::chTilde, kCIDLib::chSpace);
                strName.bReplaceChar(kCIDLib::chUnderscore, kCIDLib::chSpace);
            }

            TStringTokenizer::BuildQuotedCommaList(strName, strNames);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  The delay parameter of the time field change can be in these forms:
//
//  5       (Minutes)
//  5:2     (Minutes plus seconds)
//  :5      (Seconds)
//
tCIDLib::TBoolean
TStdFieldCmdTar::bParseTimedFldChInt(const  TString&            strToParse
                                    ,       TString&            strErr
                                    ,       tCIDLib::TCard4&    c4SecsToFill) const
{
    TString strMins = strToParse;
    strMins.StripWhitespace();
    TString strSecs;

    tCIDLib::TCard4 c4At;
    if (strMins.bFirstOccurrence(kCIDLib::chColon, c4At))
    {
        if (c4At)
        {
            // Has to be minus plus seconds, so split the value on the colon
            strMins.bSplit(strSecs, kCIDLib::chColon);
        }
         else
        {
            // Has to be seconds, with leading colon so copy over seconds and clear minutes
            strSecs.CopyInSubStr(strMins, 1);
            strMins.Clear();
        }
    }


    // Make sure there isn't a second colon
    if (strSecs.bFirstOccurrence(kCIDLib::chColon, c4At))
    {
        strErr = L"The timed field write interval cannot have two colons in it";
        return kCIDLib::False;
    }

    tCIDLib::TCard4 c4Mins = 0;
    tCIDLib::TCard4 c4Secs = 0;
    if (!strMins.bIsEmpty())
    {
        if (!strMins.bToCard4(c4Mins, tCIDLib::ERadices::Dec))
        {
            strErr = L"The timed field write interval minutes value was invalid";
            return kCIDLib::False;
        }
    }

    if (!strSecs.bIsEmpty())
    {
        if (!strSecs.bToCard4(c4Secs, tCIDLib::ERadices::Dec))
        {
            strErr = L"The timed field write interval seconds value was invalid";
            return kCIDLib::False;
        }
    }

    c4SecsToFill = (c4Mins * 60) + c4Secs;

    if ((c4SecsToFill < 10) || (c4SecsToFill > (60 * 60) * 24))
    {
        strErr = L"The timed field write interval must be 10 seconds to 24 hours";
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  CLASS: TStdMacroCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TStdMacroCmdTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TStdMacroCmdTar::TStdMacroCmdTar() :

    MCQCCmdTarIntf
    (
        kCQCKit::strCTarId_MacroEngine
        , facCQCAct().strMsg(kActMsgs::midCmdTar_Macro)
        , kCQCKit::strCTarId_MacroEngine
        , L"/Reference/Actions/Macros"
    )
    , m_pmeehLogger(nullptr)
    , m_pmefrData(nullptr)
    , m_pmeToUse(nullptr)
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);
}

TStdMacroCmdTar::~TStdMacroCmdTar()
{
    // Delete all our allocated bits
    try
    {
        delete m_pmeToUse;
    }

    catch(...)
    {
    }

    try
    {
        delete m_pmeehLogger;
    }

    catch(...)
    {
    }

    try
    {
        delete m_pmefrData;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TStdMacroCmdTar: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking
tCIDLib::TBoolean
TStdMacroCmdTar::bValidateParm( const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    return kCIDLib::True;
}


tCIDLib::TVoid TStdMacroCmdTar::CmdTarCleanup()
{
    // This one is a no-op for us
}


tCIDLib::TVoid TStdMacroCmdTar::CmdTarInitialize(const TCQCActEngine&)
{
    // This one is a no-op for us
}


// We override this to process command requests
tCQCKit::ECmdRes
TStdMacroCmdTar::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
                        , const tCIDLib::TCard4
                        , const TString&
                        , const TString&
                        ,       TStdVarsTar&            ctarGlobalVars
                        ,       tCIDLib::TBoolean&      bRes
                        ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_RunMacro)
    {
        //
        //  If we've not created or been given a macro engine yet, then do
        //  it and set it up. If already there, reset it.
        //
        if (!m_pmeToUse)
        {
            m_pmeToUse = new TCIDMacroEngine;
            SetupEngine();
        }
         else
        {
            m_pmeToUse->Reset();
        }

        // Return a false result if we fail to get it set correctly
        bRes = kCIDLib::False;

        const TString& strMacro = ccfgToDo.piAt(0).m_strValue;
        const TString& strParms = ccfgToDo.piAt(1).m_strValue;
        try
        {
            // Try to parse out the parameters
            TVector<TString> colSrcParms;
            const tCIDLib::TCard4 c4ParmCount = TExternalProcess::c4BreakOutParms
            (
                strParms, colSrcParms
            );

            // Try to download and parse the macro
            TCQCMEngClassMgr    mecmToUse(acteTar.sectUser());
            TCQCPrsErrHandler   meehToUse;
            TMacroEngParser     meprsToUse;
            TMEngClassInfo*     pmeciNew;
            if (!meprsToUse.bParse(strMacro, pmeciNew, m_pmeToUse, &meehToUse, &mecmToUse))
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_CantLoadMacro
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::CantDo
                    , strMacro
                );
            }

            //
            //  Looks like it worked, so create the value object and invoke the
            //  default ctor.
            //
            TMEngClassVal* pmecvTarget = pmeciNew->pmecvMakeStorage
            (
                L"$Main$", *m_pmeToUse, tCIDMacroEng::EConstTypes::NonConst
            );
            TJanitor<TMEngClassVal> janClass(pmecvTarget);

            // Call it's default constructor
            if (!m_pmeToUse->bInvokeDefCtor(*pmecvTarget, &acteTar.cuctxToUse()))
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_DefCtorFailed
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::CantDo
                    , strMacro
                );
            }

            //
            //  Search this class for a legal entry point. It must have
            //  particular name and form.
            //
            TMEngClassInfo& meciTarget = m_pmeToUse->meciFind(pmecvTarget->c2ClassId());
            const tCIDLib::TCard2 c2MethodId = m_pmeToUse->c2FindEntryPoint(meciTarget, 0);

            //
            //  Make sure that we have the same number of parameters as
            //  we do target params to plug them into.
            //
            TMEngMethodInfo& methiTarget = meciTarget.methiFind(c2MethodId);
            if (c4ParmCount != methiTarget.c4ParmCount())
            {
                facCIDMacroEng().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kMEngErrs::errcEng_BadEntryParmCnt
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , meciTarget.strClassPath()
                    , TCardinal(methiTarget.c4ParmCount())
                    , TCardinal(c4ParmCount)
                );
            }

            //
            //  Ask the macro engine to create a list of objects for the
            //  the input parameters to this method.
            //
            TCIDMacroEngine::TParmList colParms(tCIDLib::EAdoptOpts::Adopt);
            m_pmeToUse->c4CreateParmList(methiTarget, colParms);

            //
            //  Now we'll loop through them and load up all the incoming
            //  values into them. Normally, we'd just pass the param string as
            //  is, but we have to deal with Out and InOut parms, which have
            //  to be associated with variables.
            //
            TString strVarVal;
            for (tCIDLib::TCard2 c2Id = 0; c2Id < c4ParmCount; c2Id++)
            {
                const TMEngParmInfo& mepiCur = methiTarget.mepiFind(c2Id);
                const TString& strCurParm = colSrcParms[c2Id];
                TMEngClassVal* pmecvCur = colParms[c2Id];

                // Look up the class type for this parm
                const TMEngClassInfo& meciParm = m_pmeToUse->meciFind(mepiCur.c2ClassId());

                //
                //  If it's a const parm, then we just let it parse the
                //  current input parameter to set the value. Else, it must
                //  be associated with a variable.
                //
                if (pmecvCur->bIsConst())
                {
                    if (!pmecvCur->bParseFromText(strCurParm, meciParm))
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcCmd_BadMacroParm3
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::BadParms
                            , TCardinal(c2Id + 1)
                        );
                    }
                }
                 else
                {
                    // It must be of or derived from type string
                    if (!m_pmeToUse->bIsDerivedFrom(mepiCur.c2ClassId()
                                                    , tCIDMacroEng::EIntrinsics::String))
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcCmd_BadMacroParm2
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::BadParms
                            , TCardinal(c2Id + 1)
                        );
                    }

                    // The incoming parm must be a variable name
                    const TStdVarsTar* pctarSrc = 0;
                    if (strCurParm.bStartsWith(kCQCKit::strActVarPref_GVar))
                        pctarSrc = &ctarGlobalVars;
                    else if (strCurParm.bStartsWith(kCQCKit::strActVarPref_LVar))
                        pctarSrc = &acteTar.ctarLocals();
                    else
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcCmd_BadMacroParm1
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::BadParms
                            , TCardinal(c2Id + 1)
                        );
                    }

                    //
                    //  If it's InOut, then set the current value of the
                    //  varaible as the parameter value.
                    //
                    if (mepiCur.eDir() == tCIDMacroEng::EParmDirs::InOut)
                    {
                        if (!pctarSrc->bVarValue(strCurParm, strVarVal))
                        {
                            facCQCAct().ThrowErr
                            (
                                CID_FILE
                                , CID_LINE
                                , kActErrs::errcCmd_BadMacroParm4
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::BadParms
                                , TCardinal(c2Id + 1)
                            );
                        }

                        if (!pmecvCur->bParseFromText(strVarVal, meciParm))
                        {
                            facCQCAct().ThrowErr
                            (
                                CID_FILE
                                , CID_LINE
                                , kActErrs::errcCmd_BadMacroParm3
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::BadParms
                                , TCardinal(c2Id + 1)
                            );
                        }
                    }
                }
            }

            // And invoke the macro, passing along the configured parameters
            const tCIDLib::TInt4 i4Res = m_pmeToUse->i4Run
            (
                *pmecvTarget, colParms, &acteTar.cuctxToUse()
            );

            //
            //  And now, we have to back through the parms again, and if any
            //  were Out or InOut, we pull the values back into their
            //  respecive variables.
            //
            for (tCIDLib::TCard2 c2Id = 0; c2Id < c4ParmCount; c2Id++)
            {
                const TMEngParmInfo& mepiTar = methiTarget.mepiFind(c2Id);
                const TString& strCurParm = colSrcParms[c2Id];
                TMEngClassVal* pmecvCur = colParms[c2Id];

                if ((mepiTar.eDir() == tCIDMacroEng::EParmDirs::InOut)
                ||  (mepiTar.eDir() == tCIDMacroEng::EParmDirs::Out))
                {
                    // The incoming parm must be a variable name
                    TStdVarsTar* pctarTar = 0;
                    if (strCurParm.bStartsWith(kCQCKit::strActVarPref_GVar))
                        pctarTar = &ctarGlobalVars;
                    else if (strCurParm.bStartsWith(kCQCKit::strActVarPref_LVar))
                        pctarTar = &acteTar.ctarLocals();

                    // Look at this one as a string value
                    TMEngStringVal* pmecvStr = static_cast<TMEngStringVal*>
                    (
                        pmecvCur
                    );

                    // And put its value back into the variable
                    const tCIDLib::TBoolean bChanged = pctarTar->bAddOrUpdateVar
                    (
                        strCurParm
                        , pmecvStr->strValue()
                        , tCQCKit::EFldTypes::String
                        , TString::strEmpty()
                    );

                    if (bChanged && acteTar.pcmdtDebug())
                        acteTar.pcmdtDebug()->ActVarSet(strCurParm, pmecvStr->strValue());
                }
            }

            // Set the command result based on the return
            bRes = (i4Res != 0);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            if (facCQCAct().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            if (facCQCAct().bLogFailures())
            {
                facCQCAct().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_MacroFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , strMacro
                );
            }
            throw;
        }

        // Catch macro level exceptions
        catch(const TExceptException&)
        {
            // Log the macro exception
            m_pmeToUse->LogLastExcept();

            if (facCQCAct().bLogFailures())
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_MacroFailed
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::CantDo
                    , strMacro
                );
            }
            throw;
        }

        catch(...)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_MacroFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::CantDo
                , strMacro
            );
            throw;
        }
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TStdMacroCmdTar::QueryCmds(         TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    //
    //  Do the macro invocation command. It takes two parameters, one is the
    //  macro to run and the other is the list of parameters to pass to it.
    //
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        TCQCCmd& cmdRunMacro = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_RunMacro
                , facCQCAct().strMsg(kActMsgs::midCmd_RunMacro)
                , tCQCKit::ECmdPTypes::Macro
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MacroToRun)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
            )
        );

        //
        //  Indicate that the macro parameters are not required, and that this
        //  is both a regular and conditional command.
        //
        cmdRunMacro.SetParmReqAt(1, kCIDLib::False);
        cmdRunMacro.eType(tCQCKit::ECmdTypes::Both);
    }
}


// Provide default params for some of our commands
tCIDLib::TVoid TStdMacroCmdTar::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCKit::strCmdId_RunMacro)
    {
        // Just empty the values out
        ccfgToSet.SetParmAt(0, TString::strEmpty());
        ccfgToSet.SetParmAt(1, TString::strEmpty());
    }
}


//
//  Lets the outside world give us a macro engine to use, usually because it
//  has some tweaks they need.
//
tCIDLib::TVoid TStdMacroCmdTar::SetEngine(TCIDMacroEngine* const pmeToUse)
{
    //
    //  If we have any existing engine, clean it up. Note that the engine
    //  does not adopt the various handlers, so we can reuse them.
    //
    try
    {
        delete m_pmeToUse;
        m_pmeToUse = 0;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Delete the passed engine before we rethrow, because we own it now
        delete pmeToUse;
        throw;
    }

    // Set up the engine
    m_pmeToUse = pmeToUse;
    SetupEngine();
}


// ---------------------------------------------------------------------------
//  TStdMacroCmdTar: Public, non-virtual methods
// ---------------------------------------------------------------------------
// Provide access to our macro engine
const TCIDMacroEngine* TStdMacroCmdTar::pmeTarget() const
{
    return m_pmeToUse;
}

TCIDMacroEngine* TStdMacroCmdTar::pmeTarget()
{
    return m_pmeToUse;
}


tCIDLib::TVoid TStdMacroCmdTar::SetupEngine()
{
    // If we've not faulted in the various handlers yet, do that
    if (!m_pmefrData)
    {
        m_pmeehLogger = new TCQCMEngErrHandler;

        //
        //  Set the root path using the configured standard local macro file
        //  root that CQCAct has for us.
        //
        m_pmefrData = new TMEngFixedBaseFileResolver(facCQCKit().strMacroRootPath());
    }

    // And set them on the engine
    m_pmeToUse->SetErrHandler(m_pmeehLogger);
    m_pmeToUse->SetFileResolver(m_pmefrData);
}






// ---------------------------------------------------------------------------
//  CLASS: TTrigEvCmdTarget
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTrigEvCmdTarget: Constructors and Destructor
// ---------------------------------------------------------------------------
TTrigEvCmdTarget::TTrigEvCmdTarget() :

    MCQCCmdTarIntf
    (
        kCQCKit::strCTarId_TrigEvent
        , facCQCAct().strMsg(kActMsgs::midCmdTar_TrigEvent)
        , kCQCKit::strCTarId_TrigEvent
        , L"/Reference/Actions/TrigEvent"
    )
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);
}

TTrigEvCmdTarget::~TTrigEvCmdTarget()
{
}


// ---------------------------------------------------------------------------
//  TTrigEvCmdTarget: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking if needed
tCIDLib::TBoolean
TTrigEvCmdTarget::bValidateParm(const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    return kCIDLib::True;
}


tCIDLib::TVoid TTrigEvCmdTarget::CmdTarCleanup()
{
    // Get rid of any existing event we might be holding
    m_cptrEvent.DropRef();
}


tCIDLib::TVoid TTrigEvCmdTarget::CmdTarInitialize(const TCQCActEngine&)
{
    // A no-op for us
}


// We override this to process command requests
tCQCKit::ECmdRes
TTrigEvCmdTarget::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4
                        , const TString&
                        , const TString&
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetEventFld)
    {
        // Get the event field path we want to look up
        const TString& strPath = ccfgToDo.piAt(0).m_strValue;

        // See if this path exists and set the result accordingly
        TString strVal;
        bResult = m_cptrEvent->bValueExists(strPath, strVal);
        if (bResult)
        {
            // The value exists, so set up the variable with the value
            const TString& strKey = ccfgToDo.piAt(1).m_strValue;
            TCQCActVar& varTar = TStdVarsTar::varFind
            (
                strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            if (varTar.bSetValue(strVal) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strKey, varTar.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetFldChangeInfo)
    {
        // If it's not a fld change, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strFldKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strValKey = ccfgToDo.piAt(1).m_strValue;

            TCQCActVar& varFld = TStdVarsTar::varFind
            (
                strFldKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varVal = TStdVarsTar::varFind
            (
                strValKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            TString strFldName;
            m_cptrEvent->bQuerySrc(strFldName);

            if (varFld.bSetValue(strFldName) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strFldKey, varFld.strValue());

            if (varVal.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_FldValue))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strValKey, varVal.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetLoadChangeInfo)
    {
        // If it's not a fld change, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::LoadChange);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strStateKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strNumKey = ccfgToDo.piAt(1).m_strValue;
            const TString& strNameKey = ccfgToDo.piAt(2).m_strValue;

            TCQCActVar& varStateKey = TStdVarsTar::varFind
            (
                strStateKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varNum = TStdVarsTar::varFind
            (
                strNumKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varStateKey.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LoadState))
            && acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strStateKey, varStateKey.strValue());
            }

            if (varNum.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LoadNum))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strNumKey, varNum.strValue());
            }

            // If the name key was set, do it. It's optional
            if (!strNameKey.bIsEmpty())
            {
                // It might not be set, so be tolerant and take empty value if not
                TString strValue;
                m_cptrEvent->bValueExists(TCQCEvent::strPath_LoadName, strValue);

                TCQCActVar& varName = TStdVarsTar::varFind
                (
                    strNameKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
                );

                if (varName.bSetValue(strValue) && acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActVarSet(strNameKey, varName.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetLockStatusInfo)
    {
        // If it's not a lock status event, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::LockStatus);

        // If it is, then return the values
        if (bResult)
        {
            const TString& strStateKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strIdKey = ccfgToDo.piAt(1).m_strValue;
            const TString& strCodeKey = ccfgToDo.piAt(2).m_strValue;
            const TString& strTypeKey = ccfgToDo.piAt(3).m_strValue;

            TCQCActVar& varState = TStdVarsTar::varFind
            (
                strStateKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            TCQCActVar& varId = TStdVarsTar::varFind
            (
                strIdKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            TCQCActVar& varCode = TStdVarsTar::varFind
            (
                strCodeKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            TCQCActVar& varType = TStdVarsTar::varFind
            (
                strTypeKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varState.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LockState))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strStateKey, varState.strValue());
            }

            if (varId.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LockId))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strIdKey, varId.strValue());
            }

            if (varCode.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LockCode))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strCodeKey, varCode.strValue());
            }

            if (varType.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_LockType))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strTypeKey, varType.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetMotionInfo)
    {
        // If it's not a motion change, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::Motion);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strStateKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strNumKey = ccfgToDo.piAt(1).m_strValue;
            const TString& strNameKey = ccfgToDo.piAt(2).m_strValue;

            TCQCActVar& varStateKey = TStdVarsTar::varFind
            (
                strStateKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varNum = TStdVarsTar::varFind
            (
                strNumKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varStateKey.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_MotionType))
            && acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strStateKey, varStateKey.strValue());
            }

            if (varNum.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_MotionNum))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strNumKey, varNum.strValue());
            }

            // If the name key was set, do it. It's optional
            if (!strNameKey.bIsEmpty())
            {
                // It might not be set, so be tolerant and take empty value if not
                TString strValue;
                m_cptrEvent->bValueExists(TCQCEvent::strPath_MotionName, strValue);

                TCQCActVar& varName = TStdVarsTar::varFind
                (
                    strNameKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
                );

                if (varName.bSetValue(strValue) && acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActVarSet(strNameKey, varName.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetPresenceInfo)
    {
        // If it's not a presence event, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::Presence);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strTypeKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strIdKey = ccfgToDo.piAt(1).m_strValue;

            TCQCActVar& varType = TStdVarsTar::varFind
            (
                strTypeKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varId = TStdVarsTar::varFind
            (
                strIdKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varType.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_PresType))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strTypeKey, varType.strValue());
            }

            if (varId.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_PresIdInfo))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strIdKey, varId.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetUserActionVals)
    {
        // If it's not a user action, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strTypeKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strValKey = ccfgToDo.piAt(1).m_strValue;

            TCQCActVar& varType = TStdVarsTar::varFind
            (
                strTypeKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varVal = TStdVarsTar::varFind
            (
                strValKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varType.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_ActType))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strTypeKey, varType.strValue());
            }

            if (varVal.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_ActData))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strValKey, varVal.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetZoneAlarmInfo)
    {
        // If it's not a zone alarm, just return false
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::ZoneAlarm);

        // If it is, then return the two values
        if (bResult)
        {
            const TString& strStateKey = ccfgToDo.piAt(0).m_strValue;
            const TString& strNumKey = ccfgToDo.piAt(1).m_strValue;
            const TString& strNameKey = ccfgToDo.piAt(2).m_strValue;

            TCQCActVar& varState = TStdVarsTar::varFind
            (
                strStateKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            TCQCActVar& varNum = TStdVarsTar::varFind
            (
                strNumKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );

            if (varState.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_ZoneStatus))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strStateKey, varState.strValue());
            }

            if (varNum.bSetValue(m_cptrEvent->strValue(TCQCEvent::strPath_ZoneNum))
            &&  acteTar.pcmdtDebug())
            {
                acteTar.pcmdtDebug()->ActVarSet(strNumKey, varNum.strValue());
            }

            // If the name key was set, do it. It's optional
            if (!strNameKey.bIsEmpty())
            {
                TString strValue;
                m_cptrEvent->bValueExists(TCQCEvent::strPath_ZoneName, strValue);

                TCQCActVar& varName = TStdVarsTar::varFind
                (
                    strNameKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
                );

                if (varName.bSetValue(strValue) && acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActVarSet(strNameKey, varName.strValue());
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsDevSrc)
    {
        // Get the source and see if it's a device
        bResult = m_cptrEvent->strSource().bStartsWith(TCQCEvent::strSrc_DevPref);
        if (bResult)
        {
            // It is so copy it and remove the prefix, leaving the moniker
            TString strDev = m_cptrEvent->strSource();
            strDev.Cut(0, TCQCEvent::strSrc_DevPref.c4Length());

            const TString& strVar = ccfgToDo.piAt(0).m_strValue;
            TCQCActVar& varFld = TStdVarsTar::varFind
            (
                strVar
                , acteTar.ctarLocals()
                , ctarGlobals
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            if (varFld.bSetValue(strDev) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strVar, varFld.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsFldSrc)
    {
        // Get the source field and see if it's a field
        TString strFld;
        TString strMon;
        bResult = m_cptrEvent->TCQCEvent::bQueryFldSrc(strMon, strFld);
        if (bResult)
        {
            const TString& strVar = ccfgToDo.piAt(0).m_strValue;
            TCQCActVar& varFld = TStdVarsTar::varFind
            (
                strVar
                , acteTar.ctarLocals()
                , ctarGlobals
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            // Build up the field name and store it
            strMon.Append(kCIDLib::chPeriod);
            strMon.Append(strFld);
            if (varFld.bSetValue(strMon) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strVar, varFld.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsFldChangeEv)
    {
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsFldChangeEvFor)
    {
        bResult = m_cptrEvent->bIsDrvEvForField
        (
            tCQCKit::EStdDrvEvs::FldChange
            , ccfgToDo.piAt(0).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsUserActionEv)
    {
        bResult = m_cptrEvent->bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsUserActionEvFor)
    {
        bResult = m_cptrEvent->bIsDrvEvForDev
        (
            tCQCKit::EStdDrvEvs::UserAction, ccfgToDo.piAt(0).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsUserActionEvFrom)
    {
        bResult = m_cptrEvent->bIsDrvEvVal
        (
            tCQCKit::EStdDrvEvs::UserAction
            , ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsThisUserActionEv)
    {
        bResult = m_cptrEvent->bIsThisDrvEv
        (
            tCQCKit::EStdDrvEvs::UserAction
            , ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
        );
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TTrigEvCmdTarget::QueryCmds(        TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Get an arbitrary field from the event data
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetEventFld
            , facCQCAct().strMsg(kActMsgs::midCmd_GetEventFld)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    // Get the two values from a field change event. Returns false is not one
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetFldChangeInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetFldChangeInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    //
    //  Get the values from a load change event. Returns false is not one. The
    //  name variable was added later and is optional. If empty, it's ignored.
    //
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetLoadChangeInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetLoadInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Number)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
        );

        // Name output var is optional
        cmdNew.SetParmReqAt(2, kCIDLib::False);

        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    // Get the values from a lock status event. Returns false is not one
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetLockStatusInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetLockStatusInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Code)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    //
    //  Get the values from a motion event. Returns false is not one. The
    //  name variable was added later and is optional. If empty, it's ignored.
    //
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetMotionInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetMotionInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Number)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
        );

        // Name output var is optional
        cmdNew.SetParmReqAt(2, kCIDLib::False);

        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    // Get the two values from a presence event. Returns false is not one
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetPresenceInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetPresenceInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ID)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    // Get the two values from a user action event. Returns false is not one
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetUserActionVals
            , facCQCAct().strMsg(kActMsgs::midCmd_GetUserActionVals)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    //
    //  Get the values from a zone alarm event. Returns false is not one. The
    //  name variable was added later and is optional. If empty, it's ignored.
    //
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_GetZoneAlarmInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetZoneAlarmInfo)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Number)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
        );

        // Name output var is optional
        cmdNew.SetParmReqAt(2, kCIDLib::False);

        cmdNew.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdNew);
    }

    // A conditional to see if this event is a field change event
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsFldChangeEv
            , facCQCAct().strMsg(kActMsgs::midCmd_IsFldChangeEv)
            , 0
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // And the same but checks for a specific field
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsFldChangeEvFor
            , facCQCAct().strMsg(kActMsgs::midCmd_IsFldChangeEvFor)
            , tCQCKit::ECmdPTypes::Field
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
        );
        cmdNew.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::ReadOrWrite);
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // Return true if it's a device source and return the device, else false
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsDevSrc
            , facCQCAct().strMsg(kActMsgs::midCmd_IsDevSrc)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // Return true if it's a field source and return the field, else false
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsFldSrc
            , facCQCAct().strMsg(kActMsgs::midCmd_IsFldSrc)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // Checks to see if this event is a user action event
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsUserActionEv
            , facCQCAct().strMsg(kActMsgs::midCmd_IsUserActionEv)
            , 0
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // Checks to see if this event is a specific type of user action event
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsUserActionEvFor
            , facCQCAct().strMsg(kActMsgs::midCmd_IsUserActionEvFor)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }

    // And th same but checks for a specific field
    {
        TCQCCmd cmdNew
        (
            kCQCKit::strCmdId_IsUserActionEvFrom
            , facCQCAct().strMsg(kActMsgs::midCmd_IsUserActionEvFrom)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
        );
        cmdNew.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdNew);
    }
}


tCIDLib::TVoid TTrigEvCmdTarget::SetDefParms(TCQCCmdCfg&) const
{
    // We don't really do anything for this
}


// ---------------------------------------------------------------------------
//  TTrigEvCmdTarget: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCQCEvent& TTrigEvCmdTarget::evData() const
{
    return *m_cptrEvent.pobjData();
}


tCIDLib::TVoid TTrigEvCmdTarget::SetEvent(const tCQCKit::TCQCEvPtr& cptrEvent)
{
    m_cptrEvent = cptrEvent;
}

