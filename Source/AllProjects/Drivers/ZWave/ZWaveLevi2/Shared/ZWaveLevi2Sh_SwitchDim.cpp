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
//  This file provides the implementation for the binary/multi-level switch command
//  class derivatives.
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
RTTIDecls(TZWCCBinSwitch, TZWCmdClass)
RTTIDecls(TZWCCMLSwitch, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCBinSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCBinSwitch: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCBinSwitch::TZWCCBinSwitch(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::BinSwitch, zwdcInst, COMMAND_CLASS_SWITCH_BINARY)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"BinSw_Trigs", m_strOptKey_Trigs);
    BuildOptKey(L"BinSw_Type", m_strOptKey_Type);
}

TZWCCBinSwitch::~TZWCCBinSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCBinSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary switch, then we should get reports
//  this way, and may also get them via other command classes anyway. So we watch for
//  msgs that could deliver a level value.
//
//  1.  A binary switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current state via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current state.
//  3.  A basic report
//
tCIDLib::TBoolean
TZWCCBinSwitch::bHandleGenReport(const  tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();

    if ((c4FromInst == c4InstId())
    &&  (c4ValCnt >= 5)
    &&  (fcolVals[0] == COMMAND_CLASS_SCENE_ACTUATOR_CONF)
    &&  (fcolVals[1] == SCENE_ACTUATOR_CONF_REPORT)
    &&  (fcolVals[2] == 0))
    {
        //
        //  It's a scene actuator config report for scene 0, so we treat that as
        //  the current value.
        //
        StoreValue(fcolVals[3], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt >= 3)
          &&  (fcolVals[0] == COMMAND_CLASS_SWITCH_BINARY)
          &&  (fcolVals[1] == SWITCH_BINARY_REPORT))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt >= 3)
          &&  (fcolVals[0] == COMMAND_CLASS_BASIC)
          &&  (fcolVals[1] == BASIC_REPORT))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCBinSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCBinSwitch::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCard4 c4CmdClass;
    tCIDLib::TCard4 c4Get;
    tCIDLib::TCard4 c4Report;

    if (unitMe.eBaseClass() == eCmdClass())
    {
        c4CmdClass = COMMAND_CLASS_BASIC;
        c4Get      = BASIC_GET;
        c4Report   = BASIC_REPORT;
    }
     else
    {
        c4CmdClass = COMMAND_CLASS_SWITCH_BINARY;
        c4Get      = SWITCH_BINARY_GET;
        c4Report   = SWITCH_BINARY_REPORT;
    }

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);

    tCIDLib::TCardList fcolVals;
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);

    return kCIDLib::False;
}


tCQCKit::ECommResults
TZWCCBinSwitch::eWriteBoolFld(  const   tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bValue
                                ,       TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TCardList fcolVals(8);

    // If the base class is our class, use the basic command, else class specific
    if (unitMe.eBaseClass() == eCmdClass())
    {
        fcolVals.c4AddElement(COMMAND_CLASS_BASIC);
        fcolVals.c4AddElement(BASIC_SET);
        fcolVals.c4AddElement(bValue ? 255UL : 0UL);
    }
     else
    {
        fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_BINARY);
        fcolVals.c4AddElement(SWITCH_BINARY_SET);
        fcolVals.c4AddElement(bValue ? 255UL : 0UL);
    }

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    //
    //  If this device has configured us a known response to wait for, we can
    //  use that as a validation of the operation, and it should give us the
    //  dimmer level as well.
    //
    //  If not, then we need to query the state. since we won't otherwise get
    //  it back and will get out of sync, but only if this is a readable class
    //  on this device.
    //
    //  Worst case, just store the value to our field on the assumption it worked.
    //
    if (c4SetRespCls() && c4SetRespCmd())
    {
        const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
        (
            unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
        );

        if (bRes)
        {
            bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
        }
         else
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcUnit_NoSetResp
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NoReply
                , unitMe.strName()
                , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
            );
        }
    }
     else if (bCanRead())
    {
        bQueryFldVals(unitMe, mzwsfLevi);
    }
     else
    {
        StoreValue(bValue ? 0xFF : 0, unitMe, mzwsfLevi);
    }

    return tCQCKit::ECommResults::Success;
}


//
//  Add any of our prompts to the passed list. We have one which is whether we are a
//  dimmer or a generic multi-level value, and another whether we should send
//  triggers.
//
tCIDLib::TVoid TZWCCBinSwitch::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptType = colToFill.objAdd(TZWOption());
    zwoptType.m_strKey = m_strOptKey_Type;
    zwoptType.m_strPrompt = L"Binary Switch Type";
    zwoptType.m_strInstDescr = strInstDescr();
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Gen", L"Generic Switch"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Light", L"Light Switch"));

    TZWOption& zwoptTrigs = colToFill.objAdd(TZWOption());
    zwoptTrigs.m_strKey = m_strOptKey_Trigs;
    zwoptTrigs.m_strPrompt = L"Send Triggers";
    zwoptTrigs.m_strInstDescr = strInstDescr();
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"0", L"No"));
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"1", L"Yes"));
}


//
//  Add an opt value (with an initial default value) to the list for any options we
//  have.
//
tCIDLib::TVoid
TZWCCBinSwitch::LoadOptVals(const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Type))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Type, L"Light");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Trigs))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Trigs, L"1");
        colToFill.objAdd(zwoptvTmp);
    }
}


//
//  Make sure our fields are set to error state until good values are stored, but
//  only if readable.
//
tCIDLib::TVoid
TZWCCBinSwitch::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    if (bCanRead())
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCBinSwitch::QueryFldDefs(       tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Light")
    {
        BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        TCQCDevClass::c4LoadLightFlds(colAddTo, strTmp1, kCIDLib::False, kCIDLib::True);
    }
     else
    {
        // Just create a generic binary switch field
        BuildInstName(unitMe, L"State", strTmp1);
        flddTmp.Set(strTmp1, tCQCKit::EFldTypes::Boolean, eFldAccess());
        colAddTo.objAdd(flddTmp);
    }
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCBinSwitch::StoreFldIds(const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Light")
    {
        strTmp1 = L"LGHT#Sw_";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    }
     else
    {
        BuildInstName(unitMe, L"State", strTmp1);
    }
    m_c4FldId_State = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCBinSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWCCBinSwitch::StoreValue( const   tCIDLib::TCard4     c4Val
                            , const TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (!bCheckFldId(m_c4FldId_State, L"switch", unitMe))
        return;

    //
    //  Just write the boolean value to our field. If the value changes, and
    //  this guy is a light switch, send a load change trigger.
    //
    tCIDLib::TBoolean bChanged = mzwsfLevi.bLeviStoreBoolFld(m_c4FldId_State, c4Val != 0);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);

    // If changed and we are a light switch, send a trigger
    if (bChanged)
    {
        const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);
        const TString& strTrigs = unitMe.strFindOptVal(m_strOptKey_Trigs);
        if ((strType == L"Light") && (strTrigs == L"1"))
        {
            TString strId;
            strId.SetFormatted(unitMe.c4Id());
            TString strInstName;
            BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strInstName);

            TString strFldName;
            if (mzwsfLevi.bLeviQueryFldName(m_c4FldId_State, strFldName))
            {
                mzwsfLevi.LeviQEventTrig
                (
                    tCQCKit::EStdDrvEvs::LoadChange
                    , strFldName
                    , facCQCKit().strBoolOffOn(c4Val != 0)
                    , strId
                    , strFldName
                    , strInstName
                );
            }
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCMLSwitch
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCMLSwitch: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCMLSwitch::TZWCCMLSwitch(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::MLSwitch, zwdcInst, COMMAND_CLASS_SWITCH_MULTILEVEL)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"MLSw_Trigs", m_strOptKey_Trigs);
    BuildOptKey(L"MLSw_Type", m_strOptKey_Type);
}

TZWCCMLSwitch::~TZWCCMLSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCMLSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't MLSwitch, then we will get reports via general
//  report msgs. We handle two scenarios:
//
//  1.  A multi-level switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current level via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current level.
//  3.  A basic report
//
tCIDLib::TBoolean
TZWCCMLSwitch::bHandleGenReport(const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();

    if ((c4FromInst == c4InstId())
    &&  (c4ValCnt >= 5)
    &&  (fcolVals[0] == COMMAND_CLASS_SCENE_ACTUATOR_CONF)
    &&  (fcolVals[1] == SCENE_ACTUATOR_CONF_REPORT)
    &&  (fcolVals[2] == 0))
    {
        //
        //  It's a scene actuator config report for scene 0, so we treat that as
        //  the current value.
        //
        StoreValue(fcolVals[3], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt >= 3)
          &&  (fcolVals[0] == COMMAND_CLASS_SWITCH_MULTILEVEL)
          &&  (fcolVals[1] == SWITCH_MULTILEVEL_REPORT))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt == 3)
          &&  (fcolVals[0] == COMMAND_CLASS_BASIC)
          &&  (fcolVals[1] == BASIC_REPORT))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }

    // Indicate not something we handled
    return kCIDLib::False;
}



tCIDLib::TBoolean TZWCCMLSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_Level) || (c4FldId == m_c4FldId_State));
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCMLSwitch::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCard4 c4CmdClass;
    tCIDLib::TCard4 c4Get;
    tCIDLib::TCard4 c4Report;

    if (unitMe.eBaseClass() == eCmdClass())
    {
        c4CmdClass = COMMAND_CLASS_BASIC;
        c4Get      = BASIC_GET;
        c4Report   = BASIC_REPORT;
    }
     else
    {
        c4CmdClass  = COMMAND_CLASS_SWITCH_MULTILEVEL;
        c4Get = SWITCH_MULTILEVEL_GET;
        c4Report = SWITCH_MULTILEVEL_REPORT;
    }

    tCIDLib::TCardList fcolVals(8);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
    {
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    }
     else
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);
    }
    return kCIDLib::True;
}


tCQCKit::ECommResults
TZWCCMLSwitch::eWriteBoolFld(const  tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TBoolean   bValue
                            ,       TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Make sure it's for the state field
    if (c4FldId != m_c4FldId_State)
        return tCQCKit::ECommResults::FieldNotFound;

    try
    {
        tCIDLib::TCardList fcolVals(6);

        if (unitMe.eBaseClass() == eCmdClass())
        {
            fcolVals.c4AddElement(COMMAND_CLASS_BASIC);
            fcolVals.c4AddElement(BASIC_SET);
            fcolVals.c4AddElement(bValue ? 255UL : 0UL);
        }
         else
        {
            fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_MULTILEVEL);
            fcolVals.c4AddElement(SWITCH_MULTILEVEL_SET);
            fcolVals.c4AddElement(bValue ? 255UL : 0UL);
        }

        mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

        //
        //  If this device has configured us a known response to wait for, we can
        //  use that as a validation of the operation, and it should give us the
        //  dimmer level as well.
        //
        //  If not, then we need to query the state. since we won't otherwise get
        //  it back and will get out of sync, but only if this is a readable class
        //  on this device.
        //
        //  Worst case, just store the value to our field on the assumption it worked.
        //
        if (c4SetRespCls() && c4SetRespCmd())
        {
            const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
            (
                unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
            );

            if (bRes)
            {
                bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
            }
             else
            {
                facZWaveLevi2Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW2ShErrs::errcUnit_NoSetResp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NoReply
                    , unitMe.strName()
                    , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
                );
            }
        }
         else if (bCanRead())
        {
            bQueryFldVals(unitMe, mzwsfLevi);
        }
         else
        {
            StoreValue(bValue ? 0xFF : 0, unitMe, mzwsfLevi);
        }
    }

    catch(TError& errToCatch)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TZWCCMLSwitch::eWriteCardFld(const  tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TCard4     c4Value
                            ,       TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Make sure it's for the level field and 255 or less
    if (c4FldId != m_c4FldId_Level)
        return tCQCKit::ECommResults::FieldNotFound;
    if (c4Value > 100)
        return tCQCKit::ECommResults::BadValue;

    try
    {
        // Scale the value to the actual range
        const tCIDLib::TCard4 c4RealVal = c4ScaleToZW(c4Value);

        tCIDLib::TCardList fcolVals(6);
        fcolVals.c4AddElement(COMMAND_CLASS_SWITCH_MULTILEVEL);
        fcolVals.c4AddElement(SWITCH_MULTILEVEL_SET);
        fcolVals.c4AddElement(c4RealVal);
        mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

        //
        //  If this device has configured us a known response to wait for, we can
        //  use that as a validation of the operation, and it should give us the
        //  dimmer level as well.
        //
        //  If not, then we need to query the state. since we won't otherwise get
        //  it back and will get out of sync, but only if this is a readable class
        //  on this device.
        //
        //  Worst case, just store the value to our field on the assumption it worked.
        //
        if (c4SetRespCls() && c4SetRespCmd())
        {
            const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
            (
                unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
            );

            if (bRes)
            {
                bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
            }
             else
            {
                if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facZWaveLevi2Sh().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW2ShErrs::errcUnit_NoSetResp
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NoReply
                        , unitMe.strName()
                        , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
                    );
                }
            }
        }
         else if (bCanRead())
        {
            bQueryFldVals(unitMe, mzwsfLevi);
        }
         else
        {
            StoreValue(c4Value, unitMe, mzwsfLevi);
        }
    }

    catch(TError& errToCatch)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Add any of our prompts to the passed list. We just have a single one, which is
//  whether we are a dimmer or a generic multi-level value.
//
tCIDLib::TVoid TZWCCMLSwitch::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptType = colToFill.objAdd(TZWOption());
    zwoptType.m_strKey = m_strOptKey_Type;
    zwoptType.m_strPrompt = L"Multi-level Switch Type";
    zwoptType.m_strInstDescr = strInstDescr();
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Gen", L"Generic ML Switch"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Dim", L"Lighting Dimmer"));

    TZWOption& zwoptTrigs = colToFill.objAdd(TZWOption());
    zwoptTrigs.m_strKey = m_strOptKey_Trigs;
    zwoptTrigs.m_strPrompt = L"Send Triggers";
    zwoptTrigs.m_strInstDescr = strInstDescr();
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"0", L"No"));
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"1", L"Yes"));
}


//
//  Add an opt value (with an initial default value) to the list for any options we
//  have.
//
tCIDLib::TVoid
TZWCCMLSwitch::LoadOptVals( const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Type))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Type, L"Dim");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Trigs))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Trigs, L"1");
        colToFill.objAdd(zwoptvTmp);
    }
}



//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCMLSwitch::QueryFldDefs(        tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Dim")
    {
        BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        TCQCDevClass::c4LoadLightFlds(colAddTo, strTmp1, kCIDLib::True, kCIDLib::True);
    }
     else
    {
        // Just a generic multi-level switch
        BuildInstName(unitMe, L"Level", strTmp1);
        flddTmp.Set
        (
            strTmp1
            , tCQCKit::EFldTypes::Card
            , eFldAccess()
            , tCQCKit::EFldSTypes::Dimmer
            , L"Range: 0, 100"
        );
        colAddTo.objAdd(flddTmp);

        BuildInstName(unitMe, L"State", strTmp1);
        flddTmp.Set
        (
            strTmp1
            , tCQCKit::EFldTypes::Boolean
            , eFldAccess()
            , tCQCKit::EFldSTypes::BoolSwitch
        );
        colAddTo.objAdd(flddTmp);
    }
}


//
//  Make sure our fields are set to error state until good values are stored, but
//  only if readable.
//
tCIDLib::TVoid
TZWCCMLSwitch::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    if (bCanRead())
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);
    }
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCMLSwitch::StoreFldIds( const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Dim")
    {
        // It's a formal V2 lighting load
        strTmp1 = L"LGHT#Dim_";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        m_c4FldId_Level = mzwsfLevi.c4LeviFldIdFromName(strTmp1);

        strTmp1 = L"LGHT#Sw_";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        m_c4FldId_State = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
    }
     else
    {
        // Just a generic switch
        BuildInstName(unitMe, L"Level", strTmp1);
        m_c4FldId_Level = mzwsfLevi.c4LeviFldIdFromName(strTmp1);

        BuildInstName(unitMe, L"State", strTmp1);
        m_c4FldId_State = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
    }
}


// ---------------------------------------------------------------------------
//  TZWCCMLSwitch: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Scale from ZWave's 0 to 99 to our 0 to 100
tCIDLib::TCard4 TZWCCMLSwitch::c4ScaleFromZW(const tCIDLib::TCard4 c4ToScale)
{
    const tCIDLib::TFloat8 f8Per = tCIDLib::TFloat8(c4ToScale) / 99.0;
    tCIDLib::TFloat8 f8New = 100.0 * f8Per;
    TMathLib::Round(f8New);
    tCIDLib::TCard4 c4Ret = tCIDLib::TCard4(f8New);

    if (c4Ret > 100)
        c4Ret = 100;
    return c4Ret;
}


// Scale from our 0 to 100 to ZWave's 0 to 99
tCIDLib::TCard4 TZWCCMLSwitch::c4ScaleToZW(const tCIDLib::TCard4 c4ToScale)
{
    const tCIDLib::TFloat8 f8Per = tCIDLib::TFloat8(c4ToScale) / 100.0;
    tCIDLib::TFloat8 f8New = 99.0 * f8Per;
    TMathLib::Round(f8New);
    tCIDLib::TCard4 c4Ret = tCIDLib::TCard4(f8New);

    if (c4Ret > 99)
        c4Ret = 99;
    return c4Ret;
}


//
//  We can potentially store the value from multiple places, so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWCCMLSwitch::StoreValue(  const   tCIDLib::TCard4     c4Val
                            , const TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // If not the magic ON value, but greater than the max dimmer value
    if ((c4Val != 0xFF) && (c4Val > 100))
    {
        // Invalid value. Put the field into error state
        if (m_c4FldId_Level != kCIDLib::c4MaxCard)
            mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);

        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcUnit_BadValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , unitMe.strName()
            );
        }
        return;
    }

    // Set the state based on non-zero value
    if (bCheckFldId(m_c4FldId_State, L"state", unitMe))
    {
        if (mzwsfLevi.bLeviStoreBoolFld(m_c4FldId_State, c4Val != 0))
        {
            const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);
            const TString& strTrigs = unitMe.strFindOptVal(m_strOptKey_Trigs);
            if ((strType == L"Dim") && (strTrigs == L"1"))
            {
                TString strId;
                strId.SetFormatted(unitMe.c4Id());
                TString strInstName;
                BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strInstName);

                TString strFldName;
                if (mzwsfLevi.bLeviQueryFldName(m_c4FldId_State, strFldName))
                {
                    mzwsfLevi.LeviQEventTrig
                    (
                        tCQCKit::EStdDrvEvs::LoadChange
                        , strFldName
                        , facCQCKit().strBoolOffOn(c4Val != 0)
                        , strId
                        , strFldName
                        , strInstName
                    );
                }
            }
        }
    }

    //
    //  If it's a valid dimmer value, store that. It could be the magic 0xFF ON
    //  value, which we already will have handled in the boolean field write above.
    //
    if (c4Val <= 100)
    {
        // Scale it to our percent range before storing
        if (bCheckFldId(m_c4FldId_Level, L"dimmer", unitMe))
            mzwsfLevi.bLeviStoreCardFld(m_c4FldId_Level, c4ScaleFromZW(c4Val));
    }

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);
}


