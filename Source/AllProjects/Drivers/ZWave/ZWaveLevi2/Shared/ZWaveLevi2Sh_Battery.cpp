//
// FILE NAME: ZWaveLevi2Sh_Battery.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2014
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
//  This file provides the implementation for the battery command class derivatives.
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
RTTIDecls(TZWCCBattery, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCBattery
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCBattery: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCBattery::TZWCCBattery(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::Battery, zwdcInst, COMMAND_CLASS_BATTERY)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"Batt_Enable", m_strOptKey_Enable);

    //
    //  We are never writeable, so for efficiency make sure that's marked as such.
    //  For readable, we take whatever the base class set (from the device info
    //  we passed long to it.)
    //
    SetAccess(bCanRead(), kCIDLib::False);
}

TZWCCBattery::~TZWCCBattery()
{
}


// ---------------------------------------------------------------------------
//  TZWCCBattery: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCBattery::bHandleGenReport( const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // If not enabled, do nothing
    if ((m_c4FldId_Level == kCIDLib::c4MaxCard) || (fcolVals.c4ElemCount() < 3))
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_BATTERY)
    &&  (fcolVals[1] == BATTERY_REPORT))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
     else if ((fcolVals[0] == COMMAND_CLASS_BASIC)
          &&  ((fcolVals[1] == BASIC_REPORT) || (fcolVals[1] == BASIC_SET) ))
    {
        StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCBattery::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCBattery::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    // If not enabled, then don't do anything here
    const TString& strEnable = unitMe.strFindOptVal(m_strOptKey_Enable);
    if (strEnable != L"1")
        return kCIDLib::False;

    const tCIDLib::TCard4 c4CmdClass = COMMAND_CLASS_BATTERY;
    const tCIDLib::TCard4 c4Get      = BATTERY_GET;
    const tCIDLib::TCard4 c4Report   = BATTERY_REPORT;

    // Send a get message
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);

    tCIDLib::TCardList fcolVals(16);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);

    return kCIDLib::False;
}


//
//  Add any of our prompts to the passed list. We just have a single one, which is
//  whether we are a dimmer or a generic multi-level value.
//
tCIDLib::TVoid
TZWCCBattery::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptEnable = colToFill.objAdd(TZWOption());
    zwoptEnable.m_strKey = m_strOptKey_Enable;
    zwoptEnable.m_strPrompt = L"Enable Battery Reports";
    zwoptEnable.m_strInstDescr = strInstDescr();
    zwoptEnable.m_colValues.objAdd(TKeyValuePair(L"0", L"No"));
    zwoptEnable.m_colValues.objAdd(TKeyValuePair(L"1", L"Yes"));
}


//
//  Add any opt values (with an initial default value) to the list for any options we
//  have.
//
tCIDLib::TVoid
TZWCCBattery::LoadOptVals(  const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Enable))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Enable, L"0");
        colToFill.objAdd(zwoptvTmp);
    }
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCBattery::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    // We may not be enabled, do nothing if not, since the field id isn't set
    if (m_c4FldId_Level != kCIDLib::c4MaxCard)
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
}



//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCBattery::QueryFldDefs(         tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    // If enabled, then create our field
    const TString& strEnable = unitMe.strFindOptVal(m_strOptKey_Enable);
    if (strEnable == L"1")
    {
        BuildInstName(unitMe, L"Battery", strTmp1);
        strTmp2 = L"Range: 0, 100";
        flddTmp.Set(strTmp1, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strTmp2);
        colAddTo.objAdd(flddTmp);
    }
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCBattery::StoreFldIds(  const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    const TString& strEnable = unitMe.strFindOptVal(m_strOptKey_Enable);
    if (strEnable == L"1")
    {
        BuildInstName(unitMe, L"Battery", strTmp1);
        m_c4FldId_Level = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
    }
     else
    {
        // Make sure our field is max card
        m_c4FldId_Level = kCIDLib::c4MaxCard;
    }
}


// ---------------------------------------------------------------------------
//  TZWCCBattery: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple places, so we break out
//  the grunt work to here.
//
//  NOTE that 255 means low battery warning, so we ignore that value.
//
tCIDLib::TVoid
TZWCCBattery::StoreValue(const  tCIDLib::TCard4     c4Val
                        , const TZWaveUnit&         unitMe
                        ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (c4Val <= 100)
    {
        mzwsfLevi.bLeviStoreCardFld(m_c4FldId_Level, c4Val);
        bWaitInitVals(kCIDLib::False);
    }
     else if (c4Val != 0xFF)
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
    }
}


