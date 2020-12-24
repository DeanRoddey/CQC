//
// FILE NAME: CQCAct_EvSrvTarget.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2008
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
//  This file implements the event server action target.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAct_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TEvSrvCmdTar,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TEvSrvCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEvSrvCmdTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TEvSrvCmdTar::TEvSrvCmdTar() :

    MCQCCmdTarIntf
    (
        kCQCKit::strCTarId_EventSrv
        , facCQCKit().strMsg(kKitMsgs::midCmdTar_EventSrv)
        , kCQCKit::strCTarId_EventSrv
        , L"/Reference/Actions/EventSrv"
    )
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);
}

TEvSrvCmdTar::~TEvSrvCmdTar()
{
}


// ---------------------------------------------------------------------------
//  TEvSrvCmdTar: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking
tCIDLib::TBoolean
TEvSrvCmdTar::bValidateParm(const   TCQCCmd&        cmdSrc
                            , const TCQCCmdCfg&     ccfgTar
                            , const tCIDLib::TCard4 c4Index
                            , const TString&        strValue
                            ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if (ccfgTar.strCmdId() == kCQCKit::strCmdId_SetSchEvTime)
    {
        if (c4Index == 1)
        {
            // It has to be 3 numbers
            tCIDLib::TCard4 c4Day;
            tCIDLib::TCard4 c4Hour;
            tCIDLib::TCard4 c4Min;
            if (!TCQCSchEvent::bParseDHM(strValue, c4Day, c4Hour, c4Min))
            {
                facCQCAct().bLoadCIDMsg(kActErrs::errcEvSrv_BadTimeInfo, strErrText);
                return kCIDLib::False;
            }
        }
         else if (c4Index == 2)
        {
            // The mask must be a string of either 7 or 12 1s and 0s.
            tCIDLib::TCard4 c4BitCount;
            tCIDLib::TCard4 c4Mask;
            if (!TCQCSchEvent::bParseMask(strValue, c4Mask, c4BitCount))
            {
                facCQCAct().bLoadCIDMsg(kActErrs::errcEvSrv_BadTimeMask, strErrText);
                return kCIDLib::False;
            }
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_SetSunEvOffset) && (c4Index == 1))
    {
        //
        //  It has to be a signed value from -59 to +59 for the minute
        //  offset from sunrise or sunset
        //
        tCIDLib::TInt4 i4Val;
        if (!strValue.bToInt4(i4Val, tCIDLib::ERadices::Dec)
        ||  ((i4Val < -59) || (i4Val > 59)))
        {
            facCQCAct().bLoadCIDMsg(kActErrs::errcEvSrv_BadOffset, strErrText);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


// These are both no-ops for us
tCIDLib::TVoid TEvSrvCmdTar::CmdTarCleanup()
{
}

tCIDLib::TVoid TEvSrvCmdTar::CmdTarInitialize(const TCQCActEngine&)
{
}


// We override this to process command requests
tCQCKit::ECmdRes
TEvSrvCmdTar::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                    , const tCIDLib::TCard4
                    , const TString&
                    , const TString&
                    ,       TStdVarsTar&        ctarGlobalVars
                    ,       tCIDLib::TBoolean&
                    ,       TCQCActEngine&      acteTar)
{
    if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_EnableSchEvent)
    ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseEvMon)
    ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseSchEvent)
    ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseTrgEvent))
    {
        //
        //  NOTE: EnableSchEv is deprecated. We wanted to use the Paused or Resumed
        //        nomenclature. So we have a PauseSchEvent now as well. The only
        //        difference here how we set the state, because the meanings are the
        //        opposite.
        //
        //        We still respond to the Enable one for backwards compatibiltiy reasons.
        //

        // There are two parms, then event path and boolean state to set
        const TString& strPath = ccfgToDo.piAt(0).m_strValue;
        tCIDLib::TBoolean bState(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue));

        // If it's the old Enable command, then flip the meaning
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_EnableSchEvent)
            bState = !bState;

        //
        //  The path we get is a type relative one, i.e. /Event/Triggered/User or the
        //  like. We have to pass it the event type we think it is as well, just as
        //  a sanity check.
        //
        tCQCKit::EEvSrvTypes eType;
        if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_EnableSchEvent)
        ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseSchEvent))
        {
            eType = tCQCKit::EEvSrvTypes::SchEvent;
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseTrgEvent)
        {
            eType = tCQCKit::EEvSrvTypes::TrgEvent;
        }
         else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PauseEvMon)
        {
            eType = tCQCKit::EEvSrvTypes::EvMonitor;
        }

        //
        //  We don't care about tracking changes, so we just pass in a zero serial
        //  number and ignore the return.
        //
        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        facCQCEvCl().SetEvPauseState
        (
            eType, strPath, c4SerNum, enctLastChange, bState, acteTar.sectUser()
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetSchEvInfo)
    {
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();

        // Query info on the indicated event
        tCIDLib::TBoolean   bPaused;
        tCIDLib::TCard4     c4Day;
        tCIDLib::TCard4     c4Hour;
        tCIDLib::TCard4     c4Mask;
        tCIDLib::TCard4     c4Minute;
        tCQCKit::ESchTypes  eType;

        orbcES->QuerySchEvInfo
        (
            ccfgToDo.piAt(0).m_strValue
            , eType
            , c4Day
            , c4Hour
            , c4Minute
            , c4Mask
            , bPaused
        );

        // Format out the time stamp and set that one
        m_strTmp1.SetFormatted(c4Day);
        m_strTmp1.Append(kCIDLib::chSpace);
        m_strTmp1.AppendFormatted(c4Hour);
        m_strTmp1.Append(kCIDLib::chSpace);
        m_strTmp1.AppendFormatted(c4Minute);
        TCQCActVar& varTime = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTime.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTime.strName(), varTime.strValue());

        // Format out the mask and set that
        tCIDLib::TCard4 c4BitCount = 0;
        switch(eType)
        {
            case tCQCKit::ESchTypes::Weekly :
                c4BitCount = 7;
                break;

            case tCQCKit::ESchTypes::Monthly :
                c4BitCount = 12;
                break;

            default :
                // Leave it zero for an empty mask
                break;
        };
        m_strTmp1.Clear();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4BitCount; c4Index++)
        {
            if (c4Mask & (0x1UL << c4Index))
                m_strTmp1.Append(kCIDLib::chDigit1);
            else
                m_strTmp1.Append(kCIDLib::chDigit0);
        }
        TCQCActVar& varMask = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varMask.bSetValue(m_strTmp1) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMask.strName(), varMask.strValue());

        // And do the paused state
        TCQCActVar& varStateVar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varStateVar.bSetValue(bPaused ? kCQCKit::pszFld_True : kCQCKit::pszFld_False)
        &&  acteTar.pcmdtDebug())
        {
            acteTar.pcmdtDebug()->ActVarSet(varStateVar.strName(), varStateVar.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetSunEvInfo)
    {
        // We get an event path and an offset to set
        tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
        tCIDLib::TBoolean bPaused;
        tCIDLib::TInt4 i4Offset;
        orbcES->QuerySunBasedEvInfo(ccfgToDo.piAt(0).m_strValue, i4Offset, bPaused);

        // Give back the offset value
        TCQCActVar& varOfsVar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varOfsVar.bSetValue(i4Offset) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varOfsVar.strName(), varOfsVar.strValue());

        // Give back the paused state
        TCQCActVar& varStateVar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varStateVar.bSetValue(bPaused ? kCQCKit::pszFld_True : kCQCKit::pszFld_False)
        &&  acteTar.pcmdtDebug())
        {
            acteTar.pcmdtDebug()->ActVarSet(varStateVar.strName(), varStateVar.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetSchEvTime)
    {
        // Parse out the time values
        tCIDLib::TCard4 c4Day;
        tCIDLib::TCard4 c4Hour;
        tCIDLib::TCard4 c4Min;
        if (!TCQCSchEvent::bParseDHM(ccfgToDo.piAt(1).m_strValue, c4Day, c4Hour, c4Min))
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcEvSrv_BadTimeInfo
                , ccfgToDo.piAt(1).m_strValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // And parse out and validate the mask
        tCIDLib::TCard4 c4BitCount;
        tCIDLib::TCard4 c4Mask;
        if (!TCQCSchEvent::bParseMask(ccfgToDo.piAt(2).m_strValue, c4Mask, c4BitCount))
        {
            // The mask is invalid
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcEvSrv_BadTimeMask
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        //
        //  We don't care about tracking changes, so we just pass in a zero serial
        //  number and ignore the return.
        //
        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        facCQCEvCl().SetScheduledEvTime
        (
            ccfgToDo.piAt(0).m_strValue
            , c4SerNum
            , enctLastChange
            , c4Day
            , c4Hour
            , c4Min
            , c4Mask
            , acteTar.sectUser()
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetSunEvOffset)
    {
        //
        //  We don't care about tracking changes, so we just pass in a zero serial
        //  number and ignore the return.
        //
        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        facCQCEvCl().SetSunBasedEvOffset
        (
            ccfgToDo.piAt(0).m_strValue
            , c4SerNum
            , enctLastChange
            , ccfgToDo.piAt(1).m_strValue.i4Val()
            , acteTar.sectUser()
        );
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }
    return tCQCKit::ECmdRes::Ok;
}


// We override this to indicate the commands we support
tCIDLib::TVoid
TEvSrvCmdTar::QueryCmds(        TCollection<TCQCCmd>&   colToFill
                        , const tCQCKit::EActCmdCtx     eContext) const
{
    //
    //  We don't allow anything in IV Event or Preload contexts. Otherwise,
    //  we allow them.
    //
    if ((eContext == tCQCKit::EActCmdCtx::IVEvent)
    ||  (eContext == tCQCKit::EActCmdCtx::Preload))
    {
        return;
    }


    //
    //  Get some info about a scheduled event
    //
    //  Time is in the form "dd hh mm", for the day, hour, and minute values
    //  associated with the event. The day one might not be relevant depending on
    //  the type of event. State is the paused state, with True meaning paused. The
    //  mask is a list of 0 and 1 digits, 7 for a weekly event and 12 for a monthly
    //  event, indicating whether that day or month should be executed.
    //
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetSchEvInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetSchEvInfo)
            , tCQCKit::ECmdPTypes::SchEvId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Time)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Mask)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
        )
    );

    // Get the info for a sunrise/sunset event
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetSunEvInfo
            , facCQCAct().strMsg(kActMsgs::midCmd_GetSunEvInfo)
            , tCQCKit::ECmdPTypes::SchEvId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Offset)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
        )
    );


    // Pause/resume an event monitor
    {
        TCQCCmd cmdPause
        (
            kCQCKit::strCmdId_PauseEvMon
            , facCQCAct().strMsg(kActMsgs::midCmd_PauseEvMon)
            , tCQCKit::ECmdPTypes::EvMonId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCAct().strMsg(kActMsgs::midCmdP_Paused)
        );
        colToFill.objAdd(cmdPause);
    }

    // Pause/resume a scheduled event
    {
        TCQCCmd cmdPause
        (
            kCQCKit::strCmdId_PauseSchEvent
            , facCQCAct().strMsg(kActMsgs::midCmd_PauseSchEv)
            , tCQCKit::ECmdPTypes::SchEvId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCAct().strMsg(kActMsgs::midCmdP_Paused)
        );
        colToFill.objAdd(cmdPause);
    }

    // Pause/resume a triggered event
    {
        TCQCCmd cmdPause
        (
            kCQCKit::strCmdId_PauseTrgEvent
            , facCQCAct().strMsg(kActMsgs::midCmd_PauseTrgEv)
            , tCQCKit::ECmdPTypes::TrgEvId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCAct().strMsg(kActMsgs::midCmdP_Paused)
        );
        colToFill.objAdd(cmdPause);
    }


    //
    //  Set the execution time/day/month info for a scheduled event.
    //  The time and mask values are the same as with GetSchEvInfo
    //  above.
    //
    {
        TCQCCmd& cmdSetEvTime = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetSchEvTime
                , facCQCAct().strMsg(kActMsgs::midCmd_SetSchEvTime)
                , tCQCKit::ECmdPTypes::SchEvId
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Time)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Mask)
            )
        );

        // The mask isn't always needed
        cmdSetEvTime.cmdpAt(2).bRequired(kCIDLib::False);
    }

    // Set the offset for a sunrise/sunset event
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetSunEvOffset
            , facCQCAct().strMsg(kActMsgs::midCmd_SetSunEvOffset)
            , tCQCKit::ECmdPTypes::SchEvId
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Offset)
        )
    );
}


//
//  We override this to set up any default parameter values above and beyond
//  the ones that can be set automatically.
//
tCIDLib::TVoid TEvSrvCmdTar::SetDefParms(TCQCCmdCfg& ccfgUpdate) const
{
    // None at this time...
}


